#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <iostream>
#include "shared_buffer.h"
#include "sequencemanager.h"
#include "presetmanager.h"
#include "oscillator_ui.h"
#include "adsr_ui.h"


SequenceManager sequenceManager;

enum DevMenuMode {
    MAIN_MENU,
    SEQUENCE_SAVE,
    SEQUENCE_LOAD,
    SYNTH_SAVE,
    SYNTH_LOAD
};

bool ShowDevMenu(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Event event;
    bool running = true;
    DevMenuMode mode = MAIN_MENU;
    int selectedSlot = 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) return false;

            if (event.type == SDL_KEYDOWN) {
                SDL_Keycode key = event.key.keysym.sym;

                if (key == SDLK_ESCAPE || key == SDLK_SPACE) {
                    if (mode == MAIN_MENU)
                        running = false;
                    else
                        mode = MAIN_MENU;
                }

                if (mode == MAIN_MENU) {
                    switch (key) {
                        case SDLK_1: mode = SYNTH_SAVE; break;
                        case SDLK_2: mode = SYNTH_LOAD; break;
                        case SDLK_3: mode = SEQUENCE_SAVE; break;
                        case SDLK_4: mode = SEQUENCE_LOAD; break;
                    }
                }

                else if (mode == SEQUENCE_SAVE || mode == SEQUENCE_LOAD) {
                    switch (key) {
                        case SDLK_LEFT:
                            selectedSlot = (selectedSlot + 47) % 48;
                            break;
                        case SDLK_RIGHT:
                            selectedSlot = (selectedSlot + 1) % 48;
                            break;
                        case SDLK_RETURN:
                        case SDLK_s:
                            sequenceManager.setActiveSequence(selectedSlot);
                            if (mode == SEQUENCE_SAVE) {
                                Sequence& seq = sequenceManager.getSequence(selectedSlot);
                                for (int i = 0; i < Sequence::MAX_STEPS; ++i) {
                                    seq.steps[i] = stepSequence[i];
                                    seq.pitchOffset[i] = stepPitches[i].load();
                                }
                                seq.scaleIndex = scaleBank.getSelectedScaleIndex();
                                seq.keyIndex = scaleBank.getSelectedKeyIndex();
                                seq.name = "Slot " + std::to_string(selectedSlot);
                                sequenceManager.SaveToFile("sequences.txt");
                                std::cout << "💾 Saved Sequence to slot " << selectedSlot << "\n";
                            } else {
                                sequenceManager.LoadFromFile("sequences.txt");
                                const Sequence& seq = sequenceManager.getSequence(selectedSlot);
                                for (int i = 0; i < Sequence::MAX_STEPS; ++i) {
                                    stepSequence[i] = seq.steps[i];
                                    stepPitches[i].store(seq.pitchOffset[i]);
                                }
                                scaleBank.setSelectedScale(seq.scaleIndex);
                                scaleBank.setSelectedKey(seq.keyIndex);
                                std::cout << "📥 Loaded Sequence slot " << selectedSlot << ": " << seq.name << "\n";
                            }
                            break;
                    }
                }

                else if (mode == SYNTH_SAVE || mode == SYNTH_LOAD) {
                    switch (key) {
                        case SDLK_LEFT:
                            selectedSlot = (selectedSlot + 8) % 9; // 9 preset slots
                            break;
                        case SDLK_RIGHT:
                            selectedSlot = (selectedSlot + 1) % 9;
                            break;
                        case SDLK_RETURN:
                        case SDLK_s:
                            {
                                std::string filename = "presets/custom" + std::to_string(selectedSlot) + ".mfpreset";
                                if (mode == SYNTH_SAVE) {
                                    SaveCurrentPreset(filename);
                                    std::cout << "💾 Saved Synth Preset to " << filename << "\n";
                                } else {
                                    if (LoadPreset(filename)) {
                                        RefreshOscillatorSliders();
                                        RefreshADSRFromBuffer(); 
                                        std::cout << "📥 Loaded Synth Preset from " << filename << "\n";
                                    } else {
                                        std::cout << "⚠️ Failed to load preset: " << filename << "\n";
                                    }
                                }
                            }
                            break;
                    }
                }
            }
        }

        // DRAW UI
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_Color white = {255, 255, 255, 255};
        std::string message;

        if (mode == MAIN_MENU) {
            message =
                "DEV MENU\n\n"
                "[1] Save Synth Preset →\n"
                "[2] Load Synth Preset →\n"
                "[3] Save Sequences →\n"
                "[4] Load Sequences →\n"
                "[ESC or SPACE] Exit";
        } else {
            std::string title;
            if (mode == SEQUENCE_SAVE) title = "SAVE SEQUENCE TO SLOT";
            else if (mode == SEQUENCE_LOAD) title = "LOAD SEQUENCE FROM SLOT";
            else if (mode == SYNTH_SAVE) title = "SAVE SYNTH PRESET";
            else if (mode == SYNTH_LOAD) title = "LOAD SYNTH PRESET";

            message = title + "\n\n";
            message += "← / →  Select Slot\n";
            message += "[Enter] to Confirm\n[ESC or SPACE] to Go Back\n\n";
            message += "▶ SLOT " + std::to_string(selectedSlot) + "\n";

            if (mode == SEQUENCE_SAVE || mode == SEQUENCE_LOAD) {
                const Sequence& seq = sequenceManager.getSequence(selectedSlot);
                message += "   Name: " + seq.name + "\n";
            } else {
                message += "   File: custom" + std::to_string(selectedSlot) + ".mfpreset\n";
            }
        }

        SDL_Surface* surface = TTF_RenderText_Blended_Wrapped(font, message.c_str(), white, 600);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect dst = {50, 50, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &dst);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }



    return true;
}
