#pragma once

#include <stdio.h>
#include <JuceHeader.h>
//DraggableComponent for reordering the synth effects 
class DraggableComponent : public juce::Component
{
    
public:
    //Each of the components needs specific controls , so they share a common type of components
    DraggableComponent(String nameIn = "Draggable Component" , std::vector<std::unique_ptr<juce::Component>> comps = {}){
        
        name = nameIn ;
        setWantsKeyboardFocus(true);
        
        ownedComponents = std::move(comps); //transfer ownership to the class vector
        
        for(auto& comp : ownedComponents){
            
            addAndMakeVisible(*comp);
        }
        
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
        
        std::cout<<"Here is myX :"<<getX() <<"  : "<< name << "\n";
        
        g.setColour (juce::Colours::blue);
        
        g.drawText(name, 0, 0, 50, 50, juce::Justification::centredLeft);
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
    
    String name;
    juce::ComponentDragger dragger;
    
    std::vector<std::unique_ptr<juce::Component>> ownedComponents;

};
