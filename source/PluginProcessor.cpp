#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParameterIDs.h"

HdnRingmodAudioProcessor::HdnRingmodAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout HdnRingmodAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::mix, 1),
        "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::rateMultiplier, 1),
        "Rate Multiplier",
        juce::NormalisableRange<float>(0.1f, 8.0f, 0.01f, 0.5f),
        1.0f,
        juce::AudioParameterFloatAttributes().withLabel("x")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::manualRate, 1),
        "Manual Rate",
        juce::NormalisableRange<float>(20.0f, 5000.0f, 0.1f, 0.3f),
        440.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(ParameterIDs::mode, 1),
        "Mode",
        juce::StringArray{ "Pitch Track", "Manual" },
        0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::smoothing, 1),
        "Smoothing",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::sensitivity, 1),
        "Sensitivity",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(ParameterIDs::waveform, 1),
        "Waveform",
        juce::StringArray{ "Sine", "Triangle", "Square", "Saw" },
        0));

    return layout;
}

void HdnRingmodAudioProcessor::prepareToPlay(double, int)
{
}

void HdnRingmodAudioProcessor::releaseResources()
{
}

bool HdnRingmodAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void HdnRingmodAudioProcessor::processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&)
{
}

juce::AudioProcessorEditor* HdnRingmodAudioProcessor::createEditor()
{
    return new HdnRingmodAudioProcessorEditor(*this);
}

bool HdnRingmodAudioProcessor::hasEditor() const { return true; }

const juce::String HdnRingmodAudioProcessor::getName() const { return JucePlugin_Name; }

bool HdnRingmodAudioProcessor::acceptsMidi() const { return false; }
bool HdnRingmodAudioProcessor::producesMidi() const { return false; }
bool HdnRingmodAudioProcessor::isMidiEffect() const { return false; }
double HdnRingmodAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int HdnRingmodAudioProcessor::getNumPrograms() { return 1; }
int HdnRingmodAudioProcessor::getCurrentProgram() { return 0; }
void HdnRingmodAudioProcessor::setCurrentProgram(int) {}
const juce::String HdnRingmodAudioProcessor::getProgramName(int) { return {}; }
void HdnRingmodAudioProcessor::changeProgramName(int, const juce::String&) {}

void HdnRingmodAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xmlState = apvts.copyState().createXml())
        copyXmlToBinary(*xmlState, destData);
}

void HdnRingmodAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xmlState = getXmlFromBinary(data, sizeInBytes))
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HdnRingmodAudioProcessor();
}
