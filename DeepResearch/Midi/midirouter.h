#ifndef MIDI_ROUTER_H
#define MIDI_ROUTER_H

#include <cmath>
#include <vector>
#include <cstdint>
#include "shared_buffer.h"   // includes NoteEvent, padNoteEvents, stepSequence, etc.

class MidiRouter {
public:
    // Process a raw MIDI message (called from MidiEngine callback thread)
    static void processMidiMessage(const std::vector<uint8_t>& message);

    // Handle step sequencer output routing (called from audio thread each step)
    static void handleSequencerStep(bool stepActive, int pitchOffset);
};

#endif // MIDI_ROUTER_H
