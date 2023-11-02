#pragma once

#include <JuceHeader.h>

#include <bowed_string.h>

struct BowedStringSound : public juce::SynthesiserSound
{
    BowedStringSound(uint8_t midi_channel) : midi_channel_(midi_channel)
    {
    }

    bool appliesToNote(int) override
    {
        return true;
    }
    bool appliesToChannel(int channel) override
    {
        return channel == midi_channel_;
    }

    uint8_t getMidiChannel() const
    {
        return midi_channel_;
    }

  private:
    const uint8_t midi_channel_ = 0;
};

class BowedStringVoice : public juce::SynthesiserVoice
{
  public:
    BowedStringVoice(uint8_t midi_channel);

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
    uint8_t midi_channel_ = 0;
    dsp::BowedString bowedString_;
    bool noteOn_ = false;

    float next_freq_ = 0.f;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> force_;
};