#include "effects.h"
#include "shared_buffer.h"
#include <SDL2/SDL_ttf.h>
#include <algorithm>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 400

std::vector<EffectPanel> effectPanels = {
    {   // Chorus - Left third (Boss CE-2 style)
        {0, 0, WINDOW_WIDTH / 3, WINDOW_HEIGHT}, "Chorus",
        {
            {"Rate", &chorusRate, 0.1f, 5.0f, {31, 119, 180}},
            {"Depth", &chorusDepth, 0.0f, 1.0f, {31, 119, 180}},
            {"Mix", &chorusMix, 0.0f, 1.0f, {31, 119, 180}}
        },
        &chorusEnabled,
        {17, 45, 39}
    },
    {   // Delay - Middle third (Existing DD-7 style)
        {WINDOW_WIDTH / 3, 0, WINDOW_WIDTH / 3, WINDOW_HEIGHT}, "Delay",
        {
            {"Time", &delayTime, 0.0f, 1000.0f, {117, 191, 255}},
            {"FB", &delayFeedback, 0.0f, 0.99f, {117, 191, 255}},
            {"Mix", &delayMix, 0.0f, 1.0f, {117, 191, 255}},
            {"HPF", &delayHighCut, 100.0f, 20000.0f, {117, 191, 255}}
        },
        &delayEnabled,
        {70, 70, 150}
    },
    {   // Reverb - Right third (Boss RV-6 style)
        {2 * WINDOW_WIDTH / 3, 0, WINDOW_WIDTH / 3, WINDOW_HEIGHT}, "Reverb",
        {
            {"Decay", &reverbDecay, 0.2f, 5.0f, {255, 210, 230}},
            {"Damp", &reverbDamping, 0.0f, 1.0f, {255, 210, 230}},
            {"Mix", &reverbMix, 0.0f, 1.0f, {255, 210, 230}}
        },
        &reverbEnabled,
        {40, 25, 45}
    }
};

void DrawEffectsUI(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Color white = {255, 255, 255};
    for (auto& panel : effectPanels) {
        SDL_SetRenderDrawColor(renderer, panel.bgColor.r, panel.bgColor.g, panel.bgColor.b, 255);
        SDL_RenderFillRect(renderer, &panel.bounds);

        int sliderAreaX = panel.bounds.x + 20;
        int sliderY = 80;
        int sliderWidth = 20;
        int sliderHeight = 120;
        int spacing = (panel.bounds.w - 40) / panel.parameters.size();

        for (size_t i = 0; i < panel.parameters.size(); ++i) {
            auto& param = panel.parameters[i];
            float valNorm = (*param.param - param.min) / (param.max - param.min);
            valNorm = std::clamp(valNorm, 0.0f, 1.0f);

            SDL_Rect track = {sliderAreaX + static_cast<int>(i) * spacing, sliderY, sliderWidth, sliderHeight};
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            SDL_RenderFillRect(renderer, &track);

            SDL_Rect fill = {
                track.x, sliderY + sliderHeight - static_cast<int>(valNorm * sliderHeight),
                sliderWidth, static_cast<int>(valNorm * sliderHeight)
            };
            SDL_SetRenderDrawColor(renderer, param.color.r, param.color.g, param.color.b, 255);
            SDL_RenderFillRect(renderer, &fill);

            SDL_Surface* textSurf = TTF_RenderText_Blended(font, param.label, white);
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, textSurf);
            SDL_Rect labelRect = {track.x - 5, sliderY + sliderHeight + 10, textSurf->w, textSurf->h};
            SDL_RenderCopy(renderer, tex, nullptr, &labelRect);
            SDL_FreeSurface(textSurf);
            SDL_DestroyTexture(tex);
        }

        SDL_Rect toggle = {panel.bounds.x + panel.bounds.w / 2 - 25, panel.bounds.h - 50, 50, 30};
        SDL_SetRenderDrawColor(renderer, panel.enabled->load() ? 0 : 100, panel.enabled->load() ? 200 : 50, 0, 255);
        SDL_RenderFillRect(renderer, &toggle);
    }
}

void HandleEffectUIEvents(const SDL_Event& event) {
    static EffectPanel* activePanel = nullptr;
    static int activeSlider = -1;

    SDL_Point mouse = {event.button.x, event.button.y};

    if (event.type == SDL_MOUSEBUTTONDOWN) {
        for (auto& panel : effectPanels) {
            if (SDL_PointInRect(&mouse, &panel.bounds)) {
                activePanel = &panel;

                int sliderAreaX = panel.bounds.x + 20;
                int spacing = (panel.bounds.w - 40) / panel.parameters.size();
                int sliderY = 80, sliderHeight = 120, sliderWidth = 20;

                for (size_t i = 0; i < panel.parameters.size(); ++i) {
                    SDL_Rect sliderRect = {sliderAreaX + static_cast<int>(i) * spacing, sliderY, sliderWidth, sliderHeight};
                    if (SDL_PointInRect(&mouse, &sliderRect)) {
                        activeSlider = i;
                    }
                }

                SDL_Rect toggleRect = {panel.bounds.x + panel.bounds.w / 2 - 25, panel.bounds.h - 50, 50, 30};
                if (SDL_PointInRect(&mouse, &toggleRect)) {
                    panel.enabled->store(!panel.enabled->load());
                }
            }
        }
    }

    if (event.type == SDL_MOUSEMOTION && activePanel && activeSlider != -1) {
        auto& param = activePanel->parameters[activeSlider];
        float norm = 1.0f - (event.motion.y - 80) / 120.0f;
        norm = std::clamp(norm, 0.0f, 1.0f);
        *(param.param) = param.min + norm * (param.max - param.min);
    }

    if (event.type == SDL_MOUSEBUTTONUP) {
        activePanel = nullptr;
        activeSlider = -1;
    }
}
