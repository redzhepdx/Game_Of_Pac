#ifndef __PLAYER_SPRITE__
#define __PLAYER_SPRITE__

#define _LVL

#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>

#include "Sprite.h"
#include "AI/Agent.h"

enum PlayerControl{
	MANUAL, 
	AI_AGENT
};

class Player : public Sprite{

private:
	int m_Health;
	int m_PrevHealth = INITIAL_PLAYER_HEALTH;

	int m_Score = 0;
	int m_PrevScore = 0;

	bool m_FireOrder = false;

	std::vector<bool> m_Actions;

	uint m_TeleportTicks = 0;
	PlayerControl m_ControlType;
	std::unique_ptr<Agent> m_Agent;

	std::shared_ptr<GameState> m_PrevGameState{nullptr};
	uint32_t m_PrevAction = -1;
	float    m_PrevReward = -1;
	

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
	int getScore();

	uint getRemainingBulletCount();
	uint getTimeToTeleport();

	bool getFireStatus();
	void setFireStatus(bool status);

	Vector2<float> getPosition();

	std::unique_ptr<Player> Copy();

    uint TextureBufferID();
	
	void update(GLFWwindow* window, std::shared_ptr<GameState> currentState);

	// This is for ai but it will play a role in the future
	void updateAgent(std::shared_ptr<GameState> currentState);

	void manualControlPlayer(GLFWwindow* window);

	void executeActions();
};

#endif
