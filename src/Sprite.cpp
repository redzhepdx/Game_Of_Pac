#include "Sprite.h"

Sprite::Sprite() {
}

Sprite::Sprite(uint32_t textureBufferID, Vector2<float> position) {
    m_TextureBufferID = textureBufferID;
    m_Position = position;
    m_RotationAngle = 0.0f;
    //Set velocity to zero because it can get random value from random memory space
    m_Velocity = Vector2<float>(0.0f, 0.0f);
}

void Sprite::setArea(std::unique_ptr<Area> area) {
    m_Area = std::move(area);
}

void Sprite::setMaze(std::unique_ptr<Maze> maze) {
    m_Maze = std::move(maze);
}

void Sprite::setOffset(float offset) {
    m_Offset = offset;
}

void Sprite::setRotation(float rotation) {
    m_RotationAngle = rotation;
}

float Sprite::getRotation() {
    return m_RotationAngle;
}

void Sprite::setTextureBufferID(uint32_t textureBufferID) {
    m_TextureBufferID = textureBufferID;
}

uint32_t Sprite::getTextureBufferID() {
    return m_TextureBufferID;
}

void Sprite::setPosition(Vector2<float> newPosition) {
    m_Position = newPosition;
}

Vector2<float> Sprite::getPosition() {
    return m_Position;
}

void Sprite::setVelocity(Vector2<float> newVelocity) {
    m_Velocity = newVelocity;
}

Vector2<float> Sprite::getVelocity() {
    return m_Velocity;
}

void Sprite::render() {
    glBindTexture(GL_TEXTURE_2D, m_TextureBufferID);

    /*Rotation*/
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glTranslatef(0.5, 0.5, 0.0);
    glRotatef(m_RotationAngle, 0.0, 0.0, 1.0);
    glTranslatef(-0.5, -0.5, 0.0);
    glMatrixMode(GL_MODELVIEW);

    /*Translation*/
    glLoadIdentity();
    glTranslatef(m_Position.x, m_Position.y, 0);
    glDrawArrays(GL_QUADS, 0, 4);
}

void Sprite::update(GLFWwindow *window) {
    m_Position += m_Velocity;
}

bool Sprite::checkFullCollision(Vector2<float> speed) {
    if (speed.x > 0.0f && speed.y > 0.0f) {
        return checkDirectionCollision(Right) ||
               checkDirectionCollision(Up);
    } else if (speed.x > 0.0f && speed.y < 0.0f) {
        return checkDirectionCollision(Right) ||
               checkDirectionCollision(Down);
    } else if (speed.x < 0.0f && speed.y > 0.0f) {
        return checkDirectionCollision(Left) ||
               checkDirectionCollision(Up);
    } else if (speed.x < 0.0f && speed.y < 0.0f) {
        return checkDirectionCollision(Left) ||
               checkDirectionCollision(Down);
    } else if (speed.x == 0.0f && speed.y != 0.0f) {
        bool res = (speed.y > 0) ? checkDirectionCollision(Up) : checkDirectionCollision(Down);
        return res;
    } else if (speed.y == 0.0f && speed.x != 0.0f) {
        bool res = (speed.x > 0) ? checkDirectionCollision(Right) : checkDirectionCollision(Left);
        return res;
    }

    //Basic status idle
    return false;
}

bool Sprite::checkDirectionCollision(Direction dir) {
    //Offset for the pass between two tiles

    if (dir == Left) {
        Vector2<float> topLeft(getPosition().x - getVelocity().x + m_Offset,
                               getPosition().y + m_Offset);

        Vector2<float> bottomLeft(getPosition().x - getVelocity().x + m_Offset,
                                  getPosition().y + SQUARE_SIZE - m_Offset);

        TileType tile_type_top_left = m_Maze->pos2Tile(topLeft);
        TileType tile_type_bot_left = m_Maze->pos2Tile(bottomLeft);

        return tile_type_top_left == Wall || tile_type_bot_left == Wall;
    } else if (dir == Right) {
        Vector2<float> topRight(getPosition().x + SQUARE_SIZE + getVelocity().x - m_Offset,
                                getPosition().y + m_Offset);

        Vector2<float> bottomRight(getPosition().x + SQUARE_SIZE + getVelocity().x - m_Offset,
                                   getPosition().y + SQUARE_SIZE - m_Offset);
        TileType tile_type_top_right = m_Maze->pos2Tile(topRight);
        TileType tile_type_bot_right = m_Maze->pos2Tile(bottomRight);

        return tile_type_top_right == Wall || tile_type_bot_right == Wall;
    } else if (dir == Up) {
        Vector2<float> topLeft(getPosition().x + m_Offset,
                               getPosition().y + SQUARE_SIZE + getVelocity().y - m_Offset);
        Vector2<float> topRight(getPosition().x + SQUARE_SIZE - m_Offset,
                                getPosition().y + SQUARE_SIZE + getVelocity().y - m_Offset);

        TileType tile_type_top_left = m_Maze->pos2Tile(topLeft);
        TileType tile_type_top_right = m_Maze->pos2Tile(topRight);

        return tile_type_top_left == Wall || tile_type_top_right == Wall;
    } else if (dir == Down) {
        Vector2<float> bottomLeft(getPosition().x + m_Offset,
                                  getPosition().y - getVelocity().y + m_Offset);
        Vector2<float> bottomRight(getPosition().x + SQUARE_SIZE - m_Offset,
                                   getPosition().y - getVelocity().y + m_Offset);

        TileType tile_type_bot_left = m_Maze->pos2Tile(bottomLeft);
        TileType tile_type_bot_right = m_Maze->pos2Tile(bottomRight);

        return tile_type_bot_left == Wall || tile_type_bot_right == Wall;
    }

    return false;
}

std::unique_ptr<Sprite> Sprite::Copy() {
    std::unique_ptr<Sprite> copy = std::make_unique<Sprite>(m_TextureBufferID, m_Position);
    copy->m_Velocity = m_Velocity;
    copy->m_RotationAngle = m_RotationAngle;
    return copy;
}
