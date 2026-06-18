#pragma once

#include <stdio.h>
#include <JuceHeader.h>

class draggableComponent : public juce::Component
{
    
    public:
    draggableComponent(){
        setWantsKeyboardFocus(true);
        
        slider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        
        addAndMakeVisible(slider);
    };
    
    void mouseDown(const juce::MouseEvent& event) override {
        dragger.startDraggingComponent(this, event);
    }
    
    void mouseDrag(const juce::MouseEvent& event) override {
        dragger.dragComponent(this, event, nullptr);
    }
    
    void mouseUp(const juce::MouseEvent &event) override {
        
        
    }
    
    void paint(juce::Graphics& g )override{
        
        std::cout<<"Here is myX :"<<getX() <<"\n";
        
        g.setColour (juce::Colours::blue);
        
        g.fillEllipse(0, 0, diameter, diameter);
        
    }
    
    void resized() override{
        
        slider.setBounds(( getWidth() /2 ) - 35, (getHeight() / 2) - 35 , 70, 70);
        
    };
    
    
    
    int getPositionXCentre(){return getX() + diameter / 2;};
    int getPositionYCentre(){return getY() + diameter / 2;};
    
    bool mDown = false;
    juce::Slider slider;
private:
    
    int diameter = 100;
    
    juce::ComponentDragger dragger;
    
};
