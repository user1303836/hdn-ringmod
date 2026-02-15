#include "PluginProcessor.h"
#include "PluginEditor.h"

HdnRingmodAudioProcessor::HdnRingmodAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
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

void HdnRingmodAudioProcessor::getStateInformation(juce::MemoryBlock&)
{
}

void HdnRingmodAudioProcessor::setStateInformation(const void*, int)
{
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HdnRingmodAudioProcessor();
}
