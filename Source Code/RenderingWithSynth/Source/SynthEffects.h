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

//Base class

class AudioEffect{
    
public:
    
    virtual ~AudioEffect() = default;
    
    virtual float process( float input ) = 0;
    
private:
    
};


//Schroeder style reverb built from comb and all pass filters
class Reverb : public AudioEffect{
    
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
    
    float process(float input) override {
        
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

//Using the delay line circular buffer method but with wet dry level implementation , additionally inherits from Audio Effects for use in the the movable order  vector 
class Delay : public AudioEffect{
    
public:
    
    void prepare();
    
    void setParameters();
    
    float process(float input) override; 
    
private:
    
};

//==============================================================================

//A distortion class which has processes for differnet forms of distortion, hard clipping soft clipping ,  bit crush and then a single process block that uses an enum to select the type of distortion wanted

enum distortionType{
    hardClip,
    softClip,
    sampleReduc
    
};



class Distortion : public AudioEffect {
    
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
    
    float process(float input) override {
        
        
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

enum FilterTypeBiquad{
    
    lowpass,
    highpass,
    bandpass,
    bandreject
};

class BiquadFilter : public AudioEffect {
    
public:
    
    void prepare(double sampleRateIn){
        
        sampleRate = sampleRateIn;
        
        x1 = x2 = 0.0f;
        y1 = y2 = 0.0f;
        
    }
    
    void setParameters(float cutoffFreqIn , float resonanceIn , FilterTypeBiquad filterTypeIn){
        
        filterType = filterTypeIn;
        resonance = std::max(resonanceIn , (float)std::sqrt(0.5));
        cutoffFrequency = juce::jlimit(0.0f , (sampleRate * 0.5f), cutoffFreqIn);
        
        theta = (( 2.0f * juce::MathConstants<float>::pi * cutoffFrequency ) / sampleRate);
        
        if(filterType == lowpass || filterType == highpass){
            b2 = (2.0f * resonance - sin(theta)) / (2.0f * resonance + sin(theta));
            b1 = -(1 + b2) * cos(theta);
        }else if (filterType == bandpass || filterType == bandreject){
            
            b2 = tan((juce::MathConstants<float>::pi)/(4.0f)-(theta / (2.0f * resonance)));
            b1 = -(1 + b2) * cos(theta);
        }
        switch (filterType) {
            case lowpass:
                a0 = 0.25 * (1 + b1 + b2);
                a1 = 2 * a0;
                a2 = a0;
                break;
                
            case highpass:
                a0 = 0.25 * (1 - b1 + b2);
                a1 = -2 * a0;
                a2 = a0;
                break;
                
            case bandpass:
                a0 = 0.5 * (1-b2);
                a1 = 0;
                a2 = -a0;
                break;
                
            case bandreject:
                a0 = 0.5 * (1+b2);
                a1 = 0;
                a2 = a0;
                break;
            
            default:
                break;
        }
        
    }
    
    
    float process(float input) override {
        
        //y(n) = a0 x(n) + a1 (n-1) + a2 (x-2) - b1 y(n-1) - b2 y(n-2)
        //Filter coeffients are calculated from cutoff , resonance and sampleRate, a biquad filter can act as both a lowpass and a highpass filter dependant on the input values
        
        float output = a0 * input +a1 * x1 +a2 * x2 -b1 * y1 - b2 * y2;
        
        x2 = x1;
        x1 = input;
        
        y2 = y1;
        y1 = output;
        
        return output;
    }
    
private:
    
    //For a biquad filter need to know maximum 2 samples prior
    float x1 = 0.0f;
    float x2 = 0.0f;

    float y1 = 0.0f;
    float y2 = 0.0f;
    
    FilterTypeBiquad filterType; //Select between lowpass and highPassFilter, flipped signs on highpassfilter
    float sampleRate;
    float cutoffFrequency; //Fc
    float resonance; //(Q)
    
    //Filter Coefficients and variables for calculating
    
    float a0 = 0.0f;
    float a1 = 0.0f;
    float a2 = 0.0f;
    
    float b1 = 0.0f;
    float b2 = 0.0f;
    
    float theta = 0.0f;
    
};
