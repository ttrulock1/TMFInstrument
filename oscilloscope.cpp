#include "oscilloscope.h"

Oscilloscope::Oscilloscope(int maxSamples) : maxSamples(maxSamples) {}

void Oscilloscope::push(float value) {
    if (buffer.size() >= maxSamples) {
        buffer.erase(buffer.begin());
    }
    buffer.push_back(value);
}

void Oscilloscope::draw(SDL_Renderer* renderer, SDL_Rect area, SDL_Color color) {
    if (buffer.empty()) return;

    // Draw the oscilloscope wave (scaled to the area)
    int w = area.w;
    int h = area.h;
    int x = area.x;
    int y = area.y + h / 2;  // center y position for waveform

    // Set the color for the waveform
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    // Draw the waveform line
    for (size_t i = 1; i < buffer.size(); ++i) {
        int x0 = x + (i - 1) * (w / maxSamples);
        int y0 = y - (buffer[i - 1] * (h / 2));
        int x1 = x + i * (w / maxSamples);
        int y1 = y - (buffer[i] * (h / 2));
        SDL_RenderDrawLine(renderer, x0, y0, x1, y1);
    }
}
