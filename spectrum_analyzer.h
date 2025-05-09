#pragma once

#include <vector>
#include <SDL2/SDL.h>

class SpectrumAnalyzer {
public:
    SpectrumAnalyzer(int fftSize, int sampleRate);
    ~SpectrumAnalyzer();

    void pushSample(float sample);
    void updateSpectrum();
    void draw(SDL_Renderer* renderer, int x, int y, int width, int height);

private:
    int fftSize;
    int sampleRate;
    int writeIndex;
    bool bufferFull;

    std::vector<float> timeBuffer;
    std::vector<float> magnitudeSpectrum;

    void computeFFT();
};
