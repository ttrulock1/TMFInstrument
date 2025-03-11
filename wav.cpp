#include <cstring>  // For memcpy()
#include <cstdio>
#include <cmath>
#include <cstdint>
#include "sound.h"

struct SMinimalWaveFileHeader {
    unsigned char m_szChunkID[4];
    uint32_t m_nChunkSize;
    unsigned char m_szFormat[4];
    unsigned char m_szSubChunk1ID[4];
    uint32_t m_nSubChunkSize;
    uint16_t m_nAudioFormat;
    uint16_t m_nNumChannels;
    uint32_t m_nSampleRate;
    uint32_t m_nByteRate;
    uint16_t m_nBlockAlign;
    uint16_t m_nBitsPerSample;
    unsigned char m_szSubChunk2ID[4];
    uint32_t m_nSubChunk2Size;
};

bool WriteWaveFile(const char *szFileName, void *pData, int32_t nDataSize, int16_t nNumChannels, int32_t nSampleRate, int32_t nBitsPerSample) {
    FILE *File = fopen(szFileName, "w+b");
    if (!File) return false;

    SMinimalWaveFileHeader waveHeader;
    memcpy(waveHeader.m_szChunkID, "RIFF", 4);
    waveHeader.m_nChunkSize = nDataSize + 36;
    memcpy(waveHeader.m_szFormat, "WAVE", 4);
    memcpy(waveHeader.m_szSubChunk1ID, "fmt ", 4);
    waveHeader.m_nSubChunkSize = 16;
    waveHeader.m_nAudioFormat = 1;
    waveHeader.m_nNumChannels = nNumChannels;
    waveHeader.m_nSampleRate = nSampleRate;
    waveHeader.m_nByteRate = nSampleRate * nNumChannels * nBitsPerSample / 8;
    waveHeader.m_nBlockAlign = nNumChannels * nBitsPerSample / 8;
    waveHeader.m_nBitsPerSample = nBitsPerSample;
    memcpy(waveHeader.m_szSubChunk2ID, "data", 4);
    waveHeader.m_nSubChunk2Size = nDataSize;

    fwrite(&waveHeader, sizeof(SMinimalWaveFileHeader), 1, File);
    fwrite(pData, nDataSize, 1, File);
    fclose(File);
    return true;
}

void GenerateSequence(int16_t* pData, int nNumSamples, double amp, sound::WaveType type, sound::ADSR env) {
    double stepDuration = 0.25;  // Each step is 250ms (4 steps per second)
    double frequencies[16] = { 261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25, 
                               587.33, 659.25, 698.46, 783.99, 880.00, 987.77, 1046.50, 1174.66 };

    bool stepActive[16] = { 1, 1, 0, 1,  0, 1, 1, 0,  1, 0, 1, 1,  0, 1, 0, 1 };  // On/Off pattern

    for (int nIndex = 0; nIndex < nNumSamples; ++nIndex) {
        double time = static_cast<double>(nIndex) / 44100.0;
        int step = static_cast<int>(time / stepDuration) % 16;

        if (stepActive[step]) {
            pData[nIndex] = sound::GenerateWave(type, time, stepDuration, frequencies[step], amp, env);
        } else {
            pData[nIndex] = 0;  // Silence when the step is off
        }
    }
}

int main() {
    int32_t nSampleRate = 44100;
    int nNumSeconds = 4;
    int16_t nNumChannels = 1;
    int nBitsPerSample = 16;
    double amplitude = 0.5;

    sound::WaveType selectedWave = sound::SAWTOOTH;

    sound::ADSR envelope = { 0.1, 0.2, 0.7, 0.2 };  // ADSR: Attack 100ms, Decay 200ms, Sustain 70%, Release 200ms

    int nNumSamples = nSampleRate * nNumChannels * nNumSeconds;
    int16_t* pData = new int16_t[nNumSamples];

    GenerateSequence(pData, nNumSamples, amplitude, selectedWave, envelope);
    WriteWaveFile("drone_sequencer.wav", pData, nNumSamples * sizeof(int16_t), nNumChannels, nSampleRate, nBitsPerSample);

    delete[] pData;
    return 0;
}