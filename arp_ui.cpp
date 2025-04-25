// arp_ui.cpp
#include "arp_ui.h"
#include "arp.h"
#include <SDL2/SDL.h>
#include <string>

static SDL_Rect toggleRect = {20, 20, 100, 30};
static SDL_Rect modeRect   = {20, 60, 100, 30};
static SDL_Rect rateRect   = {20, 100, 100, 30};

static int currentMode = 0;
static int currentRate = 4; // 16th notes
static bool arpEnabled = true;

void ArpUI_Init() {


    Arp_Enable(true);
    Arp_SetMode(currentMode);
    Arp_SetRate(currentRate);
    // Inject a C major triad for test
        Arp_AddNote(261.63); // C4
        Arp_AddNote(329.63); // E4
        Arp_AddNote(392.00); // G4
        Arp_Enable(true);    // Turn on the arpeggiator for testing
}

void ArpUI_HandleEvent(SDL_Event& event) {
    if (event.type != SDL_MOUSEBUTTONDOWN) return;

    int mx = event.button.x;
    int my = event.button.y;
    SDL_Point point = {mx, my};

    if (SDL_PointInRect(&point, &toggleRect)) {
        arpEnabled = !arpEnabled;
        Arp_Enable(arpEnabled);
    } else if (SDL_PointInRect(&point, &modeRect)) {
        currentMode = (currentMode + 1) % 4;
        Arp_SetMode(currentMode);
    } else if (SDL_PointInRect(&point, &rateRect)) {
        int rates[] = {1, 2, 4, 8};
        static int rateIndex = 2;
        rateIndex = (rateIndex + 1) % 4;
        currentRate = rates[rateIndex];
        Arp_SetRate(currentRate);
    }
}

void ArpUI_Draw(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);

    SDL_RenderFillRect(renderer, &toggleRect);
    SDL_RenderFillRect(renderer, &modeRect);
    SDL_RenderFillRect(renderer, &rateRect);

    // In a real UI, render text using SDL_ttf or simple bitmap font system
    // Placeholder rectangles used here for buttons

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &toggleRect);
    SDL_RenderDrawRect(renderer, &modeRect);
    SDL_RenderDrawRect(renderer, &rateRect);
}
