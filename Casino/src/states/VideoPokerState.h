#pragma once
#include "../core/IState.h"
#include "../games/VideoPoker.h"
#include "../ui/ParticleSystem.h"
#include <memory>
#include <string>

class VideoPokerState : public IState
{
public:
    VideoPokerState(StateManager& sm, Application& app);
    void onEnter() override;
    void update(float dt) override;
    void render() override;

private:
    std::unique_ptr<VideoPoker> game;
    float betAmount = 10.0f;
    std::string resultMsg;
    bool resultResolved = false;

    ParticleSystem particles;

    void drawCard(ImDrawList* draw, ImVec2 tl, const Card& card, bool held) const;
    void resolveAchievements();
};
