#include "spectrum_analyzer.h"
#include <cmath>
#include <complex>

SpectrumAnalyzer::SpectrumAnalyzer(int fftSize, int sampleRate)
    : fftSize(fftSize), sampleRate(sampleRate), writeIndex(0), bufferFull(false)
{
    timeBuffer.resize(fftSize, 0.0f);
    magnitudeSpectrum.resize(fftSize / 2, 0.0f);
}

SpectrumAnalyzer::~SpectrumAnalyzer() {}

void SpectrumAnalyzer::pushSample(float sample) {
    timeBuffer[writeIndex++] = sample;
    if (writeIndex >= fftSize) {
        writeIndex = 0;
        bufferFull = true;
    }
}

void SpectrumAnalyzer::updateSpectrum() {
    if (!bufferFull) return;
    computeFFT();
}

void SpectrumAnalyzer::computeFFT() {
    for (int k = 0; k < fftSize / 2; ++k) {
        std::complex<float> sum = 0;
        for (int n = 0; n < fftSize; ++n) {
            float angle = -2.0f * M_PI * k * n / fftSize;
            sum += std::polar(timeBuffer[n], angle);
        }
        magnitudeSpectrum[k] = std::abs(sum);
    }
}

void SpectrumAnalyzer::draw(SDL_Renderer* renderer, int x, int y, int width, int height) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &(SDL_Rect){x, y, width, height});

    float maxMag = 1e-6f;
    for (auto mag : magnitudeSpectrum) maxMag = std::max(maxMag, mag);

    for (int i = 0; i < fftSize / 2; ++i) {
        float norm = magnitudeSpectrum[i] / maxMag;
        int barHeight = static_cast<int>(norm * height);
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderDrawLine(renderer, x + i, y + height, x + i, y + height - barHeight);
    }
}
