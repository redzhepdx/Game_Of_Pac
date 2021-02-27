#pragma once

#include <torch/torch.h>

using namespace torch::nn;

struct DeepQNetworkImpl : torch::nn::Module{
private:
    int32_t m_StateSize;
    int32_t m_ActionSize;
    int32_t m_HiddenSize = 128;

public:
    explicit DeepQNetworkImpl(int32_t state_size, int32_t action_size);

    torch::Tensor forward(torch::Tensor state, torch::Tensor action);

private:
    Linear m_FullyConnected1{nullptr};
    Linear m_FullyConnected2{{nullptr}};
    
    BatchNorm1d m_StateNormalizer{nullptr};
};

DeepQNetworkImpl::DeepQNetworkImpl(int32_t state_size, int32_t action_size) : m_StateSize(state_size), m_ActionSize(action_size){
    m_FullyConnected1 = Linear(m_StateSize, m_HiddenSize);
    m_FullyConnected2 = Linear(m_HiddenSize + m_ActionSize, m_ActionSize);
    m_StateNormalizer = BatchNorm1d(m_StateSize);
    
    register_module("state_bn", m_StateNormalizer);
    register_module("fc_1", m_FullyConnected1);
    register_module("fc_2", m_FullyConnected2);
}


torch::Tensor DeepQNetworkImpl::forward(torch::Tensor state, torch::Tensor action){
    auto xs = m_StateNormalizer(state);
    xs = torch::nn::functional::relu(m_FullyConnected1->forward(xs));
    xs = torch::cat({xs, action}, 1);
    return m_FullyConnected2->forward(xs);
}

TORCH_MODULE_IMPL(DeepQNetwork, DeepQNetworkImpl);