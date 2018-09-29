#ifndef __GAME_WINDOW__
#define __GAME_WINDOW__

#include "prefix.h"
//#include "utils.h"

typedef struct{
    GLfloat positionCoordinates[3];
    GLfloat textureCoordinates[2];
}VertexData, *PVertexData;

class GameWindow{
public:
    bool status;

    int width;
    int height;
    int maxTileCount;

    uint vertexBufferID;
    uint textureBufferID;
    uint textureBulletID;
    uint textureTileID;
    uint textureEnemyID;
    uint textureTpID;
    uint enemyCount = 20;

    float x            = 0.0f;
    float y            = 0.0f;
    float rot_angle    = 10.0f;
    float tile_percent = 0.70f;

    std::vector<std::unique_ptr<Sprite>> renderInstances;
    std::vector<std::unique_ptr<Sprite>> enemyInstances;
    std::vector<std::unique_ptr<Sprite>> tiles;

    std::unique_ptr<PlayerSprite> rocket;
    std::unique_ptr<Area> game_area;
    std::unique_ptr<Maze> maze;

    std::vector<std::vector<std::shared_ptr<Point>>> grid;

public:
    VertexData vertices[4] = {
        {{0.0f,        0.0f,        0.0f}, {0.0f, 1.0f}},
        {{Square_Size, 0.0f,        0.0f}, {1.0f, 1.0f}},
        {{Square_Size, Square_Size, 0.0f}, {1.0f, 0.0f}},
        {{0.0f,        Square_Size, 0.0f}, {0.0f, 0.0f}}
    };

private:
    uint loadAndBufferImage(const char *filename, int width, int height);
    void spawnEnemies();
    void setupGL();
    bool checkCollision(std::unique_ptr<Area> obj_1, std::unique_ptr<Area> obj_2);
    bool isPositionValid(Vector2<int> &position, int min_row, int max_row, int min_col, int max_col);
    Vector2<float> generateMap();

public:
    GameWindow(bool status, int width, int height);
    ~GameWindow();

    void fireBullet();

    void setGameStatus(bool status);
    bool getGameStatus();
    void updateGameStatus(GLFWwindow* window);

    void render(GLFWwindow* window);
    void update(GLFWwindow* window);
    void close(GLFWwindow* window);
};

#endif
