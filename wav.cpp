#include <cstdio>
#include <cstring>
#include <cstdint>

struct SMinimalWaveFileHeader
{
    // main chunk
    unsigned char m_szChunkID[4];
    uint32_t m_nChunkSize;
    unsigned char m_szFormat[4];

    // subchunk 1 "fmt "
    unsigned char m_szSubChunk1ID[4];
    uint32_t m_nSubChunkSize;
    uint16_t m_nAudioFormat;
    uint16_t m_nNumChannels;
    uint32_t m_nSampleRate;
    uint32_t m_nByteRate;
    uint16_t m_nBlockAlign;
    uint16_t m_nBitsPerSample;

    // subchunk 2 "data"
    unsigned char m_szSubChunk2ID[4];
    uint32_t m_nSubChunk2Size;
};

bool WriteWaveFile(const char *szFileName, void *pData, int32_t nDataSize, int16_t nNumChannels, int32_t nSampleRate, int32_t nBitsPerSample)
{
    // Open the file
    FILE *File = fopen(szFileName, "w+b");
    if (!File) {
        return false;
    }

    SMinimalWaveFileHeader waveHeader;
    // Fill out the main chunk
    memcpy(waveHeader.m_szChunkID, "RIFF", 4);
    waveHeader.m_nChunkSize = nDataSize + 36;
    memcpy(waveHeader.m_szFormat, "WAVE", 4);

    // Fill out subchunk 1 "fmt "
    memcpy(waveHeader.m_szSubChunk1ID, "fmt ", 4);
    waveHeader.m_nSubChunkSize = 16;
    waveHeader.m_nAudioFormat = 1; // PCM format
    waveHeader.m_nNumChannels = nNumChannels;
    waveHeader.m_nSampleRate = nSampleRate;
    waveHeader.m_nByteRate = nSampleRate * nNumChannels * nBitsPerSample / 8;
    waveHeader.m_nBlockAlign = nNumChannels * nBitsPerSample / 8;
    waveHeader.m_nBitsPerSample = nBitsPerSample;

    // Fill out subchunk 2 "data"
    memcpy(waveHeader.m_szSubChunk2ID, "data", 4);
    waveHeader.m_nSubChunk2Size = nDataSize;

    // Write the header
    fwrite(&waveHeader, sizeof(SMinimalWaveFileHeader), 1, File);

    // Write the wave data itself
    fwrite(pData, nDataSize, 1, File);

    // Close the file and return success
    fclose(File);
    return true;
}

// Example usage
int main() {
    int32_t nSampleRate = 44100;
    int nNumSeconds = 4;
    int16_t nNumChannels = 1;
    int nBitsPerSample = 16;

    int nNumSamples = nSampleRate * nNumChannels * nNumSeconds;
    int16_t *pData = new int16_t[nNumSamples];

    // Generate a simple waveform (sawtooth wave as an example)
    int16_t nValue = 0;
    for (int nIndex = 0; nIndex < nNumSamples; ++nIndex) {
        nValue = (nValue + 8000) % 32767; // Increment to create a basic wave pattern
        pData[nIndex] = nValue;
    }

    // Write to a WAV file
    WriteWaveFile("outmono.wav", pData, nNumSamples * sizeof(int16_t), nNumChannels, nSampleRate, nBitsPerSample);

    delete[] pData;
    return 0;
}