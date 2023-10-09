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
: playing(false)
{
    waveform.setNumBuffers(500);
}

AudioOutput::~AudioOutput()
{
    auto m = GetMixer();
    if(m) m->disconnect(this);
}


void AudioOutput::configure(std::shared_ptr<ofxHap::RingBuffer> buffer)
{
    _buffer = buffer;
    auto m = GetMixer();
    if(m) m->connect(this);
}

bool AudioOutput::audioOut(ofSoundBuffer& buffer)
{
    if(!playing.load()){
        return false;
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
        float *out = &buffer.getBuffer()[filled * buffer.getNumChannels()];
        av_samples_set_silence((uint8_t **)&out,
                               0,
                               wanted - filled,
                               static_cast<int>(buffer.getNumChannels()),
                               AV_SAMPLE_FMT_FLT);
    }
    
    waveform.pushBuffer(buffer);
    
    return true;
}

void AudioOutput::start(){
    playing = true;
}

void AudioOutput::stop(){
    playing = false;
}
bool AudioOutput::isPlaying() const{
    return playing.load();
}

}
