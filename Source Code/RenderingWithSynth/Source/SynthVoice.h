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

//Keeps a list of each of the parameters exposed to the user , it used so that the user can remap the controls of the midi controller
enum SynthParameter
{
    FilterCutoff,
    FilterResonance,
    ReverbRoomSize,
    ReverbCutoff,
    FilterBiquadType,
    ReverbWetDry,
    ReverbCoefficient,
    DistortionAmount,
    DistortionTypeVal,
    NumParameters //If adding more paramters add them above , number parameters auto updates as you add by increasing the number
};

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
    
    BiquadFilter biquadFilter;
    Reverb reverb;
    Distortion distortion; 
    
    
    
    
    //We have an array of floats holding the current value linked to that parameter 
    float parameters[NumParameters];
    
    
};
