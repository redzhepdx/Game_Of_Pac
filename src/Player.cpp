#include "Player.h"

Player::Player(){

}

Player::Player(uint textureBufferID, Vector2<float> position):
							Sprite(textureBufferID, position){
}

Player::~Player(){
}

void Player::setArea(std::unique_ptr<Area> area){
	this->area = std::move(area);
}

Vector2<float> Player::getPosition(){
	return this->position;
}

void Player::setHealth(int new_health){
	this->health = new_health;
}

int Player::getHealth(){
	return this->health;
}

void Player::addScore(int points){
	this->score += points;
}

int Player::getScore(){
	return this->score;
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

	copy->position  = this->position;
    copy->velocity  = this->velocity;
	
	copy->setRotation(Sprite::getRotation());
	copy->setArea(this->area->Copy());
    
	return copy;
}

void Player::update(GLFWwindow* window){

	if(this->health <= 0){
		// glfwSetWindowShouldClose(window, GLFW_TRUE);
		return;
	}

	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
		// Shaking algorithm for fun
		Vector2<float> newPos = (this->position + velocity);
		newPos.x = this->position.x + std::sin(newPos.x) * this->getVelocity().x;
		newPos.y = this->position.y + std::sin(newPos.y) * this->getVelocity().y;
		this->position = newPos;
	}

	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
		//Go Right
		Vector2<float> direction(this->velocity.x, 0.0f);
		if(!this->checkDirectionCollision(Right)){
			if(this->position.x < this->area->Box->right - SQUARE_SIZE / 1.0f){
				this->position += direction;
			}
			else if(this->maze->pos2Tile(Vector2<float>(this->area->Box->left, this->position.y)) == Teleport){
				this->position.x = this->area->Box->left + (SQUARE_SIZE * 0.75);
				this->position.y = (int)(this->position.y / SQUARE_SIZE) * SQUARE_SIZE;
			}
		}
	}

	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
		//Go Up
		Vector2<float> direction(0.0f, this->velocity.y);
		if(!this->checkDirectionCollision(Up)){
			if(this->position.y < this->area->Box->top - SQUARE_SIZE / 1.0f){
				this->position += direction;
			}
			else if(this->maze->pos2Tile(Vector2<float>(this->position.x, this->area->Box->bottom)) == Teleport){
				this->position.y = this->area->Box->bottom + (SQUARE_SIZE * 0.75);
				this->position.x = (int)(this->position.x / SQUARE_SIZE) * SQUARE_SIZE;
			}
		}
	}

	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
		//Go Down
		Vector2<float> direction(0.0f, this->velocity.y);
		if(!this->checkDirectionCollision(Down)){
			if(this->position.y > this->area->Box->bottom + SQUARE_SIZE / 1.0f){
				this->position -= direction;
			}
			else if(this->maze->pos2Tile(Vector2<float>(this->position.x, this->area->Box->top)) == Teleport){

				this->position.y = this->area->Box->top - (SQUARE_SIZE * 0.75);
				this->position.x = (int)(this->position.x / SQUARE_SIZE) * SQUARE_SIZE;
			}
		}
	}

	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
		//Go Left
		Vector2<float> direction(this->velocity.x, 0.0f);
		if(!this->checkDirectionCollision(Left)){
			if(this->position.x > this->area->Box->left + SQUARE_SIZE / 1.0f){
				this->position -= direction;
			}
			else if(this->maze->pos2Tile(Vector2<float>(this->area->Box->right, this->position.y)) == Teleport){
				this->position.x = this->area->Box->right - (SQUARE_SIZE * 0.75);
				this->position.y = (int)(this->position.y / SQUARE_SIZE) * SQUARE_SIZE;
			}
		}
	}

	//Rotataion
	if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS){
		this->rot_angle = (float)((int)(this->rot_angle - this->velocity.x) % 360);
	}

	if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){
		this->rot_angle = (float)((int)(this->rot_angle + this->velocity.x) % 360);
	}

	//Reload
	if(glfwGetKey(window, GLFW_KEY_R) == GLFW_TRUE && this->bullet_count == 0){
			std::cout << "RELOAD!" << std::endl;
			this->bullet_count = 10;
	}

	// TODO : Teleport
}
