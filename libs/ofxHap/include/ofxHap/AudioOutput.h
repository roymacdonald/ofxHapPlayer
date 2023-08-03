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

namespace ofxHap{
class AudioOutput {
public:
    AudioOutput();
    ~AudioOutput();
    void configure(std::shared_ptr<ofxHap::RingBuffer> buffer);

    bool audioOut(ofSoundBuffer& buffer);
    
    void start();
    void stop();
    bool isPlaying() const;
private:
    std::atomic<bool>   playing;
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
