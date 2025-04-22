// === scalekeybank_ui.cpp ===
#include "scalekeybank_ui.h"
#include <iostream>

ScalekeyBankUI::ScalekeyBankUI(ScaleBank& sb)
    : scaleBank(sb),
      scaleDropdown{250, 60, 150, 30},
      keyDropdown{250, 100, 150, 30},
      scaleDropdownOpen(false),
      keyDropdownOpen(false) {}

void renderText(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y) {
    SDL_Color color = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dst = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &dst);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void ScalekeyBankUI::Draw(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
    SDL_RenderFillRect(renderer, &scaleDropdown);
    std::string scaleText = "Scale: " + scaleBank.getSelectedScale().name;
    renderText(renderer, font, scaleText.c_str(), scaleDropdown.x + 10, scaleDropdown.y + 5);

    if (scaleDropdownOpen) {
        for (int i = 0; i < scaleBank.getScaleCount(); ++i) {
            SDL_Rect itemRect = {scaleDropdown.x, scaleDropdown.y + (i + 1) * scaleDropdown.h, scaleDropdown.w, scaleDropdown.h};
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            SDL_RenderFillRect(renderer, &itemRect);
            std::string name = scaleBank.getScaleName(i);
            renderText(renderer, font, name.c_str(), itemRect.x + 10, itemRect.y + 5);
        }
    }

    SDL_RenderFillRect(renderer, &keyDropdown);
    std::string keyText = "Key: " + scaleBank.getSelectedKeyName();
    renderText(renderer, font, keyText.c_str(), keyDropdown.x + 10, keyDropdown.y + 5);

    if (keyDropdownOpen) {
        static const std::vector<std::string> keyNames = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
        for (int i = 0; i < 12; ++i) {
            SDL_Rect itemRect = {keyDropdown.x, keyDropdown.y + (i + 1) * keyDropdown.h, keyDropdown.w, keyDropdown.h};
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            SDL_RenderFillRect(renderer, &itemRect);
            renderText(renderer, font, keyNames[i].c_str(), itemRect.x + 10, itemRect.y + 5);
        }
    }
}

void ScalekeyBankUI::HandleEvents(SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        int mouseX = event.button.x;
        int mouseY = event.button.y;
        SDL_Point pt = {mouseX, mouseY};

        if (SDL_PointInRect(&pt, &scaleDropdown)) {
            scaleDropdownOpen = !scaleDropdownOpen;
            keyDropdownOpen = false;
            return;
        }
        if (SDL_PointInRect(&pt, &keyDropdown)) {
            keyDropdownOpen = !keyDropdownOpen;
            scaleDropdownOpen = false;
            return;
        }

        if (scaleDropdownOpen) {
            for (int i = 0; i < scaleBank.getScaleCount(); ++i) {
                SDL_Rect itemRect = {scaleDropdown.x, scaleDropdown.y + (i + 1) * scaleDropdown.h, scaleDropdown.w, scaleDropdown.h};
                if (SDL_PointInRect(&pt, &itemRect)) {
                    scaleBank.setSelectedScale(i);
                    scaleDropdownOpen = false;
                    return;
                }
            }
        }

        if (keyDropdownOpen) {
            for (int i = 0; i < 12; ++i) {
                SDL_Rect itemRect = {keyDropdown.x, keyDropdown.y + (i + 1) * keyDropdown.h, keyDropdown.w, keyDropdown.h};
                if (SDL_PointInRect(&pt, &itemRect)) {
                    scaleBank.setSelectedKey(i);
                    keyDropdownOpen = false;
                    return;
                }
            }
        }
    }
}
