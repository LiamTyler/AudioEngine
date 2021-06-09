#include "audio_system.hpp"
#include "utils/logger.hpp"
#include "portaudio.h"

struct AudioInstance
{
    AudioFile* audioFile = nullptr;
    size_t currentFrame = 0;
};

static int patestCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    (void)inputBuffer;
    float *out = (float*)outputBuffer;
    AudioInstance* audioIn = (AudioInstance*)userData;
    size_t& currentFrame = audioIn->currentFrame;
    AudioFile* aFile = audioIn->audioFile;

    uint32_t outIdx = 0;
    for ( uint32_t i = 0; i < framesPerBuffer; i++ )
    {
        if ( currentFrame < aFile->frames )
        {
            for ( int channel = 0; channel < aFile->numChannels; ++channel )
            {
                out[outIdx++] = aFile->data[aFile->numChannels * currentFrame + channel];
            }
            ++currentFrame;
        }
        else
        {
            for ( int channel = 0; channel < aFile->numChannels; ++channel )
            {
                out[outIdx++] = 0;
            }
        }
    }

    return 0;
}

int main( int argc, char** argv )
{
    Logger_Init();
    Logger_AddLogLocation( "stdout", stdout );

    AudioSystem::Initialize();

    AudioFile* audioFile = AudioSystem::LoadAudioFile( "organfinale.wav" );
    AudioInstance instance = { audioFile, 0 };
   
    PaStreamParameters outputParameters;
    PaStream *stream;
    PaError err;

    outputParameters.device = Pa_GetDefaultOutputDevice();
    if ( outputParameters.device == paNoDevice )
    {
        LOG_ERR( "No default output device." );
        return 0;
    }
    LOG( "default low output latency: %f", Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency );

    outputParameters.channelCount = audioFile->numChannels;       /* stereo output */
    outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
    outputParameters.suggestedLatency = 0.050; // Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
              &stream,
              NULL,
              &outputParameters,
              audioFile->sampleRate,
              paFramesPerBufferUnspecified,
              paNoFlag,
              patestCallback,
              &instance );
    if ( err != paNoError )
    {
        LOG_ERR( "Could not open audio stream" );
        return 0;
    }

    err = Pa_StartStream( stream );
    Pa_Sleep( audioFile->lenInSeconds * 1000 );
    
    err = Pa_StopStream( stream );
    err = Pa_CloseStream( stream );

    AudioSystem::Shutdown();

    return 0;
}
