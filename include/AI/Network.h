#ifndef __Q_NETWORK_H_
#define __Q_NETWORK_H_

#include <torch/torch.h>

using namespace torch::nn;

struct DeepQNetworkImpl : torch::nn::Module{
private:
    int32_t m_StateSize;
    int32_t m_ActionSize;
    int32_t m_HiddenSize = 1024;

public:
    explicit DeepQNetworkImpl(int32_t state_size, int32_t action_size): m_StateSize(state_size), m_ActionSize(action_size){
        m_FullyConnected1 = Linear(m_StateSize, m_HiddenSize);
        m_FullyConnected2 = Linear(m_HiddenSize, m_ActionSize);
        m_StateNormalizer = BatchNorm1d(m_StateSize);
        
        register_module("state_bn", m_StateNormalizer);
        register_module("fc_1", m_FullyConnected1);
        register_module("fc_2", m_FullyConnected2);
    }

    torch::Tensor forward(torch::Tensor state){
        auto xs = m_StateNormalizer->forward(state);
        xs = torch::nn::functional::relu(m_FullyConnected1->forward(xs));
        return m_FullyConnected2->forward(xs);
    }

private:
    Linear m_FullyConnected1{nullptr};
    Linear m_FullyConnected2{{nullptr}};    
    BatchNorm1d m_StateNormalizer{nullptr};
};

TORCH_MODULE_IMPL(DeepQNetwork, DeepQNetworkImpl);

#endif // __Q_NETWORK_H_