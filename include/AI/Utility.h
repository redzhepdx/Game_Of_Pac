#ifndef _AI_UTILITY_
#define _AI_UTILITY_

#include <iostream>
#include <vector>
#include <memory>
#include <random>

#include <torch/torch.h>

typedef struct OUNoise {
private:
    uint32_t m_Size;
    uint32_t m_Seed;
    float m_Mu = 0.0f;
    float m_Theta = 0.15f;
    float m_Sigma = 0.1f;

    torch::Tensor m_State;
    torch::Tensor m_InitialState;

public:
    // Ornstein-Uhlenbeck process
    OUNoise(uint32_t size, uint32_t seed) : m_Size(size), m_Seed(seed) {
        m_State = torch::full({m_Size}, m_Mu);
        m_InitialState = m_State;
    }

    OUNoise(uint32_t size, uint32_t seed, float mu, float theta, float sigma) : m_Size(size), m_Seed(seed), m_Mu(mu),
                                                                                m_Theta(theta), m_Sigma(sigma) {
        m_State = torch::full({m_Size}, m_Mu);
        m_InitialState = m_State;
    }

    void reset() {
        m_State = m_InitialState;
    }

    torch::Tensor sample() {
        auto x = m_State;
        auto dx = m_Theta * (m_InitialState - x) + m_Sigma * torch::rand({m_Size});

        m_State = x + dx;

        return m_State;
    }

} OUNoise, *P_OUNoise;

#endif // _AI_UTILITY_