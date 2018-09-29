#include "PlayerSprite.h"

PlayerSprite::PlayerSprite(){

}

PlayerSprite::PlayerSprite(uint textureBufferID, Vector2<float> position):
							Sprite(textureBufferID, position){
}

PlayerSprite::~PlayerSprite(){
}

void PlayerSprite::setArea(std::unique_ptr<Area> area){
	this->area = std::move(area);
}

Vector2<float> PlayerSprite::getPosition(){
	return this->position;
}

void PlayerSprite::setHealth(int new_health){
	this->health = new_health;
}

int PlayerSprite::getHealth(){
	return this->health;
}

void PlayerSprite::setTextureBufferID(uint textureBufferID)
{
   	Sprite::setTextureBufferID(textureBufferID);
}

uint PlayerSprite::TextureBufferID(){
    return Sprite::getTextureBufferID();
}

std::unique_ptr<PlayerSprite> PlayerSprite::Copy(){
    std::unique_ptr<PlayerSprite> copy = std::make_unique<PlayerSprite>();
	copy->setTextureBufferID(Sprite::getTextureBufferID());
	copy->position  = this->position;
    copy->velocity  = this->velocity;
	copy->setRotation(Sprite::getRotation());
	copy->setArea(this->area->Copy());
    return copy;
}

void PlayerSprite::update(GLFWwindow* window){

	if(this->health <= 0){
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
			Vector2<float> newPos = (this->position + velocity);
			newPos.x = this->position.x + std::sin(newPos.x) * this->getVelocity().x;
			newPos.y = this->position.y + std::sin(newPos.y) * this->getVelocity().y;
			this->position = newPos;
	}

	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
		//Go Right
		Vector2<float> direction(this->velocity.x, 0.0f);
		if(!this->checkDirectionCollision(Right)){
			if(this->position.x < this->area->Box->right - Square_Size / 1.0f){
				this->position += direction;
			}
			else if(this->maze->pos2Tile(Vector2<float>(this->area->Box->left, this->position.y)) == Teleport){
				this->position.x = this->area->Box->left + 5.0f;
			}
		}
	}

	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
		//Go Up
		Vector2<float> direction(0.0f, this->velocity.y);
		if(!this->checkDirectionCollision(Up)){
			if(this->position.y < this->area->Box->top - Square_Size / 1.0f){
				this->position += direction;
			}
			else if(this->maze->pos2Tile(Vector2<float>(this->position.x, this->area->Box->bottom)) == Teleport){
				this->position.y = this->area->Box->bottom + 5.0f;
			}
		}
	}

	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
		//Go Down
		Vector2<float> direction(0.0f, this->velocity.y);
		if(!this->checkDirectionCollision(Down)){
			if(this->position.y > this->area->Box->bottom + Square_Size / 1.0f){
				this->position -= direction;
			}
			else if(this->maze->pos2Tile(Vector2<float>(this->position.x, this->area->Box->top)) == Empty){

				this->position.y = this->area->Box->top - 5.0f;
			}
		}
	}

	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
		//Go Left
		Vector2<float> direction(this->velocity.x, 0.0f);
		if(!this->checkDirectionCollision(Left)){
			if(this->position.x > this->area->Box->left + Square_Size / 1.0f){
				this->position -= direction;
			}
			else if(this->maze->pos2Tile(Vector2<float>(this->area->Box->right, this->position.y)) == Empty){
				this->position.x = this->area->Box->right - 5.0f;
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

}
