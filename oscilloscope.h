#pragma once
#include <vector>
#include <SDL2/SDL.h>

class Oscilloscope {
public:
    Oscilloscope(int maxSamples = 256);  // max samples in the buffer
    void push(float value);              // add LFO value to the buffer
    void draw(SDL_Renderer* renderer, SDL_Rect area, SDL_Color color); // render the oscilloscope

private:
    std::vector<float> buffer;
    int maxSamples;
};
