#include "Projectile.h"


Projectile::Projectile(uint32_t textureBufferID, const Vector2<float>& position) : Sprite(textureBufferID, position) {
    m_Type = PlayerBullet;
}

Projectile::Projectile(uint32_t textureBufferID, const Vector2<float>& position, BulletType type) : Sprite(textureBufferID,
                                                                                                    position) {
    m_Type = type;
}


void Projectile::setArea(std::unique_ptr<Area> area) {
    m_Area = std::move(area);
}

Vector2<float> Projectile::getPosition() {
    return m_Position;
}

void Projectile::setHealth(int32_t new_health) {
    m_Health = new_health;
}

int32_t Projectile::getHealth() const {
    return m_Health;
}

void Projectile::setTextureBufferID(uint32_t textureBufferID) {
    Sprite::setTextureBufferID(textureBufferID);
}

uint32_t Projectile::TextureBufferID() {
    return Sprite::getTextureBufferID();
}

BulletType Projectile::getType() {
    return m_Type;
}

void Projectile::setType(BulletType targetType) {
    m_Type = targetType;
}

std::unique_ptr<Sprite> Projectile::Copy() {
    std::unique_ptr<Projectile> copy = std::make_unique<Projectile>();
    copy->setTextureBufferID(Sprite::getTextureBufferID());

    copy->m_Position = m_Position;
    copy->m_Velocity = m_Velocity;
    copy->m_Type = m_Type;

    copy->setRotation(Sprite::getRotation());
    copy->setArea(m_Area->Copy());
    return copy;
}

void Projectile::update(GLFWwindow *window) {
    Sprite::update(window);
}