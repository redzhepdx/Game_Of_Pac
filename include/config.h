#ifndef __CONFIG_H_
#define __CONFIG_H_

#include <torch/torch.h>
#include "spdlog/spdlog.h"

#define BOOST_LOG_DYN_LINK 1

enum Direction { Left, Right, Up, Down};

typedef struct VertexData{
    GLfloat positionCoordinates[3];
    GLfloat textureCoordinates[2];
}VertexData, *PVertexData;

// Window settings
constexpr unsigned int WIDTH  = 800;
constexpr unsigned int HEIGHT = 800;

// Game update time settings
constexpr unsigned int DELAY_NUMBER         = 10000000;
constexpr double       UPDATES_PER_SECOND   = 60.0f;
constexpr float        DELTA_TIME_THRESHOLD = 1.0f;

constexpr int ENEMY_SPAWN_TICKS = 60;
constexpr int ENEMY_FIRE_TICKS  = 60;
constexpr int PLAYER_FIRE_TICKS = 30;
constexpr int PLAYER_TP_TICKS   = 300;

// Game logging settings
constexpr bool RUNNING   = true;
constexpr bool STOPPED   = false;
constexpr bool DEBUG_LOG = true;
constexpr bool INFO_LOG  = false;

// Tile Settings
constexpr GLfloat MOVE_SIZE                = 5.0f;
constexpr GLfloat SQUARE_SIZE              = 20.0f;
constexpr float   MAX_ENEMY_SPAWN_DISTANCE = 160.0f;
constexpr float   MIN_ENEMY_SPAWN_DISTANCE = 160.0f;
constexpr float   SPAWN_DIST_REDUCTION     = 0.9f;
constexpr float   MAP_TILE_PERCENTAGE      = 0.55;

// Object Velocity and Size Settings
constexpr float PLAYER_SPEED  = 3.0f;
constexpr float ENEMY_SPEED   = 2.0f;
constexpr float BULLET_SPEED  = 5.0f;
constexpr float OBJECT_OFFSET = 5.0f;

// Enemy Settings
constexpr int   ENEMY_COUNT             = 10;
constexpr int   ENEMY_DESTROY_POINTS    = 10;
constexpr uint  MAX_ENEMY_COUNT         = 10;
constexpr float SNIPER_SIGHT_DISTANCE   = 300.0f;
constexpr int   MAX_ACTIVE_BULLET_COUNT = 20;

// RL Agent Hyper-parameters
constexpr int16_t AGENT_UPDATE_RATE  = 4;
constexpr int16_t TRAIN_EVERY        = 16;
constexpr int16_t SAVE_EVERY         = 200;
constexpr int32_t SEED               = 1337;
constexpr int32_t STATE_SIZE         = 1765;
constexpr int32_t ACTION_SIZE        = 8;
constexpr int32_t EXPLORATION_UPDATE = 5000;
constexpr int32_t BUFFER_SIZE        = 10000;
constexpr int32_t BATCH_SIZE         = 256;

constexpr float   LR_RATE            = 0.0001;
constexpr float   EPS                = 0.8f;
constexpr float   EPS_REDUCTION      = 0.9f;
constexpr float   MIN_EPS            = 0.05f;
constexpr float   TAU                = 0.2f;
constexpr float   GAMMA              = 0.1f;

// Player Settings
constexpr int INITIAL_PLAYER_HEALTH  = 100;
constexpr int INITIAL_PLAYER_BULLETS = 10;
constexpr int HEALTH_LOSS_AFTER_HIT  = 10;

// Texture Settings
constexpr VertexData VERTICES[4] = {
        {{0.0f,        0.0f,        0.0f}, {0.0f, 1.0f}},
        {{SQUARE_SIZE, 0.0f,        0.0f}, {1.0f, 1.0f}},
        {{SQUARE_SIZE, SQUARE_SIZE, 0.0f}, {1.0f, 0.0f}},
        {{0.0f,        SQUARE_SIZE, 0.0f}, {0.0f, 0.0f}}
    };


enum TileType {Empty, Wall, Obstacle, Teleport};

typedef std::vector<std::vector<TileType>> tile_type_matrix;

typedef struct GameState
{
    int              m_AvailableProjectiles;
    float            m_TimeToTeleport;
    Vector2<float>   m_PlayerPos;
    tile_type_matrix m_Tiles;
    
    std::vector<Vector2<float>> m_EnemyPositions;
    std::vector<Vector2<float>> m_ActiveBulletPositions;
    std::vector<float> m_ActiveBulletDirections;

    static std::unique_ptr<GameState> createGameState(Vector2<float> player_pos, 
                                               int available_projectiles, int time_to_teleport, 
                                               tile_type_matrix tiles, 
                                               std::vector<Vector2<float>> enemy_positions,
                                               std::vector<Vector2<float>> bullet_positions,
                                               std::vector<float> bullet_directions){
        std::unique_ptr<GameState> state = std::make_unique<GameState>();

        state->m_PlayerPos              = player_pos;
        state->m_AvailableProjectiles   = available_projectiles;
        state->m_TimeToTeleport         = time_to_teleport;
        state->m_Tiles                  = tiles;

        state->m_EnemyPositions         = enemy_positions;
        state->m_ActiveBulletPositions  = bullet_positions;
        state->m_ActiveBulletDirections = bullet_directions;

        return state;

    }

    std::unique_ptr<GameState> copy(){
        std::unique_ptr<GameState> copyState = std::make_unique<GameState>();
        copyState->m_PlayerPos              = m_PlayerPos;
        copyState->m_AvailableProjectiles   = m_AvailableProjectiles;
        copyState->m_TimeToTeleport         = m_TimeToTeleport;
        copyState->m_Tiles                  = m_Tiles;
        copyState->m_EnemyPositions         = m_EnemyPositions;
        copyState->m_ActiveBulletPositions  = m_ActiveBulletPositions;
        copyState->m_ActiveBulletDirections = m_ActiveBulletDirections;
        return copyState;
    }

    torch::Tensor toTensor(){
        std::vector<float> stateVector;

        stateVector.push_back(m_PlayerPos.x / HEIGHT);
        stateVector.push_back(m_PlayerPos.y / WIDTH);

        stateVector.push_back((float)m_AvailableProjectiles);
        stateVector.push_back(m_TimeToTeleport / PLAYER_TP_TICKS);

        auto VectorInsert = [&](Vector2<float> pos){ 
            stateVector.push_back(pos.x / HEIGHT); 
            stateVector.push_back(pos.y / WIDTH);
        };
        
        // Empty Bullets
        std::vector<float> remainingEmptyEnemy((MAX_ENEMY_COUNT - m_EnemyPositions.size()) * 2, 0);
        std::vector<float> remainingEmptyBullet((MAX_ACTIVE_BULLET_COUNT - m_ActiveBulletPositions.size()) * 2, 0);

        std::for_each(m_EnemyPositions.begin(), m_EnemyPositions.end(), VectorInsert);
        stateVector.insert(stateVector.end(), remainingEmptyEnemy.begin(), remainingEmptyEnemy.end());

        std::for_each(m_ActiveBulletPositions.begin(), m_ActiveBulletPositions.end(), VectorInsert);
        stateVector.insert(stateVector.end(), remainingEmptyBullet.begin(), remainingEmptyBullet.end());

        stateVector.insert(stateVector.end(), m_ActiveBulletDirections.begin(), m_ActiveBulletDirections.end());
        stateVector.insert(stateVector.end(), remainingEmptyBullet.begin(), remainingEmptyBullet.begin() + MAX_ACTIVE_BULLET_COUNT - m_ActiveBulletPositions.size());

        for(std::vector<TileType> tile_vec : m_Tiles){
            stateVector.insert(stateVector.end(), tile_vec.begin(), tile_vec.end());
        }

        torch::Tensor stateTensor = torch::from_blob(stateVector.data(), {(long)stateVector.size()}, 
                                                     torch::TensorOptions().dtype(torch::kFloat32)).to(torch::kFloat32).unsqueeze(0);
        return stateTensor.clone();
    }

    void printState(){
        spdlog::info("State Description");
        spdlog::info("PlayerPos : {} - {}", m_PlayerPos.x, m_PlayerPos.y);
        spdlog::info("Time to Teleport : {} - {}", m_TimeToTeleport);
        spdlog::info("m_AvailableProjectiles : {}", m_AvailableProjectiles);
        
        spdlog::info("ENEMY POSITIONS : ");
        for(Vector2<float> pos : m_EnemyPositions){
            spdlog::info("{} - {}", pos.x, pos.y);
        }

        spdlog::info("BULLET POSITIONS : ");
        for(Vector2<float> pos : m_ActiveBulletPositions){
            spdlog::info("{} - {}", pos.x, pos.y);
        }
    }

}GameState, *PGameState;

#endif
