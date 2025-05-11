#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "spectrum_analyzer.h"
#include "Moog_Filter.h"
#include <cmath>
#include <complex>

SpectrumAnalyzer::SpectrumAnalyzer(int fftSize, int sampleRate)
    : fftSize(fftSize), sampleRate(sampleRate), writeIndex(0), bufferFull(false)
{
    timeBuffer.resize(fftSize, 0.0f);
    magnitudeSpectrum.resize(fftSize / 2, 0.0f);
}

SpectrumAnalyzer::~SpectrumAnalyzer() {}

void SpectrumAnalyzer::pushSample(float sample)
{
    timeBuffer[writeIndex++] = sample;
    if (writeIndex >= fftSize)
    {
        writeIndex = 0;
        bufferFull = true;
    }
}

void SpectrumAnalyzer::updateSpectrum()
{
    if (!bufferFull)
        return;
    computeFFT();
}

void SpectrumAnalyzer::computeFFT()
{
    for (int k = 0; k < fftSize / 2; ++k)
    {
        std::complex<float> sum = 0;
        for (int n = 0; n < fftSize; ++n)
        {
            float angle = -2.0f * M_PI * k * n / fftSize;
            sum += timeBuffer[n] * std::exp(std::complex<float>(0, angle));
        }
        magnitudeSpectrum[k] = std::abs(sum);
    }
}

void SpectrumAnalyzer::draw(SDL_Renderer *renderer, int x, int y, int width, int height, float cutoffHz, float resonance)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect rect = {x, y, width, height};
    SDL_RenderFillRect(renderer, &rect);

    float maxMag = 1e-6f;
    for (auto mag : magnitudeSpectrum)
        maxMag = std::max(maxMag, mag);

    for (int i = 0; i < fftSize / 2; ++i)
    {
        float norm = magnitudeSpectrum[i] / maxMag;
        float db = 20.0f * std::log10(std::max(magnitudeSpectrum[i], 0.0001f)); // Convert to decibels
        float normalized = (db + 60.0f) / 60.0f;                                // Map -60 dB → 0.0 and 0 dB → 1.0
        normalized = std::clamp(normalized, 0.0f, 1.0f);
        int barHeight = static_cast<int>(normalized * height);
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderDrawLine(renderer, x + i, y + height, x + i, y + height - barHeight);
    }
    // 🔶 FILTER OVERLAY LINE
    float cutoffBin = (cutoffHz / (sampleRate / 2.0f)) * (fftSize / 2.0f);

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // yellow line

    for (int i = 1; i < fftSize / 2; ++i)
    {
        float binRatio = float(i) / (fftSize / 2.0f);
        float freqHz = binRatio * (sampleRate / 2.0f);

        float response = 1.0f / (1.0f + std::pow(freqHz / cutoffHz, 4.0f));             // LP rolloff
        float resBoost = resonance * std::exp(-std::pow((i - cutoffBin) / 5.0f, 2.0f)); // bump
        float total = response + resBoost;

        int barHeight = static_cast<int>(std::clamp(total, 0.0f, 1.0f) * height);
        SDL_RenderDrawPoint(renderer, x + i, y + height - barHeight);
    }
}
