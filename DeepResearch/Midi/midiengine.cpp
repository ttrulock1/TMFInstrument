#include "midi_engine.h"
#include "midi_router.h"   // for forwarding input to router

#if USE_RTMIDI
// Include RtMidi implementation if available
#include "RtMidi.h"
#endif

// Static singleton instance pointer
static MidiEngine* g_midiEngineInstance = nullptr;

MidiEngine& MidiEngine::instance() {
    if (!g_midiEngineInstance) {
        g_midiEngineInstance = new MidiEngine();
    }
    return *g_midiEngineInstance;
}

MidiEngine::MidiEngine() {
    // Constructor does not open devices; call init() explicitly
}

MidiEngine::~MidiEngine() {
    close();
}

bool MidiEngine::init() {
    if (initialized) return true;
#if USE_RTMIDI
    try {
        // Create MIDI input and output instances
        midiIn  = new RtMidiIn();
        midiOut = new RtMidiOut();
    } catch (const std::exception &e) {
        std::cerr << "RtMidi initialization error: " << e.what() << std::endl;
        return false;
    }

    // Open the first available MIDI input port
    unsigned int nInputs = midiIn->getPortCount();
    if (nInputs == 0) {
        std::cerr << "No MIDI input devices found.\n";
    } else {
        try {
            midiIn->openPort(0);
        } catch (const std::exception &e) {
            std::cerr << "Failed to open MIDI input port: " << e.what() << std::endl;
        }
    }
    // Set callback for MIDI input to handle incoming messages
    try {
        midiIn->setCallback(&MidiEngine::midiInputCallback, nullptr);
        // Do not ignore any MIDI message types (include timing and sysex if any)
        midiIn->ignoreTypes(false, false, false);
    } catch (const std::exception &e) {
        std::cerr << "Failed to set MIDI input callback: " << e.what() << std::endl;
    }

    // Open the first available MIDI output port
    unsigned int nOutputs = midiOut->getPortCount();
    if (nOutputs == 0) {
        std::cerr << "No MIDI output devices found.\n";
    } else {
        try {
            midiOut->openPort(0);
        } catch (const std::exception &e) {
            std::cerr << "Failed to open MIDI output port: " << e.what() << std::endl;
        }
    }
#else  // USE_RTMIDI == 0 (e.g., on Switch)
    // ** Switch-specific initialization **
    // Here you would initialize the custom USB MIDI stack for Nintendo Switch.
    // For example, set up libnx USB host mode or TinyUSB host to detect MIDI devices.
    // This may involve starting a thread to poll USB for MIDI messages.
    // (Not implemented in this stub.)
    std::cout << "MidiEngine: Switch USB MIDI init stub.\n";
#endif

    initialized = true;
    return true;
}

void MidiEngine::close() {
#if USE_RTMIDI
    if (midiIn) {
        // Close and clean up input
        try { midiIn->cancelCallback(); } catch (...) {}
        try { midiIn->closePort(); } catch (...) {}
        delete midiIn;
        midiIn = nullptr;
    }
    if (midiOut) {
        try { midiOut->closePort(); } catch (...) {}
        delete midiOut;
        midiOut = nullptr;
    }
#endif
    initialized = false;
}

void MidiEngine::sendNoteOn(uint8_t note, uint8_t velocity) {
#if USE_RTMIDI
    if (midiOut && midiOut->isPortOpen()) {
        std::vector<uint8_t> msg(3);
        msg[0] = 0x90;              // Note On, channel 0
        msg[1] = note & 0x7F;
        msg[2] = velocity & 0x7F;
        midiOut->sendMessage(&msg);
    }
#else
    // ** Switch-specific Note On send stub **
    // Use USB host API to send a MIDI Note On (0x90) message to the device.
    // (Not implemented in this stub.)
#endif
}

void MidiEngine::sendNoteOff(uint8_t note, uint8_t velocity) {
#if USE_RTMIDI
    if (midiOut && midiOut->isPortOpen()) {
        std::vector<uint8_t> msg(3);
        msg[0] = 0x80;              // Note Off, channel 0
        msg[1] = note & 0x7F;
        msg[2] = velocity & 0x7F;
        midiOut->sendMessage(&msg);
    }
#else
    // ** Switch-specific Note Off send stub **
    // Send a MIDI Note Off (0x80) for the given note.
    // (Not implemented in this stub.)
#endif
}

// Static callback function to handle incoming MIDI messages (called on MIDI thread)
void MidiEngine::midiInputCallback(double /*deltaTime*/, std::vector<uint8_t> *message, void * /*userData*/) {
    if (!message || message->empty()) {
        return;
    }
    // Forward the raw MIDI message to the router for processing
    MidiRouter::processMidiMessage(*message);
}
