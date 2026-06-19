#pragma once

#include <JuceHeader.h>
#include "tiny_obj_loader.h"
#include "SynthAudioSource.h"
#include "LookAndFeel.h"
#include "DraggableComponent.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/

//Three definitions to choose which midi CC message the rotation reacts too
#define CCValUD 1
#define CCValLR 2
#define CCValCW 3

class MainComponent  : public juce::OpenGLAppComponent , public juce::MidiInputCallback
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;
    //==============================================================================
    //OpenGL stuff
    void initialise() override;
    void render() override;
    void shutdown() override;
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
    
    //Rendering 3D Models
    tinyobj::attrib_t  meshAttrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<float> flatVertices;
    
    float rotationAngleLR = 0.0f;
    float rotationAngleUD = 0.0f;
    float rotationAngleCW = 0.0f; 
    
    GLuint vbo = 0;
    GLuint vao = 0;
    
    std::unique_ptr<juce::OpenGLShaderProgram> shaderProgram;

    float cubeColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    
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
    
    DraggableComponent dragComp ; 
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
