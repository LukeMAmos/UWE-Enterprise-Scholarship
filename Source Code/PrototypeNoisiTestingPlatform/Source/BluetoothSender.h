/*
  ==============================================================================

    BluetoothSender.h
    Created: 29 Jun 2026 11:58:47am
    Author:  Luke Amos

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

//Structure for paramters used to send the bluetooth data
struct Parameter {
    std::string ParamID;
    std::string uuid;
    std::unique_ptr<juce::Component> paramDevice ;
    bool oldData = false; //Assume the data is fresh on start up
};




class BluetoothSender {
    
public:
    BluetoothSender();
    ~BluetoothSender();
    
    void prepareSend(Parameter& param){
        //Take the chosen parameter and send its raw data through to send value where it is then sent via bluetooth
        float value = 0.0f;
        std::string paramID = param.ParamID;
        std::string uuid = param.uuid;
        
        
        //Casting the paramDevice to its correct type and setting the value
        if (auto* s = dynamic_cast<juce::Slider*>(param.paramDevice.get()))
                value = (float) s->getValue();
            else if (auto* c = dynamic_cast<juce::ComboBox*>(param.paramDevice.get()))
                value = (float) c->getSelectedId();
        
        
        
        sendValue(value, paramID, uuid);
    }
    void sendValue(float value, const std::string paramID, const std::string uuid); //Send the parameter packaged correctly
    
private:
    void *impl = nullptr; //Pointer to OBJC delegate object 
    
};
