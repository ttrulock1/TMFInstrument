#include "adsr_ui.h"
#include "shared_buffer.h"
#include <algorithm> // for std::clamp
#include "lfo_ui.h"

extern SDL_Rect toggleBtn;

SDL_Rect adsrBounds = {30, 100, 800 / 2 - 60, 150};
SDL_Point adsrPoints[5] = {
    {adsrBounds.x, adsrBounds.y + 50},                                  // Start
    {adsrBounds.x + 80, adsrBounds.y + 100},                            // Attack peak
    {adsrBounds.x + 160, adsrBounds.y + 90},                            // End of Decay
    {adsrBounds.x + 240, adsrBounds.y + 90},                            // End of Sustain (same Y as [2])
    {adsrBounds.x + adsrBounds.w, adsrBounds.y + adsrBounds.h}         // End of Release (bottom-right)
};
bool draggingASDR[4] = {false};
bool showASDRMode = true; // Start with ADSR graph visible

float envAmount = 1.0f; // NEW: Envelope amount slider value
SDL_Rect amountSlider = {adsrBounds.x, adsrBounds.y + adsrBounds.h + 20, adsrBounds.w, 10};

void DrawADSREditor(SDL_Renderer* renderer) {
    // 💙 Draw pseudo-text using retro-style block labels
    // 💙 Draw ADSR segment labels and envelope value label
    SDL_Color labelColor = {0, 255, 0, 255};
        // 💙 Fake labels for A D S R using short green bars under each point
    for (int i = 0; i < 4; ++i) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect bar = {adsrPoints[i].x - 2, adsrPoints[i].y + 12, 4, 10};
        SDL_RenderFillRect(renderer, &bar);
    }

    // 💙 Fake "Envelope Amount" label using dashed line above the slider
    for (int x = amountSlider.x; x < amountSlider.x + amountSlider.w; x += 8) {
        SDL_RenderDrawLine(renderer, x, amountSlider.y + 15, x + 4, amountSlider.y + 15);
    }
    
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
    SDL_RenderDrawLine(renderer, adsrPoints[2].x, adsrPoints[2].y, adsrPoints[3].x, adsrPoints[3].y); // Sustain
    SDL_RenderDrawLine(renderer, adsrPoints[3].x, adsrPoints[3].y, adsrPoints[4].x, adsrPoints[4].y); // Release

    

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

        // 🌀 Draw LFO editor on right side of ADSR page
    int lfoX = adsrBounds.x + adsrBounds.w + 30;
    int lfoY = adsrBounds.y;
    int lfoWidth = 300;
    int lfoHeight = adsrBounds.h;
    DrawLFOEditor(renderer, lfoX, lfoY, lfoWidth, lfoHeight);
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
                    case 0:
                        adsrPoints[0].x = adsrBounds.x;
                        adsrPoints[0].y = newY;
                        break;
                    case 1:
                        adsrPoints[1].x = std::clamp(newX, adsrPoints[0].x + 10, adsrPoints[2].x - 10);
                        adsrPoints[1].y = newY;
                        break;
                    case 2:
                        adsrPoints[2].x = std::clamp(newX, adsrPoints[1].x + 10, adsrBounds.x + adsrBounds.w - 10);
                        adsrPoints[2].y = newY;
                        break;
                    case 3:
                        adsrPoints[3].x = std::clamp(newX, adsrPoints[2].x + 10, adsrBounds.x + adsrBounds.w - 10);
                        adsrPoints[3].y = adsrPoints[2].y;  // ✅ lock Y to match sustain level
                        break;

                }
            }
        }
                // 🔄 Pass event to LFO module
        int lfoX = adsrBounds.x + adsrBounds.w + 30;
        int lfoY = adsrBounds.y;
        int lfoWidth = 300;
        int lfoHeight = adsrBounds.h;
        HandleLFOEvents(event, lfoX, lfoY, lfoWidth, lfoHeight);
    }

    adsrPoints[3].y = adsrPoints[2].y;

        UpdateADSRParamsFromUI();  // 👈 this is key
        // 👇 Add this after updating ADSR
    adsrPoints[4].x = adsrBounds.x + adsrBounds.w;
    adsrPoints[4].y = adsrBounds.y + adsrBounds.h;
}

void UpdateADSRParamsFromUI() {
    float totalWidth = (float)(adsrBounds.w);
    float totalHeight = (float)(adsrBounds.h);

    float attackTime = (adsrPoints[1].x - adsrPoints[0].x) / totalWidth;
    float decayTime  = (adsrPoints[2].x - adsrPoints[1].x) / totalWidth;
    float sustainLevel = 1.0f - (adsrPoints[2].y - adsrBounds.y) / totalHeight;

    // ✅ Release time based on vertical drop from sustain to release point
    float releaseTime = (adsrPoints[4].x - adsrPoints[3].x) / totalWidth;

    uiAttackTime.store(std::max(attackTime * 2.0f, 0.01f));
    uiDecayTime.store(std::max(decayTime * 2.0f, 0.01f));
    uiSustainLevel.store(std::clamp(sustainLevel, 0.0f, 1.0f));
    uiReleaseTime.store(std::max(releaseTime * 2.0f, 0.01f));
}
