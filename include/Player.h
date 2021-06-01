#ifndef __PLAYER_SPRITE__
#define __PLAYER_SPRITE__

#define _LVL

#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>
#include <algorithm>

#include "Sprite.h"
#include "AI/Agent.h"

enum PlayerControl
{
	MANUAL,
	AI_AGENT
};

class Player : public Sprite
{

private:
	int32_t m_Health;
	int32_t m_PrevHealth = INITIAL_PLAYER_HEALTH;

	int32_t m_Score = 0;
	int32_t m_PrevScore = 0;

	bool m_FireOrder = false;

	std::vector<bool> m_Actions;

	uint32_t m_TeleportTicks = 0;

	PlayerControl m_ControlType;
	ObservationType m_ObservationType;

	// AI Control Unit
	int32_t m_PrevAction = -1;
	float m_PrevReward = -1;
	std::unique_ptr<Agent> m_Agent;
	std::unique_ptr<GameState> m_PrevGameState{nullptr};

	// Statistics
	boost::circular_buffer_space_optimized<int> m_RecentScoreHistory;

public:
	uint32_t m_BulletCount = INITIAL_PLAYER_BULLETS;

public:
	Player();
	Player(uint32_t textureBufferID, Vector2<float> position, PlayerControl controlType);
	Player(uint32_t textureBufferID, Vector2<float> position, PlayerControl controlType, ObservationType observationType);
	virtual ~Player() = default;

public:
	void setHealth(int32_t new_health);
	void setArea(std::unique_ptr<Area> area);
	void setTextureBufferID(uint32_t textureBufferID);
	void setFireStatus(bool status);
	void setScore(int32_t score);
	void addScore(int32_t points);

	int32_t getHealth();
	int32_t getScore();

	uint32_t getRemainingBulletCount();
	uint32_t getTimeToTeleport();

	bool getFireStatus();

	void reset();

	Vector2<float> getPosition();

	std::unique_ptr<Player> Copy();

	uint32_t TextureBufferID();

	void update(GLFWwindow *window, std::unique_ptr<GameState> currentState);

	// This is for ai but it will play a role in the future
	void updateAgent(std::unique_ptr<GameState> currentState);

	void manualControlPlayer(GLFWwindow *window);

	void executeActions();

	void showStatistics();
};

#endif
