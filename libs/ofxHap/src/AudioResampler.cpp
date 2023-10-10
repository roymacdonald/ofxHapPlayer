/*
 AudioResampler.cpp
 ofxHapPlayer

 Copyright (c) 2016, Tom Butterworth. All rights reserved.
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ofxHap/AudioResampler.h>
extern "C" {
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/channel_layout.h>
}
#include <algorithm>
#include <cmath>
#include <vector>
#include <inttypes.h>

ofxHap::AudioResampler::AudioResampler(const AudioParameters& params, int numOutchannels, int outRate)
:
#ifndef  USING_OFX_SOUND_OBJECTS
_volume(1.0),
#endif

_rate(1.0), _resampler(nullptr), _reconfigure(true), _outChannels(numOutchannels),
#if OFX_HAP_HAS_CODECPAR
_layout(params.parameters->channel_layout), _sampleRateIn(params.parameters->sample_rate), _sampleRateOut(outRate), _format(params.parameters->format)
#else
_layout(params.context->channel_layout), _sampleRateIn(params.context->sample_rate), _sampleRateOut(outRate), _format(params.context->sample_fmt), _outChannels( params.context->channels)
#endif
{
    if (_layout == 0)
    {
#if OFX_HAP_HAS_CODECPAR
        _layout = av_get_default_channel_layout(params.parameters->channels);
#else
        _layout = av_get_default_channel_layout(params.context->channels);
#endif
    }
    
    _outLayout = av_get_default_channel_layout(numOutchannels);
    
}

ofxHap::AudioResampler::~AudioResampler()
{
    if (_resampler)
    {
        swr_free(&_resampler);
    }
}
#ifndef  USING_OFX_SOUND_OBJECTS

float ofxHap::AudioResampler::getVolume() const
{
    return _volume;
}

void ofxHap::AudioResampler::setVolume(float v)
{
    if (_volume != v)
    {
        _volume = v;
        _reconfigure = true;
    }

}
#endif


float ofxHap::AudioResampler::getRate() const
{
    return _rate;
}

void ofxHap::AudioResampler::setRate(float r)
{
    if (_rate != r)
    {
        _rate = std::fabs(r);
        _reconfigure = true;
    }
}
void ofxHap::AudioResampler::setSampleRateOut(int sampleRateOut){
    if(_sampleRateOut != sampleRateOut){
        _sampleRateOut = sampleRateOut;
        _reconfigure = true;
    }
}


int ofxHap::AudioResampler::resample(const AVFrame *frame, int offset, int srcLength, float *dst, int dstLength, int& outSamplesWritten, int& outSamplesRead)
{
    if (_reconfigure)
    {
        // TODO: deal with rates > INT_MAX (the limit)
        // - we'll need to drop samples
        // and 0
        if (_resampler)
        {
            swr_free(&_resampler);
        }
        _resampler = swr_alloc_set_opts(nullptr,
                                        _outLayout,
                                        AV_SAMPLE_FMT_FLT,
                                        static_cast<int>(_sampleRateOut / _rate),
                                        _layout,
                                        static_cast<AVSampleFormat>(_format),
                                        _sampleRateIn,
                                        0,
                                        nullptr);

//        char buf1[1024];
//        av_channel_layout_describe(_layout, buf1, sizeof(buf1));
//        
        int channels = av_get_channel_layout_nb_channels(_layout);
        av_log(NULL, AV_LOG_INFO,
               "AudioResampler chans:%d out_ch_layout: %" PRIu64 " out_sample_fmt:%s out_sample_rate: %d | channels: %d in_ch_layout: %" PRIu64 " in_sample_fmt:%s  in_sample_rate: %d\nFrame: sr: %d, chanels: %d, layout: %" PRIu64 " fmt: %s\n",
               _outChannels,
               _outLayout,
               av_get_sample_fmt_name(AV_SAMPLE_FMT_FLT),
               static_cast<int>(_sampleRateOut / _rate),
               channels,
               _layout,
               av_get_sample_fmt_name(static_cast<AVSampleFormat>(_format)),
               _sampleRateIn,
               frame->sample_rate, frame->channels, frame->channel_layout, av_get_sample_fmt_name(static_cast<AVSampleFormat>(frame->format)));
               
#ifndef  USING_OFX_SOUND_OBJECTS
        std::vector<double> matrix(channels * channels);
        for (int i = 0; i < channels; i++) {
            for (int j = 0; j < channels; j++) {
                if (j == i)
                    matrix[i*channels+j] = _volume;
                else
                    matrix[i*channels+j] = 0.0;
            }
        }
        int result = swr_set_matrix(_resampler, matrix.data(), channels);
        if (result >= 0)
        {
#else
        int
#endif
            result = swr_init(_resampler);
#ifndef  USING_OFX_SOUND_OBJECTS
        }
#endif
        if (result < 0)
        {
            return result;
        }
        _reconfigure = false;
    }

    std::vector<const uint8_t *> src(frame->channels);
    if (av_sample_fmt_is_planar(static_cast<AVSampleFormat>(frame->format)))
    {
        for (int i = 0; i < frame->channels; i++) {
            src[i] = frame->extended_data[i] + (offset * av_get_bytes_per_sample(static_cast<AVSampleFormat>(frame->format)));
        }
    }
    else
    {
        src[0] = frame->data[0] + (offset * av_get_bytes_per_sample(static_cast<AVSampleFormat>(frame->format)) * frame->channels);
    }

    int result = swr_convert(_resampler,
                             (uint8_t **)&dst,
                             dstLength,
                             src.data(),
                             srcLength);
    if (result < 0)
    {
        return result;
    }
    outSamplesWritten = result;
    outSamplesRead = srcLength;
    return 0;
}
