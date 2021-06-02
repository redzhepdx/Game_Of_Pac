#ifndef __SPRITE__
#define __SPRITE__

#include <iostream>
#include <GLFW/glfw3.h>

#include "Vector2.h"
#include "Area.h"
#include "Maze.h"
#include "config.h"
#include "point.h"
#include "GameState.h"

class Sprite {
public:
    float m_RotationSpeed = 10.0;

private:
    uint32_t m_TextureBufferID{};

protected:
    float m_RotationAngle{};
    float m_Offset = 0.0f;
    Vector2<float> m_Velocity;
    Vector2<float> m_Position;

public:
    Sprite() = default;

    Sprite(uint32_t textureBufferID, const Vector2<float>& position);

    void setOffset(float offset);

    void setRotation(float rotation);

    [[nodiscard]] float getRotation() const;

    virtual void setTextureBufferID(uint32_t textureBufferID);

    [[nodiscard]] uint32_t getTextureBufferID() const;

    void setPosition(const Vector2<float> &newPosition);

    virtual Vector2<float> getPosition();

    void setVelocity(const Vector2<float> &newVelocity);

    Vector2<float> getVelocity();

    virtual std::unique_ptr<Sprite> Copy();

    bool checkDirectionCollision(Direction dir, const std::unique_ptr<Maze> &gameMaze);

    bool checkFullCollision(const Vector2<float> &speed, const std::unique_ptr<Maze> &gameMaze);

    void render() const;

    virtual void update(GLFWwindow *window);

    virtual void update(const Vector2<float> &playerPos,
                        const std::vector<std::vector<std::shared_ptr<Point>>> &grid) {}


    virtual void update(const Vector2<float> &playerPos,
                        const std::vector<std::vector<std::shared_ptr<Point>>> &grid,
                        const std::unique_ptr<Maze> &gameMaze) {}

    virtual void update(GLFWwindow *window,
                        std::unique_ptr<GameState> currentState,
                        const std::unique_ptr<Area> &gameArea,
                        const std::unique_ptr<Maze> &gameMaze) {}

};

#endif
