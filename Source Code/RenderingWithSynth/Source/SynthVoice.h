/*
  ==============================================================================

    SynthVoice.h
    Created: 18 May 2026 3:27:53pm
    Author:  Luke Amos

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Filters.h"
#include "SynthEffects.h"




class SynthVoice : public juce::SynthesiserVoice{
    
public:
    
    void prepare(const juce::dsp::ProcessSpec& spec);
    
    bool canPlaySound(juce::SynthesiserSound* sound) override;
        
    void startNote(int midiNoteNumber , float velocity , juce::SynthesiserSound* sound , int currentPitchWheelPosition) override;
    
    void stopNote(float velocity , bool allowTailOff) override;
    
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer , int startSample , int numSamples) override;
    
    void pitchWheelMoved(int newValue) override;
    
    void controllerMoved(int controllerNumber, int newValue) override;
    
    void updateParameters(); 
    
private:
    
    juce::dsp::Oscillator<float> osc;
    juce::ADSR adsr;
    
    
    
};
