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
    
private:
    
    juce::Synthesiser synth;
    
    juce::MidiBuffer midiBuffer;

};
