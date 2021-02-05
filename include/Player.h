#ifndef __PLAYER_SPRITE__
#define __PLAYER_SPRITE__

#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>

#include "Sprite.h"


class Player : public Sprite{

private:
	int health;
	int score = 0;

public:
	uint bullet_count=10;

public:
	Player();
	Player(uint textureBufferID, Vector2<float> position);
	~Player();

public:
	void setHealth(int new_health);
	void setArea(std::unique_ptr<Area> area);
	void setTextureBufferID(uint textureBufferID);
	void addScore(int points);


	int getHealth();
	Vector2<float> getPosition();
	int getScore();

	std::unique_ptr<Player> Copy();

    uint TextureBufferID();
	
	void update(GLFWwindow* window);
};

#endif
