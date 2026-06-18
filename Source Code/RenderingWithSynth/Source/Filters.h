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



//==============================================================================


