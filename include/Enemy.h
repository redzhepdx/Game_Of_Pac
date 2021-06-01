#ifndef __ENEMY__
#define __ENEMY__

#include <iostream>
#include <cmath>

#include <GLFW/glfw3.h>

#include "spdlog/spdlog.h"

#include "Sprite.h"
#include "utils.h"

enum EnemyType
{
    Chaser,
    Sniper,
    SuicideBomber,
    AI
};

class Enemy : public Sprite
{
private:
    int16_t m_Health;

    EnemyType m_Type;

    bool m_Stuck;
    bool m_OnAction;
    uint32_t m_ActionTicks;

    Vector2<int32_t> m_PrevPlayerTilePos;
    Vector2<float> m_PrevTilePos;
    Vector2<float> m_NextTilePos;

    std::vector<Vector2<int32_t>> m_PrevRoute;

    void chaserUpdate(Vector2<float> playerPos, shared_point_matrix &grid);
    void sniperUpdate(Vector2<float> playerPos, shared_point_matrix &grid);
    void suicideBomberUpdate(Vector2<float> playerPos, shared_point_matrix &grid);
    void aiUpdate(Vector2<float> playerPos, shared_point_matrix &grid);

    void move();

public:
    Enemy();
    Enemy(uint32_t textureBufferID, Vector2<float> position);
    Enemy(uint32_t textureBufferID, Vector2<float> position, EnemyType type);
    ~Enemy();

    uint32_t TextureBufferID();
    void update(Vector2<float> playerPos, shared_point_matrix &grid);

    EnemyType getType();
    Vector2<float> getPosition();
    int16_t getHealth();
    bool onAction();

    void setHealth(int32_t new_health);
    void setTextureBufferID(uint32_t textureBufferID);
    void setArea(std::unique_ptr<Area> area);
    void setType(EnemyType targetType);
    void resetAction();
    void stopAction();

    std::unique_ptr<Enemy> Copy();
};

#endif