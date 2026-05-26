#include "AudioManager.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <cmath>
#include <random>
#include <algorithm>
#include <cstring>

static constexpr float TWO_PI = 6.28318530f;

AudioManager& AudioManager::instance()
{
    static AudioManager inst;
    return inst;
}

bool AudioManager::init()
{
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) return false;
    if (Mix_OpenAudio(44100, AUDIO_S16SYS, 1, 1024) != 0) return false;
    Mix_AllocateChannels(16);
    generateAll();
    ready = true;
    return true;
}

void AudioManager::shutdown()
{
    for (int i = 0; i < static_cast<int>(Sound::COUNT); ++i) {
        if (chunks[i]) { Mix_FreeChunk(chunks[i]); chunks[i] = nullptr; }
    }
    Mix_CloseAudio();
    ready = false;
}

void AudioManager::play(Sound s, int volume)
{
    if (!ready) return;
    int idx = static_cast<int>(s);
    if (!chunks[idx]) return;
    int ch = Mix_PlayChannel(-1, chunks[idx], 0);
    if (ch >= 0) Mix_Volume(ch, std::clamp(volume, 0, 128));
}

void AudioManager::setMasterVolume(int vol)
{
    Mix_MasterVolume(std::clamp(vol, 0, 128));
}

// ---- WAV building -------------------------------------------

std::vector<int16_t> AudioManager::wrapWAV(const std::vector<int16_t>& pcm, int sr)
{
    // 44-byte WAV header + PCM data
    uint32_t dataBytes = (uint32_t)(pcm.size() * 2);
    uint32_t fileSize  = 36 + dataBytes;

    std::vector<int16_t> buf(22 + (int)pcm.size()); // 44 header bytes = 22 int16s
    uint8_t* h = reinterpret_cast<uint8_t*>(buf.data());

    auto w32 = [&](int off, uint32_t v) {
        h[off]=v&0xFF; h[off+1]=(v>>8)&0xFF; h[off+2]=(v>>16)&0xFF; h[off+3]=(v>>24)&0xFF;
    };
    auto w16 = [&](int off, uint16_t v) {
        h[off]=v&0xFF; h[off+1]=(v>>8)&0xFF;
    };

    memcpy(h,    "RIFF", 4);  w32(4, fileSize);
    memcpy(h+8,  "WAVE", 4);
    memcpy(h+12, "fmt ", 4);  w32(16, 16);
    w16(20, 1);               // PCM
    w16(22, 1);               // mono
    w32(24, (uint32_t)sr);    // sample rate
    w32(28, (uint32_t)(sr*2));// byte rate
    w16(32, 2);               // block align
    w16(34, 16);              // bits per sample
    memcpy(h+36, "data", 4);  w32(40, dataBytes);

    memcpy(buf.data() + 22, pcm.data(), pcm.size() * 2);
    return buf;
}

Mix_Chunk* AudioManager::buildChunk(const std::vector<int16_t>& pcm)
{
    auto wav = wrapWAV(pcm);
    SDL_RWops* rw = SDL_RWFromMem(wav.data(), (int)(wav.size() * 2));
    if (!rw) return nullptr;
    return Mix_LoadWAV_RW(rw, 1);
}

// ---- Waveform generators ------------------------------------

std::vector<int16_t> AudioManager::makeTone(const SoundDesc& d, int sr)
{
    int n = std::max(1, (int)(sr * d.duration));
    std::vector<int16_t> out(n);

    std::mt19937 rng(1337);
    std::uniform_real_distribution<float> nd(-1.f, 1.f);

    for (int i = 0; i < n; ++i) {
        float t  = (float)i / sr;
        float p  = t / d.duration;           // 0→1

        // Envelope
        float env;
        if (t < d.attack)
            env = t / d.attack;
        else if (t > d.duration - d.decay)
            env = std::max(0.0f, (d.duration - t) / d.decay);
        else
            env = 1.0f;

        // Frequency glide
        float freq = (d.freqEnd > 0.0f)
            ? d.freq + (d.freqEnd - d.freq) * p
            : d.freq;

        float phase = TWO_PI * freq * t;
        float wave  = 0.0f;
        switch (d.wave) {
        case Wave::Sine:
            wave = sinf(phase); break;
        case Wave::Square:
            wave = (fmodf(freq * t, 1.0f) < 0.5f) ? 1.0f : -1.0f; break;
        case Wave::Triangle: {
            float q = fmodf(freq * t, 1.0f);
            wave = (q < 0.5f) ? (4.0f * q - 1.0f) : (3.0f - 4.0f * q); break;
        }
        case Wave::Sawtooth:
            wave = 2.0f * fmodf(freq * t, 1.0f) - 1.0f; break;
        case Wave::Noise:
            wave = nd(rng); break;
        }

        out[i] = (int16_t)(32767 * d.volume * env * wave);
    }
    return out;
}

std::vector<int16_t> AudioManager::makeNoise(float dur, float vol,
                                              float atk, float dec, int sr)
{
    SoundDesc d;
    d.wave = Wave::Noise; d.freq = 0; d.duration = dur;
    d.volume = vol; d.attack = atk; d.decay = dec;
    return makeTone(d, sr);
}

std::vector<int16_t> AudioManager::makeArpeggio(const std::vector<float>& freqs,
                                                 float noteLen, float vol, int sr)
{
    std::vector<int16_t> result;
    for (float f : freqs) {
        SoundDesc d;
        d.wave = Wave::Sine; d.freq = f; d.duration = noteLen;
        d.volume = vol; d.attack = 0.01f; d.decay = noteLen * 0.4f;
        auto note = makeTone(d, sr);
        result.insert(result.end(), note.begin(), note.end());
    }
    return result;
}

std::vector<int16_t> AudioManager::mixSamples(const std::vector<int16_t>& a,
                                               const std::vector<int16_t>& b,
                                               int offset)
{
    size_t len = std::max(a.size(), b.size() + offset);
    std::vector<int16_t> out(len, 0);
    for (size_t i = 0; i < a.size(); ++i)
        out[i] = a[i];
    for (size_t i = 0; i < b.size(); ++i) {
        int idx = (int)i + offset;
        if (idx >= 0 && idx < (int)out.size()) {
            int32_t mixed = (int32_t)out[idx] + (int32_t)b[i];
            out[idx] = (int16_t)std::clamp(mixed, -32768, 32767);
        }
    }
    return out;
}

// ---- Generate all sound effects -----------------------------

void AudioManager::generateAll()
{
    int sr = 44100;

    // ButtonClick — short square click
    {
        SoundDesc d; d.wave=Wave::Square; d.freq=800; d.duration=0.04f;
        d.volume=0.4f; d.attack=0.002f; d.decay=0.03f;
        chunks[(int)Sound::ButtonClick] = buildChunk(makeTone(d, sr));
    }

    // CardFlip — noise burst + high ping
    {
        auto noise = makeNoise(0.05f, 0.5f, 0.002f, 0.04f, sr);
        SoundDesc d; d.wave=Wave::Sine; d.freq=1800; d.duration=0.08f;
        d.volume=0.3f; d.attack=0.002f; d.decay=0.07f;
        auto ping = makeTone(d, sr);
        chunks[(int)Sound::CardFlip] = buildChunk(mixSamples(noise, ping, 0));
    }

    // ReelTick — short low click
    {
        SoundDesc d; d.wave=Wave::Square; d.freq=180; d.duration=0.025f;
        d.volume=0.45f; d.attack=0.001f; d.decay=0.02f;
        chunks[(int)Sound::ReelTick] = buildChunk(makeTone(d, sr));
    }

    // ReelStop — descending thud
    {
        SoundDesc d; d.wave=Wave::Triangle; d.freq=220; d.freqEnd=90;
        d.duration=0.15f; d.volume=0.6f; d.attack=0.005f; d.decay=0.12f;
        auto tone = makeTone(d, sr);
        auto noise = makeNoise(0.05f, 0.2f, 0.001f, 0.04f, sr);
        chunks[(int)Sound::ReelStop] = buildChunk(mixSamples(tone, noise, 0));
    }

    // CoinDrop — high ping with shimmer
    {
        SoundDesc d; d.wave=Wave::Sine; d.freq=1480; d.freqEnd=1200;
        d.duration=0.3f; d.volume=0.6f; d.attack=0.005f; d.decay=0.22f;
        auto coin1 = makeTone(d, sr);
        d.freq=1760; d.freqEnd=1480; d.duration=0.25f; d.decay=0.20f;
        auto coin2 = makeTone(d, sr);
        chunks[(int)Sound::CoinDrop] = buildChunk(mixSamples(coin1, coin2, sr/10));
    }

    // WinFanfare — C-E-G-C5 arpeggio
    {
        auto arp = makeArpeggio({523.25f, 659.25f, 783.99f, 1046.5f}, 0.12f, 0.7f, sr);
        chunks[(int)Sound::WinFanfare] = buildChunk(arp);
    }

    // BigWinFanfare — triumphant ascent + chord
    {
        auto arp = makeArpeggio(
            {523.25f, 659.25f, 783.99f, 1046.5f, 1318.5f, 1046.5f},
            0.14f, 0.75f, sr);
        SoundDesc d; d.wave=Wave::Sine; d.freq=523.25f; d.duration=0.5f;
        d.volume=0.4f; d.attack=0.05f; d.decay=0.35f;
        auto chord = makeTone(d, sr);
        d.freq=659.25f; auto c2 = makeTone(d, sr);
        d.freq=783.99f; auto c3 = makeTone(d, sr);
        auto mixed = mixSamples(mixSamples(chord, c2, 0), c3, 0);
        chunks[(int)Sound::BigWinFanfare] = buildChunk(mixSamples(arp, mixed, (int)(arp.size())));
    }

    // Lose — descending minor chord
    {
        auto arp = makeArpeggio({392.0f, 349.23f, 293.66f, 261.63f}, 0.15f, 0.6f, sr);
        chunks[(int)Sound::Lose] = buildChunk(arp);
    }

    // WheelSpin — noise sweep (low → high → low)
    {
        int n = (int)(sr * 0.5f);
        std::vector<int16_t> sweep(n);
        std::mt19937 rng(42);
        std::uniform_real_distribution<float> nd(-1.f, 1.f);
        for (int i = 0; i < n; ++i) {
            float t = (float)i / n;
            float env = sinf(t * 3.14159f); // 0→1→0
            sweep[i] = (int16_t)(32767 * 0.55f * env * nd(rng));
        }
        chunks[(int)Sound::WheelSpin] = buildChunk(sweep);
    }

    // BallRoll — rapid decelerating clicks
    {
        int n = (int)(sr * 0.8f);
        std::vector<int16_t> roll(n, 0);
        // Clicks at decreasing intervals
        float interval = 0.02f;
        float t = 0.0f;
        while (t < 0.75f) {
            int pos = (int)(t * sr);
            if (pos < n) {
                SoundDesc d; d.wave=Wave::Square; d.freq=600.0f*(1.0f-t/0.9f)+200.0f;
                d.duration=0.015f; d.volume=0.3f*(1.0f-t/0.9f)+0.1f;
                d.attack=0.001f; d.decay=0.012f;
                auto click = makeTone(d, sr);
                for (size_t j = 0; j < click.size() && pos+(int)j < n; ++j) {
                    int32_t s = (int32_t)roll[pos+j] + (int32_t)click[j];
                    roll[pos+j] = (int16_t)std::clamp(s, -32768, 32767);
                }
            }
            interval *= 1.12f; // decelerate
            t += interval;
        }
        chunks[(int)Sound::BallRoll] = buildChunk(roll);
    }

    // GunCock — two metallic clicks
    {
        SoundDesc d; d.wave=Wave::Square; d.freq=1200; d.duration=0.04f;
        d.volume=0.7f; d.attack=0.001f; d.decay=0.03f;
        auto click1 = makeTone(d, sr);
        d.freq=900; d.duration=0.05f; d.volume=0.5f;
        auto click2 = makeTone(d, sr);
        auto noise1 = makeNoise(0.03f, 0.3f, 0.001f, 0.025f, sr);
        int off = sr / 10; // 100ms gap
        auto combined = mixSamples(mixSamples(click1, noise1, 0), click2, off);
        chunks[(int)Sound::GunCock] = buildChunk(combined);
    }

    // GunClickEmpty — single dry click
    {
        SoundDesc d; d.wave=Wave::Square; d.freq=2200; d.duration=0.035f;
        d.volume=0.75f; d.attack=0.001f; d.decay=0.028f;
        auto click = makeTone(d, sr);
        auto n2 = makeNoise(0.02f, 0.15f, 0.001f, 0.015f, sr);
        chunks[(int)Sound::GunClickEmpty] = buildChunk(mixSamples(click, n2, 0));
    }

    // GunBang — noise explosion + low boom
    {
        auto bang = makeNoise(0.35f, 0.9f, 0.001f, 0.25f, sr);
        SoundDesc d; d.wave=Wave::Sine; d.freq=80; d.freqEnd=30;
        d.duration=0.4f; d.volume=0.8f; d.attack=0.002f; d.decay=0.3f;
        auto boom = makeTone(d, sr);
        chunks[(int)Sound::GunBang] = buildChunk(mixSamples(bang, boom, 0));
    }

    // Chip — high ping
    {
        SoundDesc d; d.wave=Wave::Sine; d.freq=1200; d.duration=0.07f;
        d.volume=0.5f; d.attack=0.003f; d.decay=0.06f;
        chunks[(int)Sound::Chip] = buildChunk(makeTone(d, sr));
    }
}
