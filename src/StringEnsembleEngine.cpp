#include "StringEnsembleEngine.h"

constexpr float PITCH_BEND_RANGE = 48.0f;
constexpr float MAX_PITCH_BEND_VALUE = 16383.0f;
constexpr float CENTER_PITCH_BEND_VALUE = 8192.0f;

namespace ParamIDs
{
static juce::String paramVelocity{"velocity"};
static juce::String paramForce{"force"};
} // namespace ParamIDs

void StringEnsembleEngine::AddBowParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    auto velocity = std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::paramVelocity, 1), "Velocity", juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f);

    auto force = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(ParamIDs::paramForce, 1), "Force",
                                                             juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f);

    auto group =
        std::make_unique<juce::AudioProcessorParameterGroup>("bow", "Bow", "|", std::move(velocity), std::move(force));

    layout.add(std::move(group));
}

StringEnsembleEngine::StringEnsembleEngine(juce::AudioProcessorValueTreeState& treeState) : treeState_(treeState)
{
    bowParams_ = dynamic_cast<juce::AudioParameterFloat*>(treeState_.getParameter(ParamIDs::paramVelocity));
}

void StringEnsembleEngine::Init(float samplerate)
{
    stringEnsemble_.Init(samplerate);

    for (uint8_t i = 0; i < sfdsp::kStringCount; ++i)
    {
        stringData_[i].frequency = stringEnsemble_[i].GetFrequency();
        stringData_[i].pitchWheelValue = 0.f;
        stringData_[i].forceFilter.SetDecayFilter(-60, 10, samplerate);
        stringData_[i].velocityFilter.SetDecayFilter(-60, 10, samplerate);
        stringData_[i].pitchFilter.SetDecayFilter(-60, 100, samplerate);
    }
}

void StringEnsembleEngine::Tick(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    for (uint8_t i = 0; i < sfdsp::kStringCount; ++i)
    {
        auto& s = stringData_[i];
        // float pitchBend = s.pitchFilter.Tick(s.pitchWheelValue);
        float freq = sfdsp::MidiToFreq(static_cast<float>(s.lastNote) + s.pitchWheelValue);
        stringEnsemble_[i].SetFrequency(freq);

        stringEnsemble_[i].SetParameter(sfdsp::BowedString::ParamId::Velocity, s.velocityValue * maxVelocity_);

        stringEnsemble_[i].SetParameter(sfdsp::BowedString::ParamId::Force, s.forceValue);
    }
    stringEnsemble_.ProcessBlock(outputBuffer.getWritePointer(0) + startSample, static_cast<size_t>(numSamples));

    // Copy the output to the other channels
    for (auto i = outputBuffer.getNumChannels(); --i > 0;)
        outputBuffer.addFrom(i, startSample, outputBuffer, 0, startSample, numSamples);
}

void StringEnsembleEngine::HandleMidiMessage(const juce::MidiMessage& m)
{
    const uint8_t channel = static_cast<uint8_t>(m.getChannel());

    if (channel > 4)
    {
        DBG("Invalid midi channel: " << channel);
        return;
    }

    auto& stringData = stringData_[channel - 1];

    if (m.isNoteOn())
    {
        const float freq = sfdsp::MidiToFreq(static_cast<float>(m.getNoteNumber()));
        stringData.frequency = freq;
        stringData.pitchWheelValue = 0.f;
        stringData.forceValue = maxForce_;
        stringData.velocityValue = (static_cast<float>(m.getVelocity()) / 127.0f);
        stringData.lastNote = static_cast<uint8_t>(m.getNoteNumber());
    }
    else if (m.isNoteOff())
    {
        stringEnsemble_[channel - 1].SetParameter(sfdsp::BowedString::ParamId::Force, 0.0f);
        stringData.forceValue = 0.0f;
        stringData.velocityValue = 0.0f;
        stringData.pitchWheelValue = 0.f;
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff())
    {
        for (uint8_t i = 0; i < sfdsp::kStringCount; ++i)
        {
            stringData_[i].forceValue = 0.0f;
            stringData_[i].velocityValue = 0.0f;
        }
    }
    else if (m.isPitchWheel())
    {
        const int wheelPos = m.getPitchWheelValue();
        float normalizePitchWheelValue =
            (static_cast<float>(wheelPos) - CENTER_PITCH_BEND_VALUE) / MAX_PITCH_BEND_VALUE;
        normalizePitchWheelValue *= PITCH_BEND_RANGE;

        DBG("Ch: " << channel << "Value: " << wheelPos << ", Pitch Wheel: " << normalizePitchWheelValue);
        stringData.pitchWheelValue = normalizePitchWheelValue;
    }
    else if (m.isController())
    {
        uint8_t ccNum = static_cast<uint8_t>(m.getControllerNumber());
        float normalizedValue = static_cast<float>(m.getControllerValue()) / 127.0f;
        if (ccNum == 1)
        {
            maxVelocity_ = normalizedValue;
        }
        else if (ccNum == 2)
        {
            maxForce_ = normalizedValue;
        }
        else if (ccNum == 3)
        {
            stringEnsemble_.SetParameter(sfdsp::StringEnsemble::ParamId::BridgeTransmission, normalizedValue);
        }
        else if (ccNum == 7)
        {
            stringEnsemble_.SetParameter(sfdsp::StringEnsemble::ParamId::BridgeTransmissionFilterCutoff,
                                         normalizedValue);
        }
        else if (ccNum >= 12 && ccNum <= 15)
        {
            uint8_t stringNum = ccNum - 12;
            stringEnsemble_[stringNum].SetParameter(sfdsp::BowedString::ParamId::BridgeFilterCutoff, normalizedValue);
        }
        else if (ccNum >= 16 && ccNum <= 19)
        {
            uint8_t stringNum = ccNum - 16;
            stringEnsemble_[stringNum].SetParameter(sfdsp::BowedString::ParamId::NutGain, normalizedValue);
        }
        else if (ccNum >= 20 && ccNum <= 23)
        {
            uint8_t stringNum = ccNum - 20;
            stringEnsemble_[stringNum].SetParameter(sfdsp::BowedString::ParamId::FingerPressure, normalizedValue);
        }
        else if (ccNum >= 24 && ccNum <= 27)
        {
            uint8_t stringNum = ccNum - 24;
            stringEnsemble_[stringNum].SetParameter(sfdsp::BowedString::ParamId::TuningAdjustment, normalizedValue);
        }
        else
        {
            DBG("Ch: " << channel << ", CC: " << m.getControllerNumber() << ", Value: " << m.getControllerValue());
        }
    }
}