//
//  AudioMixer.h
//
//  Created by Roy Macdonald on 31-07-23.
//

#pragma once


#ifdef USING_OFX_SOUND_OBJECTS
//#include "waveformDraw.h"
#include "ofxSoundMixer.h"
#else
#include "ofSoundBaseTypes.h"
#include "ofParameter.h"

#include <mutex>
#include <atomic>

#endif

namespace ofxHap {

#ifndef USING_OFX_SOUND_OBJECTS
class AudioOutput;
class AudioMixer: public ofBaseSoundOutput {
public:
    AudioMixer();
//    virtual ~AudioMixer();
    
        
    /// sets output volume multiplier.
    /// a volume of 1 means "full volume", 0 is muted.
    void  setMasterVolume(float vol);
    float getMasterVolume();


    void audioOut(ofSoundBuffer &output) override;
    
    
    
    ofParameter<float> masterVol = {"Master Vol", 1, 0, 1};
    
    void connect(AudioOutput*  audio);
    void disconnect(AudioOutput* audio);

    size_t getCount(){return counter.load();}
    size_t getNumConnections(){
       // std::lock_guard<std::mutex> lck(connectionMutex);
        return connections.size();
    }
//#ifdef USING_OFX_SOUND_OBJECTS
//    circularBufferWaveformDraw waveform;
//#endif
    
protected:
    ofEventListener masterVolListener;
    void masterVolChanged(float& f);

    std::vector<AudioOutput*> connections;
    float masterVolume = 1.0f;

    std::mutex mutex, connectionMutex;
    
    std::atomic<size_t> counter;
    
};
    AudioMixer* GetMixer();
#else
    ofxSoundMixer* GetMixer();
#endif

}