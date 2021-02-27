#ifndef _REPLAY_MEMORY_H_
#define _REPLAY_MEMORY_H_

#include<iostream>
#include<queue>
#include<tuple>
#include<random>
#include<memory>

#include <boost/circular_buffer.hpp>

#include <torch/torch.h>

#include "config.h"

typedef std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor> GroupTensorExperience;

// Current State, Action, Reward, Next State, Done!
typedef struct Experience{
    std::shared_ptr<GameState> m_CurrentState;
    std::shared_ptr<GameState> m_NextState;
    int       m_Action;
    float     m_Reward;
    bool      m_Done;

    Experience(){}

    Experience(std::shared_ptr<GameState> current_state, int action, float reward, std::shared_ptr<GameState> next_state, bool done) : 
               m_CurrentState(current_state), m_Action(action), m_Reward(reward), m_NextState(next_state), m_Done(done)
    {
                   
    }

}Experience, *P_Experience;

typedef struct ReplayMemory{
private:
    int32_t m_ActionSize;
    int32_t m_BufferSize;
    int32_t m_BatchSize;

    boost::circular_buffer_space_optimized<Experience> m_Buffer;

public:
    ReplayMemory(int32_t action_size, int32_t buffer_size, int32_t batch_size):
                m_ActionSize(action_size), m_BufferSize(buffer_size), m_BatchSize(batch_size)
    {
        m_Buffer.resize(m_BufferSize);
    }

    void add(std::shared_ptr<GameState> current_state, int action, float reward, std::shared_ptr<GameState> next_state, bool done){
        m_Buffer.push_front(Experience(current_state, action, reward, next_state, done));
    }

    GroupTensorExperience sample(){
        
        std::random_device                 m_RandomDevice;
        std::mt19937                       random_engine(m_RandomDevice());
        std::uniform_int_distribution<int> generator(0, m_BatchSize);

        std::vector<int> random_indices(m_BatchSize);
        std::generate(random_indices.begin(), random_indices.end(), [&](){ return generator(random_engine); });

        torch::Tensor stacked_current_states = m_Buffer[random_indices[0]].m_CurrentState->toTensor();
        torch::Tensor stacked_next_states    = m_Buffer[random_indices[0]].m_NextState->toTensor();
        torch::Tensor stacked_actions        = torch::tensor((float)m_Buffer[random_indices[0]].m_Action);
        torch::Tensor stacked_rewards        = torch::tensor((float)m_Buffer[random_indices[0]].m_Reward);
        torch::Tensor stacked_dones          = torch::tensor((float)m_Buffer[random_indices[0]].m_Done);

        // Loop the current queue and stack the tensors
        for(int idx = 1; idx < m_BatchSize; ++idx){
            Experience currentExperience = m_Buffer[random_indices[idx]];

            stacked_current_states       = torch::stack({stacked_current_states, currentExperience.m_CurrentState->toTensor()});
            stacked_next_states          = torch::stack({stacked_next_states,    currentExperience.m_NextState->toTensor()});
            stacked_actions              = torch::stack({stacked_actions, torch::tensor((float)currentExperience.m_Action)});
            stacked_rewards              = torch::stack({stacked_rewards, torch::tensor((float)currentExperience.m_Reward)});
            stacked_dones                = torch::stack({stacked_dones,   torch::tensor((float)currentExperience.m_Done)});
        }

        GroupTensorExperience experiencesTensor = std::make_tuple(stacked_current_states, stacked_next_states, stacked_actions, stacked_rewards, stacked_dones);
        
        return experiencesTensor;
    }

    size_t capacity(){
        return m_Buffer.size();
    }
    

}ReplayMemory, *P_ReplayMemory;

#endif // _REPLAY_MEMORY_H_