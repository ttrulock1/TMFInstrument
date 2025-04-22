#ifndef SAVE_SWITCH_H
#define SAVE_SWITCH_H

#include <string>

// Save all synth state to persistent storage (Switch or PC)
bool SaveSynthState();

// Load all synth state from persistent storage (Switch or PC)
bool LoadSynthState();

#endif // SAVE_SWITCH_H
