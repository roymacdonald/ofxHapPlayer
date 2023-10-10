//
//  AudioOutput.h
//  example
//
//  Created by Roy Macdonald on 31-07-23.
//

#ifndef AudioOutput_h
#define AudioOutput_h
#include "ofMain.h"
#include <atomic>
#include <ofxHap/RingBuffer.h>

#ifdef USING_OFX_SOUND_OBJECTS
#include "waveformDraw.h"
#endif

namespace ofxHap{
class AudioOutput 
#ifdef USING_OFX_SOUND_OBJECTS
: public ofxSoundObject
#endif
{
public:
    AudioOutput();
    AudioOutput(int stream_index);
#ifndef USING_OFX_SOUND_OBJECTS
    ~AudioOutput();
#endif
    void configure(std::shared_ptr<ofxHap::RingBuffer> buffer);

#ifdef USING_OFX_SOUND_OBJECTS
    virtual void audioOut(ofSoundBuffer &output) override;
#else
    void audioOut(ofSoundBuffer& buffer);
#endif
    
    void start();
    void stop();
    bool isPlaying() ;
//#ifdef USING_OFX_SOUND_OBJECTS
//    circularBufferWaveformDraw waveform;
//#endif
private:
    int stream_index;
#ifndef USING_OFX_SOUND_OBJECTS
    std::atomic<bool>   playing;
#endif
    std::shared_ptr<ofxHap::RingBuffer> _buffer;
};


class SoundStream: public ofSoundStream{
public:
    SoundStream();
    ofSoundStreamSettings streamSettings;
    ofParameter<size_t> outSampleRate = {"outSampleRate", 44100, 1, 200000};
    unsigned int getBestRate(size_t rate) ;
    
    
private:
    void _setup();
    void outSRChanged (size_t&);
    ofEventListener outSRListener;
};


SoundStream& GetSoundStream();


}

#endif /* AudioOutput_h */
