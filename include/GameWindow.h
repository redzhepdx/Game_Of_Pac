#ifndef __GAME_WINDOW__
#define __GAME_WINDOW__

#include "prefix.h"
#include "config.h"
//#include "utils.h"

typedef struct{
    uint enemyCount=20;
    float tilePercent;
}GameSettings, *PGameSettings;


// Quadtree declarations
typedef struct
{
    quadtree::QuadBox<float> box;
    std::size_t id;
}QuadTreeNode, *PQuadTreeNode;

typedef struct{
    int enemyCount           = ENEMY_COUNT;
    int enemySpawnDistance   = MAX_ENEMY_SPAWN_DISTANCE;
    int chaserCount          = 3;
    int sniperCount          = 3;
    int suicideBomberCount   = 0;
}EnemyConfig;

auto GetQuadBox = [](PQuadTreeNode node){
    return node->box;
};

typedef quadtree::Quadtree<PQuadTreeNode, decltype(GetQuadBox)> t_quadTree;
typedef quadtree::QuadBox<float> f_quadBox;

class GameWindow{
private:

    // The Game is on?
    bool status;

    // Window specs
    int width;
    int height;

    // Total number of wall cells       
    int maxTileCount;

    // Texture IDs
    uint vertexBufferID;
    uint textureBufferID;
    uint textureBulletID;
    uint textureTileID;
    uint textureEnemyID;
    uint textureTpID;

    // Total number of enemies
    uint enemyCount = ENEMY_COUNT;
    uint enemySpawnDistance = MAX_ENEMY_SPAWN_DISTANCE;

    std::unique_ptr<EnemyConfig> m_EnemyConfig;

    // Game Control Settings
    float x            = 0.0f;
    float y            = 0.0f;
    float tile_percent = 0.55f;

    // Timers
    int spawnTicks     = 0;
    int fireTicks      = 0;

    // Instances to render
    std::vector<std::unique_ptr<Projectile>> bulletInstances;
    
    std::vector<std::unique_ptr<Enemy>> enemyInstances;
    std::vector<std::unique_ptr<Projectile>> enemyBulletInstances;

    std::vector<std::unique_ptr<Sprite>> tiles;
    
    // Grid locations of empty tiles
    std::vector<Vector2<int>> emptyTiles;

    std::unique_ptr<Player> rocket; // Player
    std::unique_ptr<Area> game_area;
    std::unique_ptr<Maze> maze;
    
    shared_point_matrix grid; // Special grid for pathfinding
    
    std::unique_ptr<t_quadTree> quadTree; // Quad tree for collision detection

private:
    // Setup Functions
    void setupGL();
    void setupImageBuffers();
    void setupArena();
    uint loadAndBufferImage(const char *filename, int width, int height);

private:
    // Map Generation Functions
    void generate();
    void generateRandomMap();
    void generateExitPoints();
    void initPlayer();

private :
    // Spawn Functions
    void spawnEnemies();
    void handleSpawns(GLFWwindow* window);

private:
    // Collision Detections
    void handleCollisions();
    bool checkObjectCollision(std::unique_ptr<Area> objectX, std::unique_ptr<Area> objectY);
    bool checkObjectCollision(std::shared_ptr<Area> objectX, std::shared_ptr<Area> objectY);
    bool isPositionValid(Vector2<int> &position, int min_row, int max_row, int min_col, int max_col);
    bool checkStaticWallCollision(Vector2<float> position);

private:
    // Clean up
    void removeBullets(std::vector<unsigned int>& bulletsToDestroy, bool enemy);
    void removeEnemies(std::vector<unsigned int>& enemiesToDestroy);

private:
    // Quick Functionality Test Functions
    void test_path_finding(Vector2<float> target_pos);

public:
    GameWindow(bool status, int width, int height);
    ~GameWindow();

    void firePlayerBullet();
    void fireEnemyBullet(Vector2<float> enemyPosition, Vector2<float> direction);

    void setGameStatus(bool status);
    bool getGameStatus();
    void updateGameStatus(GLFWwindow* window);

    void render(GLFWwindow* window);
    void update(GLFWwindow* window);
    void close(GLFWwindow* window);

    void play(GLFWwindow* window);
    void restart(GLFWwindow* window);
};

#endif
