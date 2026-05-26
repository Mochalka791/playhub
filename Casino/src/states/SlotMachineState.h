#pragma once
#include "../core/IState.h"
#include "../games/SlotMachine.h"
#include "../ui/AnimationSystem.h"
#include "../ui/ParticleSystem.h"
#include <memory>
#include <string>
#include <array>

class SlotMachineState : public IState
{
public:
    SlotMachineState(StateManager& sm, Application& app);
    void onEnter() override;
    void update(float dt) override;
    void render() override;

private:
    std::unique_ptr<SlotMachine> game;
    ReelAnimation reelAnim;

    float betAmount = 10.0f;
    std::string resultMsg;
    bool showResult = false;

    // Symbols shown while spinning (random until reel stops)
    std::array<SlotSymbol, 3> displayReels{};
    bool reel1WasDone = false;
    bool reel2WasDone = false;
    bool reel3WasDone = false;

    ParticleSystem particles;
    float winFlashTimer = 0.0f;

    void randomizeDisplayReels(bool r1, bool r2, bool r3);
    void updateDisplayReels();
};
