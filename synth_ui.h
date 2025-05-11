#pragma once
#include <SDL.h>
#include "spectrum_analyzer.h"


void HandleSynthEditorEvents(SDL_Event& event);
void DrawSynthEditor(SDL_Renderer* renderer);
extern SpectrumAnalyzer analyzer;
