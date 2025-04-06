#ifndef EFFECTS_H
#define EFFECTS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <atomic>

struct EffectParams {
    const char* label;
    std::atomic<float>* param;
    float min;
    float max;
    SDL_Color color;
};

struct EffectPanel {
    SDL_Rect bounds;
    const char* name;
    std::vector<EffectParams> parameters;
    std::atomic<bool>* enabled;
    SDL_Color bgColor;
};

extern std::vector<EffectPanel> effectPanels;

void DrawEffectsUI(SDL_Renderer* renderer, TTF_Font* font);
void HandleEffectUIEvents(const SDL_Event& event);

#endif // EFFECTS_H
