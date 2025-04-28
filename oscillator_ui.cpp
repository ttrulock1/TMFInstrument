// oscillator_ui.cpp
#include "oscillator_ui.h"
#include <vector>
#include <string>
#include <algorithm>
#include "shared_buffer.h"

struct Slider {
    SDL_Rect rect;
    float value = 0.5f;  // Default midpoint
    std::string label;
};

static std::vector<Slider> easySliders;
static std::vector<Slider> advancedSliders;
static SDL_Rect toggleButton = {10, 130, 50, 30};

void InitOscillatorUI() {
    int leftColumnX = 80;   // start closer to the red buttons
    int startY = 10;        // align vertically with red buttons
    int spacingY = 40;      // tighter vertical stacking
    int rightColumnX = 1100;
    int sliderWidth = 100;
    int sliderHeight = 10;

    easySliders = {
        {{leftColumnX, startY + 0 * spacingY, sliderWidth, sliderHeight}, 0.5f, "Saw"},
        {{leftColumnX, startY + 1 * spacingY, sliderWidth, sliderHeight}, 0.5f, "Square"},
        {{leftColumnX, startY + 2 * spacingY, sliderWidth, sliderHeight}, 0.5f, "Sine"},
        {{leftColumnX, startY + 3 * spacingY, sliderWidth, sliderHeight}, 0.5f, "Sub"},
        {{leftColumnX, startY + 4 * spacingY, sliderWidth, sliderHeight}, 0.8f, "Volume"},
    };

    advancedSliders = easySliders;
    advancedSliders.push_back({{rightColumnX, startY + 0 * spacingY, sliderWidth, sliderHeight}, 0.0f, "PWM"});
    advancedSliders.push_back({{rightColumnX, startY + 1 * spacingY, sliderWidth, sliderHeight}, 0.0f, "Metalizer"});
    advancedSliders.push_back({{rightColumnX, startY + 2 * spacingY, sliderWidth, sliderHeight}, 0.0f, "Ultrasaw"});
    advancedSliders.push_back({{rightColumnX, startY + 3 * spacingY, sliderWidth, sliderHeight}, 0.0f, "Saturation"});
    advancedSliders.push_back({{rightColumnX, startY + 4 * spacingY, sliderWidth, sliderHeight}, 0.0f, "Noise"});
}

void DrawOscillatorUI(SDL_Renderer* renderer, bool advancedMode) {
    if (easySliders.empty()) InitOscillatorUI();

    auto& sliders = advancedMode ? advancedSliders : easySliders;

    for (auto& slider : sliders) {
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        SDL_RenderFillRect(renderer, &slider.rect);

        int knobX = slider.rect.x + static_cast<int>(slider.value * slider.rect.w);
        SDL_Rect knob = {knobX - 5, slider.rect.y - 3, 10, slider.rect.h + 6};
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &knob);
    }

    // Toggle button for Easy/Advanced
    SDL_SetRenderDrawColor(renderer, 50, 50, 255, 255);
    SDL_RenderFillRect(renderer, &toggleButton);
}

void HandleOscillatorUIEvents(SDL_Event& event, bool& advancedMode) {
    if (event.type != SDL_MOUSEBUTTONDOWN && event.type != SDL_MOUSEMOTION)
        return;

    int mx = (event.type == SDL_MOUSEBUTTONDOWN) ? event.button.x : event.motion.x;
    int my = (event.type == SDL_MOUSEBUTTONDOWN) ? event.button.y : event.motion.y;
    SDL_Point pt = {mx, my};

    if (event.type == SDL_MOUSEBUTTONDOWN && SDL_PointInRect(&pt, &toggleButton)) {
        advancedMode = !advancedMode;
        return; // 🛑 Stop here: don't keep processing if toggled
    }

    auto& sliders = advancedMode ? advancedSliders : easySliders;

    bool sliderMoved = false; // 🧠 Track if any slider was touched

    for (auto& slider : sliders) {
        if (SDL_PointInRect(&pt, &slider.rect)) {
            float norm = float(mx - slider.rect.x) / slider.rect.w;
            slider.value = std::clamp(norm, 0.0f, 1.0f);
            sliderMoved = true; // 🧠 Mark that a slider was changed
        }
    }

    if (sliderMoved) { // ✅ Only update shared_buffer if slider was moved
        if (!advancedMode) {
            oscSawLevel.store(easySliders[0].value);
            oscSquareLevel.store(easySliders[1].value);
            oscSineLevel.store(easySliders[2].value);
            oscSubLevel.store(easySliders[3].value);
            oscVolume.store(easySliders[4].value);
        } else {
            oscPWMAmount.store(advancedSliders[5].value);
            oscMetalizerAmount.store(advancedSliders[6].value);
            oscUltrasawAmount.store(advancedSliders[7].value);
            oscSaturationAmount.store(advancedSliders[8].value);
            oscNoiseAmount.store(advancedSliders[9].value);
        }
    }
}
