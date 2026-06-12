/*
  ==============================================================================

    DelayLine.h
    Created: 20 May 2026 11:57:06am
    Author:  Luke Amos

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

//Circular Buffer class which is the base for combFilter
class DelayLine{
    
public:
    
    void prepare(double sampleRateIn, float maxDelayMs){
        
        sampleRate = sampleRateIn;
        
        int maxBufferSize = sampleRate * (maxDelayMs / 1000);
        buffer.resize(maxBufferSize, 0);
        writePosition = 0;
        
    }

    void setDelayLine(float ms){
        
        delaySamples = std::min((int)(sampleRate * (ms / 1000.0f)), (int)buffer.size() - 1);
        
    }

    float processSample(float input){
        
        int readPosition = (int)(writePosition - delaySamples + buffer.size()) % buffer.size();
        
        float delayedSample = buffer[readPosition];
        
        buffer[writePosition] = input;
        writePosition = (writePosition + 1 ) % buffer.size();
        
        return delayedSample;
    }
    
    float bufferValue(int samplePos){
        
        return buffer[samplePos];
    }
    
private:
    
    std::vector<float> buffer;
    int writePosition;
    int delaySamples;
    double sampleRate;
    
};


