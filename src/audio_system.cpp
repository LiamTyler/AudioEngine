#include "audio_system.hpp"
#include "utils/logger.hpp"
#include "portaudio.h"
#include "sndfile.h"
#include <unordered_map>

static std::unordered_map< std::string, AudioFile* > s_audioFiles;

namespace AudioSystem
{
    bool Initialize()
    {
        PaError err = Pa_Initialize();
        if ( err != paNoError )
        {
            LOG_ERR( "Failed to initialize AudioSystem" );
            return false;
        }

        return true;
    }


    void Shutdown()
    {
        Pa_Terminate();
        for ( auto& kvp : s_audioFiles )
        {
            AudioFile* audioFile = kvp.second;
            free( audioFile->data );
            delete audioFile;
        }
        s_audioFiles.clear();
    }


    AudioFile* LoadAudioFile( const std::string& fname )
    {
        SF_INFO inFileInfo;
        SNDFILE *inFile = sf_open( fname.c_str(), SFM_READ, &inFileInfo );
        if ( !inFile )
        {
            LOG_ERR( "Could not load AudioFile '%s'", fname.c_str() );
            return nullptr;
        }
        LOG( "Sample Rate = %d Hz", inFileInfo.samplerate );
        LOG( "Channels = %d", inFileInfo.channels );
        LOG( "frames = %d", inFileInfo.frames );
        LOG( "Format %x\n", inFileInfo.format );

        AudioFile* audioFile = new AudioFile;
        audioFile->filename = fname;
        audioFile->frames = inFileInfo.frames;
        audioFile->numChannels = inFileInfo.channels;
        audioFile->sampleRate = inFileInfo.samplerate;
        audioFile->data = static_cast<float*>( malloc( inFileInfo.frames * inFileInfo.channels * sizeof( float ) ) );
        audioFile->lenInSeconds = audioFile->frames / (float)audioFile->sampleRate;
        size_t framesRead = sf_readf_float( inFile, audioFile->data, inFileInfo.frames );
        if ( framesRead != inFileInfo.frames )
        {
            LOG_ERR( "Could not read AudioFile into buffer" );
        }
        sf_close( inFile );
        s_audioFiles[fname] = audioFile;

        return audioFile;
    }


} // namespace AudioSystem
