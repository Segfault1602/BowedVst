#pragma once

#include "PluginProcessor.h"

//==============================================================================
class BowedVstPluginAudioProcessorEditor : public juce::AudioProcessorEditor
{
  public:
    explicit BowedVstPluginAudioProcessorEditor(BowedVstPluginAudioProcessor&);
    ~BowedVstPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

  private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.

    BowedVstPluginAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BowedVstPluginAudioProcessorEditor)
};