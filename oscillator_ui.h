// oscillator_ui.h
#pragma once
#include <SDL2/SDL.h>

void DrawOscillatorUI(SDL_Renderer* renderer, bool advancedMode);
void HandleOscillatorUIEvents(SDL_Event& event, bool& advancedMode);

void RefreshOscillatorSliders();
