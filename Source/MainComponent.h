/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "Flute.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/

class MainComponent   : public AudioAppComponent, public MidiKeyboardStateListener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    MouseListener mouse; // for debugging

    Image fluteImg;
    ImageButton keys[6];

    MidiKeyboardState keyboardState;
    MidiKeyboardComponent keyboardComponent;
    Synthesiser synth;
    
    AudioBufferQueue<float> audioBufferQueue;
    ScopeDataCollector<float> scopeDataCollector { audioBufferQueue };
    ScopeComponent<float> scopeComponent;

    void handleKeyClicked()
    {
    /*  called when the user presses any key on the flute. First checks the configuration of
        keys, and if a valid fingering, triggers a MIDI event corresponding to playing the note.

        Parameters
        ------------
        Returns: void
    */
        keyboardState.allNotesOff(1);
        short mask = 0b000000;
        for (int i = 0; i < 6; i++)
            mask |= (keys[i].getToggleState() << i);

        switch(mask)
        {
          //case 0 :
            case 0b000000: keyboardState.noteOn(1, 73, 1.0); break;
            case 0b111111: keyboardState.noteOn(1, 74, 1.0); break;
          //case 3 :
            case 0b011111: keyboardState.noteOn(1, 76, 1.0); break;
            case 0b001111: keyboardState.noteOn(1, 77, 1.0); break;
            case 0b100111: keyboardState.noteOn(1, 78, 1.0); break;
            case 0b000111: keyboardState.noteOn(1, 79, 1.0); break;
          //case 8 :
            case 0b000011: keyboardState.noteOn(1, 81, 1.0); break;
            case 0b001001: keyboardState.noteOn(1, 82, 1.0); break;
            case 0b000001: keyboardState.noteOn(1, 83, 1.0); break;
        }
    }
    void handleNoteOn (MidiKeyboardState *source, int midiChannel, int midiNoteNumber, float velocity) override
    {
    /*  called when the user presses any key on the piano keyboard. Based on the note, first determines
        the corresponding fingering [1] and then updates keys on the flute to reflect this [2]

        Parameters
        ------------
          source        : MidiKeyboardState* : reference of the onscreen piano keyboard
          midiChannel   : int                : channel for external MIDI device connection
          midiNoteNumber: int                : note (pitch) being played on the keyboard
          velocity      : float              : how hard (volume) keyboard is being pressed
        ------------
        Returns         : void
    */
        short mask = 0b000000; // [1]
        switch(midiNoteNumber % 12)
        {
            case 0 : mask = 0b000001; break;
            case 1 : mask = 0b000000; break;
            case 2 : mask = 0b111111; break;
          //case 3 :
            case 4 : mask = 0b011111; break;
            case 5 : mask = 0b001111; break;
            case 6 : mask = 0b100111; break;
            case 7 : mask = 0b000111; break;
          //case 8 :
            case 9 : mask = 0b000011; break;
            case 10: mask = 0b001001; break;
            case 11: mask = 0b000001; break;
        }

        for (int i = 0; i < 6; i++) // [2]
        {
            if (mask & 0b1) keys[i].setToggleState(true, dontSendNotification);
            else keys[i].setToggleState(false, dontSendNotification);
            mask >>= 1;
        }
    }
    void handleNoteOff (MidiKeyboardState *source, int midiChannel, int midiNoteNumber, float velocity) override { }
    void mouseDown(const MouseEvent &event) override { //std::cout << event.x << ", " << event.y << std::endl;
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
