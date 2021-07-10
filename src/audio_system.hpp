#pragma once

#include <string>

struct AudioFile
{
    std::string filename;
    float* data;
    uint64_t frames;
    uint32_t sampleRate;
    uint8_t numChannels;
    float lenInSeconds;
};


struct AudioInstance
{
    void Play();
    void Pause();
    void Seek( uint32_t frame );

    AudioFile* audioFile = nullptr;
    uint32_t currentFrame = 0;
    bool paused = false;
    bool loop = false;
};

namespace AudioSystem
{
    static constexpr uint32_t STANDARD_SAMPLERATE = 44100;

    bool Initialize();
    void Shutdown();

    void PauseAllSounds();
    void ResumeAllSounds();
    bool IsAllAudioPaused();

    AudioFile* LoadAudioFile( const std::string& fname );
    void ReleaseAudioFile( const std::string& name );

} // namespace AudioSystem