/*
  ==============================================================================

    SynthSound.h
    Created: 18 May 2026 3:28:03pm
    Author:  Luke Amos

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>


//Synth sound asks if it should play the note , in our case we should take midi data from everychannel and pass it through to be processed there is no blocking of data taking place.
//Therefore return true for both overrided functions
class SynthSound : public juce::SynthesiserSound{
    
public:
    
    bool appliesToNote(int midiNoteNumber) override{
        
        return true;
        
    }
    
    bool appliesToChannel(int midiChannel) override{
        
        
        return true;
    }
    
    
private:
    
    
};
