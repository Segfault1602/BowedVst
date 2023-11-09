#pragma once

#include <JuceHeader.h>

#include <array>

#include <string_ensemble.h>

class StringEnsembleEngine
{
  public:
    StringEnsembleEngine(juce::AudioProcessorValueTreeState& treeState);

    void Init(float samplerate);

    void Tick(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples);
    void HandleMidiMessage(const juce::MidiMessage& m);

    static void AddBowParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);

  private:
    sfdsp::StringEnsemble stringEnsemble_;

    struct StringData
    {
        sfdsp::OnePoleFilter pitchFilter;
        sfdsp::OnePoleFilter velocityFilter;
        sfdsp::OnePoleFilter forceFilter;
        uint8_t lastNote = 0.f;

        float frequency = 0.f;
        float pitchWheelValue = 0.f;
        float velocityValue = 0.f;
        float forceValue = 0.f;

    } stringData_[sfdsp::kStringCount];

    float maxVelocity_ = 0.f;
    float maxForce_ = 0.f;

    // JUCE params
    juce::AudioProcessorValueTreeState& treeState_;
    juce::AudioParameterFloat* bowParams_ = nullptr;
};