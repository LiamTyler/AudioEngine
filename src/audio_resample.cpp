#include "audio_resample.hpp"
#include "utils/logger.hpp"

float CubicInterpolation( float x, float p0, float p1, float p2, float p3 )
{
    return p1 + 0.5f * x*(p2 - p0 + x*(2.0f*p0 - 5.0f*p1 + 4.0f*p2 - p3 + x*(3.0f*(p1 - p2) + p3 - p0)));
    //return p1 + (p2 - p1) * x;
}


void UpsampleAudio( int inputHz, int outputHz, float* inputAudio, float* outputAudio, uint64_t numInputFrames, uint32_t numChannels )
{
    float audioLen = numInputFrames / static_cast<float>( inputHz );
    uint64_t numOutputFrames = static_cast<uint64_t>( audioLen * outputHz );

    uint64_t outputFrame = 0;
    uint64_t outputIndex = 0;
    double invInputFrame = 1.0 / static_cast<double>( numInputFrames - 1 );
    double invOutputFrame = 1.0 / static_cast<double>( numOutputFrames - 1 );
    for ( uint64_t i = 0; i < numInputFrames - 1; ++i )
    {
        double startPercent = i * invInputFrame;
        double stopPercent  = (i+1) * invInputFrame;
        double outputPercent = outputFrame * invOutputFrame;
        while ( outputPercent <= stopPercent )
        {
            float x = static_cast<float>( (outputPercent - startPercent) * numInputFrames );
            uint64_t p0 = i > 0 ? i - 1 : 0;
            uint64_t p3 = std::min( numInputFrames - 1, i + 2 );
            for ( uint32_t channel = 0; channel < numChannels; ++channel )
            {
                outputAudio[outputIndex++] = CubicInterpolation( x, inputAudio[numChannels * p0], inputAudio[numChannels * i], inputAudio[numChannels * (i+1)], inputAudio[numChannels * p3] );
            }
            ++outputFrame;
            outputPercent = outputFrame * invOutputFrame;
        }
    }
    LOG( "num frames %u vs written %u", numOutputFrames, outputIndex );
}


bool ResampleAudio( uint32_t inputHz, uint32_t outputHz, float* inputAudio, float* outputAudio, uint64_t frames, uint32_t channels )
{
    if ( inputHz == outputHz )
    {
        memcpy( outputAudio, inputAudio, frames * channels * sizeof( float ) );
    }
    else if ( inputHz < outputHz )
    {
        UpsampleAudio( inputHz, outputHz, inputAudio, outputAudio, frames, channels );
    }
    else
    {
        LOG_ERR( "Downsampling of audio is not supported yet. Trying to go from %u to %u Hz.", inputHz, outputHz );
        return false;
    }

    return true;
}