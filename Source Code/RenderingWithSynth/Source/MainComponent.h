#pragma once

#include <JuceHeader.h>
#include "tiny_obj_loader.h"
#include "SynthAudioSource.h"
#include "LookAndFeel.h"
#include "DraggableComponent.h"
#include "Pages.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/

//Three definitions to choose which midi CC message the rotation reacts too
#define CCValUD 1
#define CCValLR 2
#define CCValCW 3

class MainComponent  : public juce::Component , public juce::MidiInputCallback
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;
    //==============================================================================
    //Midi stuff
    void handleIncomingMidiMessage(juce::MidiInput* source , const juce::MidiMessage& message) override;
    

    //==============================================================================
    
    bool keyPressed (const juce::KeyPress& key) override;
    void mouseDown (const juce::MouseEvent& event) override;
    
    void visibilityChanged() override;
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    // Your private member variables go here...
    
    CustomLookAndFeel customLookAndFeel ;
    
    //Buttons to switch views
    juce::TextButton ModelButton{"View Device"} , SynthEffectButton{"Synth"} , MappingButton{"Mappings"};
    
    //Pages
    ModelPage modelPage;
    SynthEffectsPage synthEffectsPage;
    MappingPage mappingPage; 
    
    //Audio
    juce::AudioDeviceManager audioDeviceManager;
    juce::AudioSourcePlayer audioSourcePlayer;
    SynthAudioSource synthAudioSource;
    
    //MidiBuffer for visualiser
    juce::MidiBuffer visMidiBuffer;
    juce::CriticalSection midiMutex;
    
    //Passing data safely through to the visualising midi block
    void addVisMidiMessage(const juce::MidiMessage& message){
        
        juce::ScopedLock lock(midiMutex);
        visMidiBuffer.addEvent(message, 0);
    }
   
    
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
