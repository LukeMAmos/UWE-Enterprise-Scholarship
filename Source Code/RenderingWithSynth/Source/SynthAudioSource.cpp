/*
  ==============================================================================

    SynthAudioSource.cpp
    Created: 18 May 2026 3:28:28pm
    Author:  Luke Amos

  ==============================================================================
*/

#include "SynthAudioSource.h"


SynthAudioSource::SynthAudioSource(){
    //8 note polyphony
    for(int i = 0 ; i < 8 ; i++){
        synth.addVoice(new SynthVoice());
    }
    //pass through the sound rules (let all midi data through )
    synth.addSound(new SynthSound());
    
}


void SynthAudioSource::prepareToPlay(int samplesPerBlockExpected ,double sampleRate){
    
    synth.setCurrentPlaybackSampleRate(sampleRate);
    
    juce::dsp::ProcessSpec processSpec;
    processSpec.sampleRate = sampleRate;
    processSpec.maximumBlockSize = samplesPerBlockExpected;
    processSpec.numChannels = 2;
    
    for(int i = 0 ; i < synth.getNumVoices() ; i++){
        //Dynamic cast safly converts from the base synth voice that the synth returns using .getVoice , to our derived version with the correct prepare function
        auto voice = dynamic_cast<SynthVoice*>(synth.getVoice(i));
        
        if (voice != nullptr)
            voice->prepare(processSpec);
    }
    
}

void SynthAudioSource::releaseResources(){
    
}


void SynthAudioSource::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill){
    
    bufferToFill.clearActiveBufferRegion();
    synth.renderNextBlock(*bufferToFill.buffer, midiBuffer, bufferToFill.startSample, bufferToFill.numSamples);
}
