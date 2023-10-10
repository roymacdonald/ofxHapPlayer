//
//  AudioMixer.cpp
//  example
//
//  Created by Roy Macdonald on 31-07-23.
//

#include <ofxHap/AudioMixer.h>
#ifndef USING_OFX_SOUND_OBJECTS
#include <ofxHap/AudioOutput.h>
#include "ofSoundBuffer.h"
#endif

namespace ofxHap{
#ifndef USING_OFX_SOUND_OBJECTS
//----------------------------------------------------
AudioMixer::AudioMixer():
counter(0)
{    
//#ifdef USING_OFX_SOUND_OBJECTS
//    waveform.setNumBuffers(500);
//    waveform.setGridSpacingByNumSamples(256);
//#endif
    masterVolListener = masterVol.newListener(this, &AudioMixer::masterVolChanged);
}
//----------------------------------------------------
void AudioMixer::masterVolChanged(float& f) {
    //inherently threadsafe. gets called from within a mutex lock
    masterVolume = masterVol;
}
////----------------------------------------------------
//AudioMixer::~AudioMixer(){
//
//}

//----------------------------------------------------
void AudioMixer::setMasterVolume(float vol){
    std::lock_guard<std::mutex> lck(mutex);
    masterVolume = vol;
    
}
//----------------------------------------------------
float AudioMixer::getMasterVolume(){
    return masterVol.get();
}

//----------------------------------------------------
void AudioMixer::connect(AudioOutput*  audio){
    std::lock_guard<std::mutex> lck(connectionMutex);
    for (auto & a: connections) {
        if (audio == a) {
            ofLogNotice("AudioMixer::setInput") << " already connected";
            return;
        }
    }
    connections.push_back(audio);
}

//----------------------------------------------------
void AudioMixer::disconnect(AudioOutput* audio){
    std::lock_guard<std::mutex> lck(connectionMutex);
    ofRemove(connections, [audio](AudioOutput* a){return a == audio;});
}

//----------------------------------------------------
// this pulls the audio through from earlier links in the chain and sums up the total output
void AudioMixer::audioOut(ofSoundBuffer &output) {
    if(connections.size()>0) {
        output.set(0);//clears the output buffer as its memory might come with junk
        ofSoundBuffer tempBuffer;
        tempBuffer.resize(output.size());
        tempBuffer.setNumChannels(output.getNumChannels());
        tempBuffer.setSampleRate(output.getSampleRate());
        bool bHasOutput = false;
        bool bNeedsRemoval = false;
        for(auto c: connections){
            if (c) {
//                tempBuffer.set(0);
//                if(
                c->audioOut(tempBuffer);
                    for (int j = 0; j < tempBuffer.size(); j++) {
                        output.getBuffer()[j] += tempBuffer.getBuffer()[j];
                    }
                    bHasOutput = true;
//                }
            }else{
                bNeedsRemoval  = true;
            }
        }
        if(bHasOutput){
            counter ++;
        }
        
        if(bNeedsRemoval){
            ofRemove(connections, [](AudioOutput* a){return !a;});
        }
//#ifdef USING_OFX_SOUND_OBJECTS
//    waveform.pushBuffer(output);
//#endif
    
//        if(bHasOutput && !ofIsFloatEqual(masterVolume, 1.0f)){
//            output*=masterVolume;
//        }
    }
}
//----------------------------------------------------


AudioMixer* GetMixer(){
    static unique_ptr<AudioMixer> mxr = make_unique<AudioMixer>();
    return mxr.get();
}
#else
ofxSoundMixer* GetMixer(){
    static unique_ptr<ofxSoundMixer> mxr = make_unique<ofxSoundMixer>();
    return mxr.get();
}
#endif

}
