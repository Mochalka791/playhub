#include "Application.h"
#include "StateManager.h"
#include "../states/MenuState.h"
#include "../models/Player.h"
#include "../ui/Theme.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include <stdexcept>
#include <string>

Application::Application()
    : stateManager(std::make_unique<StateManager>())
{}

Application::~Application()
{
    shutdown();
}

bool Application::init(const std::string& title, int w, int h)
{
    width  = w;
    height = h;
    return initSDL(title) && initImGui();
}

bool Application::initSDL(const std::string& title)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
        return false;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    window = SDL_CreateWindow(title.c_str(),
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              width, height, flags);
    if (!window) return false;

    glContext = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(1); // vsync
    return true;
}

bool Application::initImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    Theme::apply();

    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Push initial state
    stateManager->pushState(std::make_unique<MenuState>(*stateManager, *this));
    stateManager->applyPendingChanges();
    return true;
}

void Application::run()
{
    running = true;
    Uint64 lastTick = SDL_GetPerformanceCounter();

    while (running && !stateManager->isEmpty()) {
        processEvents();

        Uint64 now  = SDL_GetPerformanceCounter();
        float  dt   = static_cast<float>(now - lastTick)
                      / static_cast<float>(SDL_GetPerformanceFrequency());
        lastTick    = now;

        stateManager->update(dt);

        newFrame();
        stateManager->render();
        endFrame();
    }
}

void Application::processEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        ImGui_ImplSDL2_ProcessEvent(&e);
        if (e.type == SDL_QUIT)
            running = false;
        if (e.type == SDL_WINDOWEVENT
            && e.window.event == SDL_WINDOWEVENT_CLOSE
            && e.window.windowID == SDL_GetWindowID(window))
            running = false;
    }
}

void Application::newFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void Application::endFrame()
{
    ImGui::Render();
    SDL_GL_GetDrawableSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(0.06f, 0.06f, 0.06f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
}

void Application::setPlayer(std::unique_ptr<Player> p)
{
    player = std::move(p);
}

void Application::shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    if (glContext) SDL_GL_DeleteContext(glContext);
    if (window)   SDL_DestroyWindow(window);
    SDL_Quit();
}
