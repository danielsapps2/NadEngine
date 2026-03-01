#include <SDL.h>
#include <iostream>
#include <algorithm>
#include "gfx/buffer.h"
#include "components/entity.h"

const int WIDTH = 160;
const int HEIGHT = 144;
const int SCALE = 5;

Entity player(60,20,10,10,RGBA(70,255,120));

void update(double dt){
    //updates for movement & physics locked to 60fps
   player.update(dt, WIDTH,HEIGHT);
}
void render(PixelBuffer& buffer){
    buffer.clear(RGBA(0,0,0));

    player.render(buffer);
}

int main( int argv, char* argc[]){

    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        std::cout << "SDL Failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    //Create window
    SDL_Window* window = SDL_CreateWindow(
        "NadEngine",
        SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
        WIDTH * SCALE, HEIGHT * SCALE,
        SDL_WINDOW_SHOWN
    );
    if (!window){
        SDL_Log("Created window failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    //Create SDL Renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer){
        SDL_Log("Create renderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    //Set nearest neighbor
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"0");

    //SDL Texture to write buffer to
    SDL_Texture* tex = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH,HEIGHT
    );
    if(!tex){
        SDL_Log("Create texture failed: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    //Buffer from gfx/buffer.cpp
    PixelBuffer buffer(WIDTH,HEIGHT);

    //gameloop start
    const double dt = 1.0 / 60.0;
    const double maxFrameTime = 0.25;

    uint64_t freq = SDL_GetPerformanceFrequency();
    uint64_t last = SDL_GetPerformanceCounter();

    //fps count
    uint64_t fpsLastTime = SDL_GetPerformanceCounter();
    int frameCount = 0;
    double fps = 0.0;

    double accumulator = 0.0;
    bool running = true;
    while (running){
        SDL_Event e;
        while (SDL_PollEvent(&e)){
            //allow us out of the gameloop with escape key & pressing X
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        //Timing
        uint64_t now = SDL_GetPerformanceCounter();
        double frameTime = double(now - last) / double(freq);
        last = now;

        //Prevent huge catch-up steps (alt-tab or breakpoints)
        frameTime = std::min(frameTime, maxFrameTime);
        accumulator += frameTime;

        //Fixed updates
        while (accumulator >= dt){
            update(dt);
            accumulator -= dt;
        }

        //Interpolation for smooth rendering
        double alpha = accumulator / dt;

        (void)alpha;
        render(buffer);

        //push the buffer's pixels to the texture
        const int pitch = WIDTH * int(sizeof(uint32_t));
        if (SDL_UpdateTexture(tex, nullptr, buffer.data(),pitch) != 0){
            SDL_Log("Update texture failed: %s", SDL_GetError());
        }

        //clear the renderer & define a rect for the frame.
        SDL_RenderClear(renderer);
        SDL_Rect dst;
        dst.x = 0;
        dst.y = 0;
        dst.w = WIDTH * SCALE;
        dst.h = HEIGHT * SCALE;

        //Push the texture to the renderer using the dst frame rect coords
        SDL_RenderCopy(renderer, tex, nullptr, &dst);
        //show the renderer
        SDL_RenderPresent(renderer);

        //update fps counter
        frameCount++;
        uint64_t fpsnow = SDL_GetPerformanceCounter();
        double elapsed = double(fpsnow - fpsLastTime) /SDL_GetPerformanceFrequency();
        if (elapsed >= 1.0){
            fps = frameCount / elapsed;
            frameCount = 0;
            fpsLastTime = fpsnow;
            char title[64];
            snprintf(title, sizeof(title), "FPS: %.2f", fps);
            SDL_SetWindowTitle(window,title);
            std::cout << "FPS: " << fps << std::endl;
        }
    }

    //Cleanup, destroy renderer, window and quit SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}