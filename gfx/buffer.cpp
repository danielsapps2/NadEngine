#include "buffer.h"
#include <algorithm>

uint32_t RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return (uint32_t(r) << 24) |
           (uint32_t(g) << 16) |
           (uint32_t(b) << 8)  |
           uint32_t(a);
}

PixelBuffer::PixelBuffer(int width, int height)
    : w(width), h(height), pixels(size_t(width) * size_t(height), 0) {}

void PixelBuffer::clear(uint32_t color) {
    std::fill(pixels.begin(), pixels.end(), color);
}

void PixelBuffer::putPixel(int x, int y, uint32_t color) {
    if (x < 0 || y < 0 || x >= w || y >= h) return;
    pixels[size_t(y) * size_t(w) + size_t(x)] = color;
}

void PixelBuffer::fillRect(int x, int y, int rw, int rh, uint32_t color) {
    int x0 = std::max(0, x);
    int y0 = std::max(0, y);
    int x1 = std::min(w, x + rw);
    int y1 = std::min(h, y + rh);

    for (int yy = y0; yy < y1; ++yy) {
        uint32_t* row = &pixels[size_t(yy) * size_t(w)];
        for (int xx = x0; xx < x1; ++xx) {
            row[xx] = color;
        }
    }
}