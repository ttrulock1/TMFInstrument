// pad_screen.h
#ifndef PAD_SCREEN_H
#define PAD_SCREEN_H

#include <SDL2/SDL.h>

void HandlePadEvents(SDL_Event& event);
void DrawPads(SDL_Renderer* renderer);

#endif
