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
    
    
    
    
    //Left channel effects
    biquadFilterL.prepare(sampleRate);
    biquadFilterL.setParameters(8000.0f, 0.8 , lowpass);
    
    reverbL.prepare(sampleRate , 100.0f);
    reverbL.setParameters(0.7 , 2500.f , 1.0f , 0.5f);
    
    distortionL.prepare(sampleRate);
    distortionL.setParameters(5, softClip);
    
    //Right channel effects 
    biquadFilterR.prepare(sampleRate);
    biquadFilterR.setParameters(8000.0f, 0.8 , lowpass);
    
    reverbR.prepare(sampleRate , 100.0f);
    reverbR.setParameters(0.7 , 2500.f , 1.0f , 0.5f);
    
    distortionR.prepare(sampleRate);
    distortionR.setParameters(5, softClip);
}

void SynthAudioSource::releaseResources(){
    
}


void SynthAudioSource::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill){
    
    auto& buffer = *bufferToFill.buffer;
    int startSample = bufferToFill.startSample;
    int numSamples = bufferToFill.numSamples;
    
    
    //Get the output of the synth and then apply the effects and then fill buffer
    bufferToFill.clearActiveBufferRegion();
    synth.renderNextBlock(buffer, midiBuffer, startSample, numSamples);
    
    
    
    for(int n = 0 ; n < buffer.getNumChannels(); n++){ //Apply audio effects to the synth output
        
        for(int i = startSample; i < startSample + numSamples ; i++){
            
            //Passing the audio through the effects in a single order
            float effectOut = buffer.getSample(n, i);
        
            for (auto &effect : effectsVectorPerChannel[n]){
                //For each of the effects in effect Vector , as the process block is the same it doesnt need to be cast back into its specific type
                //the new effect out is equal to the current effect out processed by teh next effect in the block
                effectOut = effect->process(effectOut);
                
            }
        
            buffer.setSample(n, i, effectOut);
        }
    }
    
}

void SynthAudioSource::updateParameters(){
    //Update the parameters used by the effects, Map the values from 0-127 to the right amounts , eg -1 to 1 for coefficient etc etc
    
    float cutoffFreq = juce::jmap(parameters[FilterCutoff], 0.0f, 127.0f, 100.0f, 12000.0f);
    float resonance = juce::jmap(parameters[FilterResonance] , 0.0f , 127.0f, 0.0f , 5.0f);
    FilterTypeBiquad filterType = (FilterTypeBiquad)(int)parameters[FilterBiquadType];
    biquadFilterL.setParameters(cutoffFreq, resonance, filterType);
    biquadFilterR.setParameters(cutoffFreq, resonance, filterType);
    
    float coeffiecent = juce::jmap(parameters[ReverbCoefficient] , 0.0f , 127.0f, -1.0f , 1.0f);
    float roomSize = juce::jmap(parameters[ReverbRoomSize] , 0.0f , 127.0f, 0.01f , 5.0f);
    float wetAmm = juce::jmap(parameters[ReverbWetDry] , 0.0f , 127.0f, 0.0f , 1.0f);
    float reverbCutoffHz = juce::jmap(parameters[ReverbCutoff] , 0.0f , 127.0f, 0.0f , 5000.0f);
    
    reverbL.setParameters(coeffiecent, reverbCutoffHz, roomSize, wetAmm);
    reverbR.setParameters(coeffiecent, reverbCutoffHz, roomSize, wetAmm);
    
    float distortionAmount = juce::jmap(parameters[DistortionAmount] , 0.0f , 127.0f, 0.0f , 10.0f);
    distortionType typeIn = (distortionType)(int)parameters[DistortionTypeVal];
    distortionL.setParameters(distortionAmount, typeIn);
    distortionR.setParameters(distortionAmount, typeIn);
    
}
