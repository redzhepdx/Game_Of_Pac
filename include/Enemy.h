#ifndef __ENEMY__
#define __ENEMY__

#include <iostream>
#include <cmath>

#include <GLFW/glfw3.h>

#include "spdlog/spdlog.h"

#include "Sprite.h"
#include "utils.h"

enum EnemyType {
    Chaser,
    Sniper,
    SuicideBomber,
    AI
};

class Enemy : public Sprite {
private:
    int32_t m_Health{};

    EnemyType m_Type{Chaser};

    bool m_Stuck{};
    bool m_OnAction{};
    uint32_t m_ActionTicks{};

    Vector2<int32_t> m_PrevPlayerTilePos;
    Vector2<float> m_PrevTilePos;
    Vector2<float> m_NextTilePos;

    std::vector<Vector2<int32_t>> m_PrevRoute;

    void chaserUpdate(const Vector2<float> &playerPos, const shared_point_matrix &grid);

    void sniperUpdate(const Vector2<float> &playerPos, const shared_point_matrix &grid);

    void suicideBomberUpdate(const Vector2<float> &playerPos, const shared_point_matrix &grid);

    void aiUpdate(const Vector2<float> &playerPos, const shared_point_matrix &grid);

    void move();

public:
    Enemy();

    Enemy(uint32_t textureBufferID, const Vector2<float> &position);

    Enemy(uint32_t textureBufferID, const Vector2<float> &position, EnemyType type);

    ~Enemy();

    uint32_t TextureBufferID();

    void update(const Vector2<float> &playerPos, const shared_point_matrix &grid) override;

    EnemyType getType();

    Vector2<float> getPosition() override;

    [[nodiscard]] int32_t getHealth() const;

    [[nodiscard]] bool onAction() const;

    void setHealth(int32_t new_health);

    void setTextureBufferID(uint32_t textureBufferID) override;

    void setArea(std::unique_ptr<Area> area) override;

    void setType(EnemyType targetType);

    void resetAction();

    void stopAction();

    std::unique_ptr<Sprite> Copy() override;
};

#endif