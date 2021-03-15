#include "Player.h"

Player::Player(){
	m_ControlType = MANUAL;

	m_Actions.resize(ACTION_SIZE);
}

Player::Player(uint textureBufferID, Vector2<float> position, PlayerControl controlType):
							Sprite(textureBufferID, position){
	this->m_ControlType = controlType;

	m_Actions.resize(ACTION_SIZE);

	if (this->m_ControlType == AI_AGENT){
		if(torch::cuda::is_available()){
			m_Agent = std::make_unique<Agent>(STATE_SIZE, ACTION_SIZE, torch::kCUDA);
		}else{
			m_Agent = std::make_unique<Agent>(STATE_SIZE, ACTION_SIZE, torch::kCPU);
		}
	}
}

Player::~Player(){
}

void Player::setArea(std::unique_ptr<Area> area){
	this->m_Area = std::move(area);
}

Vector2<float> Player::getPosition(){
	return this->m_Position;
}

void Player::setHealth(int new_health){
	this->m_Health = new_health;
}

int Player::getHealth(){
	return this->m_Health;
}

uint Player::getRemainingBulletCount(){
	return this->m_BulletCount;
}

uint Player::getTimeToTeleport(){
	return this->m_TeleportTicks;
}

void Player::addScore(int points){
	this->m_Score += points;
}

void Player::setScore(int points){
	this->m_Score = points;
}

int Player::getScore(){
	return this->m_Score;
}

bool Player::getFireStatus(){
	return m_FireOrder;
}

void Player::setFireStatus(bool status){
	m_FireOrder = status;
}

void Player::setTextureBufferID(uint textureBufferID)
{
   	Sprite::setTextureBufferID(textureBufferID);
}

void Player::reset(){
	spdlog::info("Player Reset");
	m_Health        = INITIAL_PLAYER_HEALTH;
	m_PrevHealth    = INITIAL_PLAYER_HEALTH;
	m_Score         = 0;
	m_PrevScore     = 0;
	m_PrevGameState = nullptr;
	m_PrevAction    = -1;
	m_PrevReward    = -1;
	m_BulletCount   = 10;
}

uint Player::TextureBufferID(){
    return Sprite::getTextureBufferID();
}

std::unique_ptr<Player> Player::Copy(){
    std::unique_ptr<Player> copy = std::make_unique<Player>();
	copy->setTextureBufferID(Sprite::getTextureBufferID());

	copy->m_Position  = this->m_Position;
    copy->m_Velocity  = this->m_Velocity;
	
	copy->setRotation(Sprite::getRotation());
	copy->setArea(this->m_Area->Copy());
    
	return copy;
}

void Player::update(GLFWwindow* window, std::unique_ptr<GameState> currentState){
	std::fill(m_Actions.begin(), m_Actions.end(), false);

	if(this->m_Health <= 0){
		// glfwSetWindowShouldClose(window, GLFW_TRUE);
		return;
	}
	if(m_ControlType == MANUAL){
		if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
			// Shaking algorithm for fun
			Vector2<float> newPos = (this->m_Position + m_Velocity);
			newPos.x = this->m_Position.x + std::sin(newPos.x) * this->getVelocity().x;
			newPos.y = this->m_Position.y + std::sin(newPos.y) * this->getVelocity().y;
			this->m_Position = newPos;
		}

		manualControlPlayer(window);
	}else{
		updateAgent(std::move(currentState));
	}

	executeActions();

	if (m_TeleportTicks > 0){
		--m_TeleportTicks;
	}
}

void Player::executeActions(){
	if(m_Actions[0]){
		//Go Right
		Vector2<float> direction(this->m_Velocity.x, 0.0f);
		if(!this->checkDirectionCollision(Right)){
			if(this->m_Position.x < this->m_Area->m_Box->right - SQUARE_SIZE / 1.0f){
				this->m_Position += direction;
			}
			else if(this->m_Maze->pos2Tile(Vector2<float>(this->m_Area->m_Box->left, this->m_Position.y)) == Teleport && m_TeleportTicks == 0){
				this->m_Position.x = this->m_Area->m_Box->left + (SQUARE_SIZE * 0.75);
				this->m_Position.y = (int)(this->m_Position.y / SQUARE_SIZE) * SQUARE_SIZE;

				m_TeleportTicks = PLAYER_TP_TICKS;
			}
		}
	}

	if(m_Actions[1]){
		//Go Up
		Vector2<float> direction(0.0f, this->m_Velocity.y);
		if(!this->checkDirectionCollision(Up)){
			if(this->m_Position.y < this->m_Area->m_Box->top - SQUARE_SIZE / 1.0f){
				this->m_Position += direction;
			}
			else if(this->m_Maze->pos2Tile(Vector2<float>(this->m_Position.x, this->m_Area->m_Box->bottom)) == Teleport && m_TeleportTicks == 0){
				this->m_Position.y = this->m_Area->m_Box->bottom + (SQUARE_SIZE * 0.75);
				this->m_Position.x = (int)(this->m_Position.x / SQUARE_SIZE) * SQUARE_SIZE;

				m_TeleportTicks = PLAYER_TP_TICKS;
			}
		}
	}

	if(m_Actions[2]){
		//Go Down
		Vector2<float> direction(0.0f, this->m_Velocity.y);
		if(!this->checkDirectionCollision(Down)){
			if(this->m_Position.y > this->m_Area->m_Box->bottom + SQUARE_SIZE / 1.0f){
				this->m_Position -= direction;
			}
			else if(this->m_Maze->pos2Tile(Vector2<float>(this->m_Position.x, this->m_Area->m_Box->top)) == Teleport && m_TeleportTicks == 0){
				this->m_Position.y = this->m_Area->m_Box->top - (SQUARE_SIZE * 0.75);
				this->m_Position.x = (int)(this->m_Position.x / SQUARE_SIZE) * SQUARE_SIZE;

				m_TeleportTicks = PLAYER_TP_TICKS;
			}
		}
	}

	if(m_Actions[3]){
		//Go Left
		Vector2<float> direction(this->m_Velocity.x, 0.0f);
		if(!this->checkDirectionCollision(Left)){
			if(this->m_Position.x > this->m_Area->m_Box->left + SQUARE_SIZE / 1.0f){
				this->m_Position -= direction;
			}
			else if(this->m_Maze->pos2Tile(Vector2<float>(this->m_Area->m_Box->right, this->m_Position.y)) == Teleport &&  m_TeleportTicks == 0){
				this->m_Position.x = this->m_Area->m_Box->right - (SQUARE_SIZE * 0.75);
				this->m_Position.y = (int)(this->m_Position.y / SQUARE_SIZE) * SQUARE_SIZE;

				m_TeleportTicks = PLAYER_TP_TICKS;
			}
		}
	}

	//Rotataion
	if(m_Actions[4]){
		this->m_RotationAngle = (float)((int)(this->m_RotationAngle - this->m_Velocity.x) % 360);
	}

	if(m_Actions[5]){
		this->m_RotationAngle = (float)((int)(this->m_RotationAngle + this->m_Velocity.x) % 360);
	}

	//Reload
	if(m_Actions[6]){
		this->m_BulletCount = 10;
	}

	// Shoot
	if(m_Actions[7]){
		setFireStatus(true);
	}
}

void Player::manualControlPlayer(GLFWwindow* window){
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
		m_Actions[0] = true;
	}

	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
		m_Actions[1] = true;
	}
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
		m_Actions[2] = true;
	}

	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
		m_Actions[3] = true;
	}

	//Rotataion
	if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS){
		m_Actions[4] = true;
	}

	if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){
		m_Actions[5] = true;
	}

	//Reload
	if(glfwGetKey(window, GLFW_KEY_R) == GLFW_TRUE){
		m_Actions[6] = true;
	}

	if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS){
		m_Actions[7] = true;
	}
}

void Player::updateAgent(std::unique_ptr<GameState> currentState){
	uint32_t action = m_Agent->act(std::move(currentState->copy()));

	// if there is no score gain, punish little. If player gets a hit increase the punishment
	float    reward = (m_Score - m_PrevScore - ENEMY_DESTROY_POINTS / 5.0f) + (m_Health - m_PrevHealth) / 5.0f;
	bool     done   = (m_Health <= 0);

	// There is no observation
	if(m_PrevAction != -1){
		m_Agent->step(std::move(m_PrevGameState->copy()), std::move(currentState->copy()), m_PrevAction, m_PrevReward, done);
	}

	m_PrevGameState = std::move(currentState);
	m_PrevAction    = action;
	m_PrevReward    = reward;
	m_PrevHealth    = m_Health;
	m_PrevScore     = m_Score;


	if(m_Agent->totalStepCount() % 100 == 0){
		spdlog::info("Current Player Score : {}", m_PrevScore);
	}

	// Execute a single Action
	m_Actions[action] = true;
}
