#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include "shared_buffer.h"

#include <iostream>
#include "sequencemanager.h"

SequenceManager sequenceManager;

enum DevMenuMode {
    MAIN_MENU,
    SEQUENCE_SAVE,
    SEQUENCE_LOAD
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

                // ESC always exits or returns to main menu
                if (key == SDLK_ESCAPE || key == SDLK_SPACE) {
                    if (mode == MAIN_MENU)
                        running = false;
                    else
                        mode = MAIN_MENU;
                }

                if (mode == MAIN_MENU) {
                    switch (key) {
                        case SDLK_3:
                            mode = SEQUENCE_SAVE;
                            break;
                        case SDLK_4:
                            mode = SEQUENCE_LOAD;
                            break;
                        // Add more entries here like SDLK_1, SDLK_2 for synth
                    }
                } else if (mode == SEQUENCE_SAVE || mode == SEQUENCE_LOAD) {
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
                                std::cout << "💾 Saved to slot " << selectedSlot << "\n";
                            } else if (mode == SEQUENCE_LOAD) {
                                sequenceManager.LoadFromFile("sequences.txt");

                                const Sequence& seq = sequenceManager.getSequence(selectedSlot);
                                for (int i = 0; i < Sequence::MAX_STEPS; ++i) {
                                    stepSequence[i] = seq.steps[i];
                                    stepPitches[i].store(seq.pitchOffset[i]);
                                }
                                scaleBank.setSelectedScale(seq.scaleIndex);
                                scaleBank.setSelectedKey(seq.keyIndex);

                                std::cout << "📥 Loaded slot " << selectedSlot << ": " << seq.name << "\n";

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
                "[1] Save Synth (disabled)\n"
                "[2] Load Synth (disabled)\n"
                "[3] Save Sequences →\n"
                "[4] Load Sequences →\n"
                "[ESC or SPACE] Exit";
        } else {
            std::string title = (mode == SEQUENCE_SAVE) ? "SAVE TO SLOT" : "LOAD FROM SLOT";
            Sequence& seq = sequenceManager.getSequence(selectedSlot);
            message = title + "\n\n";
            message += "← / →  Select Slot\n";
            message += "[Enter] to Confirm\n[ESC or SPACE] to Go Back\n\n";
            message += "▶ SLOT " + std::to_string(selectedSlot) + "\n";
            message += "   Name: " + seq.name + "\n";
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
