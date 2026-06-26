#include "MainComponent.h"
#include <cmath>

//==============================================================================
MainComponent::MainComponent()
{
    setSize (600, 400);
    setOpaque(true);

    //Custom look and feel
    setLookAndFeel(&customLookAndFeel); 
    
    //Buttons for switching pages
    addAndMakeVisible(ModelButton);
    addAndMakeVisible(SynthEffectButton);
    addAndMakeVisible(MappingButton);
    
    setWantsKeyboardFocus(true);
    
    
    //Audio connections
    
    audioDeviceManager.initialiseWithDefaultDevices(0, 2); //Open Audio Hardware
    
    audioSourcePlayer.setSource(&synthAudioSource); //set the Audio Source in this case the synthesiser
    
    audioDeviceManager.addAudioCallback(&audioSourcePlayer); // pass the audioSource to the Audiodevice manager to be outputted through the hardware
    
    //Midi connections
    
    auto midiDevices = juce::MidiInput::getAvailableDevices();
    
    //loop through the list of devices and set them all to be enables , this means that all midi inputs are passed through
    for (auto& device : midiDevices){
        
        audioDeviceManager.setMidiInputDeviceEnabled(device.identifier, true);
        audioDeviceManager.addMidiInputDeviceCallback(device.identifier, this);
        
    }
    

}

void MainComponent::visibilityChanged()
{
    if (isShowing())
    {
        grabKeyboardFocus();
    }
}





void MainComponent::handleIncomingMidiMessage(juce::MidiInput* source , const juce::MidiMessage& message){
    
    //Midi messages are sent through straight to the synth as well as sending through to the visual midi buffer where it is used to update the 3D model of the device
    
    //Pass the midiMessages through to the synthesiser to be used
    synthAudioSource.addMidiMessage(message);
    
    //Pass midi messages through to main component to be used to control visualisation
    addVisMidiMessage(message); 
}


bool MainComponent::keyPressed (const juce::KeyPress& key){
    
    
    bool val = false;
    /*
    switch (key.getTextCharacter()) {
        case 'a':
            rotationAngleLR += 0.1;
            val = true;
            break;
        case 's':
            rotationAngleUD -=0.1;
            val = true;
            break;
        case 'd':
            rotationAngleLR -= 0.1;
            val = true;
            break;
        case 'w':
            rotationAngleUD += 0.1;
            val = true;
            break;
            
        default:
            break;
    }
    */
    return val;
}

void MainComponent::mouseDown (const juce::MouseEvent& event)
{
    // Generate random values between 0.0f and 1.0f for Red, Green, and Blue
    auto& random = juce::Random::getSystemRandom();
    /*
    cubeColor[0] = random.nextFloat(); // Red
    cubeColor[1] = random.nextFloat(); // Green
    cubeColor[2] = random.nextFloat(); // Blue
    cubeColor[3] = 1.0f;               // Alpha (fully opaque)
    */
    
    //On every cick update the arrangement of the effects
    
    
}



MainComponent::~MainComponent()
{
    //Release all resources
    
    audioDeviceManager.removeAudioCallback(&audioSourcePlayer);
    audioSourcePlayer.setSource(nullptr);
    
    for (auto& device : juce::MidiInput::getAvailableDevices())
    {
        audioDeviceManager.removeMidiInputDeviceCallback(device.identifier, this);
    }
    
    setLookAndFeel(nullptr);
    
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{


    g.setColour (juce::Colours::white);


    g.drawText ("Use W, A, S, D to rotate the cube",
                10, 10, 400, 30,
                juce::Justification::left);

    /*
    //Want to keep these values inbetween 0 and 1 ,
    float rotValLR = std::fmod(rotationAngleLR , 1.0f);
    float rotValUD = std::fmod(rotationAngleUD, 1.0f);
    
    if (rotValLR < 0.0f) rotValLR += 1.0f;
    if (rotValUD < 0.0f) rotValUD += 1.0f;
    
    juce::String debugInfo = "Rotation LR: " + juce::String ((rotValLR), 2)
                           + " | UD: " + juce::String (rotValUD, 2);
    
    g.drawText (debugInfo,
                10, getHeight() - 40, getWidth() - 20, 30,
                juce::Justification::left);
     */
}


void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    
    ModelButton.setBounds(<#Rectangle<int> newBounds#>);
    SynthEffectButton.setBounds();
    MappingButton.setBounds(); 
    
    
}
