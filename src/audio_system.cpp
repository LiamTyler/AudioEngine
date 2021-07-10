#include "audio_system.hpp"
#include "assert.hpp"
#include "utils/logger.hpp"
#include "portaudio.h"
#include "sndfile.h"
#include "audio_resample.hpp"
#include <unordered_map>


static std::unordered_map< std::string, AudioFile* > s_audioFiles;
static PaStream* s_audioStream;
static std::vector< AudioInstance* > s_activeSounds;
static bool s_allAudioPaused;


static int patestCallback( const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData )
{
    (void)inputBuffer;
    float *out = (float*)outputBuffer;
    //AudioInstance* audioIn = (AudioInstance*)userData;
    //size_t& currentFrame = audioIn->currentFrame;
    //AudioFile* aFile = audioIn->audioFile;
    //
    //uint32_t outIdx = 0;
    //for ( uint32_t i = 0; i < framesPerBuffer; i++ )
    //{
    //    if ( currentFrame < aFile->frames )
    //    {
    //        //for ( int channel = 0; channel < aFile->numChannels; ++channel )
    //        //{
    //        //    out[outIdx++] = aFile->data[aFile->numChannels * currentFrame + channel];
    //        //}
    //        out[outIdx++] = aFile->data[currentFrame];
    //        out[outIdx++] = aFile->data[currentFrame];
    //        ++currentFrame;
    //    }
    //    else
    //    {
    //        for ( int channel = 0; channel < aFile->numChannels; ++channel )
    //        {
    //            out[outIdx++] = 0;
    //        }
    //    }
    //}

    return paContinue;
}


namespace AudioSystem
{
    bool Initialize()
    {
        s_allAudioPaused = false;
        PaError err = Pa_Initialize();
        if ( err != paNoError )
        {
            LOG_ERR( "Failed to initialize AudioSystem" );
            return false;
        }

        PaStreamParameters outputParameters;
        outputParameters.device = Pa_GetDefaultOutputDevice();
        if ( outputParameters.device == paNoDevice )
        {
            LOG_ERR( "No default output device." );
            return false;
        }
        outputParameters.channelCount = 2;
        outputParameters.sampleFormat = paFloat32;
        outputParameters.suggestedLatency = 0.05; // Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
        outputParameters.hostApiSpecificStreamInfo = NULL;

        err = Pa_OpenStream( &s_audioStream, NULL, &outputParameters, STANDARD_SAMPLERATE, paFramesPerBufferUnspecified, paNoFlag, patestCallback, NULL );
        if ( err != paNoError )
        {
            LOG_ERR( "Could not open audio stream" );
            return false;
        }
        err = Pa_StartStream( s_audioStream );
        if ( err != paNoError )
        {
            LOG_ERR( "Could not start audio stream" );
            return false;
        }

        return true;
    }


    void Shutdown()
    {
        Pa_StopStream( s_audioStream );
        Pa_CloseStream( s_audioStream );
        Pa_Terminate();
        for ( auto& kvp : s_audioFiles )
        {
            AudioFile* audioFile = kvp.second;
            free( audioFile->data );
            delete audioFile;
        }
        s_audioFiles.clear();
    }


    void PauseAllSounds() { s_allAudioPaused = true; }
    void ResumeAllSounds() { s_allAudioPaused = false; }
    bool IsAllAudioPaused() { return s_allAudioPaused; }


    AudioFile* LoadAudioFile( const std::string& fname )
    {
        SF_INFO inFileInfo;
        SNDFILE *inFile = sf_open( fname.c_str(), SFM_READ, &inFileInfo );
        if ( !inFile )
        {
            LOG_ERR( "Could not load AudioFile '%s'", fname.c_str() );
            return nullptr;
        }
        LOG( "Original Sample Rate = %d Hz", inFileInfo.samplerate );
        LOG( "Channels = %d", inFileInfo.channels );
        LOG( "frames = %d", inFileInfo.frames );
        LOG( "Format %x\n", inFileInfo.format );

        float* data = static_cast<float*>( malloc( inFileInfo.frames * inFileInfo.channels * sizeof( float ) ) );
        size_t framesRead = sf_readf_float( inFile, data, inFileInfo.frames );
        sf_close( inFile );
        if ( framesRead != inFileInfo.frames )
        {
            free( data );
            LOG_ERR( "Could not read AudioFile into buffer" );
            return nullptr;
        }
        
        AudioFile* audioFile = new AudioFile;
        audioFile->filename = fname;
        audioFile->frames = inFileInfo.frames;
        audioFile->numChannels = inFileInfo.channels;

        if ( inFileInfo.samplerate != STANDARD_SAMPLERATE )
        {
            float audioLen = inFileInfo.frames / static_cast<float>( inFileInfo.samplerate );
            audioFile->frames = static_cast<uint64_t>( audioLen * STANDARD_SAMPLERATE );
            float* outputData = static_cast<float*>( malloc( audioFile->frames * inFileInfo.channels * sizeof( float ) ) );
        
            if ( !ResampleAudio( inFileInfo.samplerate, STANDARD_SAMPLERATE, data, outputData, inFileInfo.frames, inFileInfo.channels ) )
            {
                free( data );
                free( outputData );
                delete audioFile;
                LOG_ERR( "Could not resample audio" );
                return nullptr;
            }
            free( data );
            data = outputData;
        }
        audioFile->sampleRate = STANDARD_SAMPLERATE;
        
        //audioFile->sampleRate = inFileInfo.samplerate;
        audioFile->data = data;
        audioFile->lenInSeconds = audioFile->frames / (float)audioFile->sampleRate;
        s_audioFiles[fname] = audioFile;

        return audioFile;
    }


    void ReleaseAudioFile( const std::string& name )
    {
        PG_ASSERT( s_audioFiles.find( name ) != s_audioFiles.end(), "No audio file with name '" + name + "' currently loaded" );
        s_audioFiles.erase( name );
    }


} // namespace AudioSystem

void AudioInstance::Play()
{
    paused = false;
}


void AudioInstance::Pause()
{
    paused = true;
}


void AudioInstance::Seek( uint32_t frame )
{
    currentFrame = frame;
}