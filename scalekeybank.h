#pragma once
#include <vector>
#include <string>
#include <cmath>

struct Scale {
    std::string name;
    std::vector<int> intervals;
};

class ScaleBank {
public:
    ScaleBank();

    int getSelectedKey() const { return selectedKeyIndex; }


    void addScale(const std::string& name, const std::vector<int>& intervals);
    void setSelectedScale(int index);
    void setSelectedKey(int index);
    double applyScale(double frequency) const;
    void nextScale();
    void nextKey();
    int getSelectedScaleIndex() const;
    int getSelectedKeyIndex() const;
    const Scale& getSelectedScale() const;
    std::string getSelectedKeyName() const;
    int getScaleCount() const;
    std::string getScaleName(int index) const;

private:
    std::vector<Scale> scales;
    int selectedScaleIndex;
    int selectedKeyIndex;
};
