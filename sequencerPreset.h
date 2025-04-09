struct SynthPreset {
    std::string name;

    // Sequencer
    SequencerPreset sequencer;
};

struct SequencerPreset {
    std::string name;
    int notes[16]; // MIDI notes or 0 for rests
    int key
    
};
