#ifndef __CONFIG_H_
#define __CONFIG_H_

#include <torch/torch.h>

#define BOOST_LOG_DYN_LINK 1

enum Direction { Left, Right, Up, Down};

typedef struct{
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
constexpr int16_t AGENT_UPDATE_RATE = 2;
constexpr int32_t SEED              = 1337;
constexpr int32_t STATE_SIZE        = 1765;
constexpr int32_t ACTION_SIZE       = 7;
constexpr int32_t EXPLORATION_RATE  = 100;
constexpr int32_t BUFFER_SIZE       = 100;
constexpr int32_t BATCH_SIZE        = 10;

constexpr float   LR_RATE           = 0.001;
constexpr float   EPS               = 0.3f;
constexpr float   EPS_REDUCTION     = 0.3f;
constexpr float   TAU               = 0.1f;
constexpr float   GAMMA             = 0.1f;


// Player Settings
constexpr int INITIAL_PLAYER_HEALTH = 100;
constexpr int HEALTH_LOSS_AFTER_HIT = 10;

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

    torch::Tensor toTensor(){
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

        stateVector.insert(stateVector.end(), m_ActiveBulletDirections.begin(), m_ActiveBulletDirections.end());
        stateVector.insert(stateVector.end(), remainingEmptyBullet.begin(), remainingEmptyBullet.begin() + MAX_ACTIVE_BULLET_COUNT - m_ActiveBulletPositions.size());

        for(std::vector<TileType> tile_vec : m_Tiles){
            stateVector.insert(stateVector.end(), tile_vec.begin(), tile_vec.end());
        }

        torch::Tensor stateTensor = torch::from_blob(stateVector.data(), {(long)stateVector.size()}, 
                                                     torch::TensorOptions().dtype(torch::kFloat32)).to(torch::kFloat32).unsqueeze(0);

        return stateTensor;
    }

}GameState, *PGameState;

#endif
