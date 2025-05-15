// Filtered_Feedback_Drive.h
#ifndef FILTERED_FEEDBACK_DRIVE_H
#define FILTERED_FEEDBACK_DRIVE_H

#include "steiner_filter.h"

class FilteredFeedbackDrive {
public:
    explicit FilteredFeedbackDrive(double sampleRate = 44100.0);

    void setDrive(float driveAmount);           // 0.0 - 1.0
    void setFeedbackAmount(float feedback);     // 0.0 - 1.0

    void setCutoff(float cutoffHz);
    void setResonance(float resonance);
    void setVcaLevel(float level);

    float processSample(float input);
    float getFeedbackAmount() const;

private:
    double sampleRate = 44100.0;
    float driveAmount = 0.0f;
    float feedbackAmount = 0.0f;
    float resonance = 0.0f;      // ✅ add this
    float vca = 1.0f;            // ✅ and this

    SteinerFilter steinerFilter;
};

#endif // FILTERED_FEEDBACK_DRIVE_H
