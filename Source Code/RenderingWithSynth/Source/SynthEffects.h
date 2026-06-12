/*
  ==============================================================================

    SynthEffects.h
    Created: 19 May 2026 8:28:46pm
    Author:  Luke Amos

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Filters.h"

//Schroeder style reverb built from comb and all pass filters 
class Reverb{
    
public:
    
    void prepare(double sampleRate , float maxDelayMs ){
        
        for(int i = 0 ; i < 6 ; i++){
            
            combFilters[i].prepare(sampleRate , maxDelayMs);
        }
        
        for(int i = 0; i < 4 ; i++){
            
            allPassFilters[i].prepare(sampleRate , maxDelayMs);
        }
    }
    
    void setParameters(float coeffiecent , float cutoffHz , float roomSize , float wetAmountIn){
        
        for(int i = 0 ; i < 6 ; i++){
            
            combFilters[i].setParameters(gains[i], cutoffHz, times[i] * roomSize); // roomSize multiples the times values to create a larger preciving reverb
        }
        
        for(int i = 0; i < 4 ; i++){
            
            allPassFilters[i].setParameters(apfTimes[i], coeffiecent);
        }
        
        wetAmount = wetAmountIn;
        
    }
    
    float process(float input){
        
        juce::ScopedNoDenormals noDenormals;
        
        //Shroeder style reverb processes the comb filters in parrellel and then  the all pass filters in series afterwards
        //Loop through the comb filters and store the output value of each in a single variable to be outputted
        float combOut = 0.0f;
        for(int i = 0 ; i < 6; i ++){
            
            combOut += combFilters[i].processSample(input);
            
        }
        
        combOut *= 1.0f / 6.0f; //Normalise the values
        
        //Pass the output of the combFilters into the first apf and then into the next subsequent all pass filter
        
        float a0 = allPassFilters[0].process(combOut);
        float a1 = allPassFilters[1].process(a0);
        float a2 = allPassFilters[2].process(a1);
        float a3 = allPassFilters[3].process(a2);
        
        float output = a3;
        
        return input * (1-wetAmount)+ output * wetAmount;
    }
    
private:
    
    CombFilter combFilters[6];
    AllPassFilter allPassFilters[4];
    
    //All Pass filter times
    float apfTimes[4] = { 5.118, 7.762, 9.957, 12.402 };
    
    //COMB Filter Times
    float times[6] = { 21.884f, 25.885f, 28.934f, 30.739f, 53.229f, 62.829f };
    float gains[6] = { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };
    
    float wetAmount = 0.0f;
    
};

//==============================================================================

//A distortion class which has processes for differnet forms of distortion, hard clipping soft clipping ,  bit crush and then a single process block that uses an enum to select the type of distortion wanted

enum distortionType{
    hardClip,
    softClip,
    sampleReduc
    
};



class Distortion{
    
public:
    
    void prepare(double sampleRateIn){
        
        sampleRate = sampleRateIn;
        distortionAmount = 0.0f;
        counter = 0;
        heldValue = 0.0f;
        
    }
    
    void setParameters(float distortionAmountIn , distortionType typeIn ){
        
        distortionAmount = std::max(0.001f, distortionAmountIn);;
        type = typeIn;
        
    }
    
    float process(float input){
        
        
        float output = 0.0f;
        
        switch (type) {
            case hardClip:
                output = hardClipping(input);
                break;
             
            case softClip:
                output = softClipping(input);
                break;
                
            case sampleReduc:
                output = sampleReduce(input);
                break;
                
                
            default:
                break;
        }
        
        
        return output;
    }
    
    
private:
    
    float sampleRate;
    
    float distortionAmount;
    distortionType type;
    
    int counter;
    float heldValue;
    
    float hardClipping(float input){
        
        
        float output = 0.0f;
        
        input *= distortionAmount;
        
        if(input > 1 ){
            input = 1;
        }else if(input < -1 ){
            input = -1;
        }
        
        output = input / distortionAmount;
        
        return output;
    }
    
    float softClipping(float input){
        
        
        float output = 0.0f;
        
        
        input *= distortionAmount;
        
        output = tanh(input) / tanh(distortionAmount);
        
        return output;
    }
    
    float sampleReduce(float input){
        
        float output = 0.0f;
        
        int sampleCountMax = distortionAmount * 50;
        
        if(counter >= sampleCountMax){
            
            heldValue = input;
            counter = 0;
        }
        
        output = heldValue;
        counter++;
        
        return output;
    }
    

    
};

//==============================================================================

