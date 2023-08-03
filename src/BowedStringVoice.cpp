#include "BowedStringVoice.h"

#include <dsp_base.h>

constexpr float PITCH_BEND_RANGE = 2.0f;
constexpr float MAX_PITCH_BEND_VALUE = 16383.0f;
constexpr float CENTER_PITCH_BEND_VALUE = 8192.0f;

BowedStringVoice::BowedStringVoice()
{
}

bool BowedStringVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<BowedStringSound*>(sound) != nullptr;
}

void BowedStringVoice::setCurrentPlaybackSampleRate(double newRate)
{
    juce::SynthesiserVoice::setCurrentPlaybackSampleRate(newRate);

    // TODO: BowedString should probably support changing the sample rate during runtime.
    if (newRate > 0)
        bowedString_.Init(static_cast<float>(newRate));
}

void BowedStringVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* /* sound */,
                                 int currentPitchWheelPosition)
{
    float normalizePitchWheelValue =
        (static_cast<float>(currentPitchWheelPosition) - CENTER_PITCH_BEND_VALUE) / MAX_PITCH_BEND_VALUE;
    normalizePitchWheelValue *= PITCH_BEND_RANGE;

    float freq = dsp::MidiToFreq(static_cast<float>(midiNoteNumber) + normalizePitchWheelValue);

    bowedString_.SetFrequency(freq);
    bowedString_.SetLastMidiNote(static_cast<float>(midiNoteNumber));
    bowedString_.SetVelocity(0.f);
    bowTable_.SetForce(3.f);

    velocity = 0.03f + (0.2f * velocity);

    velocity_.reset(getSampleRate(), .1f);
    velocity_.setCurrentAndTargetValue(0.f);
    velocity_.setTargetValue(velocity);
    noteOn_ = true;
}

void BowedStringVoice::stopNote(float /* velocity */, bool /* allowTailOff */)
{
    noteOn_ = false;
}

void BowedStringVoice::pitchWheelMoved(int newPitchWheelValue)
{
    float normalizePitchWheelValue =
        (static_cast<float>(newPitchWheelValue) - CENTER_PITCH_BEND_VALUE) / MAX_PITCH_BEND_VALUE;
    normalizePitchWheelValue *= PITCH_BEND_RANGE;

    float freq = dsp::MidiToFreq(static_cast<float>(getCurrentlyPlayingNote()) + normalizePitchWheelValue);
    bowedString_.SetFrequency(freq);
}

void BowedStringVoice::controllerMoved(int /* controllerNumber */, int /* newControllerValue */)
{
}

void BowedStringVoice::aftertouchChanged(int newAftertouchValue)
{
    float normalizedAftertouchValue = static_cast<float>(newAftertouchValue) / 127.0f;
    normalizedAftertouchValue *= 5.f;

    bowTable_.SetForce(normalizedAftertouchValue);
}

void BowedStringVoice::channelPressureChanged(int newChannelPressureValue)
{
    float normalizedAftertouchValue = static_cast<float>(newChannelPressureValue) / 127.0f;
    normalizedAftertouchValue *= 4.f;

    bowTable_.SetForce(5.f - normalizedAftertouchValue);
}

void BowedStringVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    while (--numSamples >= 0)
    {
        bowedString_.SetVelocity(velocity_.getNextValue());
        dsp::ExcitationModel* excitation = (noteOn_) ? &bowTable_ : nullptr;
        float currentSample = bowedString_.Tick(excitation);

        for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
            outputBuffer.addSample(i, startSample, currentSample);

        ++startSample;
    }
}
