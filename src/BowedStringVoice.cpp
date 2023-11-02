#include "BowedStringVoice.h"

#include <dsp_base.h>

constexpr float PITCH_BEND_RANGE = 48.0f;
constexpr float MAX_PITCH_BEND_VALUE = 16383.0f;
constexpr float CENTER_PITCH_BEND_VALUE = 8192.0f;

BowedStringVoice::BowedStringVoice(uint8_t midi_channel) : midi_channel_(midi_channel)
{
    // One midi channel per string. Channel 1 to 4 correspond to the bottom 4 rows on the Linnstrument.
    switch (midi_channel)
    {
    case 1: // G
        break;
    case 2: // D
        break;
    case 3: // A
        break;
    case 4: // E
        break;
    default: // TODO: handle arbitrary amount of strings tuned in fifth?
        break;
    }
}

bool BowedStringVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    if (dynamic_cast<BowedStringSound*>(sound) == nullptr)
        return false;

    BowedStringSound* stringSound = dynamic_cast<BowedStringSound*>(sound);
    return stringSound->getMidiChannel() == midi_channel_;
}

void BowedStringVoice::setCurrentPlaybackSampleRate(double newRate)
{
    juce::SynthesiserVoice::setCurrentPlaybackSampleRate(newRate);

    // TODO: BowedString should probably support changing the sample rate during runtime.
    if (newRate > 0)
    {
        bowedString_.Init(static_cast<float>(newRate));
        next_freq_ = bowedString_.GetFrequency();
    }
}

void BowedStringVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* /* sound */,
                                 int currentPitchWheelPosition)
{
    float normalizePitchWheelValue =
        (static_cast<float>(currentPitchWheelPosition) - CENTER_PITCH_BEND_VALUE) / MAX_PITCH_BEND_VALUE;
    normalizePitchWheelValue *= PITCH_BEND_RANGE;

    float freq = dsp::MidiToFreq(static_cast<float>(midiNoteNumber) + normalizePitchWheelValue);
    next_freq_ = freq;
    bowedString_.SetFrequency(freq);
    bowedString_.SetVelocity(0.f);
    bowedString_.SetForce(0.f);

    force_.reset(getSampleRate(), 0.2f);
    force_.setCurrentAndTargetValue(0.f);
    force_.setTargetValue(velocity);
    noteOn_ = true;
}

void BowedStringVoice::stopNote(float /* velocity */, bool /* allowTailOff */)
{
    noteOn_ = false;
    bowedString_.SetVelocity(0.f);
}

void BowedStringVoice::pitchWheelMoved(int newPitchWheelValue)
{
    float normalizePitchWheelValue =
        (static_cast<float>(newPitchWheelValue) - CENTER_PITCH_BEND_VALUE) / MAX_PITCH_BEND_VALUE;
    normalizePitchWheelValue *= PITCH_BEND_RANGE;

    float freq = dsp::MidiToFreq(static_cast<float>(getCurrentlyPlayingNote()) + normalizePitchWheelValue);
    next_freq_ = freq;
    // bowedString_.SetFrequency(freq);
}

void BowedStringVoice::controllerMoved(int /* controllerNumber */, int /* newControllerValue */)
{
}

void BowedStringVoice::aftertouchChanged(int newAftertouchValue)
{
    float normalizedAftertouchValue = static_cast<float>(newAftertouchValue) / 127.0f;
    normalizedAftertouchValue *= 5.f;

    bowedString_.SetForce(normalizedAftertouchValue);
}

void BowedStringVoice::channelPressureChanged(int newChannelPressureValue)
{
    float normalizedAftertouchValue = static_cast<float>(newChannelPressureValue) / 127.0f;

    if (noteOn_)
        bowedString_.SetVelocity(normalizedAftertouchValue);
}

void BowedStringVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    float current_freq = bowedString_.GetFrequency();
    float freq_diff = next_freq_ - current_freq;
    float freq_dt = freq_diff / static_cast<float>(numSamples);
    while (--numSamples >= 0)
    {
        bowedString_.SetFrequency(current_freq += freq_dt);
        bowedString_.SetForce(force_.getNextValue());
        float currentSample = bowedString_.Tick(true);

        for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
            outputBuffer.addSample(i, startSample, currentSample);

        ++startSample;
    }
}
