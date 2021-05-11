#ifndef __GAME_WINDOW__
#define __GAME_WINDOW__

#include "prefix.h"
#include "config.h"

// Quadtree declarations
typedef struct
{
    quadtree::QuadBox<float> box;
    std::size_t id;
}QuadTreeNode, *PQuadTreeNode;

typedef struct EnemyConfig{
    uint32_t enemyCount      = ENEMY_COUNT;
    int enemySpawnDistance   = MAX_ENEMY_SPAWN_DISTANCE;
    int chaserCount          = ENEMY_COUNT - 3;
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
    bool m_Status;

    // Window specs
    int m_Width;
    int m_Height;

    // Total number of wall cells       
    int m_MaxTileCount;

    // Texture IDs
    uint m_VertexBufferID;
    uint m_TextureBufferID;
    uint m_TextureBulletID;
    uint m_TextureTileID;
    uint m_TextureEnemyID;
    uint m_TextureTpID;

    // Total number of enemies
    uint m_EnemyCount = ENEMY_COUNT;
    uint m_EnemySpawnDistance = MAX_ENEMY_SPAWN_DISTANCE;

    ObservationType m_ObservationType = GAME_STATE_TYPE;
    boost::circular_buffer_space_optimized<BinaryImage> m_ImageBuffer;

    std::unique_ptr<EnemyConfig> m_EnemyConfig;

    // Game Control Settings
    float m_TilePercent = MAP_TILE_PERCENTAGE;

    // Timers
    int m_SpawnTicks     = 0;
    int m_FireTicks      = 0;

    // Instances to render
    std::vector<std::unique_ptr<Projectile>> m_BulletInstances;
    
    std::vector<std::unique_ptr<Enemy>> m_EnemyInstances;
    std::vector<std::unique_ptr<Projectile>> m_EnemyBulletInstances;

    std::vector<std::unique_ptr<Sprite>> m_Tiles;
    
    // Grid locations of empty tiles
    std::vector<Vector2<int>> m_EmptyTiles;

    std::unique_ptr<Player> m_Player; // Player
    std::unique_ptr<Area> m_GameArena;
    std::unique_ptr<Maze> m_Maze;
    
    shared_point_matrix m_Grid; // Special grid for pathfinding
    
    std::unique_ptr<t_quadTree> m_QuadTree; // Quad tree for collision detection

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
    bool checkObjectCollision(std::unique_ptr<Area>& objectX, std::unique_ptr<Area>& objectY);
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

private:
    // Environment Functions
    std::unique_ptr<GameState> getCurrentGameState(GLFWwindow* window);
    std::vector<unsigned char> getCurrentImageGameState(GLFWwindow* window);
    std::unique_ptr<GameState> getCurrentInternalGameState();

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
