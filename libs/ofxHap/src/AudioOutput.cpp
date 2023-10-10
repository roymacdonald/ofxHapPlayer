//
//  AudioOutput.cpp
//  example
//
//  Created by Roy Macdonald on 31-07-23.
//

#include <ofxHap/AudioOutput.h>
#include <ofxHap/AudioMixer.h>
extern "C"{
#include <libavformat/avformat.h>
}

#include <ofxHap/AudioThread.h>

namespace ofxHap{
SoundStream::SoundStream(){
        _setup();
        outSRListener = outSampleRate.newListener(this, &SoundStream::outSRChanged);
}
void SoundStream::outSRChanged (size_t&){
    if(streamSettings.sampleRate != outSampleRate.get()){
        _setup();
    }
}
void SoundStream::_setup(){
    streamSettings.numInputChannels = 0;
    streamSettings.numOutputChannels = 2;
    streamSettings.sampleRate = outSampleRate.get();
    streamSettings.bufferSize = 256;
    streamSettings.numBuffers = 2;
    
    streamSettings.setOutListener(GetMixer());
    if(!setup(streamSettings)){
        ofLogError("ofxHapPlayer", "Error starting audio stream.");
    }
//    start();
}


unsigned int SoundStream::getBestRate(size_t r)
{
    auto devices = getDeviceList();
    for (const auto& device : devices) {
        if (device.isDefaultOutput)
        {
            auto rates = device.sampleRates;
            auto bestRate = outSampleRate.get();
            if(bestRate >= r) return bestRate;
            for (auto rate : rates) {
                if (rate == r)
                {
                    bestRate = rate;
                    break;
//                    return rate;
                }
                else if (rate < r && rate > bestRate)
                {
                    bestRate = rate;
                }
            }
            if (bestRate == 0)
            {
                bestRate = r;
            }
            if(bestRate != outSampleRate.get()){
                outSampleRate =  bestRate;
            }
            return bestRate;
        }
    }
    return r;
}


SoundStream& GetSoundStream(){
    static std::unique_ptr<SoundStream> stream = std::make_unique<SoundStream>();
    return *stream.get();
}


//-------------------------------------------------------------------------------------
AudioOutput::AudioOutput()
#ifdef USING_OFX_SOUND_OBJECTS
:ofxSoundObject(OFX_SOUND_OBJECT_PROCESSOR),
#else
:
playing(false),
#endif
 stream_index(-1)
{
#ifdef USING_OFX_SOUND_OBJECTS
//    waveform.setNumBuffers(500);
//    waveform.setGridSpacingByNumSamples(256);
#endif
    
}
AudioOutput::AudioOutput(int index)
#ifdef USING_OFX_SOUND_OBJECTS
:ofxSoundObject(OFX_SOUND_OBJECT_PROCESSOR),
#else
:playing(false),
#endif
 stream_index(index)
{
#ifdef USING_OFX_SOUND_OBJECTS
//    waveform.setNumBuffers(500);
//    waveform.setGridSpacingByNumSamples(256);
#endif
}
#ifndef USING_OFX_SOUND_OBJECTS
AudioOutput::~AudioOutput()
{
    auto m = GetMixer();
    if(m) m->disconnect(this);
}

#endif
void AudioOutput::configure( std::shared_ptr<ofxHap::RingBuffer> buffer)
{
    _buffer = buffer;
#ifndef USING_OFX_SOUND_OBJECTS
    auto m = GetMixer();
    if(m) m->connect(this);
//#else
    
//    if(m) this->connectTo(*m);
#endif
}

void AudioOutput::audioOut(ofSoundBuffer& buffer)
{
    if(!isPlaying()){
        return;// false;
    }
    
    
    int wanted = static_cast<int>(buffer.getNumFrames());
    int filled = 0;

    const float *src[2];
    int count[2];

    _buffer->readBegin(src[0], count[0], src[1], count[1]);

    for (int i = 0; i < 2; i++)
    {
        int todo = std::min(wanted - filled, count[i]);
        if (todo > 0)
        {
            size_t copy = todo * sizeof(float) * buffer.getNumChannels();
            float *out = &buffer.getBuffer()[filled * buffer.getNumChannels()];
            memcpy(out, src[i], copy);
            filled += todo;
        }
    }

    _buffer->readEnd(filled);

    if (filled < wanted)
    {
        cout << "filled < wanted : " <<  filled << " < " << wanted << " stream_index: " << stream_index << "\n";
        float *out = &buffer.getBuffer()[filled * buffer.getNumChannels()];
        av_samples_set_silence((uint8_t **)&out,
                               0,
                               wanted - filled,
                               static_cast<int>(buffer.getNumChannels()),
                               AV_SAMPLE_FMT_FLT);
    }
#ifdef USING_OFX_SOUND_OBJECTS
//    waveform.pushBuffer(buffer);
#endif
    
//    return true;
}

void AudioOutput::start(){
#ifdef USING_OFX_SOUND_OBJECTS
    setBypassed(false);
#else
    playing = true;
#endif
}

void AudioOutput::stop(){
#ifdef USING_OFX_SOUND_OBJECTS
    setBypassed(true);
#else
    playing = false;
#endif
}
bool AudioOutput::isPlaying() {
#ifdef USING_OFX_SOUND_OBJECTS
    return !isBypassed();
#else
    return playing.load();
#endif
    
}

}
