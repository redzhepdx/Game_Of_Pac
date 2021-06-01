#ifndef __CONFIG_H_
#define __CONFIG_H_

#include <torch/torch.h>
#include <torch/script.h>
#include "spdlog/spdlog.h"

#define BOOST_LOG_DYN_LINK 1

enum Direction
{
    Left,
    Right,
    Up,
    Down
};
enum ObservationType
{
    Internal,
    Image
};

typedef struct VertexData
{
    GLfloat positionCoordinates[3];
    GLfloat textureCoordinates[2];
} VertexData, *PVertexData;

// Window settings
constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 800;

// Game update time settings
constexpr uint32_t DELAY_NUMBER = 10000000;
constexpr double UPDATES_PER_SECOND = 60.0f;
constexpr float DELTA_TIME_THRESHOLD = 1.0f;

constexpr int32_t ENEMY_SPAWN_TICKS = 60;
constexpr int32_t ENEMY_FIRE_TICKS = 60;
constexpr int32_t PLAYER_FIRE_TICKS = 30;
constexpr int32_t PLAYER_TP_TICKS = 300;
constexpr bool REGENERATE_MAP = false;

// Game logging settings
constexpr bool RUNNING = true;
constexpr bool STOPPED = false;
constexpr bool DEBUG_LOG = true;
constexpr bool INFO_LOG = false;

// Tile Settings
constexpr GLfloat MOVE_SIZE = 5.0f;
constexpr GLfloat SQUARE_SIZE = 20.0f;
constexpr float MAX_ENEMY_SPAWN_DISTANCE = 160.0f;
constexpr float MIN_ENEMY_SPAWN_DISTANCE = 160.0f;
constexpr float SPAWN_DIST_REDUCTION = 0.9f;
constexpr float MAP_TILE_PERCENTAGE = 0.55;

// Object Velocity and Size Settings
constexpr float PLAYER_SPEED = 3.0f;
constexpr float ENEMY_SPEED = 2.0f;
constexpr float BULLET_SPEED = 5.0f;
constexpr float OBJECT_OFFSET = 5.0f;

// Enemy Settings
constexpr int32_t ENEMY_COUNT = 10;
constexpr int32_t ENEMY_DESTROY_POINTS = 10;
constexpr uint32_t MAX_ENEMY_COUNT = 10;
constexpr float SNIPER_SIGHT_DISTANCE = 300.0f;
constexpr int32_t MAX_ACTIVE_BULLET_COUNT = 20;

// RL Agent Hyper-parameters
constexpr int16_t AGENT_UPDATE_RATE = 32;
constexpr int16_t TRAIN_EVERY = 32;
constexpr int16_t SAVE_EVERY = 1000;
constexpr int32_t SEED = 1337;
constexpr int32_t STATE_SIZE = 1886; //4 for Image
constexpr int32_t ACTION_SIZE = 8;
constexpr int32_t EXPLORATION_UPDATE = 10000;
constexpr int32_t BUFFER_SIZE = 10000;
constexpr int32_t BATCH_SIZE = 512;
constexpr int32_t SAMPLING_ITERATION = 2;

constexpr float LR_RATE = 0.0001;
constexpr float EPS = 1.0f;
constexpr float EPS_REDUCTION = 0.995f;
constexpr float MIN_EPS = 0.1f;
constexpr float TAU = 0.001f;
constexpr float GAMMA = 0.99f;

// Game Observation Settings
constexpr ObservationType GAME_STATE_TYPE = Internal;
constexpr uint32_t IMAGE_OBS_COUNT = 4;

// Player Settings
constexpr int32_t INITIAL_PLAYER_HEALTH = 100;
constexpr int32_t INITIAL_PLAYER_BULLETS = 10;
constexpr int32_t HEALTH_LOSS_AFTER_HIT = 10;

// Texture Settings
constexpr VertexData VERTICES[4] = {
    {{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    {{SQUARE_SIZE, 0.0f, 0.0f}, {1.0f, 1.0f}},
    {{SQUARE_SIZE, SQUARE_SIZE, 0.0f}, {1.0f, 0.0f}},
    {{0.0f, SQUARE_SIZE, 0.0f}, {0.0f, 0.0f}}};

enum TileType
{
    Empty,
    Wall,
    Obstacle,
    Teleport
};

typedef std::vector<std::vector<TileType>> tile_type_matrix;

typedef std::vector<u_char> BinaryImage;

#endif
