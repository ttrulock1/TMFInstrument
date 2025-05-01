#include "oscillator_ui.h"
#include <vector>
#include <string>
#include <algorithm>
#include "shared_buffer.h"

// ✅ Move this to the top BEFORE using Slider
struct Slider {
    SDL_Rect rect;
    float value = 0.5f;
    std::string label;
};

std::vector<Slider> easySliders;
std::vector<Slider> advancedSliders;

void RefreshOscillatorSliders() {
    easySliders[0].value = oscSawLevel.load();
    easySliders[1].value = oscSquareLevel.load();
    easySliders[2].value = oscSineLevel.load();
    easySliders[3].value = oscSubLevel.load();
    easySliders[4].value = oscVolume.load();

    advancedSliders[0].value = oscPWMAmount.load();
    advancedSliders[1].value = oscMetalizerAmount.load();
    advancedSliders[2].value = oscUltrasawAmount.load();
    advancedSliders[3].value = oscSaturationAmount.load();
    advancedSliders[4].value = oscNoiseAmount.load();
}

// struct Slider {
//     SDL_Rect rect;
//     float value = 0.5f;  // Default midpoint
//     std::string label;
// };

// static std::vector<Slider> easySliders;
// static std::vector<Slider> advancedSliders;
static SDL_Rect toggleButton = {10, 130, 50, 30};

void InitOscillatorUI() {
    int leftColumnX = 80;   // start closer to the red buttons
    int startY = 10;        // align vertically with red buttons
    int spacingY = 40;      // tighter vertical stacking
    int sliderWidth = 100;
    int sliderHeight = 10;

    easySliders = {
        {{leftColumnX, startY + 0 * spacingY, sliderWidth, sliderHeight}, 0.5f,
"Saw"},
        {{leftColumnX, startY + 1 * spacingY, sliderWidth, sliderHeight}, 0.5f,
"Square"},
        {{leftColumnX, startY + 2 * spacingY, sliderWidth, sliderHeight}, 0.5f,
"Sine"},
        {{leftColumnX, startY + 3 * spacingY, sliderWidth, sliderHeight}, 0.5f,
"Sub"},
        {{leftColumnX, startY + 4 * spacingY, sliderWidth, sliderHeight}, 0.8f,
"Volume"},
    };

    advancedSliders = {
        {{leftColumnX, startY + 0 * spacingY, sliderWidth, sliderHeight}, 0.0f,
"PWM"},
        {{leftColumnX, startY + 1 * spacingY, sliderWidth, sliderHeight}, 0.0f,
"Metalizer"},
        {{leftColumnX, startY + 2 * spacingY, sliderWidth, sliderHeight}, 0.0f,
"Ultrasaw"},
        {{leftColumnX, startY + 3 * spacingY, sliderWidth, sliderHeight}, 0.0f,
"Saturation"},
        {{leftColumnX, startY + 4 * spacingY, sliderWidth, sliderHeight}, 0.0f,
"Noise"},
    };
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
            oscPWMAmount.store(advancedSliders[0].value);
            oscMetalizerAmount.store(advancedSliders[1].value);
            oscUltrasawAmount.store(advancedSliders[2].value);
            oscSaturationAmount.store(advancedSliders[3].value);
            oscNoiseAmount.store(advancedSliders[4].value);
        }
    }
}
