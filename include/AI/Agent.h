#ifndef __AI_AGENT_H_
#define __AI_AGENT_H_

#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <ctime>

#include <boost/range/combine.hpp>

#include "config.h"

#include "Network.h"
#include "Utility.h"
#include "Memory.h"

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

private:
    void updateLearningRate(float new_lr_rate){
        static_cast<torch::optim::AdamOptions&>(m_Optimizer->param_groups()[0].options()).lr(new_lr_rate);
    }

    void updateOptimizerParameters(const std::vector<torch::Tensor> network_parameters){
        m_Optimizer = std::move(std::make_unique<torch::optim::Adam>(network_parameters, LR_RATE));
    }

public:
    Agent(int32_t state_size, int32_t action_size) : m_StateSize(state_size), m_ActionSize(action_size){
        m_ActorLocal  = DeepQNetwork(m_StateSize, m_ActionSize);
        m_ActorTarget = DeepQNetwork(m_StateSize, m_ActionSize);

        m_Optimizer   = std::move(std::make_unique<torch::optim::Adam>(m_ActorLocal->parameters(), LR_RATE));

        m_Noise       = std::make_unique<OUNoise>(m_ActionSize, SEED);
        m_Memory      = std::make_unique<ReplayMemory>(m_ActionSize, BUFFER_SIZE, BATCH_SIZE);
        m_Epsilon     = EPS;

        this->initWeights(m_ActorLocal);
        this->initWeights(m_ActorTarget);
    }

    void step(std::shared_ptr<GameState> current_state, int action, float reward, std::shared_ptr<GameState> next_state, bool done){
        m_Memory->add(current_state, action, reward, next_state, done);


        if(m_Memory->capacity() > BATCH_SIZE && m_Step % TRAIN_EVERY == 0){
            GroupTensorExperience experiences = m_Memory->sample();
            this->learn(experiences);
        }
        
        // Update Epsilon
        if(m_Step % EXPLORATION_UPDATE == 0){
            m_Epsilon *= EPS_REDUCTION;
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

        // std::cout << "[INFO]Loss : " << loss.item<double>() << std::endl;
        std::cout << "\033[31m[INFO] Loss : " << loss.item<double>() << std::endl;

        if(m_Step % AGENT_UPDATE_RATE == 0){
            this->softUpdate();
        }
    }

    uint32_t act(std::shared_ptr<GameState> state){
        float random_value = ((float)std::rand() / RAND_MAX);

        if(random_value > m_Epsilon){
            torch::Tensor stateTensor = state->toTensor();
            
            m_ActorLocal->eval();
            
            // Lock Gradient Calculations
            torch::autograd::GradMode::set_enabled(false);

            auto action_values = m_ActorLocal->forward(stateTensor);
            // std::cout << "\033[38m[INFO] Prediction : " << action_values << std::endl;
            
            // Unlock the Gradient Calculations
            torch::autograd::GradMode::set_enabled(true);

            m_ActorLocal->train();
        
            auto action = torch::argmax(action_values.cpu());
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
            std::cout << p.sizes() << std::endl;
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
        std::cout << "-----------------------Actor Local-------------------" << std::endl;
        printNetwork(m_ActorLocal);
        std::cout << "-----------------------Actor Target------------------" << std::endl;
        printNetwork(m_ActorTarget);
    }
};

#endif // __AI_AGENT_H_