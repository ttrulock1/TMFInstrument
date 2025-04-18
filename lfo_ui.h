#ifndef LFO_UI_H
#define LFO_UI_H

#include <SDL2/SDL.h>

#include <atomic>

void ApplyLFOPitch(double& frequency);
void ApplyLFOAmplitude(float& drySample);


void DrawLFOEditor(SDL_Renderer* renderer, int startX, int startY, int width, int height);
void HandleLFOEvents(SDL_Event& event, int startX, int startY, int width, int height);
void ApplyLFOPitch(float& frequency);
// void ApplyLFOAmplitude(float& drySample);
// void ApplyLFOFilter(float& drySample);

extern std::atomic<int> lfoTargetRouting;  // 0 = Pitch, 1 = Amp, 2 = Shape
extern std::atomic<float> currentLFOValue;

#endif