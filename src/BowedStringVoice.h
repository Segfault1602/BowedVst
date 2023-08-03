#pragma once

#include <JuceHeader.h>

#include <bowed_string.h>

struct BowedStringSound : public juce::SynthesiserSound
{
    BowedStringSound()
    {
    }

    bool appliesToNote(int) override
    {
        return true;
    }
    bool appliesToChannel(int) override
    {
        return true;
    }
};

class BowedStringVoice : public juce::SynthesiserVoice
{
  public:
    BowedStringVoice();

    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void setCurrentPlaybackSampleRate(double newRate) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound,
                   int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void aftertouchChanged(int newAftertouchValue) override;
    void channelPressureChanged(int newChannelPressureValue) override;

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

  private:
    dsp::BowedString bowedString_;
    dsp::BowTable bowTable_;
    bool noteOn_ = false;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> velocity_;
};