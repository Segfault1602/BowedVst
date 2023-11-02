#include "StringEnsembleEngine.h"

StringEnsembleEngine::StringEnsembleEngine()
{
}

void StringEnsembleEngine::Init(float samplerate)
{
    stringEnsemble_.Init(samplerate);
}

void StringEnsembleEngine::Tick(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    while (--numSamples >= 0)
    {
        float currentSample = stringEnsemble_.Tick();

        for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
            outputBuffer.addSample(i, startSample, currentSample);

        ++startSample;
    }
}

void StringEnsembleEngine::HandleMidiMessage(const juce::MidiMessage& m)
{
    const uint8_t channel = static_cast<uint8_t>(m.getChannel());

    if (channel > 4)
    {
        DBG("Invalid midi channel: " << channel);
        return;
    }

    if (m.isNoteOn())
    {
        // stringEnsemble_.SetForce(channel - 1, 0.5f);
        // stringEnsemble_.SetVelocity(channel - 1, 0.5f);
        const float freq = dsp::MidiToFreq(static_cast<float>(m.getNoteNumber()));
        stringEnsemble_.SetFrequency(channel - 1, freq);
    }
    else if (m.isNoteOff())
    {
        stringEnsemble_.FingerOff(channel - 1);
        // stringEnsemble_.SetForce(channel - 1, 0.0f);
        // stringEnsemble_.SetVelocity(channel - 1, 0.0f);
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff())
    {
        for (uint8_t i = 0; i < dsp::kStringCount; ++i)
        {
            stringEnsemble_.SetForce(i - 1, 0.0f);
            stringEnsemble_.SetVelocity(i - 1, 0.0f);
        }
    }
    else if (m.isController())
    {
        uint8_t ccNum = static_cast<uint8_t>(m.getControllerNumber());
        float normalizedValue = static_cast<float>(m.getControllerValue()) / 127.0f;
        if (ccNum >= 1 && ccNum <= 4)
        {
            stringEnsemble_.SetForce(ccNum - 1, normalizedValue);
            stringEnsemble_.SetVelocity(ccNum - 1, normalizedValue);
        }
        else
        {
            DBG("Ch: " << channel << ", CC: " << m.getControllerNumber() << ", Value: " << m.getControllerValue());
        }
    }
}