#pragma once
#include <cstdint>
#include <vector>

uint32_t RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

class PixelBuffer {
public:
    PixelBuffer(int width, int height);

    void clear(uint32_t color);
    void putPixel(int x, int y, uint32_t color);
    void fillRect(int x, int y, int rw, int rh, uint32_t color);

    int width() const { return w; }
    int height() const { return h; }

    const uint32_t* data() const { return pixels.data(); }
    uint32_t* data() { return pixels.data(); }

private:
    int w;
    int h;
    std::vector<uint32_t> pixels;
};