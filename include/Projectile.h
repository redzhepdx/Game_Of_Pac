#ifndef __PROJECTILE__
#define __PROJECTILE__

#include <iostream>
#include <cmath>

#include <GLFW/glfw3.h>

#include "Sprite.h"

enum BulletType {PlayerBullet, EnemyBullet, AIBullet};

class Projectile : public Sprite{
    private:
	    int m_Health;
        
        BulletType m_Type;

    public:
        Projectile();
        Projectile(uint textureBufferID, Vector2<float> position);
        Projectile(uint textureBufferID, Vector2<float> position, BulletType type);
        ~Projectile();

    public:
        uint TextureBufferID();
        void update(GLFWwindow* window);

        Vector2<float> getPosition();
        int getHealth();
        BulletType getType();

        void setHealth(int new_health);
        void setTextureBufferID(uint textureBufferID);
        void setArea(std::unique_ptr<Area> area);
        void setType(BulletType targetType);

        std::unique_ptr<Projectile> Copy();
};


#endif