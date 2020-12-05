/*
  ==============================================================================

    Util.h
    Author:  Kangrui Xue

  ==============================================================================
*/
#pragma once
#include "../JuceLibraryCode/JuceHeader.h"

template <typename Type>
class DelayLine
{
public:
    DelayLine() { data.resize(1); }
    void clear() noexcept
    {
        std::fill (data.begin(), data.end(), Type (0));
    }
    size_t size() const noexcept
    {
        return data.size();
    }
    void resize (size_t newValue)
    {
        data.resize (newValue);
        leastRecentIndex = 0;
    }
    Type back() const noexcept
    {
        return data[leastRecentIndex];
    }
    Type get (size_t delayInSamples) const noexcept
    {
        jassert (delayInSamples >= 0 && delayInSamples < size());
        return data[(leastRecentIndex + 1 + delayInSamples) % size()];
    }
    /** Set the specified sample in the delay line */
    void set (size_t delayInSamples, Type newValue) noexcept
    {
        jassert (delayInSamples >= 0 && delayInSamples < size());
        data[(leastRecentIndex + 1 + delayInSamples) % size()] = newValue;
    }
    /** Adds a new value to the delay line, overwriting the least recently added sample */
    void push (Type valueToAdd) noexcept
    {
        data[leastRecentIndex] = valueToAdd;
        leastRecentIndex = leastRecentIndex == 0 ? size() - 1 : leastRecentIndex - 1;
    }

private:
    std::vector<Type> data;
    size_t leastRecentIndex = 0;
};

class Sine
{
public:
    Sine() {}
    void setFreq(double freqHz, double sampleRateHz)
    {
        currentAngle = 0.0;
        angleDelta = freqHz * 2.0 * MathConstants<double>::pi / sampleRateHz;
    }
    float nextFloat()
    {
        currentAngle += angleDelta;
        return (float) std::sin (currentAngle);
    }

private:
    float currentAngle, angleDelta;
};


//==============================================================================
template <typename SampleType>
class AudioBufferQueue
{
public:
    //==============================================================================
    static constexpr size_t order = 9;
    static constexpr size_t bufferSize = 1U << order;
    static constexpr size_t numBuffers = 5;
    
    //==============================================================================
    void push (const SampleType* dataToPush, size_t numSamples)
    {
        jassert (numSamples <= bufferSize);
        
        int start1, size1, start2, size2;
        abstractFifo.prepareToWrite (1, start1, size1, start2, size2);
        
        jassert (size1 <= 1);
        jassert (size2 == 0);
        
        if (size1 > 0)
            FloatVectorOperations::copy (buffers[(size_t) start1].data(), dataToPush, (int) jmin (bufferSize, numSamples));
        
        abstractFifo.finishedWrite (size1);
    }
    
    //==============================================================================
    void pop (SampleType* outputBuffer)
    {
        int start1, size1, start2, size2;
        abstractFifo.prepareToRead (1, start1, size1, start2, size2);
        
        jassert (size1 <= 1);
        jassert (size2 == 0);
        
        if (size1 > 0)
            FloatVectorOperations::copy (outputBuffer, buffers[(size_t) start1].data(), (int) bufferSize);
        
        abstractFifo.finishedRead (size1);
    }
    
private:
    //==============================================================================
    AbstractFifo abstractFifo { numBuffers };
    std::array<std::array<SampleType, bufferSize>, numBuffers> buffers;
    };
    
    //==============================================================================
    template <typename SampleType>
    class ScopeDataCollector
    {
    public:
        //==============================================================================
        ScopeDataCollector (AudioBufferQueue<SampleType>& queueToUse)
        : audioBufferQueue (queueToUse)
        {}
        
        //==============================================================================
        void process (const SampleType* data, size_t numSamples)
        {
            size_t index = 0;
            
            if (state == State::waitingForTrigger)
            {
                while (index++ < numSamples)
                {
                    auto currentSample = *data++;
                    
                    if (currentSample >= triggerLevel && prevSample < triggerLevel)
                    {
                        numCollected = 0;
                        state = State::collecting;
                        break;
                    }
                    
                    prevSample = currentSample;
                }
            }
            
            if (state == State::collecting)
            {
                while (index++ < numSamples)
                {
                    buffer[numCollected++] = *data++;
                    
                    if (numCollected == buffer.size())
                    {
                        audioBufferQueue.push (buffer.data(), buffer.size());
                        state = State::waitingForTrigger;
                        prevSample = SampleType (100);
                        break;
                    }
                }
            }
        }
        
    private:
        //==============================================================================
        AudioBufferQueue<SampleType>& audioBufferQueue;
        std::array<SampleType, AudioBufferQueue<SampleType>::bufferSize> buffer;
        size_t numCollected;
        SampleType prevSample = SampleType (100);
        
        static constexpr auto triggerLevel = SampleType (0.05);
        
        enum class State { waitingForTrigger, collecting } state { State::waitingForTrigger };
    };
    
    //==============================================================================
    template <typename SampleType>
    class ScopeComponent  : public juce::Component,
    private Timer
    {
    public:
        using Queue = AudioBufferQueue<SampleType>;
        
        //==============================================================================
        ScopeComponent (Queue& queueToUse)
        : audioBufferQueue (queueToUse)
        {
            sampleData.fill (SampleType (0));
            setFramesPerSecond (30);
        }
        
        //==============================================================================
        void setFramesPerSecond (int framesPerSecond)
        {
            jassert (framesPerSecond > 0 && framesPerSecond < 1000);
            startTimerHz (framesPerSecond);
        }
        
        //==============================================================================
        void paint (Graphics& g) override
        {
            g.fillAll (juce::Colours::black);
            g.setColour (juce::Colours::white);
            
            auto area = getLocalBounds();
            auto h = (SampleType) area.getHeight();
            auto w = (SampleType) area.getWidth();
            
            // Oscilloscope
            auto scopeRect = Rectangle<SampleType> { SampleType (0), SampleType (0), w, h*2/3 };
            plot (sampleData.data(), sampleData.size(), g, scopeRect, SampleType (1), h/6);
            
            // Spectrum
            //auto spectrumRect = Rectangle<SampleType> { SampleType (0), h / 2, w, h / 2 };
            //plot (spectrumData.data(), spectrumData.size() / 4, g, spectrumRect);
        }
        
        //==============================================================================
        void resized() override {}
        
    private:
        //==============================================================================
        Queue& audioBufferQueue;
        std::array<SampleType, Queue::bufferSize> sampleData;
        
        juce::dsp::FFT fft { Queue::order };
        using WindowFun = juce::dsp::WindowingFunction<SampleType>;
        WindowFun windowFun { (size_t) fft.getSize(), WindowFun::hann };
        std::array<SampleType, 2 * Queue::bufferSize> spectrumData;
        
        //==============================================================================
        void timerCallback() override
        {
            audioBufferQueue.pop (sampleData.data());
            FloatVectorOperations::copy (spectrumData.data(), sampleData.data(), (int) sampleData.size());
            
            auto fftSize = (size_t) fft.getSize();
            
            jassert (spectrumData.size() == 2 * fftSize);
            windowFun.multiplyWithWindowingTable (spectrumData.data(), fftSize);
            fft.performFrequencyOnlyForwardTransform (spectrumData.data());
            
            static constexpr auto mindB = SampleType (-160);
            static constexpr auto maxdB = SampleType (0);
            
            for (auto& s : spectrumData)
                s = jmap (jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (s) - juce::Decibels::gainToDecibels (SampleType (fftSize))), mindB, maxdB, SampleType (0), SampleType (1));
            
            repaint();
        }
        
        //==============================================================================
        static void plot (const SampleType* data,
                          size_t numSamples,
                          Graphics& g,
                          juce::Rectangle<SampleType> rect,
                          SampleType scaler = SampleType (1),
                          SampleType offset = SampleType (0))
        {
            auto w = rect.getWidth();
            auto h = rect.getHeight();
            auto right = rect.getRight();
            
            auto center = rect.getBottom() - offset;
            auto gain = h * scaler;
            
            for (size_t i = 1; i < numSamples; ++i)
                g.drawLine ({ jmap (SampleType (i - 1), SampleType (0), SampleType (numSamples - 1), SampleType (right - w), SampleType (right)),
                    center - gain * data[i - 1],
                    jmap (SampleType (i), SampleType (0), SampleType (numSamples - 1), SampleType (right - w), SampleType (right)),
                    center - gain * data[i] });
        }
    };
