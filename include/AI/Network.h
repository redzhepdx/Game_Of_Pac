#ifndef __Q_NETWORK_H_
#define __Q_NETWORK_H_

#include <torch/torch.h>

using namespace torch::nn;

struct DQNetworkImpl: torch::nn::Module{
    explicit DQNetworkImpl(){}

    virtual torch::Tensor forward(torch::Tensor state){ return torch::Tensor();}

    ~DQNetworkImpl(){}
};

struct DeepQNetworkImpl: public DQNetworkImpl{
private:
    int32_t m_StateSize;
    int32_t m_ActionSize;
    int32_t m_HiddenSize = 2048;

public:
    explicit DeepQNetworkImpl(int32_t state_size, int32_t action_size): m_StateSize(state_size), m_ActionSize(action_size){
        m_FullyConnected1 = Linear(m_StateSize, m_HiddenSize);
        m_FullyConnected2 = Linear(m_HiddenSize, m_HiddenSize);
        m_FullyConnected3 = Linear(m_HiddenSize, m_ActionSize);

        register_module("fc_1", m_FullyConnected1);
        register_module("fc_2", m_FullyConnected2);
        register_module("fc_3", m_FullyConnected3);
    }

    torch::Tensor forward(torch::Tensor state){
        auto xs = torch::nn::functional::relu(m_FullyConnected2->forward(state));
        xs = torch::nn::functional::relu(m_FullyConnected2->forward(xs));
        
        return m_FullyConnected3->forward(xs);
    }

private:
    Linear m_FullyConnected1{{nullptr}};
    Linear m_FullyConnected2{{nullptr}};    
    Linear m_FullyConnected3{{nullptr}};    
};


// Dueling DQN Network
struct DuelingDeepQNetworkImpl : public DQNetworkImpl{
private:
    int32_t m_StateSize;
    int32_t m_ActionSize;
    int32_t m_HiddenSize = 1024;

public:
    explicit DuelingDeepQNetworkImpl(int32_t state_size, int32_t action_size): m_StateSize(state_size), m_ActionSize(action_size){
        m_FullyConnected1 = Linear(m_StateSize, m_HiddenSize);
        m_FullyConnected2 = Linear(m_HiddenSize, m_HiddenSize);
        m_AdvantageOutput = Linear(m_HiddenSize, m_ActionSize);
        m_ValueOutput     = Linear(m_HiddenSize, 1);

        register_module("fc_1",      m_FullyConnected1);
        register_module("fc_2",      m_FullyConnected2);
        register_module("advantage", m_AdvantageOutput);
        register_module("value",     m_ValueOutput);
    }

    torch::Tensor forward(torch::Tensor state){
        
        auto xs = torch::nn::functional::relu(m_FullyConnected1->forward(state));
        xs = torch::nn::functional::relu(m_FullyConnected2->forward(xs));
        
        // Calculate the advantage and value functions
        auto advantage = m_AdvantageOutput->forward(xs);
        auto value     = m_ValueOutput->forward(xs);

        value          = value.expand_as(advantage);

        // Q_S = Value(V[s]) + Advantges(A[s, a]) - mean(A[s, a']) 
        return value + advantage - advantage.mean(1, true).expand_as(advantage);
    }

private:
    Linear m_FullyConnected1{{nullptr}};
    Linear m_FullyConnected2{{nullptr}};    
    Linear m_AdvantageOutput{{nullptr}};    
    Linear m_ValueOutput{{nullptr}};
};

struct DeepQCNNNetworkImpl : public DQNetworkImpl{
private:
    int32_t m_StateSize;
    int32_t m_ActionSize;

public:
    explicit DeepQCNNNetworkImpl(int32_t state_size, int32_t action_size): m_StateSize(state_size), m_ActionSize(action_size){
        
        m_FullyConnected = torch::nn::Linear(32 * 11 * 11 , action_size);

        register_module("conv_block_1", m_ConvolutionBlock1);
        register_module("conv_block_2", m_ConvolutionBlock2);
        register_module("conv_block_3", m_ConvolutionBlock3);
        register_module("out", m_FullyConnected);
    }

    torch::Tensor forward(torch::Tensor state){
        auto feature = m_ConvolutionBlock1->forward(state / 255.0);
        feature = m_ConvolutionBlock2->forward(feature);
        feature = m_ConvolutionBlock3->forward(feature);

        // Flatten
        feature = feature.view({feature.size(0), -1});

        return m_FullyConnected->forward(feature);
    }

private:
    torch::nn::Sequential m_ConvolutionBlock1{
        torch::nn::Conv2d(Conv2dOptions(m_StateSize, 32, 8).stride(4)),
        torch::nn::BatchNorm2d(32),
        torch::nn::ReLU()
    };
    torch::nn::Sequential m_ConvolutionBlock2{
        torch::nn::Conv2d(Conv2dOptions(32, 64, 4).stride(2).padding(2)),
        torch::nn::BatchNorm2d(64),
        torch::nn::ReLU()
    };


    torch::nn::Sequential m_ConvolutionBlock3{
        torch::nn::Conv2d(Conv2dOptions(64, 32, 3).stride(1)),
        torch::nn::BatchNorm2d(32),
        torch::nn::ReLU()
    }; // 32 x 11 x 11

    torch::nn::Linear m_FullyConnected{{nullptr}};
};

// Dueling DQN Network
struct DuelingDeepQCNNNetworkImpl : public DQNetworkImpl{
private:
    int32_t m_StateSize;
    int32_t m_ActionSize;

public:
    explicit DuelingDeepQCNNNetworkImpl(int32_t state_size, int32_t action_size): m_StateSize(state_size), m_ActionSize(action_size){
        
        m_Advantages = torch::nn::Linear(32 * 11 * 11, action_size);
        m_Value      = torch::nn::Linear(32 * 11 * 11, 1);

        // register_module("state_bn", m_StateNormalizer);
        register_module("conv_block_1", m_ConvolutionBlock1);
        register_module("conv_block_2", m_ConvolutionBlock2);
        register_module("conv_block_3", m_ConvolutionBlock3);
        register_module("advantages", m_Advantages);
        register_module("value", m_Value);
    }

    torch::Tensor forward(torch::Tensor state){

        auto feature = m_ConvolutionBlock1->forward(state / 255.0);
        feature = m_ConvolutionBlock2->forward(feature);
        feature = m_ConvolutionBlock3->forward(feature);

        // Flatten
        feature = feature.view({feature.size(0), -1});

        auto advantages = m_Advantages->forward(feature);
        auto value      = m_Value->forward(feature);

        value            = value.expand_as(advantages);

        // Q_S = Value(V[s]) + Advantges(A[s, a]) - mean(A[s, a']) 
        return value + advantages - advantages.mean(1, true).expand_as(advantages);
    }

private:
    torch::nn::Sequential m_ConvolutionBlock1{
        torch::nn::Conv2d(Conv2dOptions(m_StateSize, 32, 8).stride(4)),
        torch::nn::BatchNorm2d(32),
        torch::nn::ReLU()
    };

    torch::nn::Sequential m_ConvolutionBlock2{
        torch::nn::Conv2d(Conv2dOptions(32, 64, 4).stride(2).padding(2)),
        torch::nn::BatchNorm2d(64),
        torch::nn::ReLU()
    }; 


    torch::nn::Sequential m_ConvolutionBlock3{
        torch::nn::Conv2d(Conv2dOptions(64, 32, 3).stride(1)),
        torch::nn::BatchNorm2d(32),
        torch::nn::ReLU()
    }; // 32 x 11 x 11

    torch::nn::Linear m_Value{{nullptr}};
    torch::nn::Linear m_Advantages{{nullptr}};
};


// TORCH_MODULE_IMPL(DQNetwork, DQNetworkImpl);

// TORCH_MODULE_IMPL(DeepQNetwork, DeepQNetworkImpl);
// TORCH_MODULE_IMPL(DuelingDeepQNetwork, DuelingDeepQNetworkImpl);

// TORCH_MODULE_IMPL(DeepQCNNNetwork, DeepQCNNNetworkImpl);
// TORCH_MODULE_IMPL(DuelingDeepQCNNNetwork, DuelingDeepQCNNNetworkImpl);


#endif // __Q_NETWORK_H_