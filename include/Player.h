#ifndef __PLAYER_SPRITE__
#define __PLAYER_SPRITE__

#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>

#include "Sprite.h"

enum PlayerControl{
	MANUAL, 
	AI_AGENT
};

class Player : public Sprite{

private:
	int m_Health;
	int m_Score = 0;
	uint m_TeleportTicks = 0;
	PlayerControl m_ControlType;

public:
	uint m_BulletCount=10;

public:
	Player();
	Player(uint textureBufferID, Vector2<float> position, PlayerControl controlType);
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

	// This is for ai but it will play a role in the future
	void updateAgent(GLFWwindow* window);
};

#endif
