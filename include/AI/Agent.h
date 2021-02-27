#include <iostream>
#include <vector>
#include <memory>

#include "Network.h"
#include "Utility.h"
#include "Memory.h"

constexpr int16_t AGENT_UPDATE_RATE = 2;
constexpr float   LR_RATE           = 0.001;
constexpr float   BUFFER_SIZE       = 100;
constexpr float   BATCH_SIZE        = 10;
constexpr int32_t SEED              = 1337;

class Agent{

private:
    int32_t m_StateSize;
    int32_t m_ActionSize;

    int16_t m_UpdateRate = AGENT_UPDATE_RATE;
    int64_t m_Step = 0;

    std::unique_ptr<OUNoise> m_Noise;
    std::unique_ptr<ReplayMemory> m_Memory;

    DeepQNetwork m_ActorLocal{nullptr};
    DeepQNetwork m_ActorTarget{nullptr};

    std::unique_ptr<torch::optim::Adam> m_Optimizer;

private:
    void updateLearningRate(float new_lr_rate){
        static_cast<torch::optim::AdamOptions&>(m_Optimizer->param_groups()[0].options()).lr(new_lr_rate);
    }

public:
    Agent(int32_t state_size, int32_t action_size) : m_StateSize(state_size), m_ActionSize(action_size){
        m_ActorLocal  = DeepQNetwork(m_StateSize, m_ActionSize);
        m_ActorTarget = DeepQNetwork(m_StateSize, m_ActionSize);

        m_Optimizer->add_parameters(m_ActorLocal->parameters());
        this->updateLearningRate(LR_RATE);

        m_Noise       = std::make_unique<OUNoise>(m_ActionSize, SEED);
        m_Memory      = std::make_unique<ReplayMemory>(m_ActionSize, BUFFER_SIZE, BATCH_SIZE);
    }

    void step(GameState current_state, int action, float reward, GameState next_state, bool done){
        m_Memory->add(current_state, action, reward, next_state, done);

        if(m_Memory->capacity() > BATCH_SIZE){
            GroupTensorExperience experiences = m_Memory->sample();
            this->learn(experiences);
        }

        ++m_Step;
    }

    void learn(GroupTensorExperience experiences){
        // TODO : In Progress
    }

    uint32_t act(){
        // TODO : In Progress
        return 0;
    }

    void softUpdate(){
        // TODO : In Progress
    }
};