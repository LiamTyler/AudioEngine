#pragma once

#include "audio_file.hpp"
#include <string>

namespace AudioSystem
{

    bool Initialize();
    void Shutdown();

    AudioFile* LoadAudioFile( const std::string& fname );

} // namespace AudioSystem