#ifndef ADSR_UI_H
#define ADSR_UI_H

#include <SDL2/SDL.h>

extern SDL_Rect adsrBounds;
extern SDL_Point adsrPoints[4];
extern bool draggingASDR[4];
extern bool showASDRMode;

void DrawADSREditor(SDL_Renderer* renderer);
void HandleADSREvents(SDL_Event& event);

#endif
