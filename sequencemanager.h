#include <array>
#include <string>


// sequence_manager.h
struct Sequence {
    std::string name;              // Name or label of the sequence (optional)
    static const int MAX_STEPS = 16;   // Number of steps per sequence (e.g. 16 or 48)
    bool steps[MAX_STEPS];         // Step on/off state for each position
    int pitchOffset[MAX_STEPS];    // (Optional) pitch offset per step (e.g. in semitones)
    int scaleIndex;                // Index of the scale used (in a ScaleBank list)
    int keyIndex;                  // Index of the musical key (0=C,1=C#,...,11=B)
};

// sequence_manager.h
class SequenceManager {
public:
    SequenceManager();
    Sequence& getSequence(int index);
    void setActiveSequence(int index);
    int  getActiveSequenceIndex() const;
    // ...other methods for editing sequences...
    bool SaveToFile(const std::string& filename);
    bool LoadFromFile(const std::string& filename);


private:
    std::array<Sequence, 48> sequences;
    int activeSequenceIndex;
};
