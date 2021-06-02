#ifndef __PROJECTILE__
#define __PROJECTILE__

#include <iostream>
#include <cmath>

#include <GLFW/glfw3.h>

#include "Sprite.h"

enum BulletType {
    PlayerBullet,
    EnemyBullet,
    AIBullet
};

class Projectile : public Sprite {
private:
    int32_t m_Health{};

    BulletType m_Type{PlayerBullet};

public:
    Projectile() = default;

    Projectile(uint32_t textureBufferID, const Vector2<float> &position);

    Projectile(uint32_t textureBufferID, const Vector2<float> &position, BulletType type);

    ~Projectile() = default;

public:
    uint32_t TextureBufferID();

    void update(GLFWwindow *window) override;

    Vector2<float> getPosition() override;

    [[nodiscard]] int32_t getHealth() const;

    BulletType getType();

    void setHealth(int32_t new_health);

    void setTextureBufferID(uint32_t textureBufferID) override;

    void setType(BulletType targetType);

    std::unique_ptr<Sprite> Copy() override;
};

#endif