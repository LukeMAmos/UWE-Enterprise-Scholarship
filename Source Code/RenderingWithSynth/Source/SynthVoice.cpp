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
    
}

void SynthVoice::pitchWheelMoved(int newValue){
    
    
    
}

void SynthVoice::controllerMoved(int controllerNumber, int newValue){
    
    //Each CC message is linked to a continouus control input which is sent via midi , eg XYZ components from an accelerometer , these may come through on midi CC 0 1 2 , but its up to the user to decide what controls what , In this method it takes what the user has decided to map that message to and updates the array of floats for that specific parameter
    
    updateParameters();
}


