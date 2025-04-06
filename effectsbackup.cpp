//#include "effects.h"
// #include "shared_buffer.h"
// #include <string>
// #include <SDL2/SDL_ttf.h>
// #include <cmath>

// // 📁 Local helper
// float GetSliderValue(int index) {
//     switch (index) {
//         case 0: return delayTime.load();
//         case 1: return delayFeedback.load();
//         case 2: return delayMix.load();
//         case 3: return delayHighCut.load();
//         default: return 0.0f;
//     }
// }

// void SetSliderValue(int index, float value) {
//     switch (index) {
//         case 0: delayTime.store(value); break;
//         case 1: delayFeedback.store(value); break;
//         case 2: delayMix.store(value); break;
//         case 3: delayHighCut.store(value); break;
//     }
// }

// const char* sliderLabels[] = {"Time", "FB", "Mix", "HPF"};
// SDL_Rect sliders[4];

// void DrawEffectsUI(SDL_Renderer* renderer, TTF_Font* font) {
//     // 🎨 Full screen clear with Boss dark blue
//     SDL_SetRenderDrawColor(renderer, 70, 70, 150, 255);
//     SDL_RenderClear(renderer);

//     SDL_Color white = {255, 255, 255};
//     SDL_Color bossLightBlue = {117, 191, 255, 255}; // DD-7 style
//     SDL_Color bossDarkBlue = {0, 87, 158, 255};     // DD-3 style
//     SDL_Color lightGrey = {220, 220, 220, 255};     // UI panel background

//     // 🧱 Left 1/3 light grey panel
//     SDL_Rect leftPanel = {0, 0, 800 / 3, 400};
//     SDL_SetRenderDrawColor(renderer, lightGrey.r, lightGrey.g, lightGrey.b, lightGrey.a);
//     SDL_RenderFillRect(renderer, &leftPanel);

//     // Layout inside left third
//     int sliderAreaX = 20;
//     int sliderY = 80;
//     int sliderWidth = 20;
//     int sliderHeight = 120;
//     int spacing = 60;

//     for (int i = 0; i < 4; ++i) {
//         int sliderX = sliderAreaX + i * spacing;
//         float raw = GetSliderValue(i);
//         float valNorm = 0.0f;
//         switch (i) {
//             case 0: valNorm = raw / 1000.0f; break;                       // Time: 0–1000 ms
//             case 1: valNorm = raw; break;                                // Feedback: 0–1
//             case 2: valNorm = raw; break;                                // Mix: 0–1
//             case 3: valNorm = (raw - 100.0f) / 19900.0f; break;           // HPF: 100–20000 Hz
//         }
// valNorm = std::clamp(valNorm, 0.0f, 1.0f);        int fillHeight = static_cast<int>(valNorm * sliderHeight);

//         SDL_Rect bg = {sliderX, sliderY, sliderWidth, sliderHeight};
//         SDL_SetRenderDrawColor(renderer, bossDarkBlue.r, bossDarkBlue.g, bossDarkBlue.b, 255);
//         SDL_RenderFillRect(renderer, &bg);

//         SDL_Rect fill = {
//             sliderX,
//             sliderY + sliderHeight - fillHeight,
//             sliderWidth,
//             fillHeight
//         };
//         SDL_SetRenderDrawColor(renderer, bossLightBlue.r, bossLightBlue.g, bossLightBlue.b, 255);
//         SDL_RenderFillRect(renderer, &fill);

//         sliders[i] = bg;

//         SDL_Surface* textSurface = TTF_RenderText_Solid(font, sliderLabels[i], white);
//         SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, textSurface);
//         SDL_Rect labelRect = {sliderX - 5, sliderY + sliderHeight + 10, textSurface->w, textSurface->h};
//         SDL_RenderCopy(renderer, textTex, nullptr, &labelRect);
//         SDL_FreeSurface(textSurface);
//         SDL_DestroyTexture(textTex);
//     }

//     // 🎛 Delay toggle
//     SDL_Rect toggle = {sliderAreaX, sliderY + sliderHeight + 50, 50, 30};
//     SDL_SetRenderDrawColor(renderer, delayEnabled.load() ? bossLightBlue.r : 150,
//                                           delayEnabled.load() ? bossLightBlue.g : 150,
//                                           delayEnabled.load() ? bossLightBlue.b : 150,
//                                           255);
//     SDL_RenderFillRect(renderer, &toggle);

//     SDL_Surface* toggleSurface = TTF_RenderText_Solid(font, "Delay", white);
//     SDL_Texture* toggleTex = SDL_CreateTextureFromSurface(renderer, toggleSurface);
//     SDL_Rect toggleLabel = {toggle.x + 60, toggle.y + 5, toggleSurface->w, toggleSurface->h};
//     SDL_RenderCopy(renderer, toggleTex, nullptr, &toggleLabel);
//     SDL_FreeSurface(toggleSurface);
//     SDL_DestroyTexture(toggleTex);
// }

// void HandleEffectUIEvents(const SDL_Event& event) {
//     static int draggingSlider = -1;
//     int mx = event.button.x;
//     int my = event.button.y;

//     if (event.type == SDL_MOUSEBUTTONDOWN) {
//         for (int i = 0; i < 4; ++i) {
//             if (mx >= sliders[i].x && mx <= sliders[i].x + sliders[i].w &&
//                 my >= sliders[i].y && my <= sliders[i].y + sliders[i].h) {
//                 draggingSlider = i;
//             }
//         }

//         if (mx >= 20 && mx <= 70 && my >= 200 && my <= 230) {
//             delayEnabled.store(!delayEnabled.load());
//         }
//     }

//     if (event.type == SDL_MOUSEBUTTONUP) {
//         draggingSlider = -1;
//     }

//     if (event.type == SDL_MOUSEMOTION && draggingSlider != -1) {
//         float norm = 1.0f - static_cast<float>(my - sliders[draggingSlider].y) / sliders[draggingSlider].h;
//         norm = std::clamp(norm, 0.0f, 1.0f);
//         float value = 0.0f;
//         switch (draggingSlider) {
//             case 0: value = norm * 1000.0f; break;
//             case 1: value = norm; break;
//             case 2: value = norm; break;
//             case 3: value = 100.0f + norm * (20000.0f - 100.0f); break;

//         }
//         SetSliderValue(draggingSlider, value);
//     }
// }