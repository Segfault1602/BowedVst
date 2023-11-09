#pragma once

#include <JuceHeader.h>

#include "StringEnsembleEngine.h"

//==============================================================================
class BowedVstPluginAudioProcessor : public foleys::MagicProcessor
{
  public:
    //==============================================================================
    BowedVstPluginAudioProcessor();
    ~BowedVstPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

  private:
    juce::AudioProcessorValueTreeState treeState_;

    StringEnsembleEngine stringEnsembleEngine_;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BowedVstPluginAudioProcessor)
};