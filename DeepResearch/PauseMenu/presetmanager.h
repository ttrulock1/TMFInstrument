// preset_manager.h
struct Preset {
    std::string name;
    // Oscillator / wavetable settings
    int waveformIndex;      // e.g., 0 = Sine, 1 = Square, 2 = Saw, etc.
    int wavetableIndex;     // if using wavetable synth, an index for which table
    // Envelope (ADSR) settings
    float attack;
    float decay;
    float sustain;
    float release;
    // LFO settings
    float lfoRate;
    float lfoDepth;
    int   lfoTarget;        // what the LFO modulates (0=amp,1=pitch,etc.)
    // Effect settings
    bool  delayEnabled;
    float delayTime;
    float delayFeedback;
    float delayMix;
    bool  reverbEnabled;
    float reverbLevel;
    // ... any other synth parameters (chorus, filters, etc.)
};
