#include "utils/logger.hpp"
#include "portaudio.h"
#include "sndfile.h"
#include <vector>


int main( int argc, char** argv )
{
    Logger_Init();
    Logger_AddLogLocation( "stdout", stdout );

    char *inFileName;
    SNDFILE *inFile;
    SF_INFO inFileInfo;
    int fs;

    inFileName = "sm64_mario_hoo.wav";

    inFile = sf_open(inFileName, SFM_READ, &inFileInfo);
    if ( !inFile )
    {
        LOG_ERR( "Could not load WAV file" );
        return 0;
    }
    std::vector<float> buffer( inFileInfo.frames * inFileInfo.channels );
    sf_read_float( inFile, buffer.data(), inFileInfo.frames * inFileInfo.channels );
    sf_close(inFile);

    LOG( "Sample Rate = %d Hz", inFileInfo.samplerate );
    LOG( "Channels = %d Hz", inFileInfo.channels );
    LOG( "frames = %d Hz", inFileInfo.frames );
    printf( "Format %x\n", inFileInfo.format );

    PaStreamParameters outputParameters;
    PaStream *stream;
    PaError err;
    //float buffer[FRAMES_PER_BUFFER][2]; /* stereo output buffer */

    LOG_WARN( "PRE init" );
    err = Pa_Initialize();
    LOG_WARN( "Past init" );
    if ( err != paNoError )
    {
        LOG_ERR( "Failed to initialize portaudio" );
        return 0;
    }

    outputParameters.device = Pa_GetDefaultOutputDevice();
    if ( outputParameters.device == paNoDevice )
    {
        LOG_ERR( "No default output device." );
        return 0;
    }
    LOG( "default low output latency: %f", Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency );

    outputParameters.channelCount = inFileInfo.channels;       /* stereo output */
    outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
    outputParameters.suggestedLatency = 0.050; // Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
              &stream,
              NULL, /* no input */
              &outputParameters,
              inFileInfo.samplerate,
              inFileInfo.frames,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              NULL, /* no callback, use blocking API */
              NULL ); /* no callback, so no callback userData */
    if ( err != paNoError )
    {
        LOG_ERR( "Could not open audio stream" );
        return 0;
    }

    err = Pa_StartStream( stream );
    err = Pa_WriteStream( stream, buffer.data(), buffer.size() );
    LOG_WARN(" After write" );
    Pa_Sleep( 1000 );
    LOG_WARN( "after sleep" );
    
    err = Pa_StopStream( stream );

    err = Pa_CloseStream( stream );

    Pa_Terminate();

    return 0;
}
