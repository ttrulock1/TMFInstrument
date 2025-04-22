#include "sequencer_backend.h"
#include "shared_buffer.h"  // For BPM; assumes BPM is atomic<int>
#include <cmath>
#include <algorithm>

SequencerBackend::SequencerBackend()
    : stepCounter(0), stepIndex(0)
{
}

SequencerBackend::~SequencerBackend() {
    // Nothing to clean up
}

void SequencerBackend::reset() {
    stepCounter = 0;
    stepIndex = 0;
}

bool SequencerBackend::updateSequencer(int samplesProcessed, double& outStepDuration) {
    // Compute step length in samples: step = (SAMPLE_RATE * 60) / (BPM * 4)
    int bpm = BPM.load();
    int stepLength = static_cast<int>((SAMPLE_RATE * 60.0) / (bpm * 4));
    outStepDuration = static_cast<double>(stepLength) / SAMPLE_RATE;

    stepCounter += samplesProcessed;
    if (stepCounter >= stepLength) {
        stepCounter %= stepLength; // keep remainder
        stepIndex = (stepIndex + 1) % 16;
        return true;  // A new step has been triggered
    }
   
