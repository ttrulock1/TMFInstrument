// this structure is the minimal required header data for a wav file.
struct SMinimalWaveFileHeader
{
    //the main chunk
    unsigned char m_szChunkID[4];
    uint32 m_nChunkSize;
    unsigned char m_szFormat[4];

    //sub chunk 1 "fmt "
    unsigned char m_szSubChunk1ID[4];
    uint32 m_nSubChunkSize;
    uint16 m_nAudioFormat;
    uint16 m_nNumChannels;
    uint32 m_nSampleRate;
    uint32 m_nByteRate;
    uint16 m_nBlockAlign;
    uint16 m_nBitsPersample;

    //sub chunk 2 "data"
    unsigned char m_szSubChunk2ID[4];

    uint32 m_nChunkSize;
    unsigned char m_szFormat[4];

    //sub chunk 1 "fmt "
    unsigned char m_szSubChunk1ID[4];
    uint32 m_nSubChunkSize;
    uint16 m_nAudioFormat;
    uint16 m_nNumChannels;
    uint32 m_nSampleRate;
    uint32 m_nByteRate;
    uint16 m_nBlockAlign;
    uint16 m_nBitsPersample;
};

bool WriteWaveFile(const char *szFileName, void *pData, int32 nDataSize, int16 nNumChannels, int32 nSampleRate, int32 nBitsPersample)
{
    // Open the file if we can
    FILE *File = fopen(szFileName, "w+b");
    if(!File){
        return false;
    }
    SMinimalWaveFileHeader waveHeader;
    //fill out the main chunk
    memcpy(waveHeader.m_szChunkID,"Riff", 4);
    waveHeader.m_nChunkSize = nDataSize + 36;
    memcpy(waveHeader.m_szFormat, "Wave", 4);

    // fill out sub chunk 1 "fmt"
    memcpy(waveHeader.m_szSubChunk1ID, "FMT", 4);
    waveHeader.m_nSubChunkSize= 16;
    waveHeader.m_nAudioFormat=1;
    waveHeader.m_nNumChannels= nNumChannels;
    waveHeader.m_nSampleRate= nSampleRate;
    waveHeader.m_nByteRate = nSampleRate * nNumChannels * nBitsPersample / 8;
    waveHeader

}