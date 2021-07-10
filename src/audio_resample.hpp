#pragma once

#include <cstdint>

bool ResampleAudio( uint32_t inputHz, uint32_t outputHz, float* inputAudio, float* outputAudio, uint64_t frames, uint32_t channels );