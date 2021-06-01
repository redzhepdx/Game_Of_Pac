#ifndef __GAME_WINDOW__
#define __GAME_WINDOW__

#include "prefix.h"
#include "config.h"

// Quadtree declarations
typedef struct QuadTreeNode {
    quadtree::QuadBox<float> box;
    std::size_t id{};
} QuadTreeNode, *PQuadTreeNode;

typedef struct EnemyConfig {
    uint32_t enemyCount = ENEMY_COUNT;
    int32_t enemySpawnDistance = MAX_ENEMY_SPAWN_DISTANCE;
    int32_t chaserCount = ENEMY_COUNT - 3;
    int32_t sniperCount = 3;
    int32_t suicideBomberCount = 0;
} EnemyConfig;

auto GetQuadBox = [](PQuadTreeNode node) {
    return node->box;
};

typedef quadtree::Quadtree<PQuadTreeNode, decltype(GetQuadBox)> t_quadTree;
typedef quadtree::QuadBox<float> f_quadBox;

class GameWindow {
private:
    // The Game is on?
    bool m_Status;
    bool m_MapRegeneration = true;

    // Window specs
    int32_t m_Width;
    int32_t m_Height;

    // Total number of wall cells
    int32_t m_MaxTileCount{};

    // Texture IDs
    uint32_t m_VertexBufferID;
    uint32_t m_TextureBufferID{};
    uint32_t m_TextureBulletID{};
    uint32_t m_TextureTileID{};
    uint32_t m_TextureEnemyID{};
    uint32_t m_TextureTpID{};

    // Total number of enemies
    uint32_t m_EnemyCount = ENEMY_COUNT;
    uint32_t m_EnemySpawnDistance = MAX_ENEMY_SPAWN_DISTANCE;

    ObservationType m_ObservationType = GAME_STATE_TYPE;
    boost::circular_buffer_space_optimized<BinaryImage> m_ImageBuffer;

    std::unique_ptr<EnemyConfig> m_EnemyConfig;

    // Game Control Settings
    float m_TilePercent = MAP_TILE_PERCENTAGE;

    // Timers
    int32_t m_SpawnTicks = 0;
    int32_t m_FireTicks = 0;

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

    static uint32_t loadAndBufferImage(const char *filename, int32_t width, int32_t height);

private:
    // Map Generation Functions
    void generate();

    void generateRandomMap();

    void generateExitPoints();

    void initPlayer();

private:
    // Spawn Functions
    void spawnEnemies();

    void handleSpawns(GLFWwindow *window);

private:
    // Collision Detections
    void handleCollisions();

    static bool checkObjectCollision(const std::unique_ptr<Area> &objectX, const std::unique_ptr<Area> &objectY);

    [[maybe_unused]] static bool checkObjectCollision(const std::shared_ptr<Area> &objectX, const std::shared_ptr<Area> &objectY);

    bool isPositionValid(Vector2<int> &position, int32_t min_row, int32_t max_row, int32_t min_col, int32_t max_col);

    [[maybe_unused]] bool checkStaticWallCollision(const Vector2<float> &position);

private:
    // Clean up
    void removeBullets(std::vector<unsigned int> &bulletsToDestroy, bool enemy);

    void removeEnemies(std::vector<unsigned int> &enemiesToDestroy);

private:
    // Quick Functionality Test Functions
    void test_path_finding(const Vector2<float> &target_pos);

private:
    // Environment Functions
    std::unique_ptr<GameState> getCurrentGameState(GLFWwindow *window);

    std::vector<unsigned char> getCurrentImageGameState(GLFWwindow *window);

    std::unique_ptr<GameState> getCurrentInternalGameState();

public:
    GameWindow(bool status, int32_t width, int32_t height);

    ~GameWindow();

    void firePlayerBullet();

    void fireEnemyBullet(const Vector2<float> &enemyPosition, const Vector2<float>& direction);

    void setGameStatus(bool status);

    [[nodiscard]] bool getGameStatus() const;

    void updateGameStatus(GLFWwindow *window);

    void render(GLFWwindow *window);

    void update(GLFWwindow *window);

    static void close(GLFWwindow *window);

    void play(GLFWwindow *window);

    void restart(GLFWwindow *window);
};

#endif
