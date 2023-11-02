#pragma once

#include <JuceHeader.h>

#include <array>

#include <string_ensemble.h>

class StringEnsembleEngine
{
  public:
    StringEnsembleEngine();

    void Init(float samplerate);

    void Tick(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples);
    void HandleMidiMessage(const juce::MidiMessage& m);

  private:
    dsp::StringEnsemble stringEnsemble_;

    std::array<uint8_t, dsp::kStringCount> lastNote_{};
};