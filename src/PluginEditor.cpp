#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
BowedVstPluginAudioProcessorEditor::BowedVstPluginAudioProcessorEditor(BowedVstPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(200, 300);
}

BowedVstPluginAudioProcessorEditor::~BowedVstPluginAudioProcessorEditor()
{
}

//==============================================================================
void BowedVstPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(15.0f);

    g.drawFittedText("StringEnsemble", getLocalBounds(), juce::Justification::centredTop, 1);
}

void BowedVstPluginAudioProcessorEditor::resized()
{
}