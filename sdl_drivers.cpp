#include <SDL2/SDL.h>
#include <iostream>

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    int numDrivers = SDL_GetNumVideoDrivers();
    std::cout << "Available SDL Video Drivers:\n";
    for (int i = 0; i < numDrivers; ++i) {
        std::cout << "  " << SDL_GetVideoDriver(i) << std::endl;
    }

    SDL_Quit();
    return 0;
}

