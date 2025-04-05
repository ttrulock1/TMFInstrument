// This ensures only one mode is active at a time (pad, ASDR, or effects).



void HandleGlobalKeyEvents(SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_TAB) {
            showPadMode = !showPadMode;
            showASDRMode = false;
            showEffectsMode = false;
        } else if (event.key.keysym.sym == SDLK_RETURN) {
            showEffectsMode = !showEffectsMode;
            showASDRMode = false;
            showPadMode = false;
        } else if (event.key.keysym.sym == SDLK_a) {
            showASDRMode = !showASDRMode;
            showEffectsMode = false;
            showPadMode = false;
        }
    }
}