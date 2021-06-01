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
#include "GameState.h"

#include "spdlog/spdlog.h"

class Agent {

private:
    int32_t m_StateSize = 100;
    int32_t m_ActionSize = 8;

    int16_t m_UpdateRate = 100;
    int64_t m_Step = 0;

    float m_Epsilon = 1.0;

    ObservationType m_ObservationType = Internal;

    std::unique_ptr<OUNoise> m_Noise{nullptr};
    std::unique_ptr<ReplayMemory> m_Memory{nullptr};

    std::shared_ptr<DQNetworkImpl> m_ActorLocal{nullptr};
    std::shared_ptr<DQNetworkImpl> m_ActorTarget{nullptr};

    std::unique_ptr<torch::optim::Adam> m_Optimizer{nullptr};

    torch::Device m_Device = torch::kCPU;

private:
    void updateLearningRate(float new_lr_rate) {
        dynamic_cast<torch::optim::AdamOptions &>(m_Optimizer->param_groups()[0].options()).lr(new_lr_rate);
    }

    void updateOptimizerParameters(const std::vector<torch::Tensor>& network_parameters) {
        m_Optimizer = std::make_unique<torch::optim::Adam>(network_parameters, LR_RATE);
    }

public:
    Agent(int32_t state_size, int32_t action_size, torch::Device device, ObservationType observation_type)
            : m_StateSize(state_size), m_ActionSize(action_size), m_Device(device),
              m_ObservationType(observation_type) {

        at::globalContext().setBenchmarkCuDNN(true);
        if (m_ObservationType == Image) {
            m_ActorLocal = std::make_shared<DuelingDeepQCNNNetworkImpl>(m_StateSize, m_ActionSize);
            m_ActorTarget = std::make_shared<DuelingDeepQCNNNetworkImpl>(m_StateSize, m_ActionSize);
        } else {
            m_ActorLocal = std::make_shared<DuelingDeepQNetworkImpl>(m_StateSize, m_ActionSize);
            m_ActorTarget = std::make_shared<DuelingDeepQNetworkImpl>(m_StateSize, m_ActionSize);
        }

        m_Optimizer = std::make_unique<torch::optim::Adam>(m_ActorLocal->parameters(), LR_RATE);

        m_Noise = std::make_unique<OUNoise>(m_ActionSize, SEED);
        m_Memory = std::make_unique<ReplayMemory>(m_ActionSize, BUFFER_SIZE, BATCH_SIZE, m_Device);
        m_Epsilon = EPS;
        m_UpdateRate = AGENT_UPDATE_RATE;

        m_ActorLocal->train();
        m_ActorTarget->train();

        m_ActorLocal->to(m_Device);
        m_ActorTarget->to(m_Device);

        this->printNetworks();

        this->loadNetworks();
    }

    void step(std::unique_ptr<GameState> current_state, std::unique_ptr<GameState> next_state, int action, float reward,
              bool done) {
        m_Memory->add(std::move(current_state), std::move(next_state), action, reward, done);

        if (m_Memory->capacity() > BATCH_SIZE && m_Step % TRAIN_EVERY == 0) {
            for (uint32_t sampling_iter = 0; sampling_iter < SAMPLING_ITERATION; ++sampling_iter) {
                GroupTensorExperience experiences = m_Memory->sample();
                this->learn(experiences);
            }
        }

        if (m_Step % SAVE_EVERY == 0 && m_Step > 0) {
            this->saveNetworks();
        }

        ++m_Step;
    }

    void learn(const GroupTensorExperience& experiences) {
        // Double Q Learning

        torch::Tensor current_states;
        torch::Tensor next_states;
        torch::Tensor actions;
        torch::Tensor rewards;
        torch::Tensor dones;

        std::tie(current_states, next_states, actions, rewards, dones) = experiences;

        actions = actions.unsqueeze(1);
        rewards = rewards.unsqueeze(1);
        dones = dones.unsqueeze(1);

        // Get next action estimation with local q network
        auto q_targets_next_expected = m_ActorLocal->forward(next_states).detach();
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

        spdlog::debug("Loss : {}", loss.item<double>());

        if (m_Step % AGENT_UPDATE_RATE == 0) {
            this->softUpdate();
        }
    }

    uint32_t act(std::unique_ptr<GameState> &state) {
        float random_value = ((float) std::rand() / RAND_MAX);

        if (random_value > m_Epsilon) {
            torch::Tensor stateTensor = state->toTensor();
            stateTensor = stateTensor.to(m_Device);

            m_ActorLocal->eval();

            // Lock Gradient Calculations
            torch::autograd::GradMode::set_enabled(false);

            auto action_values = m_ActorLocal->forward(stateTensor).detach();

            // Unlock the Gradient Calculations
            torch::autograd::GradMode::set_enabled(true);

            m_ActorLocal->train();

            auto action = torch::argmax(action_values).cpu();

            // std::cout << "AI Action : " << action << "\n Action Vector : " << action_values << "\n";

            return (uint32_t) action.item<int>();
        }

        uint32_t action = (uint32_t) (std::rand() % m_ActionSize);
        // std::cout << "Random Action : " << action << "\n";
        return action;
    }

    void softUpdate() {
        spdlog::debug("Updating The Target Network");
        // Update Target Network's parameters slowly
        torch::autograd::GradMode::set_enabled(false);

        auto target_params = m_ActorTarget->named_parameters(); // implement this
        auto params = m_ActorLocal->named_parameters(true /*recurse*/);
        auto buffers = m_ActorLocal->named_buffers(true /*recurse*/);
        for (auto &val : params) {
            auto name = val.key();
            auto *t = target_params.find(name);
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

    void updateEpsilon() {
        m_Epsilon = std::max(MIN_EPS, m_Epsilon * EPS_REDUCTION);

        spdlog::info("Step : {} Random Exploration Rate Reduction -> New Value : {}", m_Step, m_Epsilon);
        spdlog::critical("Memory Size : {}", m_Memory->capacity());

        // Update Epsilon
        // if (m_Step % EXPLORATION_UPDATE == 0)
        // {
        //     if (m_Epsilon > MIN_EPS && m_Step != 0)
        //     {
        //         m_Epsilon *= EPS_REDUCTION;
        //     }

        // }
    }

    [[nodiscard]] uint16_t totalStepCount() const {
        return m_Step;
    }

    static void printNetwork(const std::shared_ptr<DQNetworkImpl> &network) {
        for (const auto &p : network->parameters()) {
            std::cout << p.sizes() << "\n";
        }
    }

    static void initWeights(std::shared_ptr<DQNetworkImpl> &network) {
        torch::autograd::GradMode::set_enabled(false);

        for (auto &parameter : network->named_parameters(true)) {
            auto param_name = parameter.key();
            auto param = parameter.value();

            if (param_name.compare(2, 6, "weight") == 0) {
                torch::nn::init::xavier_uniform_(param);
            } else if (param_name.compare(2, 4, "bias") == 0) {
                torch::nn::init::constant_(param, 0.01);
            }
        }

        torch::autograd::GradMode::set_enabled(true);
    }

    void printNetworks() {
        spdlog::info("-----------------------Actor Local-------------------");
        printNetwork(m_ActorLocal);
        spdlog::info("-----------------------Actor Target------------------");
        printNetwork(m_ActorTarget);
    }

    static void saveNetwork(std::shared_ptr<DQNetworkImpl> &network, const std::string& model_path) {
        torch::serialize::OutputArchive output_archive;
        network->save(output_archive);
        output_archive.save_to(model_path);
    }

    void saveNetworks() {
        spdlog::info("Saving Networks");

        saveNetwork(m_ActorLocal, "ActorLocal.pt");
        saveNetwork(m_ActorTarget, "ActorTarget.pt");
    }

    void loadNetwork(std::shared_ptr<DQNetworkImpl> &network, const std::string& model_path) {
        if (boost::filesystem::exists(model_path.c_str())) {
            torch::serialize::InputArchive archive;
            archive.load_from(model_path);
            network->load(archive);
            m_Epsilon = MIN_EPS;
        } else {
            spdlog::info("There is no {} named file!", model_path);
            spdlog::info("Random Weight Initialization");
            initWeights(network);
        }
    }

    void loadNetworks() {
        spdlog::info("Loading Pretrained Networks");

        loadNetwork(m_ActorLocal, "ActorLocal.pt");
        loadNetwork(m_ActorTarget, "ActorTarget.pt");
    }
};

#endif // __AI_AGENT_H_