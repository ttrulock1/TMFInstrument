// pause_menu_ui.h
class PauseMenuUI {
public:
    PauseMenuUI(SDL_Renderer* renderer, SequenceManager& seqMan, PresetManager& preMan);
    void open();    // Activate the pause menu
    void close();   // Deactivate the menu
    void handleEvent(const SDL_Event& e);
    void update();  // Any animations or state updates
    void render();  // Draw the pause menu

private:
    SDL_Renderer* renderer;
    SequenceManager& seqManager;
    PresetManager& presetManager;
    bool isOpen;
    enum Page { SequencePage, PresetPage /*, maybe SettingsPage*/ } currentPage;
    int selectedIndex;        // index of currently highlighted slot on the current page
    // Graphics assets
    SDL_Texture* sequenceIconTex;
    SDL_Texture* presetIconTex;
    SDL_Texture* cursorTex;
    TTF_Font* font;
    // ... (positions, maybe precomputed rects for slots, etc.)
};
