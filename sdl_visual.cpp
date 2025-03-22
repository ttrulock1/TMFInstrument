#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "shared_buffer.h"
#include "sound.h"
#include <atomic>
#include <string>

using sound::WaveType;
extern std::atomic<WaveType> currentWaveform;
extern std::atomic<int> stepPitches[16];

// Window dimensions
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 400;

// BPM Slider Variables
const int BPM_MIN = 40;
const int BPM_MAX = 200;
const int SLIDER_WIDTH = 300;
const int SLIDER_HEIGHT = 20;
const int SLIDER_X = 250;
const int SLIDER_Y = 20;
bool adjustingBPM = false;

// Waveform Button Area
const int WAVE_BTN_WIDTH = 60;
const int WAVE_BTN_HEIGHT = 30;
const int WAVE_BTN_X = 10;
const int WAVE_BTN_Y = 10;

// Pitch Slider Config
const int PITCH_SLIDER_WIDTH = 20;
const int PITCH_SLIDER_HEIGHT = 100;
const int PITCH_SLIDER_Y = 220;
bool adjustingPitch[16] = {false};


void DrawStepSequencer(SDL_Renderer* renderer, bool stepSequence[]);
void HandleStepToggle(SDL_Event& event, bool stepSequence[]);

void StartOscilloscope(SDL_Renderer* renderer) {
    bool quit = false;

    if (TTF_Init() == -1) {
        SDL_Log("Failed to initialize SDL_ttf: %s", TTF_GetError());
        return;
    }

    TTF_Font* font = TTF_OpenFont("/System/Library/Fonts/Supplemental/Times New Roman.ttf", 24);
    if (!font) {
        SDL_Log("Failed to load font: %s", TTF_GetError());
        return;
    }

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || 
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                quit = true;
            }

            int mouseX = event.button.x;
            int mouseY = event.button.y;

            if (event.type == SDL_MOUSEBUTTONDOWN &&
                mouseX >= SLIDER_X && mouseX <= SLIDER_X + SLIDER_WIDTH &&
                mouseY >= SLIDER_Y && mouseY <= SLIDER_Y + SLIDER_HEIGHT) {
                adjustingBPM = true;
            }
            if (event.type == SDL_MOUSEBUTTONUP) {
                adjustingBPM = false;
            }
            if (event.type == SDL_MOUSEMOTION && adjustingBPM) {
                int newBPM = BPM_MIN + ((mouseX - SLIDER_X) * (BPM_MAX - BPM_MIN) / SLIDER_WIDTH);
                BPM.store(std::max(BPM_MIN, std::min(newBPM, BPM_MAX)));
            }

            if (event.type == SDL_MOUSEBUTTONDOWN) {
                for (int i = 0; i < 3; ++i) {
                    int btnX = WAVE_BTN_X;
                    int btnY = WAVE_BTN_Y + i * (WAVE_BTN_HEIGHT + 10);
                    if (mouseX >= btnX && mouseX <= btnX + WAVE_BTN_WIDTH &&
                        mouseY >= btnY && mouseY <= btnY + WAVE_BTN_HEIGHT) {
                        currentWaveform.store(static_cast<WaveType>(i));
                    }
                }
            }

            for (int i = 0; i < 16; ++i) {
                int sliderX = i * 50 + 15;
                if (event.type == SDL_MOUSEBUTTONDOWN &&
                    mouseX >= sliderX && mouseX <= sliderX + PITCH_SLIDER_WIDTH &&
                    mouseY >= PITCH_SLIDER_Y && mouseY <= PITCH_SLIDER_Y + PITCH_SLIDER_HEIGHT) {
                    adjustingPitch[i] = true;
                }
                if (event.type == SDL_MOUSEBUTTONUP) {
                    adjustingPitch[i] = false;
                }
                if (event.type == SDL_MOUSEMOTION && adjustingPitch[i]) {
                    int newVal = 12 - ((mouseY - PITCH_SLIDER_Y) * 25 / PITCH_SLIDER_HEIGHT);
                    stepPitches[i].store(std::max(-12, std::min(newVal, 12)));
                }
            }

            HandleStepToggle(event, stepSequence);
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        int w, h;
        SDL_GetRendererOutputSize(renderer, &w, &h);
        int centerY = h / 2;
        int x = 0;
        int prevY = centerY;
        int16_t sample;
        while (audioRingBuffer.pop(sample) && x < w) {
            int y = centerY - (sample * centerY / 32767);
            if (x > 0) SDL_RenderDrawLine(renderer, x - 1, prevY, x, y);
            prevY = y;
            ++x;
        }

        DrawStepSequencer(renderer, stepSequence);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect sliderBar = {SLIDER_X, SLIDER_Y, SLIDER_WIDTH, SLIDER_HEIGHT};
        SDL_RenderFillRect(renderer, &sliderBar);

        int knobX = SLIDER_X + ((BPM.load() - BPM_MIN) * SLIDER_WIDTH / (BPM_MAX - BPM_MIN));
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect knob = {knobX - 5, SLIDER_Y, 10, SLIDER_HEIGHT};
        SDL_RenderFillRect(renderer, &knob);

        SDL_Color textColor = {255, 255, 255, 255};
        std::string bpmText = "BPM: " + std::to_string(BPM.load());
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, bpmText.c_str(), textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = {SLIDER_X + SLIDER_WIDTH + 20, SLIDER_Y, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

        for (int i = 0; i < 3; ++i) {
            SDL_Rect btn = {WAVE_BTN_X, WAVE_BTN_Y + i * (WAVE_BTN_HEIGHT + 10), WAVE_BTN_WIDTH, WAVE_BTN_HEIGHT};
            if (currentWaveform.load() == static_cast<WaveType>(i)) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            }
            SDL_RenderFillRect(renderer, &btn);
        }

        for (int i = 0; i < 16; ++i) {
            int sliderX = i * 50 + 15;
            int semitone = stepPitches[i].load();
            int sliderKnobY = PITCH_SLIDER_Y + ((12 - semitone) * PITCH_SLIDER_HEIGHT / 25);
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_Rect sliderBg = {sliderX, PITCH_SLIDER_Y, PITCH_SLIDER_WIDTH, PITCH_SLIDER_HEIGHT};
            SDL_RenderFillRect(renderer, &sliderBg);
            SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
            SDL_Rect knob = {sliderX, sliderKnobY - 5, PITCH_SLIDER_WIDTH, 10};
            SDL_RenderFillRect(renderer, &knob);
        }

        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(font);
    TTF_Quit();
}

void HandleStepToggle(SDL_Event& event, bool stepSequence[]) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        int mouseX = event.button.x;
        int mouseY = event.button.y;
        int stepWidth = 50;
        int stepY = 350;
        if (mouseY >= stepY && mouseY <= stepY + 50) {
            int stepIndex = mouseX / stepWidth;
            if (stepIndex >= 0 && stepIndex < 16) {
                stepSequence[stepIndex] = !stepSequence[stepIndex];
            }
        }
    }
}

void DrawStepSequencer(SDL_Renderer* renderer, bool stepSequence[]) {
    int stepWidth = 50;
    int stepHeight = 50;
    int yPos = 350;
    for (int i = 0; i < 16; ++i) {
        SDL_Rect stepRect = {i * stepWidth, yPos, stepWidth - 5, stepHeight};
        if (stepSequence[i]) {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        }
        SDL_RenderFillRect(renderer, &stepRect);
    }
}