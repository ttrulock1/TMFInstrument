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

void SpectrumAnalyzer::draw(SDL_Renderer* renderer, int x, int y, int width, int height, float cutoffHz, float resonance) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect rect = {x, y, width, height};
    SDL_RenderFillRect(renderer, &rect);

    float nyquist = float(sampleRate) / 2.0f;
    float fcNorm = cutoffHz / nyquist;

    // Draw spectrum bars
    float maxMag = 1e-6f;
    for (auto mag : magnitudeSpectrum) maxMag = std::max(maxMag, mag);

    for (int i = 1; i < fftSize / 2; ++i) {
        float normFreq = float(i) / (fftSize / 2.0f);
        float freqHz = normFreq * nyquist;

        float db = 20.0f * std::log10(std::max(magnitudeSpectrum[i], 0.0001f));
        float dbNorm = (db + 60.0f) / 60.0f;
        dbNorm = std::clamp(dbNorm, 0.0f, 1.0f);
        int barHeight = static_cast<int>(dbNorm * height);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderDrawLine(renderer, x + i, y + height, x + i, y + height - barHeight);
    }

    // Draw filter overlay (yellow)
    for (int i = 1; i < fftSize / 2; ++i) {
        float binFreq = float(i) / (fftSize / 2.0f);
        float response = 1.0f / (1.0f + std::pow(binFreq / fcNorm, 6.0f));
        float bump = resonance * std::exp(-std::pow((binFreq - fcNorm) * 12.0f, 2.0f));
        float total = std::clamp(response + bump, 0.0f, 1.0f);
        int overlayHeight = static_cast<int>(total * height);

        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderDrawPoint(renderer, x + i, y + height - overlayHeight);
    }

    // Draw vertical cutoff handle line
    int cutoffX = x + static_cast<int>(fcNorm * width);
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderDrawLine(renderer, cutoffX, y, cutoffX, y + height);

    // Optional handle circle
    SDL_Rect circle = {cutoffX - 3, y + height - 10, 6, 6};
    SDL_RenderFillRect(renderer, &circle);
}

