#ifndef __PLAYER_SPRITE__
#define __PLAYER_SPRITE__

#include <GLFW/glfw3.h>
#include <iostream>
#include "Sprite.h"


class PlayerSprite : public Sprite{

private:
	int health;

public:
	uint bullet_count=10;

public:
	PlayerSprite();
	PlayerSprite(uint textureBufferID, Vector2<float> position);
	~PlayerSprite();
	void setHealth(int new_health);
	int getHealth();
	void setTextureBufferID(uint textureBufferID);
    uint TextureBufferID();
	void update(GLFWwindow* window);
	void setArea(std::unique_ptr<Area> area);
	std::unique_ptr<PlayerSprite> Copy();
	Vector2<float> getPosition();
};

#endif
