/*
  ==============================================================================

    Filters.h
    Created: 20 May 2026 12:10:00pm
    Author:  Luke Amos

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "DelayLine.h"
#include <cmath>

class CombFilter{
    
public:
    
    void prepare(double sampleRateIn ,float maxDelayMs){
        
        delayLine.prepare(sampleRateIn , maxDelayMs);
        sampleRate = sampleRateIn;
        filterState = 0.0f;
    }
    
    void setParameters(float feedbackIn , float cutoffHz , float delayTimeMs){
        
        feedback = juce::jlimit(0.0f , 0.99f , feedbackIn);
        damping = expf(-2.0f * juce::MathConstants<float>::pi * cutoffHz / sampleRate); //Equation to find the damping amount from cutoff frequency
        delayLine.setDelayLine(delayTimeMs);
    }
    
    float processSample(float input){
        
        float delayed = delayLine.processSample(input + filterState * feedback);

        filterState = delayed * (1.0f - damping)
                    + filterState * damping;

        return delayed;
        
    }

private:

    DelayLine delayLine;
    float feedback;
    float sampleRate;
    float damping;
    float filterState;
    
};

//==============================================================================
class AllPassFilter{
    
public:
    
    void prepare(double sampleRateIn , float maxDelayMs){
        int maxSamples = sampleRateIn * (maxDelayMs / 1000);
        
        inputBuffer.resize(maxSamples, 0.0f);
        outputBuffer.resize(maxSamples , 0.0f);
        
        writeIndex = 0;
        delaySamples = 0;
        sampleRate = sampleRateIn;
        
    }
    
    void setParameters(float delayMs , float coefficient){
        
        delaySamples = std::min((int)(sampleRate * (delayMs / 1000)) , (int)(inputBuffer.size() -1 ));
        a = juce::jlimit(-0.99f, 0.99f, coefficient);
        
        std::fill(inputBuffer.begin(), inputBuffer.end(), 0.0f);
        std::fill(outputBuffer.begin(), outputBuffer.end(), 0.0f);
        
    }
    
    float process(float input){
        
        int readIndex = (writeIndex - delaySamples + inputBuffer.size()) % inputBuffer.size();
        
        float xd = inputBuffer[readIndex];
        float yd = outputBuffer[readIndex];
        
        float y = -a * input + xd + a * yd;
        
        inputBuffer[writeIndex] = input;
        outputBuffer[writeIndex] = y;
        
        writeIndex = (writeIndex + 1) % inputBuffer.size();
        
        return y;
    }

private:
    
    std::vector<float> inputBuffer;
    std::vector<float> outputBuffer;
    
    int writeIndex = 0;
    int delaySamples = 0;
    float a = 0.0f;
    double sampleRate = 0.0;
};

//==============================================================================
enum FilterTypeBiquad{
    
    lowpass,
    highpass,
    bandpass,
    bandreject
};

class BiquadFilter{
    
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
    
    
    float process(float input){
        
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


//==============================================================================


