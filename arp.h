// arp.h
#ifndef ARP_H
#define ARP_H

#include <vector>

void Arp_Init(double sampleRate);
void Arp_Update(); // Call every frame
void Arp_AddNote(double freq);
void Arp_RemoveNote(double freq);
void Arp_SetBPM(int bpm);
void Arp_SetRate(int divisionsPerBeat); // e.g., 4 = 16th notes
void Arp_Enable(bool on);
bool Arp_IsActive();

#endif // ARP_H
