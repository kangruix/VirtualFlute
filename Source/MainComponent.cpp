/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() :
    keyboardComponent (keyboardState, MidiKeyboardComponent::horizontalKeyboard),
    scopeComponent (audioBufferQueue)
{   addMouseListener(&mouse, true);
    setSize (600, 275);

    // Load flute image and initialize buttons corresponding to keys on the flute
    fluteImg = ImageFileFormat::loadFrom(BinaryData::flute_png, BinaryData::flute_pngSize);
    Image keyNormalImg(Image::RGB, 16, 16, true);
    Graphics(keyNormalImg).drawEllipse(0, 0, 15, 15, 2);
    Image keyDownImg(Image::RGB, 16, 16, true);
    Graphics(keyDownImg).fillEllipse(0, 0, 16, 16);
    for (int i = 0; i < 6; i++)
    {
        addAndMakeVisible(&keys[i]);
        keys[i].setClickingTogglesState(true);
        keys[i].setImages(true, false, false, keyNormalImg, 1.0, Colour(), keyNormalImg, 1.0, Colour(), keyDownImg, 0.5, Colour());
        keys[i].onClick = [this] { handleKeyClicked(); };
    }
    
    // Add piano keyboard to screen and allow MIDI message control
    addAndMakeVisible (keyboardComponent);
    keyboardState.addListener(this);
    
    // Add waveform display to screen
    addAndMakeVisible (scopeComponent);

    // Starter code to request for permission to open input/output channels
    if (RuntimePermissions::isRequired (RuntimePermissions::recordAudio) && ! RuntimePermissions::isGranted (RuntimePermissions::recordAudio)) {
        RuntimePermissions::request (RuntimePermissions::recordAudio, [&] (bool granted) {
            if (granted)  setAudioChannels (0, 2); }); }
    else { setAudioChannels (0, 2); }
}

MainComponent::~MainComponent() { shutdownAudio();}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    synth.addVoice (new FluteVoice({ sampleRate, (uint32) samplesPerBlockExpected, 2 }));
    synth.addSound (new FluteSound());
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
}
void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    MidiBuffer incomingMidi;
    keyboardState.processNextMidiBuffer (incomingMidi, bufferToFill.startSample, bufferToFill.numSamples, true);
    synth.renderNextBlock (*bufferToFill.buffer, incomingMidi, bufferToFill.startSample, bufferToFill.numSamples);
    scopeDataCollector.process(bufferToFill.buffer->getReadPointer(0), (size_t) bufferToFill.numSamples);
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()
}
void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}
//==============================================================================
void MainComponent::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    g.drawImageAt(fluteImg, 20, 20);
    // You can add your drawing code here!
}
void MainComponent::resized()
{
    keys[0].setBounds(288, 50, 16, 16);
    keys[1].setBounds(320, 50, 16, 16);
    keys[2].setBounds(340, 50, 16, 16);
    keys[3].setBounds(397, 52, 16, 16);
    keys[4].setBounds(418, 52, 16, 16);
    keys[5].setBounds(443, 52, 16, 16);
    scopeComponent.setBounds(0, getHeight()-160, getWidth(), 80);
    keyboardComponent.setBounds(0, getHeight()-69, getWidth(), 69);
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
