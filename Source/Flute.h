/*
  ==============================================================================

  Flute.h
  Author:  Kangrui Xue

  ==============================================================================
*/

#include "Util.h"

struct FluteSound   : public SynthesiserSound
{
    FluteSound() {}
    bool appliesToNote    (int) override        { return true; }
    bool appliesToChannel (int) override        { return true; }
};

class FluteVoice   : public SynthesiserVoice
{
public:
    FluteVoice() { }
    FluteVoice(const juce::dsp::ProcessSpec& spec)
    {
    /*  Sets up filters and waveform generators to match the specified audio processing context

        Parameters
        ------------
          spec : ProcessSpec : wrapper for sampling rate, sample block size, and number of channels
    */
        reflection.prepare (spec);
        float pole = 0.7 - ( 0.1 * 22050.0 / getSampleRate() );
        reflection.coefficients = new juce::dsp::IIR::Coefficients<float>(0.35, 0.0, 1.0, -pole);
        dcBlock.prepare (spec);
        dcBlock.coefficients = new juce::dsp::IIR::Coefficients<float>(1.0, -1.0, 1.0, -0.995);
        sine.setFreq(5.925, getSampleRate());
    }
    bool canPlaySound (SynthesiserSound* sound) override
    {
        return dynamic_cast<FluteSound*> (sound) != nullptr;
    }
    void startNote (int midiNoteNumber, float velocity, SynthesiserSound*, int) override
    {
    /*  triggered on MIDI signal, configures DSP flute based on MIDI data

        Parameters
        ------------
          midiNoteNumber: int   : MIDI data corresponding to the desired pitch
          velocity      : float : MIDI data corresponding to desired volume (unused)
        ------------
        Returns         : void
    */
        auto length = (size_t) juce::roundToInt(getSampleRate() / MidiMessage::getMidiNoteInHertz(midiNoteNumber-19));
        boreDelay.resize(length);
        jetDelay.resize(0.33*length);
        boreDelay.clear();
        jetDelay.clear();
        airSpeed = 1.0;
    }
    void stopNote (float velocity, bool allowTailOff) override
    {
        airSpeed = 0.0;
    }
    void pitchWheelMoved (int) override      {}
    void controllerMoved (int, int) override {}

    void renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
    {
    /*  Copies DSP flute output into audio buffer (stereo)

        Parameters
        ------------
          outputBuffer : AudioSampleBuffer : audio buffer to interface with speaker firmware
          startSample  : int : start index of first sample to copy
          numSamples   : int : number of samples to copy
        ------------
        Returns         : void
    */
        float* bufferLeft = outputBuffer.getWritePointer(0, startSample);
        float* bufferRight = outputBuffer.getWritePointer(1, startSample);
        for (int i = 0; i < numSamples; ++i) {
            bufferLeft[i] = processSample();
            bufferRight[i] = bufferLeft[i];
        }
    }

private:
    float airSpeed = 0.0;

    Random random;
    Sine sine;
    DelayLine<float> boreDelay;
    DelayLine<float> jetDelay;
    juce::dsp::IIR::Filter<float> reflection;
    juce::dsp::IIR::Filter<float> dcBlock;

    double processSample() noexcept
    {
    /*  Iterates through sample by sample.
        [1] computes response of jet (mouth to flute length) delay
        [2] computes response of bore (actual length of flute) delay

        Parameters
        ------------
        Returns: double
    */
        float input = airSpeed;
        input += (0.15*airSpeed) * ( 2.0*random.nextFloat() - 1.0 ); // noise
        input += (0.05*airSpeed) * sine.nextFloat(); // vibrato

        float boreOut = boreDelay.back();
        float temp = reflection.processSample (boreOut);
        temp = dcBlock.processSample (temp);

        jetDelay.push (input + 0.5*temp);
        float jetOut = jetTable(jetDelay.back());
        boreDelay.push (jetOut - 0.5*temp);

        return 0.3*boreOut;
    }
    float jetTable (float input)
    {
    /*  cubic interpolation to introduce non-linearity

        Parameters
        ------------
          input: float : DSP sample value
        ------------
        Returns: float
    */
        float output = input * (input*input - 1.0);
        if ( output > 1.0 ) output = 1.0;
        if ( output < -1.0 ) output = -1.0;
        return output;
    }
};
