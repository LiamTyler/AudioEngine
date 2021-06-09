#pragma once

#include <cstdint>
#include <string>

struct AudioFile
{
    std::string filename;
    float* data;
    uint32_t frames;
    uint32_t sampleRate;
    uint8_t numChannels;
    int format;
};