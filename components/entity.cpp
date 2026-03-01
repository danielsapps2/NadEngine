#include <random>
#include "entity.h"
#include "../gfx/buffer.h"

Entity::Entity(float px,float py, float pw, float ph, uint32_t c){
    x = px;
    y = py;
    w = pw;
    h = ph;

    dirX = 1;
    dirY = 1;
    color = c;

    setStartingDirection();
}

void Entity::setStartingDirection()
{
    dirX = randomInt(0, 1) ? 1 : -1;
    dirY = randomInt(0, 1) ? 1 : -1;
}

void Entity::update(double dt, int screenWidth, int screenHeight){

    if (x + w >= screenWidth || x <= 0){
        dirX = (dirX >= 0) ? -1 : 1;
    }
    if (y + h >= screenHeight || y <= 0){
        dirY = (dirY >= 0) ? -1 : 1;
    }

    x += 1 * dirX;
    y += 1 * dirY;
}

void Entity::render(PixelBuffer& buffer){
    buffer.fillRect((int)x,(int)y,(int)w, (int) h, color);
}

static int randomInt(int start, int end)
{
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(start, end);
    return dist(gen);
}