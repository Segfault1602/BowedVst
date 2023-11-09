#include "PluginProcessor.h"

#include "BowedStringVoice.h"

static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    StringEnsembleEngine::AddBowParameters(layout);
    return layout;
}

//==============================================================================
BowedVstPluginAudioProcessor::BowedVstPluginAudioProcessor()
    : foleys::MagicProcessor(
          juce::AudioProcessor::BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      treeState_(*this, nullptr, ProjectInfo::projectName, createParameterLayout()), stringEnsembleEngine_(treeState_)
{
    FOLEYS_SET_SOURCE_PATH(__FILE__);
}

BowedVstPluginAudioProcessor::~BowedVstPluginAudioProcessor()
{
}

//==============================================================================
const juce::String BowedVstPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BowedVstPluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool BowedVstPluginAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool BowedVstPluginAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double BowedVstPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BowedVstPluginAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
              // so this should be at least 1, even if you're not really implementing programs.
}

int BowedVstPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BowedVstPluginAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String BowedVstPluginAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void BowedVstPluginAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void BowedVstPluginAudioProcessor::prepareToPlay(double sampleRate, int /* samplesPerBlock */)
{
    stringEnsembleEngine_.Init(static_cast<float>(sampleRate));
}

void BowedVstPluginAudioProcessor::releaseResources()
{
}

bool BowedVstPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void BowedVstPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    int currentSampleIdx = 0;
    int numSamples = buffer.getNumSamples();

    auto midiIterator = midiMessages.findNextSamplePosition(currentSampleIdx);

    for (; numSamples > 0; ++midiIterator)
    {
        if (midiIterator == midiMessages.cend())
        {
            // no more midi events
            stringEnsembleEngine_.Tick(buffer, currentSampleIdx, numSamples);
            break;
        }

        const auto metadata = *midiIterator;
        const int samplesToNextMidiMessage = metadata.samplePosition - currentSampleIdx;

        if (samplesToNextMidiMessage >= numSamples)
        {
            // no more midi events in this block
            stringEnsembleEngine_.Tick(buffer, currentSampleIdx, samplesToNextMidiMessage);
            stringEnsembleEngine_.HandleMidiMessage(metadata.getMessage());
            break;
        }

        if (samplesToNextMidiMessage < 1)
        {
            stringEnsembleEngine_.HandleMidiMessage(metadata.getMessage());
            continue;
        }

        stringEnsembleEngine_.Tick(buffer, currentSampleIdx, samplesToNextMidiMessage);

        stringEnsembleEngine_.HandleMidiMessage(metadata.getMessage());
        currentSampleIdx += samplesToNextMidiMessage;
        numSamples -= samplesToNextMidiMessage;
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BowedVstPluginAudioProcessor();
}
