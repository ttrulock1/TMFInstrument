#include "lfo_ui.h"
#include "lfo_engine.h"  // ✅ ADD THIS
#include "shared_buffer.h"
#include <cmath>
#include <algorithm>

std::atomic<int> lfoTargetRouting = 0;
std::atomic<float> currentLFOValue = 0.0f;

static int currentWave = 0;
static float lfoDepth = 0.5f;
static float lfoRate = 2.0f;

SDL_Rect sineBtn, squareBtn, triBtn;
SDL_Rect rateSlider, depthSlider;
SDL_Rect routingBtns[3];

void DrawLFOEditor(SDL_Renderer* renderer, int x, int y, int w, int h) {
    // Background
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_Rect bg = {x, y, w, h};
    SDL_RenderFillRect(renderer, &bg);

    // Axis
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_RenderDrawLine(renderer, x, y + h / 2, x + w, y + h / 2);

    // Waveform preview
    SDL_SetRenderDrawColor(renderer, 0, 200, 255, 255);
    float step = (2 * 3.14159265f) / w;
    for (int i = 1; i < w; ++i) {
        float t0 = (i - 1) * step * lfoRate;
        float t1 = i * step * lfoRate;
        float v0 = 0, v1 = 0;
        switch (currentWave) {
            case 0: v0 = std::sin(t0); v1 = std::sin(t1); break;
            case 1: v0 = (std::sin(t0) > 0 ? 1.0f : -1.0f); v1 = (std::sin(t1) > 0 ? 1.0f : -1.0f); break;
            case 2:
                v0 = 2.0f * std::fabs(2.0f * (t0 / (2 * 3.14159265f) - std::floor(t0 / (2 * 3.14159265f) + 0.5))) - 1.0f;
                v1 = 2.0f * std::fabs(2.0f * (t1 / (2 * 3.14159265f) - std::floor(t1 / (2 * 3.14159265f) + 0.5))) - 1.0f;
                break;
        }
        int y0 = y + h / 2 - static_cast<int>(v0 * lfoDepth * (h / 2));
        int y1 = y + h / 2 - static_cast<int>(v1 * lfoDepth * (h / 2));
        SDL_RenderDrawLine(renderer, x + i - 1, y0, x + i, y1);
    }

    // Waveform buttons
    sineBtn = {x, y + h + 10, 40, 20};
    squareBtn = {x + 50, y + h + 10, 40, 20};
    triBtn = {x + 100, y + h + 10, 40, 20};
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &sineBtn);
    SDL_RenderDrawRect(renderer, &squareBtn);
    SDL_RenderDrawRect(renderer, &triBtn);

    // Rate slider
    rateSlider = {x, y + h + 40, w, 10};
    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
    SDL_RenderFillRect(renderer, &rateSlider);
    int rateKnobX = rateSlider.x + static_cast<int>((lfoRate / 10.0f) * rateSlider.w);
    SDL_Rect rateKnob = {rateKnobX - 5, rateSlider.y - 2, 10, 14};
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &rateKnob);

    // Depth slider
    depthSlider = {x, y + h + 70, w, 10};
    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
    SDL_RenderFillRect(renderer, &depthSlider);
    int depthKnobX = depthSlider.x + static_cast<int>(lfoDepth * depthSlider.w);
    SDL_Rect depthKnob = {depthKnobX - 5, depthSlider.y - 2, 10, 14};
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderFillRect(renderer, &depthKnob);

    // Routing buttons (Pitch, Amp, Shape)
    for (int i = 0; i < 3; ++i) {
        routingBtns[i] = {x + i * 50, y + h + 100, 40, 20};
        SDL_SetRenderDrawColor(renderer, lfoTargetRouting.load() == i ? 255 : 120, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &routingBtns[i]);
    }
}

void HandleLFOEvents(SDL_Event& event, int x, int y, int w, int h) {
    if (event.type != SDL_MOUSEBUTTONDOWN && event.type != SDL_MOUSEMOTION) return;
    int mx = event.button.x;
    int my = event.button.y;

    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (SDL_PointInRect(&SDL_Point{mx, my}, &sineBtn)) currentWave = 0;
        else if (SDL_PointInRect(&SDL_Point{mx, my}, &squareBtn)) currentWave = 1;
        else if (SDL_PointInRect(&SDL_Point{mx, my}, &triBtn)) currentWave = 2;
        for (int i = 0; i < 3; ++i)
            if (SDL_PointInRect(&SDL_Point{mx, my}, &routingBtns[i]))
                lfoTargetRouting.store(i);
    }

    if (SDL_PointInRect(&SDL_Point{mx, my}, &rateSlider)) {
        float norm = float(mx - rateSlider.x) / rateSlider.w;
        lfoRate = std::clamp(norm * 10.0f, 0.1f, 10.0f);
    }

    if (SDL_PointInRect(&SDL_Point{mx, my}, &depthSlider)) {
        float norm = float(mx - depthSlider.x) / depthSlider.w;
        lfoDepth = std::clamp(norm, 0.0f, 1.0f);
    }

    // Real-time value for audio mod
    float t = SDL_GetTicks() / 1000.0f;
    float phase = t * lfoRate * 2 * 3.14159265f;
    switch (currentWave) {
        case 0: currentLFOValue.store(std::sin(phase) * lfoDepth); break;
        case 1: currentLFOValue.store(((std::sin(phase) > 0) ? 1.0f : -1.0f) * lfoDepth); break;
        case 2:
            currentLFOValue.store((2.0f * std::fabs(2.0f * (phase / (2 * 3.14159265f) - std::floor(phase / (2 * 3.14159265f) + 0.5))) - 1.0f) * lfoDepth);
            break;
    }
}