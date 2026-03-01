#pragma once
#include <cstdint>

class PixelBuffer;
static int randomInt(int start,int end);

class Entity {
    public:
        Entity(float x, float y, float w, float h, uint32_t color);
        void update(double dt, int screenWidth, int screenHeight);
        void render(PixelBuffer& buffer);
        void setStartingDirection();
    private:
        float x;
        float y;
        float w;
        float h;

        float vx;
        float vy;

        float dirX;
        float dirY;
        uint32_t color;
};