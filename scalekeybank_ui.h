// === scalekeybank_ui.h ===
#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "scalekeybank.h"

class ScalekeyBankUI {
public:
    ScalekeyBankUI(ScaleBank& scaleBank);
    void Draw(SDL_Renderer* renderer, TTF_Font* font);
    void HandleEvents(SDL_Event& event);

private:
    ScaleBank& scaleBank;
    SDL_Rect scaleDropdown;
    SDL_Rect keyDropdown;
    bool scaleDropdownOpen;
    bool keyDropdownOpen;
};
