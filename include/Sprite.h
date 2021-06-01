#ifndef __SPRITE__
#define __SPRITE__

#include <iostream>
#include <GLFW/glfw3.h>

#include "Vector2.h"
#include "Area.h"
#include "Maze.h"
#include "config.h"
#include "point.h"

class Sprite
{
public:
    float m_RotationSpeed = 10.0;

private:
    uint32_t m_TextureBufferID;

protected:
    std::unique_ptr<Area> m_Area;
    std::unique_ptr<Maze> m_Maze;

    float m_RotationAngle;
    float m_Offset = 0.0f;
    Vector2<float> m_Velocity;
    Vector2<float> m_Position;

public:
    Sprite();
    Sprite(uint32_t textureBufferID, Vector2<float> position);

    void setArea(std::unique_ptr<Area> area);

    void setOffset(float offset);

    void setMaze(std::unique_ptr<Maze> maze);

    void setRotation(float rotation);
    float getRotation();

    void setTextureBufferID(uint32_t textureBufferID);
    uint32_t getTextureBufferID();

    void setPosition(Vector2<float> newPosition);
    Vector2<float> getPosition();

    void setVelocity(Vector2<float> newVelocity);
    Vector2<float> getVelocity();

    std::unique_ptr<Sprite> Copy();

    bool checkDirectionCollision(Direction dir);
    bool checkFullCollision(Vector2<float> speed);

    void render();
    virtual void update(GLFWwindow *window);
    virtual void update(Vector2<float> playerPos, std::vector<std::vector<std::shared_ptr<Point>>> grid) {}
};

#endif
