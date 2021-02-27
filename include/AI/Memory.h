#pragma once

#include<iostream>
#include<queue>
#include<tuple>

#include <boost/circular_buffer.hpp>

#include <torch/torch.h>

#include "Utility.h"

typedef std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor> GroupTensorExperience;

// Current State, Action, Reward, Next State, Done!
typedef struct Experience{
    GameState m_CurrentState;
    GameState m_NextState;
    int       m_Action;
    float     m_Reward;
    bool      m_Done;

    Experience(){}

    Experience(GameState current_state, int action, float reward, GameState next_state, bool done) : 
               m_CurrentState(current_state), m_Action(action), m_Reward(reward), m_NextState(next_state), m_Done(done)
    {
                   
    }

}Experience, *P_Experience;

typedef struct ReplayMemory{
private:
    int32_t m_ActionSize;
    int32_t m_BufferSize;
    int32_t m_BatchSize;

    boost::circular_buffer<Experience> m_Buffer;

public:
    ReplayMemory(int32_t action_size, int32_t buffer_size, int32_t batch_size):
                m_ActionSize(action_size), m_BufferSize(buffer_size), m_BatchSize(batch_size)
    {
        m_Buffer.resize(m_BufferSize);
    }

    void add(GameState current_state, int action, float reward, GameState next_state, bool done){
        m_Buffer.push_front(Experience(current_state, action, reward, next_state, done));
    }

    GroupTensorExperience sample(){

        torch::Tensor stacked_current_states;
        torch::Tensor stacked_next_states;
        torch::Tensor stacked_actions;
        torch::Tensor stacked_rewards;
        torch::Tensor stacked_dones;

        // TODO Loop the current queue and stack the tensors

        GroupTensorExperience experiences = std::make_tuple(stacked_current_states, stacked_next_states, stacked_actions, stacked_rewards, stacked_dones);
        
        return experiences;
    }

    size_t capacity(){
        return m_Buffer.size();
    }
    

}ReplayMemory, *P_ReplayMemory;