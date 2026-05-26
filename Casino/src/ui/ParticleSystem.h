#pragma once
#include <imgui.h>
#include <vector>

enum class ParticleType { Coin, Star, Confetti, Smoke, Flash };

struct Particle
{
    float  x, y;
    float  vx, vy;
    float  life;
    float  maxLife;
    float  size;
    ImU32  color;
    ParticleType type;
    float  rotation = 0.0f;
    float  rotSpeed = 0.0f;
};

class ParticleSystem
{
public:
    void emit(float winX, float winY,      // window-local origin
              int count, ParticleType type);
    void update(float dt);
    void render(ImDrawList* draw, ImVec2 winPos);
    bool isEmpty() const { return particles.empty(); }
    void clear()         { particles.clear(); }

private:
    std::vector<Particle> particles;
    static constexpr int MAX_PARTICLES = 400;
};
