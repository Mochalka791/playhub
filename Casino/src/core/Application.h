#pragma once
#include <string>
#include <memory>

// Forward declarations to avoid pulling in SDL / OpenGL headers here
struct SDL_Window;
typedef void* SDL_GLContext;

class StateManager;
class Player;

class Application
{
public:
    Application();
    ~Application();

    bool init(const std::string& title, int width, int height);
    void run();
    void stop() { running = false; }

    StateManager& getStateManager() { return *stateManager; }
    int getWidth()  const { return width; }
    int getHeight() const { return height; }

    // Player ownership (set from LoginState)
    Player*      getPlayer() const { return player.get(); }
    void         setPlayer(std::unique_ptr<Player> p);

    void         setFullscreen(bool on);

private:
    SDL_Window*   window    = nullptr;
    SDL_GLContext glContext  = nullptr;
    bool          running   = false;
    int           width     = 1024;
    int           height    = 768;

    std::unique_ptr<StateManager> stateManager;
    std::unique_ptr<Player>       player;

    bool initSDL(const std::string& title);
    bool initImGui();
    void processEvents();
    void newFrame();
    void endFrame();
    void shutdown();
};
