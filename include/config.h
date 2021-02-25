#ifndef __CONFIG_H_
#define __CONFIG_H_

enum Direction { Left, Right, Up, Down};

typedef struct{
    GLfloat positionCoordinates[3];
    GLfloat textureCoordinates[2];
}VertexData, *PVertexData;

// Window settings
constexpr unsigned int WIDTH  = 800;
constexpr unsigned int HEIGHT = 800;

// Game update time settings
constexpr unsigned int DELAY_NUMBER  = 10000000;
constexpr double UPDATES_PER_SECOND  = 60.0f;
constexpr float DELTA_TIME_THRESHOLD = 1.0f;

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
constexpr GLfloat MOVE_SIZE              = 5.0f;
constexpr GLfloat SQUARE_SIZE            = 20.0f;
constexpr float MAX_ENEMY_SPAWN_DISTANCE = 160.0f;
constexpr float MIN_ENEMY_SPAWN_DISTANCE = 160.0f;
constexpr float SPAWN_DIST_REDUCTION     = 0.9f;

// Object Velocity and Size Settings
constexpr float PLAYER_SPEED  = 3.0f;
constexpr float ENEMY_SPEED   = 2.0f;
constexpr float BULLET_SPEED  = 5.0f;
constexpr float OBJECT_OFFSET = 5.0f;

// Enemy Settings
constexpr int ENEMY_COUNT             = 20;
constexpr int ENEMY_DESTROY_POINTS    = 10;
constexpr uint MAX_ENEMY_COUNT        = 25;
constexpr float SNIPER_SIGHT_DISTANCE = 300.0f;

// Texture Settings
constexpr VertexData VERTICES[4] = {
        {{0.0f,        0.0f,        0.0f}, {0.0f, 1.0f}},
        {{SQUARE_SIZE, 0.0f,        0.0f}, {1.0f, 1.0f}},
        {{SQUARE_SIZE, SQUARE_SIZE, 0.0f}, {1.0f, 0.0f}},
        {{0.0f,        SQUARE_SIZE, 0.0f}, {0.0f, 0.0f}}
    };


#endif
