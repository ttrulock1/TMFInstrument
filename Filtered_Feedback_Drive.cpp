#include "Filtered_Feedback_Drive.h"
#include "steiner_filter.h"
#include <cmath>
#include <algorithm>

FilteredFeedbackDrive::FilteredFeedbackDrive(double sampleRate)
    : steinerFilter(sampleRate) {}

void FilteredFeedbackDrive::setDrive(float drive) {
    driveAmount = std::clamp(drive, 0.0f, 1.0f);
}

void FilteredFeedbackDrive::setFeedbackAmount(float feedback) {
    feedbackAmount = std::clamp(feedback, 0.0f, 1.0f);
}

void FilteredFeedbackDrive::setCutoff(float cutoffHz) {
    steinerFilter.setCutoff(cutoffHz);  // ✅ Steiner only uses cutoff directly
}

void FilteredFeedbackDrive::setResonance(float r) {
    resonance = std::clamp(r, 0.0f, 1.0f);  // ✅ Stored in this class
}

void FilteredFeedbackDrive::setVcaLevel(float level) {
    vca = std::clamp(level, 0.0f, 2.0f);    // ✅ Stored in this class
}

float FilteredFeedbackDrive::processSample(float input) {
    // they had me change this and it made it softer.
    float feedbackInput = input * feedbackAmount;
    // float feedbackInput = input;  // don't attenuate input here
    float gain = 1.0f + 25.0f * driveAmount;
    float saturated = std::tanh(feedbackInput * gain);

    // 🔁 This feeds the *adjusted* signal into the Steiner filter with feedback-controlled input
    return steinerFilter.process(saturated, resonance, vca);
    
}

float FilteredFeedbackDrive::getFeedbackAmount() const {
    return feedbackAmount;
}
