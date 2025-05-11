#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "shared_buffer.h"
#include "adsr_ui.h"
#include "scalekeybank_ui.h"
#include "dev_menu.h"
#include "oscillator_ui.h"
#include "synth_ui.h"

#include "arp_ui.h"
#include "sound.h"
#include <atomic>
#include <string>
#include <cmath>
#include "effects.h"

using sound::WaveType;
extern std::atomic<WaveType> currentWaveform;
extern std::atomic<int> stepPitches[16];
extern std::atomic<int> BPM;
extern bool stepSequence[16];
extern void HandlePadEvents(SDL_Event &event);
extern void DrawPads(SDL_Renderer *renderer);
bool showEffectsMode = false;
bool showDevMenu = false;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 400;

// 🔒 ASDR editing area bounds (left 50% of screen, 30px padding)
// const SDL_Rect adsrBounds = {30, 100, WINDOW_WIDTH / 2 - 60, 150};

// BPM slider
const int BPM_MIN = 40;
const int BPM_MAX = 200;
const int SLIDER_WIDTH = 300;
const int SLIDER_HEIGHT = 20;
const int SLIDER_X = 250;
const int SLIDER_Y = 20;
bool adjustingBPM = false;

// Waveform buttons
const int WAVE_BTN_WIDTH = 60;
const int WAVE_BTN_HEIGHT = 30;
const int WAVE_BTN_X = 10;
const int WAVE_BTN_Y = 10;

// Pitch sliders
const int PITCH_SLIDER_WIDTH = 20;
const int PITCH_SLIDER_HEIGHT = 100;
const int PITCH_SLIDER_Y = 220;
bool adjustingPitch[16] = {false};

// // 🆕 ASDR Editor Toggle + Control
// SDL_Point adsrPoints[4] = {
//     {50, 300}, {150, 350}, {250, 330}, {350, 380}
// };
// bool draggingASDR[4] = {false};
// bool showASDRMode = false;
bool showSynthEditor = false;
SDL_Rect synthBtn = {WINDOW_WIDTH - 60, 130, 50, 30}; // like toggleBtn, lower position

bool showPadMode = false; // Add this global toggle for pad screen mode

SDL_Rect toggleBtn = {WINDOW_WIDTH - 60, 10, 50, 30}; // 🔘 Top right corner

SDL_Rect effectsBtn = {WINDOW_WIDTH - 60, 50, 50, 30}; // 🎛 Button below ASDR

void HandleGlobalKeyEvents(SDL_Event &event)
{
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_TAB)
    {
        showPadMode = !showPadMode; // Toggle pad screen mode
    }
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE)
    {
        showDevMenu = true;
    }
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_9)
    {
        showSynthEditor = !showSynthEditor;
    }
}

void DrawStepSequencer(SDL_Renderer *renderer, bool stepSequence[])
{
    for (int i = 0; i < 16; ++i)
    {
        SDL_Rect rect = {i * 50, 350, 45, 50};
        SDL_SetRenderDrawColor(renderer, stepSequence[i] ? 0 : 100, stepSequence[i] ? 255 : 100, 0, 255);
        SDL_RenderFillRect(renderer, &rect);
    }
}

void HandleStepToggle(SDL_Event &event, bool stepSequence[])
{
    if (event.type == SDL_MOUSEBUTTONDOWN)
    {
        int mouseX = event.button.x;
        int mouseY = event.button.y;
        if (mouseY >= 350 && mouseY <= 400)
        {
            int index = mouseX / 50;
            if (index >= 0 && index < 16)
                stepSequence[index] = !stepSequence[index];
        }
    }
}

void HandleToggleButtonEvent(SDL_Event &event)
{
    int mouseX = event.button.x;
    int mouseY = event.button.y;
    SDL_Point pt = {mouseX, mouseY};

    if (event.type == SDL_MOUSEBUTTONDOWN)
    {
        if (SDL_PointInRect(&pt, &toggleBtn))
        {
            showASDRMode = !showASDRMode;
        }
        if (SDL_PointInRect(&pt, &synthBtn))
        {
            showSynthEditor = !showSynthEditor;
        }
    }
}

void StartOscilloscope(SDL_Renderer *renderer)
{
    TTF_Init();
    TTF_Font *font = TTF_OpenFont("/System/Library/Fonts/Supplemental/Times New Roman.ttf", 24);
    if (!font)
        return;

    ScalekeyBankUI scaleUI(scaleBank); // uses global scaleBank

    bool quit = false;
    bool advancedMode = false;

    while (!quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            scaleUI.HandleEvents(event);
            HandleOscillatorUIEvents(event, advancedMode);

            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
                quit = true;

            HandleGlobalKeyEvents(event);
            // 👽 Toggle Effects UI with Return/Enter key
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN)
            {
                showEffectsMode = !showEffectsMode;
            }

            if (showPadMode)
            {
                HandlePadEvents(event);
                ArpUI_HandleEvent(event); //
                continue;                 //
            }

            if (showSynthEditor)
            {
                HandleSynthEditorEvents(event);
                continue;
            }

            if (showASDRMode)
            {
                HandleADSREvents(event);
                continue;
            }

            if (showEffectsMode)
            {
                HandleEffectUIEvents(event);
                continue;
            }

            if (showDevMenu)
            {
                ShowDevMenu(renderer, font);
                showDevMenu = false;
                continue; // skip rest of frame to avoid drawing underneath
            }

            HandleToggleButtonEvent(event);

            int mouseX = event.button.x;
            int mouseY = event.button.y;

            if (event.type == SDL_MOUSEBUTTONDOWN &&
                mouseX >= SLIDER_X && mouseX <= SLIDER_X + SLIDER_WIDTH &&
                mouseY >= SLIDER_Y && mouseY <= SLIDER_Y + SLIDER_HEIGHT)
            {
                adjustingBPM = true;
            }

            if (event.type == SDL_MOUSEBUTTONUP)
                adjustingBPM = false;

            if (event.type == SDL_MOUSEMOTION && adjustingBPM)
            {
                int newBPM = BPM_MIN + ((mouseX - SLIDER_X) * (BPM_MAX - BPM_MIN) / SLIDER_WIDTH);
                BPM.store(std::clamp(newBPM, BPM_MIN, BPM_MAX));
            }

            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                for (int i = 0; i < 3; ++i)
                {
                    int btnX = WAVE_BTN_X;
                    int btnY = WAVE_BTN_Y + i * (WAVE_BTN_HEIGHT + 10);
                    if (mouseX >= btnX && mouseX <= btnX + WAVE_BTN_WIDTH &&
                        mouseY >= btnY && mouseY <= btnY + WAVE_BTN_HEIGHT)
                    {
                        currentWaveform.store(static_cast<WaveType>(i));
                    }
                }
            }

            for (int i = 0; i < 16; ++i)
            {
                int sliderX = i * 50 + 15;
                if (event.type == SDL_MOUSEBUTTONDOWN &&
                    mouseX >= sliderX && mouseX <= sliderX + PITCH_SLIDER_WIDTH &&
                    mouseY >= PITCH_SLIDER_Y && mouseY <= PITCH_SLIDER_Y + PITCH_SLIDER_HEIGHT)
                {
                    adjustingPitch[i] = true;
                }

                if (event.type == SDL_MOUSEBUTTONUP)
                    adjustingPitch[i] = false;

                if (event.type == SDL_MOUSEMOTION && adjustingPitch[i])
                {
                    int val = 12 - ((mouseY - PITCH_SLIDER_Y) * 25 / PITCH_SLIDER_HEIGHT);
                    stepPitches[i].store(std::clamp(val, -12, 12));
                }
            }

            HandleStepToggle(event, stepSequence);
        }

        // 😉 Fixed block 2: Gate the main drawing if FX mode is active
        if (showEffectsMode)
        {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            DrawEffectsUI(renderer, font);
            SDL_RenderPresent(renderer);
            continue; // Skip drawing the normal UI
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (showPadMode)
        {
            DrawPads(renderer);   // Draws the pad screen UI
            ArpUI_Draw(renderer); // arp controls

            SDL_RenderPresent(renderer); // Updates the screen to show pads immediately
            continue;                    // Skip the rest to avoid drawing other screens over pads
        }
        if (showSynthEditor)
        {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            int16_t sample;
            while (audioRingBuffer.pop(sample))
            {
                analyzer.pushSample(static_cast<float>(sample) / 32767.0f);
            }
            DrawSynthEditor(renderer);
            SDL_RenderPresent(renderer);
            continue;
        }

        if (showASDRMode)
        {
            DrawADSREditor(renderer);
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &toggleBtn);
            SDL_RenderPresent(renderer);
            continue;
        }

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        int w, h;
        SDL_GetRendererOutputSize(renderer, &w, &h);
        int centerY = h / 2;
        int x = 0, prevY = centerY;
        int16_t sample;
        while (audioRingBuffer.pop(sample) && x < w)
        {
            analyzer.pushSample(static_cast<float>(sample) / 32767.0f); // Feed analyzer
            int y = centerY - (sample * centerY / 32767);
            if (x > 0)
                SDL_RenderDrawLine(renderer, x - 1, prevY, x, y);
            prevY = y;
            ++x;
        }

        DrawStepSequencer(renderer, stepSequence);

        // 🎹 Draw Scale & Key Dropdowns
        scaleUI.Draw(renderer, font);
        DrawOscillatorUI(renderer, advancedMode);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect bar = {SLIDER_X, SLIDER_Y, SLIDER_WIDTH, SLIDER_HEIGHT};
        SDL_RenderFillRect(renderer, &bar);
        int knobX = SLIDER_X + ((BPM.load() - BPM_MIN) * SLIDER_WIDTH / (BPM_MAX - BPM_MIN));
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect knob = {knobX - 5, SLIDER_Y, 10, SLIDER_HEIGHT};
        SDL_RenderFillRect(renderer, &knob);

        SDL_Color color = {0, 0, 0, 255};
        std::string bpmStr = "BPM: " + std::to_string(BPM.load());
        SDL_Surface *surface = TTF_RenderText_Solid(font, bpmStr.c_str(), color);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        // 🧠 Center the text inside the BPM slider bar
        SDL_Rect textRect = {
            SLIDER_X + (SLIDER_WIDTH - surface->w) / 2,
            SLIDER_Y + (SLIDER_HEIGHT - surface->h) / 2,
            surface->w,
            surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &textRect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);

        for (int i = 0; i < 3; ++i)
        {
            SDL_Rect btn = {WAVE_BTN_X, WAVE_BTN_Y + i * (WAVE_BTN_HEIGHT + 10), WAVE_BTN_WIDTH, WAVE_BTN_HEIGHT};
            SDL_SetRenderDrawColor(renderer, currentWaveform.load() == static_cast<WaveType>(i) ? 255 : 100, 0, 0, 255);
            SDL_RenderFillRect(renderer, &btn);
        }

        for (int i = 0; i < 16; ++i)
        {
            int sliderX = i * 50 + 15;
            int semitone = stepPitches[i].load();
            int sliderKnobY = PITCH_SLIDER_Y + ((12 - semitone) * PITCH_SLIDER_HEIGHT / 25);

            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_Rect bg = {sliderX, PITCH_SLIDER_Y, PITCH_SLIDER_WIDTH, PITCH_SLIDER_HEIGHT};
            SDL_RenderFillRect(renderer, &bg);

            SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
            SDL_Rect knob = {sliderX, sliderKnobY - 5, PITCH_SLIDER_WIDTH, 10};
            SDL_RenderFillRect(renderer, &knob);
        }
        // Always show ASDR toggle button (top-right corner)
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &toggleBtn);

        // 👽 Draw Effects Button BEFORE presenting the frame
        SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);
        SDL_RenderFillRect(renderer, &effectsBtn);

        SDL_Surface *fxSurface = TTF_RenderText_Solid(font, "FX", color);
        SDL_Texture *fxTexture = SDL_CreateTextureFromSurface(renderer, fxSurface);
        SDL_Rect fxRect = {effectsBtn.x + 10, effectsBtn.y + 5, fxSurface->w, fxSurface->h};
        SDL_RenderCopy(renderer, fxTexture, nullptr, &fxRect);
        SDL_FreeSurface(fxSurface);
        SDL_DestroyTexture(fxTexture);

        // Present the final frame
        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(font);
    TTF_Quit();
}
