// Synth Ui Layout (previous version you requested)
#include "adsr_ui.h"
#include "lfo_ui.h"
#include "spectrum_analyzer.h"
#include "Moog_Filter.h"
#include "shared_buffer.h"
#include <algorithm>

const int windowWidth = 900;
const int windowHeight = 400;

// SDL_Rect adsrBounds = { 0, 0, windowWidth / 3, 250 };
SDL_Rect spectrumBounds = {windowWidth / 3, 0, windowWidth / 3, 250};
SDL_Rect filterControlBounds = {windowWidth / 3, 260, windowWidth / 3, 140};
SDL_Rect lfoBounds = {2 * windowWidth / 3, 0, windowWidth / 3, 250};

// SDL_Point adsrPoints[5];
// bool draggingASDR[4] = {false};
// bool showASDRMode = true;
// float envAmount = 1.0f;
// SDL_Rect amountSlider;

SpectrumAnalyzer analyzer(512, 44100);

void DrawSynthEditor(SDL_Renderer *renderer)
{
    
    // ADSR
    SDL_SetRenderDrawColor(renderer, 0, 60, 0, 255);
    SDL_RenderFillRect(renderer, &adsrBounds);
    DrawADSREditor(renderer);

    // Spectrum Analyzer
    analyzer.updateSpectrum();
    analyzer.draw(renderer, spectrumBounds.x, spectrumBounds.y, spectrumBounds.w, spectrumBounds.h, moogFilter.getCutoff(), moogFilter.getResonance());

    // Moog Filter Controls
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderFillRect(renderer, &filterControlBounds);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &filterControlBounds);
    SDL_Rect cutoffSlider = {filterControlBounds.x + 20, filterControlBounds.y + 20, filterControlBounds.w - 40, 10};
    SDL_RenderFillRect(renderer, &cutoffSlider);
    int knobX = cutoffSlider.x + static_cast<int>((moogFilter.getCutoff() / 8000.0f) * cutoffSlider.w);
    SDL_Rect knob = {knobX - 5, cutoffSlider.y - 3, 10, 16};
    SDL_RenderFillRect(renderer, &knob);

    SDL_Rect resoSlider = {filterControlBounds.x + 20, filterControlBounds.y + 50, filterControlBounds.w - 40, 10};
    SDL_RenderFillRect(renderer, &resoSlider);
    int knobX2 = resoSlider.x + static_cast<int>((moogFilter.getResonance() / 4.0f) * resoSlider.w);
    SDL_Rect knob2 = {knobX2 - 5, resoSlider.y - 3, 10, 16};
    SDL_RenderFillRect(renderer, &knob2);

    // LFO Editor
    DrawLFOEditor(renderer, lfoBounds.x, lfoBounds.y, lfoBounds.w, lfoBounds.h);
}

void HandleSynthEditorEvents(SDL_Event &event)
{
    static bool draggingCutoff = false;
    static bool draggingResonance = false;
    static bool draggingCutoffFromSpectrum = false;
    static bool draggingResonanceFromSpectrum = false;

    int mx = event.button.x;
    int my = event.button.y;
    SDL_Point pt = {mx, my};

    if (SDL_PointInRect(&pt, &adsrBounds))
    {
        HandleADSREvents(event);
    }
    SDL_Rect cutoffSlider = {filterControlBounds.x + 20, filterControlBounds.y + 20, filterControlBounds.w - 40, 10};
    SDL_Rect resoSlider = {filterControlBounds.x + 20, filterControlBounds.y + 50, filterControlBounds.w - 40, 10};

    if (event.type == SDL_MOUSEBUTTONDOWN)
    {
        if (SDL_PointInRect(&pt, &cutoffSlider))
            draggingCutoff = true;
        if (SDL_PointInRect(&pt, &resoSlider))
            draggingResonance = true;
    }

    if (event.type == SDL_MOUSEBUTTONUP)
    {
        draggingCutoff = false;
        draggingResonance = false;
    }

    if (event.type == SDL_MOUSEMOTION)
    {
        if (draggingCutoff)
        {
            float norm = float(mx - cutoffSlider.x) / cutoffSlider.w;
            moogFilter.setCutoff(std::clamp(norm, 0.0f, 1.0f) * 8000.0f);
        }
        if (draggingResonance)
        {
            float norm = float(mx - resoSlider.x) / resoSlider.w;
            moogFilter.setResonance(std::clamp(norm, 0.0f, 1.0f) * 4.0f);
        }
    }

    if (SDL_PointInRect(&pt, &lfoBounds))
    {
        HandleLFOEvents(event, lfoBounds.x, lfoBounds.y, lfoBounds.w, lfoBounds.h);
    }
    // 🌟 Spectrum Drag for Cutoff/Resonance (with grab handle)

// 👇 Convert moogFilter cutoff (in Hz) to X-position in the spectrum
int cutoffX = spectrumBounds.x + static_cast<int>((moogFilter.getCutoff() / 8000.0f) * spectrumBounds.w);

// 👇 Define a draggable zone around that X-line (±6 pixels wide)
SDL_Rect dragZone = {
    cutoffX - 6,
    spectrumBounds.y,
    12,                      // width
    spectrumBounds.h         // full height
};

// 👇 START DRAGGING
if (event.type == SDL_MOUSEBUTTONDOWN) {
    if (SDL_PointInRect(&pt, &dragZone)) {
        draggingCutoffFromSpectrum = true;     // ✅ Start dragging cutoff if click inside zone
    }
    else if ((SDL_GetModState() & KMOD_SHIFT) && SDL_PointInRect(&pt, &spectrumBounds)) {
        draggingResonanceFromSpectrum = true;  // ✅ Start dragging resonance if Shift+click inside full spectrum
    }
}

// 👇 STOP DRAGGING
if (event.type == SDL_MOUSEBUTTONUP) {
    draggingCutoffFromSpectrum = false;
    draggingResonanceFromSpectrum = false;
}

// 👇 HANDLE DRAG MOTION
if (event.type == SDL_MOUSEMOTION) {
    if (draggingCutoffFromSpectrum) {
        float normX = float(mx - spectrumBounds.x) / spectrumBounds.w;
        normX = std::clamp(normX, 0.0f, 1.0f);
        moogFilter.setCutoff(normX * 8000.0f);  // 👈 Map drag X back to Hz and update filter
    }
    if (draggingResonanceFromSpectrum) {
        float normY = 1.0f - float(my - spectrumBounds.y) / spectrumBounds.h;
        normY = std::clamp(normY, 0.0f, 1.0f);
        moogFilter.setResonance(normY * 4.0f);  // 👈 Map drag Y to resonance strength
    }
}

}
