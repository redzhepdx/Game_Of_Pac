#include "Player.h"

Player::Player(){
	m_ControlType = MANUAL;
}

Player::Player(uint textureBufferID, Vector2<float> position, PlayerControl controlType):
							Sprite(textureBufferID, position){
	this->m_ControlType = controlType;
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

void Player::addScore(int points){
	this->m_Score += points;
}

int Player::getScore(){
	return this->m_Score;
}

void Player::setTextureBufferID(uint textureBufferID)
{
   	Sprite::setTextureBufferID(textureBufferID);
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

void Player::update(GLFWwindow* window){

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

		if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
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

		if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
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

		if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
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

		if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
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
		if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS){
			this->m_RotationAngle = (float)((int)(this->m_RotationAngle - this->m_Velocity.x) % 360);
		}

		if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){
			this->m_RotationAngle = (float)((int)(this->m_RotationAngle + this->m_Velocity.x) % 360);
		}

		//Reload
		if(glfwGetKey(window, GLFW_KEY_R) == GLFW_TRUE && this->m_BulletCount == 0){
			std::cout << "RELOAD!" << std::endl;
			this->m_BulletCount = 10;
		}

		// TODO : Teleport
	}

	if (m_TeleportTicks > 0){
		--m_TeleportTicks;
	}
}

void Player::updateAgent(GLFWwindow* window){
	
}
