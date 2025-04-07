#include "adsr_ui.h"

#include <algorithm> // for std::clamp

extern SDL_Rect toggleBtn;

SDL_Rect adsrBounds = {30, 100, 800 / 2 - 60, 150};
SDL_Point adsrPoints[4] = {
    {30, 100}, {150, 100}, {200, 100}, {350, 350}
};
bool draggingASDR[4] = {false};
bool showASDRMode = false;

float envAmount = 1.0f; // NEW: Envelope amount slider value
SDL_Rect amountSlider = {adsrBounds.x, adsrBounds.y + adsrBounds.h + 20, adsrBounds.w, 10};

void DrawADSREditor(SDL_Renderer* renderer) {
    // 🌌 Draw retro radar-style grid background
    SDL_SetRenderDrawColor(renderer, 0, 60, 0, 255); // Dark green
    for (int x = adsrBounds.x; x < adsrBounds.x + adsrBounds.w; x += 20) {
        SDL_RenderDrawLine(renderer, x, adsrBounds.y, x, adsrBounds.y + adsrBounds.h);
    }
    for (int y = adsrBounds.y; y < adsrBounds.y + adsrBounds.h; y += 20) {
        SDL_RenderDrawLine(renderer, adsrBounds.x, y, adsrBounds.x + adsrBounds.w, y);
    }

    // ⚡ Draw ADSR envelope
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Bright green lines
    SDL_RenderDrawLine(renderer, adsrPoints[0].x, adsrPoints[0].y, adsrPoints[1].x, adsrPoints[1].y);
    SDL_RenderDrawLine(renderer, adsrPoints[1].x, adsrPoints[1].y, adsrPoints[2].x, adsrPoints[2].y);
    SDL_RenderDrawLine(renderer, adsrPoints[2].x, adsrPoints[2].y, adsrPoints[3].x, adsrPoints[3].y);

    // 🔘 Draw knobs
    for (int i = 0; i < 4; ++i) {
        SDL_Rect knob = {adsrPoints[i].x - 5, adsrPoints[i].y - 5, 10, 10};
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Neon green
        SDL_RenderFillRect(renderer, &knob);
    }

    // 🎚 Draw envelope amount slider
    SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255);
    SDL_RenderFillRect(renderer, &amountSlider);
    int knobX = amountSlider.x + (int)(envAmount * amountSlider.w);
    SDL_Rect sliderKnob = {knobX - 5, amountSlider.y - 3, 10, amountSlider.h + 6};
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &sliderKnob);
}

void HandleADSREvents(SDL_Event& event) {
    int mouseX = event.button.x;
    int mouseY = event.button.y;

    if (event.type == SDL_MOUSEBUTTONDOWN) {
        for (int i = 0; i < 4; ++i) {
            SDL_Rect knob = {adsrPoints[i].x - 5, adsrPoints[i].y - 5, 10, 10};
            SDL_Point pt = {mouseX, mouseY};
            if (SDL_PointInRect(&pt, &knob)) {
                draggingASDR[i] = true;
            }
        }

        SDL_Point pt = {mouseX, mouseY};
        if (SDL_PointInRect(&pt, &toggleBtn)) {
            showASDRMode = false; // Switch back to main screen
            return;
        }
        if (SDL_PointInRect(&pt, &amountSlider)) {
            envAmount = (float)(mouseX - amountSlider.x) / amountSlider.w;
            envAmount = std::clamp(envAmount, 0.0f, 1.0f);
        }
    }

    if (event.type == SDL_MOUSEBUTTONUP) {
        for (int i = 0; i < 4; ++i)
            draggingASDR[i] = false;
    }

    if (event.type == SDL_MOUSEMOTION) {
        for (int i = 0; i < 4; ++i) {
            if (draggingASDR[i]) {
                int newX = std::clamp(event.motion.x, adsrBounds.x, adsrBounds.x + adsrBounds.w);
                int newY = std::clamp(event.motion.y, adsrBounds.y, adsrBounds.y + adsrBounds.h);

                switch (i) {
                    case 0: adsrPoints[0].x = adsrBounds.x; adsrPoints[0].y = newY; break;
                    case 1:
                        adsrPoints[1].x = std::clamp(newX, adsrPoints[0].x + 10, adsrPoints[2].x - 10);
                        adsrPoints[1].y = newY;
                        break;
                    case 2:
                        adsrPoints[2].x = std::clamp(newX, adsrPoints[1].x + 10, adsrPoints[3].x - 10);
                        adsrPoints[2].y = newY;
                        break;
                    case 3:
                        adsrPoints[3].x = std::clamp(newX, adsrPoints[2].x + 10, adsrBounds.x + adsrBounds.w);
                        adsrPoints[3].y = newY;
                        break;
                }
            }
        }
    }
}
