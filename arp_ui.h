// arp_ui.h
#ifndef ARP_UI_H
#define ARP_UI_H

#include <SDL2/SDL.h>

void ArpUI_Init();
void ArpUI_HandleEvent(SDL_Event& event);
void ArpUI_Draw(SDL_Renderer* renderer);

#endif // ARP_UI_H