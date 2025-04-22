#include "midi_router.h"
#include "midi_engine.h"
#include <iostream>

// Convert a MIDI note number to frequency (Hz) using A4=440Hz reference
static double midiNoteToFrequency(int midiNote) {
    return 440.0 * std::pow(2.0, (midiNote - 69) / 12.0);
}

void MidiRouter::processMidiMessage(const std::vector<uint8_t>& message) {
    if (message.empty()) return;
    uint8_t status = message[0];
    uint8_t type   = status & 0xF0;    // high nibble identifies message type (NoteOn, etc.)
    uint8_t channel= status & 0x0F;    // low nibble is MIDI channel (0-15)

    // Only handle messages on channel 0 (MIDI channel 1) for now
    if (channel != 0) {
        return;
    }

    if (type == 0x90 || type == 0x80) {
        // Note On or Note Off event
        uint8_t note     = message.size() > 1 ? message[1] : 0;
        uint8_t velocity = message.size() > 2 ? message[2] : 0;
        if (type == 0x90 && velocity > 0) {
            // Note On with nonzero velocity
            double freq = midiNoteToFrequency(note);
            // Create a NoteEvent for padVoice
            NoteEvent evt;
            evt.frequency = freq;
            evt.duration  = 0.0;  // duration can be set 0; envelope sustain will hold note
            evt.env = ADSR(uiAttackTime.load(), uiDecayTime.load(),
                           uiSustainLevel.load(), uiReleaseTime.load());
            // Push the event into the pad note queue (to be picked up by audio thread)
            padNoteEvents.push(evt);
            std::cout << "MIDI IN: Note On " << (int)note << " (vel " 
                      << (int)velocity << ") -> freq " << freq << " Hz\n";
        } else {
            // Note Off event (or Note On with velocity 0 treated as Note Off)
            NoteEvent evt;
            evt.frequency = -1.0;   // convention: negative frequency signals note-off
            evt.duration  = 0.0;
            // The specific note to turn off isn't tracked per voice (single padVoice design),
            // so we just signal a general pad note off.
            padNoteEvents.push(evt);
            std::cout << "MIDI IN: Note Off " << (int)note << "\n";
        }
    } else {
        // Other MIDI message types can be handled here (e.g., CC, Pitch Bend).
        // For now, ignore them or implement as needed.
    }
}

void MidiRouter::handleSequencerStep(bool stepActive, int pitchOffset) {
    // static state to track last active MIDI note for sequencer
    static int lastMidiNote = -1;
    if (stepActive) {
        // Compute MIDI note number relative to A4 (69) for the given pitch offset
        int midiNote = 69 + pitchOffset;
        // If a note was previously on, send it a Note Off before starting a new one
        if (lastMidiNote != -1) {
            MidiEngine::instance().sendNoteOff(static_cast<uint8_t>(lastMidiNote));
        }
        // Send new Note On for this step's note
        MidiEngine::instance().sendNoteOn(static_cast<uint8_t>(midiNote), 127);
        lastMidiNote = midiNote;
    } else {
        // No note on this step: if the previous step had a note, turn it off
        if (lastMidiNote != -1) {
            MidiEngine::instance().sendNoteOff(static_cast<uint8_t>(lastMidiNote));
            lastMidiNote = -1;
        }
    }
}
