// Filtered_Feedback_Drive.cpp
#include "Filtered_Feedback_Drive.h"
#include <cmath>
#include <algorithm>

FilteredFeedbackDrive::FilteredFeedbackDrive(double sampleRate)
    : sampleRate(sampleRate),
      driveAmount(0.0f),
      feedbackAmount(0.0f),
      steinerFilter(sampleRate) {}  // initialize steiner filter

void FilteredFeedbackDrive::setDrive(float drive) {
    driveAmount = std::clamp(drive, 0.0f, 1.0f);
}

void FilteredFeedbackDrive::setFeedbackAmount(float feedback) {
    feedbackAmount = std::clamp(feedback, 0.0f, 1.0f);
}

void FilteredFeedbackDrive::setCutoff(float cutoffHz) {
    steinerFilter.setCutoff(cutoffHz);
}

void FilteredFeedbackDrive::setResonance(float resonance) {
    steinerFilter.setResonance(resonance);
}

void FilteredFeedbackDrive::setVcaLevel(float level) {
    steinerFilter.setVcaLevel(level);
}

float FilteredFeedbackDrive::processSample(float input) {
    float gain = 1.0f + 20.0f * driveAmount;
    float saturated = std::tanh(input * gain);

    float filtered = steinerFilter.process(saturated);
    return filtered;
}

float FilteredFeedbackDrive::getFeedbackAmount() const {
    return feedbackAmount;
}
