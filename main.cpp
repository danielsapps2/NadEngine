#include <SDL.h>
#include <iostream>
#include "gfx/buffer.h"

int main(int argc, char* argv[]){
    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        std::cout << "SDL Failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    const int WIDTH = 160;
    const int HEIGHT = 144;
    const int SCALE = 5;

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

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"0");

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

    PixelBuffer buffer(WIDTH,HEIGHT);

    bool running = true;
    while (running){
        SDL_Event e;
        while (SDL_PollEvent(&e)){
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;

            buffer.clear(RGBA(20,20,30));

            buffer.fillRect(10,10,40,30, RGBA(255,60,60));
            buffer.fillRect(60,20,50,20, RGBA(60,255,120));
            buffer.fillRect(30,60,90,40, RGBA(60,120,255));

            const int pitch = WIDTH * int(sizeof(uint32_t));
            if (SDL_UpdateTexture(tex, nullptr, buffer.data(),pitch) != 0){
                SDL_Log("Update texture failed: %s", SDL_GetError());
            }

            SDL_RenderClear(renderer);
            SDL_Rect dst;
            dst.x = 0;
            dst.y = 0;
            dst.w = WIDTH * SCALE;
            dst.h = HEIGHT * SCALE;

            SDL_RenderCopy(renderer, tex, nullptr, &dst);
            SDL_RenderPresent(renderer);
        }
    }

    std::cout << "Hello from NadEngine!" << std::endl;
    std::cout << "Run successful, no errors." << std::endl;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}