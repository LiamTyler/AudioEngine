#pragma once

#include <cstdint>
#include <string>

struct AudioFile
{
    std::string filename;
    float* data;
    size_t frames;
    uint32_t sampleRate;
    uint8_t numChannels;
    float lenInSeconds;
};