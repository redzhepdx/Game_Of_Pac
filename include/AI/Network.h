#ifndef __Q_NETWORK_H_
#define __Q_NETWORK_H_

#include <torch/torch.h>

using namespace torch::nn;

struct DeepQNetworkImpl : torch::nn::Module{
private:
    int32_t m_StateSize;
    int32_t m_ActionSize;
    int32_t m_HiddenSize = 2048;

public:
    explicit DeepQNetworkImpl(int32_t state_size, int32_t action_size): m_StateSize(state_size), m_ActionSize(action_size){
        m_FullyConnected1 = Linear(m_StateSize, m_HiddenSize);
        m_FullyConnected2 = Linear(m_HiddenSize, m_HiddenSize);
        m_FullyConnected3 = Linear(m_HiddenSize, m_ActionSize);
        m_StateNormalizer = BatchNorm1d(m_StateSize);
        
        register_module("state_bn", m_StateNormalizer);
        register_module("fc_1", m_FullyConnected1);
        register_module("fc_2", m_FullyConnected2);
        register_module("fc_3", m_FullyConnected3);
    }

    torch::Tensor forward(torch::Tensor state){
        auto xs = m_StateNormalizer->forward(state);

        xs = torch::nn::functional::relu(m_FullyConnected1->forward(xs));
        xs = torch::nn::functional::relu(m_FullyConnected2->forward(xs));
        
        return m_FullyConnected3->forward(xs);
    }

private:
    Linear m_FullyConnected1{nullptr};
    Linear m_FullyConnected2{{nullptr}};    
    Linear m_FullyConnected3{{nullptr}};    
    BatchNorm1d m_StateNormalizer{nullptr};
};


// Dueling DQN Network
struct DuelingDeepQNetworkImpl : torch::nn::Module{
private:
    int32_t m_StateSize;
    int32_t m_ActionSize;
    int32_t m_HiddenSize = 2048;

public:
    explicit DuelingDeepQNetworkImpl(int32_t state_size, int32_t action_size): m_StateSize(state_size), m_ActionSize(action_size){
        m_FullyConnected1 = Linear(m_StateSize, m_HiddenSize);
        m_FullyConnected2 = Linear(m_HiddenSize, m_HiddenSize);
        m_AdvantageOutput = Linear(m_HiddenSize, m_ActionSize);
        m_ValueOutput     = Linear(m_HiddenSize, 1);
        m_StateNormalizer = BatchNorm1d(m_StateSize);
        
        register_module("state_bn", m_StateNormalizer);
        register_module("fc_1", m_FullyConnected1);
        register_module("fc_2", m_FullyConnected2);
        register_module("fc_3", m_AdvantageOutput);
    }

    torch::Tensor forward(torch::Tensor state){
        auto xs = m_StateNormalizer->forward(state);
        xs = torch::nn::functional::relu(m_FullyConnected1->forward(xs));
        xs = torch::nn::functional::relu(m_FullyConnected2->forward(xs));
        
        // Calculate the advantage and value functions
        auto advantage = m_AdvantageOutput->forward(xs);
        auto value     = m_ValueOutput->forward(xs);

        value          = value.expand_as(advantage);

        // Q_S = Value(V[s]) + Advantges(A[s, a]) - mean(A[s, a']) 
        return value + advantage - advantage.mean(1, true).expand_as(advantage);
    }

private:
    Linear m_FullyConnected1{nullptr};
    Linear m_FullyConnected2{{nullptr}};    
    Linear m_AdvantageOutput{{nullptr}};    
    Linear m_ValueOutput{{nullptr}};    
    BatchNorm1d m_StateNormalizer{nullptr};
};


TORCH_MODULE_IMPL(DeepQNetwork, DeepQNetworkImpl);
TORCH_MODULE_IMPL(DuelingDeepQNetwork, DuelingDeepQNetworkImpl);

#endif // __Q_NETWORK_H_