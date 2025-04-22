// Replaces UI-local lfoRate, lfoDepth, and currentWave with direct assignments to shared_buffer's `lfo`

#include "lfo_ui.h"
#include "shared_buffer.h"
#include "lfo_engine.h" 

#include <cmath>
#include <algorithm>

// #include "<oscilloscope.h>"

//Inside DrawLFOEditor(), just after defining the buttons and sliders, add:
// // Create an instance of Oscilloscope for LFO display
// static Oscilloscope lfoScope(256);  // 256 samples in the buffer

// // Push the current LFO sample into the oscilloscope buffer
// float lfoMod = lfo.sample(1.0f / SAMPLE_RATE);  // Get LFO value for the current sample
// lfoScope.push(lfoMod);

// this goes somether else

//Define the display area for LFO waveform:

// SDL_Rect lfoArea = {x, y + h + 110, 300, 100};  // Position under the sliders (adjust as needed)


//Draw the oscilloscope waveform:

// lfoScope.draw(renderer, lfoArea, {255, 130, 0, 255});  // Orange color for LFO visual


std::atomic<int> lfoTargetRouting = 0;
std::atomic<float> currentLFOValue = 0.0f;

SDL_Rect sineBtn, squareBtn, triBtn;
SDL_Rect rateSlider, depthSlider;
SDL_Rect routingBtns[3];


void DrawLFOEditor(SDL_Renderer* renderer, int x, int y, int w, int h) {
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_Rect bg = {x, y, w, h};
    SDL_RenderFillRect(renderer, &bg);

    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_RenderDrawLine(renderer, x, y + h / 2, x + w, y + h / 2);

    SDL_SetRenderDrawColor(renderer, 0, 200, 255, 255);
    float step = (2 * M_PI) / w;
    for (int i = 1; i < w; ++i) {
        float t0 = (i - 1) * step * lfo.rateHz;
        float t1 = i * step * lfo.rateHz;
        float v0 = 0, v1 = 0;
        switch (lfo.waveform) {
            case LFOWaveform::Sine:     v0 = std::sin(t0); v1 = std::sin(t1); break;
            case LFOWaveform::Square:   v0 = (std::sin(t0) > 0 ? 1.0f : -1.0f); v1 = (std::sin(t1) > 0 ? 1.0f : -1.0f); break;
            case LFOWaveform::Triangle:
                v0 = 2.0f * std::fabs(2.0f * (t0 / (2 * M_PI) - std::floor(t0 / (2 * M_PI) + 0.5))) - 1.0f;
                v1 = 2.0f * std::fabs(2.0f * (t1 / (2 * M_PI) - std::floor(t1 / (2 * M_PI) + 0.5))) - 1.0f;
                break;
        }
        int y0 = y + h / 2 - static_cast<int>(v0 * lfo.depth * (h / 2));
        int y1 = y + h / 2 - static_cast<int>(v1 * lfo.depth * (h / 2));
        SDL_RenderDrawLine(renderer, x + i - 1, y0, x + i, y1);
    }

    sineBtn = {x, y + h + 10, 40, 20};
    squareBtn = {x + 50, y + h + 10, 40, 20};
    triBtn = {x + 100, y + h + 10, 40, 20};
// 🟧 Highlight selected waveform in orange, others in green
auto highlightWaveBtn = [&](SDL_Rect& btn, LFOWaveform type) {
    if (lfo.waveform == type) {
        SDL_SetRenderDrawColor(renderer, 255, 130, 0, 255); // 🟧 active
        SDL_RenderFillRect(renderer, &btn);
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);   // 🟩 inactive
        SDL_RenderFillRect(renderer, &btn);
    }
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // white border
    SDL_RenderDrawRect(renderer, &btn);
};

highlightWaveBtn(sineBtn, LFOWaveform::Sine);
highlightWaveBtn(squareBtn, LFOWaveform::Square);
highlightWaveBtn(triBtn, LFOWaveform::Triangle);


    rateSlider = {x, y + h + 40, w, 10};
    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
    SDL_RenderFillRect(renderer, &rateSlider);
    int rateKnobX = rateSlider.x + static_cast<int>((lfo.rateHz / 10.0f) * rateSlider.w);
    SDL_Rect rateKnob = {rateKnobX - 5, rateSlider.y - 2, 10, 14};
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &rateKnob);

    depthSlider = {x, y + h + 70, w, 10};
    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
    SDL_RenderFillRect(renderer, &depthSlider);
    int depthKnobX = depthSlider.x + static_cast<int>(lfo.depth * depthSlider.w);
    SDL_Rect depthKnob = {depthKnobX - 5, depthSlider.y - 2, 10, 14};
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderFillRect(renderer, &depthKnob);

    for (int i = 0; i < 3; ++i) {
        routingBtns[i] = {x + i * 50, y + h + 100, 40, 20};
        if (lfoTargetRouting.load() == i) {
            SDL_SetRenderDrawColor(renderer, 255, 130, 0, 255);  // 🟧 #FF8200 = active selection
            SDL_RenderFillRect(renderer, &routingBtns[i]);
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // 🟩 green when not selected
            SDL_RenderFillRect(renderer, &routingBtns[i]);
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Border
        SDL_RenderDrawRect(renderer, &routingBtns[i]);
    }
}


void HandleLFOEvents(SDL_Event& event, int x, int y, int w, int h) {

    
int mx = 0, my = 0;
if (event.type == SDL_MOUSEBUTTONDOWN) {
    mx = event.button.x;
    my = event.button.y;
} else if (event.type == SDL_MOUSEMOTION) {
    mx = event.motion.x;
    my = event.motion.y;
}

SDL_Point pt = {mx, my};



if (event.type == SDL_MOUSEBUTTONDOWN) {
    if (SDL_PointInRect(&pt, &sineBtn)) {
        lfo.waveform = LFOWaveform::Sine;
    }
    if (SDL_PointInRect(&pt, &squareBtn)) {
        lfo.waveform = LFOWaveform::Square;
    }
    if (SDL_PointInRect(&pt, &triBtn)) {
        lfo.waveform = LFOWaveform::Triangle;
    }

    // ✅ Not nested — always run
    for (int i = 0; i < 3; ++i) {
        if (SDL_PointInRect(&pt, &routingBtns[i])) {
            lfoTargetRouting.store(i);
        }
    }
}

    if (SDL_PointInRect(&pt, &rateSlider)) {
        float norm = float(mx - rateSlider.x) / rateSlider.w;
        lfo.rateHz = std::clamp(norm * 10.0f, 0.1f, 10.0f);
    }

    if (SDL_PointInRect(&pt, &depthSlider)) {
        float norm = float(mx - depthSlider.x) / depthSlider.w;
        lfo.depth = std::clamp(norm, 0.0f, 1.0f);
    }

    float t = SDL_GetTicks() / 1000.0f;
    float phase = t * lfo.rateHz * 2 * M_PI;
    float value = 0.0f;
    switch (lfo.waveform) {
        case LFOWaveform::Sine:     value = std::sin(phase); break;
        case LFOWaveform::Square:   value = (std::sin(phase) > 0 ? 1.0f : -1.0f); break;
        case LFOWaveform::Triangle:
            value = 2.0f * std::fabs(2.0f * (phase / (2 * M_PI) - std::floor(phase / (2 * M_PI) + 0.5))) - 1.0f;
            break;
    }

    
    currentLFOValue.store(value * lfo.depth);
}
