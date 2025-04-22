// === effects_backend.h ===
#ifndef EFFECTS_BACKEND_H
#define EFFECTS_BACKEND_H

#include <atomic>

void InitEffects(float sampleRate);
float ApplyEffects(float inputSample);

// Public effect parameters
extern std::atomic<bool> reverbEnabled;
extern std::atomic<float> reverbDecay;
extern std::atomic<float> reverbDamping;
extern std::atomic<float> reverbMix;

extern std::atomic<bool> delayEnabled;
extern std::atomic<float> delayTime;
extern std::atomic<float> delayFeedback;
extern std::atomic<float> delayMix;

extern std::atomic<bool> chorusEnabled;
extern std::atomic<float> chorusRate;
extern std::atomic<float> chorusDepth;
extern std::atomic<float> chorusMix;

#endif

// === effects_backend.cpp ===
#include "effects_backend.h"
#include "delay.h"
#include "reverb.h"
#include "chorus.h"

static Delay delayEffect(44100, 1000);
static Reverb reverbEffect(44100);
static Chorus chorusEffect(44100);

std::atomic<bool> reverbEnabled{true};
std::atomic<float> reverbDecay{0.5f};
std::atomic<float> reverbDamping{0.2f};
std::atomic<float> reverbMix{0.3f};

std::atomic<bool> delayEnabled{true};
std::atomic<float> delayTime{400};
std::atomic<float> delayFeedback{0.5f};
std::atomic<float> delayMix{0.4f};

std::atomic<bool> chorusEnabled{true};
std::atomic<float> chorusRate{1.2f};
std::atomic<float> chorusDepth{0.8f};
std::atomic<float> chorusMix{0.4f};

void InitEffects(float sampleRate) {
    delayEffect = Delay(sampleRate, 1000);
    reverbEffect = Reverb(sampleRate);
    chorusEffect = Chorus(sampleRate);
}

float ApplyEffects(float inputSample) {
    float sample = inputSample;
    if (chorusEnabled.load()) {
        chorusEffect.setRate(chorusRate);
        chorusEffect.setDepth(chorusDepth);
        chorusEffect.setMix(chorusMix);
        sample = chorusEffect.process(sample);
    }

    if (delayEnabled.load()) {
        delayEffect.setDelayTime(static_cast<int>(delayTime));
        delayEffect.setFeedback(delayFeedback);
        delayEffect.setMix(delayMix);
        sample = delayEffect.process(sample);
    }

    if (reverbEnabled.load()) {
        reverbEffect.setDecay(reverbDecay);
        reverbEffect.setDamping(reverbDamping);
        reverbEffect.setMix(reverbMix);
        sample = reverbEffect.process(sample);
    }

    return sample;
}

// === sequencer_backend.h ===
#ifndef SEQUENCER_BACKEND_H
#define SEQUENCER_BACKEND_H

#include <atomic>
#include <vector>
#include <cstdint>

void InitSequencer(int sampleRate);
void AdvanceSequencer();
bool ShouldTriggerStep();
float GetSequencerFrequency();

extern std::atomic<bool> stepSequence[16];
extern std::atomic<int> stepPitches[16];
extern std::atomic<int> BPM;

#endif

// === sequencer_backend.cpp ===
#include "sequencer_backend.h"
#include <cmath>

static int stepIndex = 0;
static int stepCounter = 0;
static int stepLength = 0;
static int sampleRate = 44100;

std::atomic<bool> stepSequence[16];
std::atomic<int> stepPitches[16];
std::atomic<int> BPM{120};

void InitSequencer(int sr) {
    sampleRate = sr;
    stepIndex = 0;
    stepCounter = 0;
    stepLength = static_cast<int>((sampleRate * 60.0f) / (BPM * 4));
}

void AdvanceSequencer() {
    stepCounter++;
    if (stepCounter >= stepLength) {
        stepCounter = 0;
        stepIndex = (stepIndex + 1) % 16;
    }
}

bool ShouldTriggerStep() {
    return stepCounter == 0 && stepSequence[stepIndex].load();
}

float GetSequencerFrequency() {
    int pitchOffset = stepPitches[stepIndex].load();
    return 440.0f * std::pow(2.0f, pitchOffset / 12.0f);
}
