#ifndef SEQUENCER_BACKEND_H
#define SEQUENCER_BACKEND_H

// SequencerBackend encapsulates the step sequencer's timing and state.
// This module is used by both the audio callback and the UI to maintain a unified sequencer state.
class SequencerBackend {
public:
    SequencerBackend();
    ~SequencerBackend();

    // Update the sequencer based on number of samples processed.
    // If a new step is triggered, returns true and outputs the step duration (in seconds) via outStepDuration.
    bool updateSequencer(int samplesProcessed, double& outStepDuration);

    // Reset sequencer state.
    void reset();

    // Get the current step index (0 to 15)
    int getCurrentStep() const;

private:
    int stepCounter;  // Counts samples within the current step
    int stepIndex;    // Current step (0-15)
};

#endif // SEQUENCER_BACKEND_H
