#include "Sprite.h"

Sprite::Sprite(){

}

Sprite::Sprite(uint textureBufferID, Vector2<float> position){
    this->m_TextureBufferID = textureBufferID;
    this->m_Position        = position;
    this->m_RotationAngle       = 0.0f;
    //Set velocity to zero because it can get random value from random memory space
    this->m_Velocity        = Vector2<float>(0.0f, 0.0f);
}

void Sprite::setArea(std::unique_ptr<Area> area){
    this->m_Area = std::move(area);
}

void Sprite::setMaze(std::unique_ptr<Maze> maze){
    this->m_Maze = std::move(maze);
}

void Sprite::setOffset(float offset)
{
    this->m_Offset = offset;
}

void Sprite::setRotation(float rotation){
    this->m_RotationAngle = rotation;
}

float Sprite::getRotation(){
    return this->m_RotationAngle;
}

void Sprite::setTextureBufferID(uint textureBufferID)
{
    this->m_TextureBufferID = textureBufferID;
}

uint Sprite::getTextureBufferID(){
    return this->m_TextureBufferID;
}

void Sprite::setPosition(Vector2<float> newPosition){
    this->m_Position = newPosition;
}

Vector2<float> Sprite::getPosition(){
    return this->m_Position;
}

void Sprite::setVelocity(Vector2<float> newVelocity){
    this->m_Velocity = newVelocity;
}

Vector2<float> Sprite::getVelocity(){
    return this->m_Velocity;
}

void Sprite::render(){
    glBindTexture(GL_TEXTURE_2D, this->m_TextureBufferID);

    /*Rotation*/
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glTranslatef(0.5, 0.5, 0.0);
    glRotatef(this->m_RotationAngle, 0.0, 0.0, 1.0);
    glTranslatef(-0.5, -0.5, 0.0);
    glMatrixMode(GL_MODELVIEW);

    /*Translation*/
    glLoadIdentity();
    glTranslatef(this->m_Position.x, this->m_Position.y, 0);
    glDrawArrays(GL_QUADS, 0, 4);
}

void Sprite::update(GLFWwindow* window){
    this->m_Position += this->m_Velocity;
}

bool Sprite::checkFullCollision(Vector2<float> speed){
    if(speed.x > 0.0f && speed.y > 0.0f){
        return this->checkDirectionCollision(Right) ||
               this->checkDirectionCollision(Up);
    }
    else if(speed.x > 0.0f && speed.y < 0.0f){
        return this->checkDirectionCollision(Right) ||
               this->checkDirectionCollision(Down);
    }
    else if(speed.x < 0.0f && speed.y > 0.0f){
        return this->checkDirectionCollision(Left) ||
               this->checkDirectionCollision(Up);
    }
    else if(speed.x < 0.0f && speed.y < 0.0f){
        return this->checkDirectionCollision(Left) ||
               this->checkDirectionCollision(Down);
    }
    else if(speed.x == 0.0f && speed.y != 0.0f){
        bool res = (speed.y > 0) ? this->checkDirectionCollision(Up) : this->checkDirectionCollision(Down);
        return res;
    }
    else if(speed.y == 0.0f && speed.x != 0.0f){
        bool res = (speed.x > 0) ? this->checkDirectionCollision(Right) : this->checkDirectionCollision(Left);
        return res;
    }
    
    //Basic status idle
    return false;
}

bool Sprite::checkDirectionCollision(Direction dir){
    //Offset for the pass between two tiles

    if(dir == Left){
        Vector2<float> topLeft(this->getPosition().x - this->getVelocity().x + this->m_Offset,
                               this->getPosition().y + this->m_Offset);

        Vector2<float> bottomLeft(this->getPosition().x - this->getVelocity().x + this->m_Offset,
                                  this->getPosition().y + SQUARE_SIZE - this->m_Offset);

        TileType tile_type_top_left = this->m_Maze->pos2Tile(topLeft);
        TileType tile_type_bot_left = this->m_Maze->pos2Tile(bottomLeft);

        return tile_type_top_left == Wall || tile_type_bot_left == Wall;
    }
    else if(dir == Right){
        Vector2<float> topRight(this->getPosition().x + SQUARE_SIZE + this->getVelocity().x - this->m_Offset,
                                this->getPosition().y + this->m_Offset);

        Vector2<float> bottomRight(this->getPosition().x + SQUARE_SIZE + this->getVelocity().x - this->m_Offset, 
                                   this->getPosition().y + SQUARE_SIZE - this->m_Offset);
        TileType tile_type_top_right = this->m_Maze->pos2Tile(topRight);
        TileType tile_type_bot_right = this->m_Maze->pos2Tile(bottomRight);
        
        return tile_type_top_right == Wall || tile_type_bot_right == Wall;
    }
    else if(dir == Up){
        Vector2<float> topLeft(this->getPosition().x + this->m_Offset, 
                               this->getPosition().y + SQUARE_SIZE + this->getVelocity().y - this->m_Offset);
        Vector2<float> topRight(this->getPosition().x + SQUARE_SIZE - this->m_Offset, 
                                this->getPosition().y +  SQUARE_SIZE + this->getVelocity().y - this->m_Offset);

        TileType tile_type_top_left = this->m_Maze->pos2Tile(topLeft);
        TileType tile_type_top_right = this->m_Maze->pos2Tile(topRight);
        
        return tile_type_top_left == Wall || tile_type_top_right == Wall;
    }
    else if(dir == Down){
        Vector2<float> bottomLeft(this->getPosition().x + this->m_Offset,
                                  this->getPosition().y - this->getVelocity().y + this->m_Offset);
        Vector2<float> bottomRight(this->getPosition().x + SQUARE_SIZE - this->m_Offset, 
                                   this->getPosition().y - this->getVelocity().y + this->m_Offset);

        TileType tile_type_bot_left = this->m_Maze->pos2Tile(bottomLeft);
        TileType tile_type_bot_right = this->m_Maze->pos2Tile(bottomRight);

        return tile_type_bot_left == Wall || tile_type_bot_right == Wall;
    }

    return false;
}

std::unique_ptr<Sprite> Sprite::Copy(){
    std::unique_ptr<Sprite> copy = std::make_unique<Sprite>(this->m_TextureBufferID, this->m_Position);
    copy->m_Velocity  = this->m_Velocity;
    copy->m_RotationAngle = this->m_RotationAngle;
    return copy;
}
