/*
  ==============================================================================

    SynthAudioSource.h
    Created: 18 May 2026 3:28:28pm
    Author:  Luke Amos

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "SynthVoice.h"
#include "SynthSound.h"
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

class SynthAudioSource : public juce::AudioSource{
    
    
    public:
    SynthAudioSource();
    
    void prepareToPlay(int samplesPerBlockExpected ,double sampleRate ) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)override;
    
    //Passing midiMessages through to the midi buffer safely
    void addMidiMessage(const juce::MidiMessage& message){
        
        midiBuffer.addEvent(message, 0);
    }
    juce::Synthesiser& getSynth(){return synth;}
    
    void updateParameters();
    
    private:
    
    juce::Synthesiser synth;
    
    juce::MidiBuffer midiBuffer;
    
    
    //We have an array of floats holding the current value linked to that parameter
    float parameters[NumParameters];
    
    //Effect instancing
    class BiquadFilter biquadFilterL;
    class BiquadFilter biquadFilterR;
    class Reverb reverbL;
    class Reverb reverbR;
    class Distortion distortionL;
    class Distortion distortionR;
    
    //Implementating a reordable system for the effects
    std::vector<std::vector<AudioEffect*>> effectsVectorPerChannel {{&biquadFilterL , &reverbL , &distortionL} , {&biquadFilterR , &reverbR , &distortionR}};

};
