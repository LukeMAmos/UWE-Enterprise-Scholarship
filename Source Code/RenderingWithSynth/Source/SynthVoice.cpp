/*
  ==============================================================================

 SynthVoice.cpp
    Created: 18 May 2026 3:27:53pm
    Author:  Luke Amos

  ==============================================================================
*/

#include "SynthVoice.h"

bool SynthVoice::canPlaySound(juce::SynthesiserSound* sound){
    
    return true;
}


void SynthVoice::prepare(const juce::dsp::ProcessSpec &spec){
    //Send the spec to audio objects
    osc.prepare(spec);
    adsr.setSampleRate(spec.sampleRate);
    
    osc.initialise([](float x) { return std::sin(x); }); //Sine wave function 
    
    biquadFilter.prepare(spec.sampleRate);
    biquadFilter.setParameters(8000.0f, 0.8 , lowpass);
    
    reverb.prepare(spec.sampleRate , 100.0f);
    reverb.setParameters(0.7 , 2500.f , 1.0f , 0.5f);
    
    distortion.prepare(spec.sampleRate);
    distortion.setParameters(5, softClip); 
}

void SynthVoice::startNote(int midiNoteNumber , float velocity , juce::SynthesiserSound* sound , int currentPitchWheelPosition){
    
    //start note
    auto frequency =  juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    osc.setFrequency(frequency);
    
    
    adsr.noteOn(); 
    
}

void SynthVoice::stopNote(float velocity , bool allowTailOff){
    //stop note 
    adsr.noteOff();
    
    if(!allowTailOff)
        clearCurrentNote();
    
}

void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer , int startSample , int numSamples){
    
    juce::dsp::AudioBlock<float> audioBlock(outputBuffer);
        
    auto block = audioBlock.getSubBlock(startSample, numSamples);
        

    juce::dsp::ProcessContextReplacing<float> context(block);
    
    osc.process(context);
    
    adsr.applyEnvelopeToBuffer(outputBuffer, startSample, numSamples);
    
    for(int n = 0 ; n < outputBuffer.getNumChannels(); n++){ //Apply audio effects to the synth output
        
        for(int i = startSample; i < startSample + numSamples ; i++){
            
            //Passing the audio through the effects in a single order
            float effectOut = outputBuffer.getSample(n, i);
        
            for (auto &effect : effectsVector){
                //For each of the effects in effect Vector , as the process block is the same it doesnt need to be cast back into its specific type
                //the new effect out is equal to the current effect out processed by teh next effect in the block 
                effectOut = effect->process(effectOut);
                
            }
        
            outputBuffer.setSample(n, i, effectOut);
        }
    }
    
}

void SynthVoice::pitchWheelMoved(int newValue){
    
    
    
}

void SynthVoice::controllerMoved(int controllerNumber, int newValue){
    
    //Each CC message is linked to a continouus control input which is sent via midi , eg XYZ components from an accelerometer , these may come through on midi CC 0 1 2 , but its up to the user to decide what controls what , In this method it takes what the user has decided to map that message to and updates the array of floats for that specific parameter
    
    updateParameters();
}

void SynthVoice::updateParameters(){
    //Update the parameters used by the effects, Map the values from 0-127 to the right amounts , eg -1 to 1 for coefficient etc etc
    
    float cutoffFreq = juce::jmap(parameters[FilterCutoff], 0.0f, 127.0f, 100.0f, 12000.0f);
    float resonance = juce::jmap(parameters[FilterResonance] , 0.0f , 127.0f, 0.0f , 5.0f);
    FilterTypeBiquad filterType = (FilterTypeBiquad)(int)parameters[FilterBiquadType];
    biquadFilter.setParameters(cutoffFreq, resonance, filterType);
    
    float coeffiecent = juce::jmap(parameters[ReverbCoefficient] , 0.0f , 127.0f, -1.0f , 1.0f);
    float roomSize = juce::jmap(parameters[ReverbRoomSize] , 0.0f , 127.0f, 0.01f , 5.0f);
    float wetAmm = juce::jmap(parameters[ReverbWetDry] , 0.0f , 127.0f, 0.0f , 1.0f);
    float reverbCutoffHz = juce::jmap(parameters[ReverbCutoff] , 0.0f , 127.0f, 0.0f , 5000.0f);
    
    reverb.setParameters(coeffiecent, reverbCutoffHz, roomSize, wetAmm);
    
    float distortionAmount = juce::jmap(parameters[DistortionAmount] , 0.0f , 127.0f, 0.0f , 10.0f);
    distortionType typeIn = (distortionType)(int)parameters[DistortionTypeVal];
    distortion.setParameters(distortionAmount, typeIn);
    
}
