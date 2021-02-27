#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <random>

#include <torch/torch.h>

#include "Vector2.h"
#include "Maze.h"
#include "utils.h"
#include "config.h"

constexpr int MAX_ACTIVE_BULLET_COUNT = 20;

typedef struct GameState
{
    int              m_AvailableProjectiles;
    float            m_TimeToTeleport;
    Vector2<float>   m_PlayerPos;
    tile_type_matrix m_Tiles;
    
    std::vector<Vector2<float>> m_EnemyPositions;
    std::vector<Vector2<float>> m_ActiveBulletPositions;
    std::vector<Vector2<float>> m_ActiveBulletDirections;

    torch::Tensor stateTotensor(){
        std::vector<float> stateVector;

        stateVector.push_back(m_PlayerPos.x);
        stateVector.push_back(m_PlayerPos.y);

        stateVector.push_back((float)m_AvailableProjectiles);
        stateVector.push_back(m_TimeToTeleport);

        auto VectorInsert = [&](Vector2<float> pos){ 
            stateVector.push_back(pos.x); 
            stateVector.push_back(pos.y);
        };
        
        // Empty Bullets
        std::vector<float> remainingEmptyEnemy((MAX_ENEMY_COUNT - m_EnemyPositions.size()) * 2);
        std::vector<float> remainingEmptyBullet((MAX_ACTIVE_BULLET_COUNT - m_ActiveBulletPositions.size()) * 2);

        std::for_each(m_EnemyPositions.begin(), m_EnemyPositions.end(), VectorInsert);
        stateVector.insert(stateVector.end(), remainingEmptyEnemy.begin(), remainingEmptyEnemy.end());

        std::for_each(m_ActiveBulletPositions.begin(), m_ActiveBulletPositions.end(), VectorInsert);
        stateVector.insert(stateVector.end(), remainingEmptyBullet.begin(), remainingEmptyBullet.end());

        std::for_each(m_ActiveBulletDirections.begin(), m_ActiveBulletDirections.end(), VectorInsert);
        stateVector.insert(stateVector.end(), remainingEmptyBullet.begin(), remainingEmptyBullet.end());

        for(std::vector<TileType> tile_vec : m_Tiles){
            stateVector.insert(stateVector.end(), tile_vec.begin(), tile_vec.end());
        }

        torch::Tensor stateTensor = torch::from_blob(stateVector.data(), {(long)stateVector.size()}, torch::TensorOptions().dtype(torch::kInt32)).to(torch::kInt64);

        return stateTensor;
    }

}GameState, *PGameState;

std::unique_ptr<GameState> createGameState(Vector2<float> player_pos, 
                          int available_projectiles, int time_to_teleport, 
                          tile_type_matrix tiles, 
                          std::vector<Vector2<float>> enemy_positions,
                          std::vector<Vector2<float>> bullet_positions,
                          std::vector<Vector2<float>> bullet_directions)
{    
    std::unique_ptr<GameState> state = std::make_unique<GameState>();

    state->m_PlayerPos              = player_pos;
    state->m_AvailableProjectiles   = available_projectiles;
    state->m_TimeToTeleport         = time_to_teleport;
    state->m_Tiles                  = tiles;

    state->m_EnemyPositions.assign(enemy_positions.begin(), enemy_positions.end());
    state->m_ActiveBulletPositions.assign(bullet_positions.begin(), bullet_positions.end());
    state->m_ActiveBulletDirections.assign(bullet_directions.begin(), bullet_directions.end());

    return std::move(state);
}


typedef struct OUNoise
{
private:
    uint32_t m_Size;
    uint32_t m_Seed;
    float    m_Mu    = 0.0f;
    float    m_Theta = 0.15f;
    float    m_Sigma = 0.1f;
    
    torch::Tensor m_State;
    torch::Tensor m_InitialState;

public:
    // Ornstein-Uhlenbeck process
    OUNoise(uint32_t size, uint32_t seed) : m_Size(size), m_Seed(seed){
        m_State = torch::full({m_Size}, m_Mu);
        m_InitialState = m_State;
    }

    OUNoise(uint32_t size, uint32_t seed, float mu, float theta, float sigma) : 
            m_Size(size), m_Seed(seed), m_Mu(mu), m_Theta(theta), m_Sigma(sigma){
        m_State = torch::full({m_Size}, m_Mu);
        m_InitialState = m_State;
    }

    void reset(){
        m_State = m_InitialState;
    }

    torch::Tensor sample(){
        auto x  = m_State;
        auto dx = m_Theta * (m_InitialState - x) + m_Sigma * torch::rand({m_Size});
        
        m_State = x + dx;
        
        return m_State;
    }

}OUNoise, *P_OUNoise;