#include "Projectile.h"

Projectile::Projectile(){

}

Projectile::Projectile(uint textureBufferID, Vector2<float> position):
							Sprite(textureBufferID, position){
	m_Type = PlayerBullet;
}

Projectile::Projectile(uint textureBufferID, Vector2<float> position, BulletType type):
							Sprite(textureBufferID, position){
	this->m_Type = type;
}

Projectile::~Projectile(){
}

void Projectile::setArea(std::unique_ptr<Area> area){
	this->m_Area = std::move(area);
}

Vector2<float> Projectile::getPosition(){
	return this->m_Position;
}

void Projectile::setHealth(int new_health){
	this->m_Health = new_health;
}

int Projectile::getHealth(){
	return this->m_Health;
}

void Projectile::setTextureBufferID(uint textureBufferID)
{
   	Sprite::setTextureBufferID(textureBufferID);
}

uint Projectile::TextureBufferID(){
    return Sprite::getTextureBufferID();
}

BulletType Projectile::getType(){
	return this->m_Type;
}

void Projectile::setType(BulletType targetType){
	this->m_Type = targetType;
}

std::unique_ptr<Projectile> Projectile::Copy(){
    std::unique_ptr<Projectile> copy = std::make_unique<Projectile>();
	copy->setTextureBufferID(Sprite::getTextureBufferID());
	
	copy->m_Position  = this->m_Position;
    copy->m_Velocity  = this->m_Velocity;
    copy->m_Type      = this->m_Type;

	copy->setRotation(Sprite::getRotation());
	copy->setArea(this->m_Area->Copy());
    return copy;
}

void Projectile::update(GLFWwindow* window){
    Sprite::update(window);
}