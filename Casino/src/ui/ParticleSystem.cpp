#include "ParticleSystem.h"
#include <random>
#include <cmath>
#include <algorithm>

static constexpr float PI_F = 3.14159265f;

static std::mt19937& rng() {
    static std::mt19937 r(std::random_device{}());
    return r;
}
static float rf(float lo, float hi) {
    return std::uniform_real_distribution<float>(lo, hi)(rng());
}

void ParticleSystem::emit(float x, float y, int count, ParticleType type)
{
    for (int i = 0; i < count && (int)particles.size() < MAX_PARTICLES; ++i) {
        Particle p;
        p.x    = x;
        p.y    = y;
        p.type = type;

        switch (type) {
        case ParticleType::Coin:
            p.vx       = rf(-180.f, 180.f);
            p.vy       = rf(-320.f, -80.f);
            p.size     = rf(6.f, 12.f);
            p.maxLife  = rf(0.9f, 1.6f);
            p.rotSpeed = rf(-4.f, 4.f);
            {
                // golden coins
                uint8_t r2 = (uint8_t)rf(200, 255);
                uint8_t g2 = (uint8_t)rf(160, 210);
                p.color = IM_COL32(r2, g2, 0, 255);
            }
            break;

        case ParticleType::Star:
            p.vx      = rf(-220.f, 220.f);
            p.vy      = rf(-260.f, -60.f);
            p.size    = rf(3.f, 7.f);
            p.maxLife = rf(0.6f, 1.2f);
            p.rotSpeed = rf(-6.f, 6.f);
            {
                // bright stars: white/yellow/cyan
                int choice = (int)rf(0, 3);
                if (choice == 0)      p.color = IM_COL32(255, 255, 255, 255);
                else if (choice == 1) p.color = IM_COL32(255, 240, 60,  255);
                else                  p.color = IM_COL32(80,  220, 255, 255);
            }
            break;

        case ParticleType::Confetti:
            p.vx      = rf(-200.f, 200.f);
            p.vy      = rf(-280.f, -60.f);
            p.size    = rf(4.f, 8.f);
            p.maxLife = rf(1.0f, 2.0f);
            p.rotSpeed = rf(-8.f, 8.f);
            {
                // rainbow colours
                int hue = (int)rf(0, 6);
                const ImU32 cols[] = {
                    IM_COL32(255, 50,  50,  255),
                    IM_COL32(255, 165, 0,   255),
                    IM_COL32(255, 255, 50,  255),
                    IM_COL32(50,  220, 50,  255),
                    IM_COL32(50,  180, 255, 255),
                    IM_COL32(200, 80,  255, 255),
                };
                p.color = cols[hue];
            }
            break;

        case ParticleType::Smoke:
            p.vx      = rf(-40.f, 40.f);
            p.vy      = rf(-80.f, -20.f);
            p.size    = rf(8.f, 20.f);
            p.maxLife = rf(0.4f, 0.8f);
            p.color   = IM_COL32(140, 120, 100, 180);
            break;

        case ParticleType::Flash:
            p.vx      = rf(-60.f, 60.f);
            p.vy      = rf(-60.f, 60.f);
            p.size    = rf(12.f, 28.f);
            p.maxLife = rf(0.15f, 0.3f);
            p.color   = IM_COL32(255, 220, 80, 220);
            break;
        }

        p.life     = p.maxLife;
        p.rotation = rf(0.f, PI_F * 2.f);
        particles.push_back(p);
    }
}

void ParticleSystem::update(float dt)
{
    static constexpr float GRAVITY = 320.0f;

    for (auto& p : particles) {
        p.x  += p.vx * dt;
        p.y  += p.vy * dt;
        p.vy += GRAVITY * dt; // gravity
        p.vx *= 0.98f;        // air resistance
        p.rotation += p.rotSpeed * dt;
        p.life -= dt;
    }
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [](const Particle& p){ return p.life <= 0.f; }),
        particles.end());
}

void ParticleSystem::render(ImDrawList* draw, ImVec2 winPos)
{
    for (const auto& p : particles) {
        float alpha = std::clamp(p.life / p.maxLife, 0.0f, 1.0f);
        ImU32 col = (p.color & 0x00FFFFFF) | ((uint32_t)(alpha * 255) << 24);

        float sx = winPos.x + p.x;
        float sy = winPos.y + p.y;
        float sz = p.size * (0.5f + 0.5f * alpha);

        switch (p.type) {
        case ParticleType::Coin: {
            // Draw as ellipse (coin seen from angle)
            float scaleX = fabsf(cosf(p.rotation)) * 0.5f + 0.5f;
            draw->AddEllipseFilled({sx, sy}, {sz * scaleX, sz}, col, 0.0f, 10);
            draw->AddEllipse({sx, sy}, {sz * scaleX, sz},
                             (col & 0x00FFFFFF) | 0xFF000000, 0.0f, 10, 1.2f);
            break;
        }
        case ParticleType::Star: {
            // 4-point sparkle
            float r = sz;
            float ri = r * 0.35f;
            ImVec2 pts[8];
            for (int i = 0; i < 8; ++i) {
                float a = p.rotation + i * PI_F * 0.25f;
                float rad = (i % 2 == 0) ? r : ri;
                pts[i] = {sx + cosf(a) * rad, sy + sinf(a) * rad};
            }
            draw->AddConvexPolyFilled(pts, 8, col);
            break;
        }
        case ParticleType::Confetti:
        case ParticleType::Flash: {
            // Rotated rectangle
            float hw = sz, hh = sz * 0.4f;
            float ca = cosf(p.rotation), sa = sinf(p.rotation);
            ImVec2 verts[4] = {
                {sx + ca*(-hw) - sa*(-hh), sy + sa*(-hw) + ca*(-hh)},
                {sx + ca*( hw) - sa*(-hh), sy + sa*( hw) + ca*(-hh)},
                {sx + ca*( hw) - sa*( hh), sy + sa*( hw) + ca*( hh)},
                {sx + ca*(-hw) - sa*( hh), sy + sa*(-hw) + ca*( hh)},
            };
            draw->AddConvexPolyFilled(verts, 4, col);
            break;
        }
        case ParticleType::Smoke:
            draw->AddCircleFilled({sx, sy}, sz, col, 12);
            break;
        }
    }
}
