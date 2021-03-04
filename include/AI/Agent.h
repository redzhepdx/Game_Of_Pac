#ifndef __AI_AGENT_H_
#define __AI_AGENT_H_

#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <ctime>

#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS

#include "config.h"

#include "Network.h"
#include "Utility.h"
#include "Memory.h"

#include "spdlog/spdlog.h"

class Agent{

private:
    int32_t m_StateSize;
    int32_t m_ActionSize;

    int16_t m_UpdateRate = AGENT_UPDATE_RATE;
    int64_t m_Step = 0;

    float m_Epsilon;

    std::unique_ptr<OUNoise> m_Noise;
    std::unique_ptr<ReplayMemory> m_Memory;

    DeepQNetwork m_ActorLocal{nullptr};
    DeepQNetwork m_ActorTarget{nullptr};

    std::unique_ptr<torch::optim::Adam> m_Optimizer;

    torch::Device m_Device;

private:
    void updateLearningRate(float new_lr_rate){
        static_cast<torch::optim::AdamOptions&>(m_Optimizer->param_groups()[0].options()).lr(new_lr_rate);
    }

    void updateOptimizerParameters(const std::vector<torch::Tensor> network_parameters){
        m_Optimizer = std::move(std::make_unique<torch::optim::Adam>(network_parameters, LR_RATE));
    }

public:
    Agent(int32_t state_size, int32_t action_size, torch::Device device) : m_StateSize(state_size), m_ActionSize(action_size), m_Device(device){

        m_ActorLocal  = DeepQNetwork(m_StateSize, m_ActionSize);
        m_ActorTarget = DeepQNetwork(m_StateSize, m_ActionSize);

        m_Optimizer   = std::move(std::make_unique<torch::optim::Adam>(m_ActorLocal->parameters(), LR_RATE));

        m_Noise       = std::make_unique<OUNoise>(m_ActionSize, SEED);
        m_Memory      = std::make_unique<ReplayMemory>(m_ActionSize, BUFFER_SIZE, BATCH_SIZE, m_Device);
        m_Epsilon     = EPS;        

        m_ActorLocal->train();
        m_ActorTarget->train();
        
        m_ActorLocal->to(m_Device);
        m_ActorTarget->to(m_Device);

        this->printNetworks();

        this->loadNetworks();
    }

    void step(std::unique_ptr<GameState> current_state, int action, float reward, std::unique_ptr<GameState> next_state, bool done){
        m_Memory->add(std::move(current_state), action, reward, std::move(next_state), done);


        if(m_Memory->capacity() > BATCH_SIZE && m_Step % TRAIN_EVERY == 0){
            GroupTensorExperience experiences = m_Memory->sample();
            this->learn(experiences);
        }

        if(m_Step % SAVE_EVERY == 0 && m_Step > 0){
            this->saveNetworks();
        }
        
        // Update Epsilon
        if(m_Step % EXPLORATION_UPDATE == 0 && m_Epsilon > MIN_EPS){
            m_Epsilon *= EPS_REDUCTION;
            spdlog::info("Random Exploration Rate Reduction -> New Value : {}", m_Epsilon);
        }

        ++m_Step;
    }

    void learn(GroupTensorExperience experiences){
        // Double Q Learning

        torch::Tensor current_states;
        torch::Tensor next_states;
        torch::Tensor actions;
        torch::Tensor rewards;
        torch::Tensor dones;

        std::tie(current_states, next_states, actions, rewards, dones) = experiences;

        actions = actions.unsqueeze(1);
        rewards = rewards.unsqueeze(1);
        dones   = dones.unsqueeze(1);

        // Get next action estimation with local q network
        auto q_targets_next_expected         = m_ActorLocal->forward(next_states).detach();
        auto q_targets_next_expected_actions = std::get<1>(q_targets_next_expected.max(1)).unsqueeze(1);

        // Calculate New Targets
        auto q_targets_next = m_ActorTarget->forward(next_states).gather(1, q_targets_next_expected_actions);

        // Non over estimate targets - Bellman Equation
        auto q_targets = rewards + (GAMMA * q_targets_next * (1 - dones));

        // Expected Values
        auto q_expected = m_ActorLocal->forward(current_states).gather(1, actions);
                
        // Mean Squarred Error
        auto loss = torch::nn::functional::mse_loss(q_expected, q_targets);
        
        m_Optimizer->zero_grad();
        loss.backward();
        m_Optimizer->step();

        spdlog::info("Loss : {}", loss.item<double>());

        if(m_Step % AGENT_UPDATE_RATE == 0){
            this->softUpdate();
        }
    }

    uint32_t act(std::unique_ptr<GameState> state){
        float random_value = ((float)std::rand() / RAND_MAX);

        if(random_value > m_Epsilon){
            torch::Tensor stateTensor = state->toTensor();
            stateTensor = stateTensor.to(m_Device);
            m_ActorLocal->eval();
            
            // Lock Gradient Calculations
            torch::autograd::GradMode::set_enabled(false);

            auto action_values = m_ActorLocal->forward(stateTensor);

            // Unlock the Gradient Calculations
            torch::autograd::GradMode::set_enabled(true);

            m_ActorLocal->train();
        
            auto action = torch::argmax(action_values.detach().cpu());
            return (uint32_t)action.item<int>();
         }
        
        return (uint32_t) (std::rand() % m_ActionSize);
    }

    void softUpdate(){
        // Update Target Network's parameters slowly
        torch::autograd::GradMode::set_enabled(false);

        auto target_params = m_ActorTarget->named_parameters(); // implement this
        auto params = m_ActorLocal->named_parameters(true /*recurse*/);
        auto buffers = m_ActorLocal->named_buffers(true /*recurse*/);
        for (auto& val : params) {
            auto name = val.key();
            auto* t = target_params.find(name);
            if (t != nullptr) {
                t->copy_(val.value() * TAU + (1 - TAU) * t->data());
            } else {
                t = buffers.find(name);
                if (t != nullptr) {
                    t->copy_(val.value() * TAU + (1 - TAU) * t->data());
                }
            }
        }
        torch::autograd::GradMode::set_enabled(true);
    }

    uint16_t totalStepCount(){
        return m_Step;
    }

    void printNetwork(const DeepQNetwork network){
        for (const auto& p : network->parameters()) {
            std::cout << p.sizes() << "\n";
        }
    }

    void initWeights(DeepQNetwork& network)
    {
        torch::autograd::GradMode::set_enabled(false);
        
        for(auto& parameter : network->named_parameters(true)){
            auto param_name = parameter.key();
            auto param      = parameter.value();
            
            if (param_name.compare(2, 6, "weight") == 0){
                torch::nn::init::xavier_uniform_(param);
            }
            else if (param_name.compare(2, 4, "bias") == 0)
            {
                torch::nn::init::constant_(param, 0.01);
            }
        }

        torch::autograd::GradMode::set_enabled(true);
    }

    void printNetworks() {
        spdlog::info("-----------------------Actor Local-------------------" );
        printNetwork(m_ActorLocal);
        spdlog::info("-----------------------Actor Target------------------" );
        printNetwork(m_ActorTarget);
    }

    void saveNetwork(DeepQNetwork& network, std::string model_path) {
        torch::serialize::OutputArchive output_archive;
        network->save(output_archive);
        output_archive.save_to(model_path);
    }

    void saveNetworks(){
        spdlog::info("Saving Networks");

        saveNetwork(m_ActorLocal, "ActorLocal.pt");
        saveNetwork(m_ActorTarget, "ActorTarget.pt");
    }

    void loadNetwork(DeepQNetwork& network, std::string model_path){
        if(boost::filesystem::exists(model_path.c_str())){
            torch::serialize::InputArchive archive;
            archive.load_from(model_path);
            network->load(archive);
            m_Epsilon = MIN_EPS;
        }
        else{
            spdlog::info("There is no {} named file!", model_path);
            spdlog::info("Random Weight Initialization");
            initWeights(network);
        }
    }

    void loadNetworks(){
        spdlog::info("Loading Pretrained Networks");

        loadNetwork(m_ActorLocal, "ActorLocal.pt");
        loadNetwork(m_ActorTarget, "ActorTarget.pt");
    }
};

#endif // __AI_AGENT_H_