#pragma once

#include <stdio.h>
#include <JuceHeader.h>
//DraggableComponent for reordering the synth effects 
class DraggableComponent : public juce::Component
{
    
    public:
    DraggableComponent(){
        setWantsKeyboardFocus(true);
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
        
        g.drawRoundedRectangle(0, 0, 100, 100, 8, 8);
        
    }
    
    void resized() override{
        
        slider.setBounds(( getWidth() /2 ) - 35, (getHeight() / 2) - 35 , 70, 70);
        
    };
    
    
    
    int getPositionXCentre(){return getX() + 100 / 2;};
    int getPositionYCentre(){return getY() + 100 / 2;};
    
    bool mDown = false;
    juce::Slider slider;
private:
    
    
    juce::ComponentDragger dragger;
    
};
