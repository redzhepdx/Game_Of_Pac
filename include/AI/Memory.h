#ifndef _REPLAY_MEMORY_H_
#define _REPLAY_MEMORY_H_

#include<iostream>
#include<queue>
#include<tuple>
#include<random>
#include<memory>
#include<unordered_set>

#include <boost/circular_buffer.hpp>

#include <torch/torch.h>

#include "config.h"
#include "GameState.h"

typedef std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor> GroupTensorExperience;

// Current State, Action, Reward, Next State, Done!
typedef struct Experience{
    std::unique_ptr<GameState> m_CurrentState;
    std::unique_ptr<GameState> m_NextState;
    int       m_Action;
    float     m_Reward;
    bool      m_Done;

    Experience(){}

    Experience(std::unique_ptr<GameState> current_state, std::unique_ptr<GameState> next_state, int action, float reward, bool done) : 
               m_CurrentState(std::move(current_state)), m_NextState(std::move(next_state)), m_Action(action), m_Reward(reward), m_Done(done)
    {
                   
    }

}Experience, *P_Experience;

typedef struct ReplayMemory{
private:
    int32_t  m_ActionSize;
    int32_t  m_BufferSize;
    uint32_t m_BatchSize;

    boost::circular_buffer_space_optimized<Experience> m_Buffer;

    torch::Device m_Device;

public:
    ReplayMemory(int32_t action_size, int32_t buffer_size, int32_t batch_size, torch::Device device):
                m_ActionSize(action_size), m_BufferSize(buffer_size), m_BatchSize(batch_size), m_Device(device)
    {
        m_Buffer.set_capacity(m_BufferSize);
    }

    void add(std::unique_ptr<GameState> current_state, std::unique_ptr<GameState> next_state, int action, float reward, bool done){
        m_Buffer.push_back(Experience(std::move(current_state), std::move(next_state), action, reward, done));
    }

    GroupTensorExperience sample(){
        std::random_device                 m_RandomDevice;
        std::mt19937                       random_engine(m_RandomDevice());
        std::uniform_int_distribution<int> generator(0, m_Buffer.size() - 1);

        // Get n unique experiments
        std::unordered_set<int> random_index_set;
        while(random_index_set.size() != m_BatchSize){
            random_index_set.insert(generator(random_engine));
        }
        std::vector<int> random_indices(random_index_set.begin(), random_index_set.end());

        torch::Tensor stacked_current_states = m_Buffer[random_indices[0]].m_CurrentState->toTensor();
        torch::Tensor stacked_next_states    = m_Buffer[random_indices[0]].m_NextState->toTensor();
        
        torch::Tensor stacked_actions        = torch::tensor((int64_t)m_Buffer[random_indices[0]].m_Action).unsqueeze(0);
        torch::Tensor stacked_rewards        = torch::tensor((float)m_Buffer[random_indices[0]].m_Reward).unsqueeze(0);
        torch::Tensor stacked_dones          = torch::tensor((uint8_t)m_Buffer[random_indices[0]].m_Done).unsqueeze(0);

        // Loop the current queue and stack the tensors
        for(uint32_t idx = 1; idx < m_BatchSize; ++idx){

            stacked_current_states       = torch::cat({stacked_current_states, m_Buffer[random_indices[idx]].m_CurrentState->toTensor()}, 0);
            stacked_next_states          = torch::cat({stacked_next_states,    m_Buffer[random_indices[idx]].m_NextState->toTensor()},    0);

            stacked_actions              = torch::cat({stacked_actions, torch::tensor((int64_t)m_Buffer[random_indices[idx]].m_Action).unsqueeze(0)});
            stacked_rewards              = torch::cat({stacked_rewards, torch::tensor((float)  m_Buffer[random_indices[idx]].m_Reward).unsqueeze(0)});
            stacked_dones                = torch::cat({stacked_dones,   torch::tensor((uint8_t)m_Buffer[random_indices[idx]].m_Done).unsqueeze(0)});
        }

        stacked_current_states = stacked_current_states.to(m_Device);
        stacked_next_states    = stacked_next_states.to(m_Device);
        stacked_actions        = stacked_actions.to(m_Device);
        stacked_rewards        = stacked_rewards.to(m_Device);
        stacked_dones          = stacked_dones.to(m_Device);

        GroupTensorExperience experiencesTensor = std::make_tuple(stacked_current_states, stacked_next_states, stacked_actions, stacked_rewards, stacked_dones);

        return experiencesTensor;
    }

    size_t capacity(){
        return m_Buffer.size();
    }
    

}ReplayMemory, *P_ReplayMemory;

#endif // _REPLAY_MEMORY_H_