#include "Projectile.h"

Projectile::Projectile(){

}

Projectile::Projectile(uint textureBufferID, Vector2<float> position):
							Sprite(textureBufferID, position){
	type = PlayerBullet;
}

Projectile::Projectile(uint textureBufferID, Vector2<float> position, BulletType type):
							Sprite(textureBufferID, position){
	this->type = type;
}

Projectile::~Projectile(){
}

void Projectile::setArea(std::unique_ptr<Area> area){
	this->area = std::move(area);
}

Vector2<float> Projectile::getPosition(){
	return this->position;
}

void Projectile::setHealth(int new_health){
	this->health = new_health;
}

int Projectile::getHealth(){
	return this->health;
}

void Projectile::setTextureBufferID(uint textureBufferID)
{
   	Sprite::setTextureBufferID(textureBufferID);
}

uint Projectile::TextureBufferID(){
    return Sprite::getTextureBufferID();
}

BulletType Projectile::getType(){
	return this->type;
}

void Projectile::setType(BulletType targetType){
	this->type = targetType;
}

std::unique_ptr<Projectile> Projectile::Copy(){
    std::unique_ptr<Projectile> copy = std::make_unique<Projectile>();
	copy->setTextureBufferID(Sprite::getTextureBufferID());
	
	copy->position  = this->position;
    copy->velocity  = this->velocity;
    copy->type      = this->type;

	copy->setRotation(Sprite::getRotation());
	copy->setArea(this->area->Copy());
    return copy;
}

void Projectile::update(GLFWwindow* window){
    Sprite::update(window);
}