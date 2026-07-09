#pragma once

#include <JuceHeader.h>
#include "LookAndFeel.h"
#include "BluetoothSender.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::Component
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    // Your private member variables go here...
    CustomLookAndFeel customLookAndFeel ;
    
    BluetoothSender bluetoothSender;
        
    //Parameter data
    std::string SynthVoiceUUID = "1baa65f0-bc68-4e6a-99a1-b0145f960382";
    std::string FilterUUID     = "afda78fa-ec12-4bcb-8fca-791e43416598";
    std::string ReverbUUID     = "b5ecef56-3dad-49c9-a7de-84f29933cf9b";
    std::string DistortionUUID = "e1d5977d-8227-4d6d-94de-ba28562ac507";
    
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
