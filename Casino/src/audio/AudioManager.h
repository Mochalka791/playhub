#pragma once
#include <unordered_map>
#include <vector>
#include <cstdint>

// Forward-declare Mix_Chunk to avoid SDL_mixer header in public API
struct Mix_Chunk;

enum class Sound {
    ButtonClick,
    CardFlip,
    ReelTick,       // rapid tick during spin
    ReelStop,       // per-reel thud
    CoinDrop,       // small win
    WinFanfare,     // medium win
    BigWinFanfare,  // jackpot
    Lose,
    WheelSpin,      // roulette wheel whoosh
    BallRoll,       // roulette ball clicks
    GunCock,        // metallic cock
    GunClickEmpty,  // survived / empty chamber
    GunBang,        // shot
    Chip,           // chip placed
    COUNT
};

class AudioManager
{
public:
    static AudioManager& instance();

    bool init();
    void shutdown();

    void play(Sound s, int volume = 110);   // volume 0-128
    void setMasterVolume(int vol);          // 0-128
    bool isReady() const { return ready; }

private:
    AudioManager() = default;
    bool ready = false;

    Mix_Chunk* chunks[static_cast<int>(Sound::COUNT)] = {};

    void generateAll();

    // --- Procedural WAV helpers ---
    enum class Wave { Sine, Square, Triangle, Sawtooth, Noise };

    struct SoundDesc {
        Wave   wave     = Wave::Sine;
        float  freq     = 440.0f;
        float  duration = 0.2f;
        float  volume   = 0.6f;
        float  attack   = 0.01f;
        float  decay    = 0.08f;
        float  freqEnd  = -1.0f;   // if > 0: linear frequency glide
    };

    Mix_Chunk* buildChunk(const std::vector<int16_t>& pcm);
    std::vector<int16_t> makeTone(const SoundDesc& d, int sr = 44100);
    std::vector<int16_t> makeNoise(float duration, float volume,
                                   float attack, float decay, int sr = 44100);
    std::vector<int16_t> makeArpeggio(const std::vector<float>& freqs,
                                      float noteLen, float vol, int sr = 44100);
    std::vector<int16_t> mixSamples(const std::vector<int16_t>& a,
                                    const std::vector<int16_t>& b,
                                    int offsetSamples = 0);
    std::vector<int16_t> wrapWAV(const std::vector<int16_t>& pcm, int sr = 44100);
};
