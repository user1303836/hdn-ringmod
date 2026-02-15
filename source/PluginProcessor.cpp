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

void HdnRingmodAudioProcessor::prepareToPlay(double sampleRate, int)
{
    pitchDetector.prepare(sampleRate);
    oscillator.prepare(sampleRate);
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

void HdnRingmodAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    if (numChannels == 0 || numSamples == 0)
        return;

    float mix = apvts.getRawParameterValue(ParameterIDs::mix)->load() / 100.0f;
    float rateMultiplier = apvts.getRawParameterValue(ParameterIDs::rateMultiplier)->load();
    float manualRate = apvts.getRawParameterValue(ParameterIDs::manualRate)->load();
    int mode = static_cast<int>(apvts.getRawParameterValue(ParameterIDs::mode)->load());
    float smoothing = apvts.getRawParameterValue(ParameterIDs::smoothing)->load() / 100.0f;
    float sensitivity = apvts.getRawParameterValue(ParameterIDs::sensitivity)->load() / 100.0f;
    int waveformIdx = static_cast<int>(apvts.getRawParameterValue(ParameterIDs::waveform)->load());

    pitchSmoother.setSmoothingAmount(smoothing);
    pitchSmoother.setSensitivity(sensitivity);
    oscillator.setWaveform(static_cast<Oscillator::Waveform>(waveformIdx));

    auto* inputData = buffer.getReadPointer(0);

    for (int i = 0; i < numSamples; ++i)
    {
        pitchDetector.feedSample(inputData[i]);

        float oscFreq;

        if (mode == 0)
        {
            auto result = pitchDetector.getResult();
            float smoothedFreq = pitchSmoother.process(result.frequency, result.confidence);
            oscFreq = smoothedFreq * rateMultiplier;

            currentPitchHz.store(result.frequency, std::memory_order_relaxed);
            currentConfidence.store(result.confidence, std::memory_order_relaxed);
        }
        else
        {
            oscFreq = manualRate;
            currentPitchHz.store(0.0f, std::memory_order_relaxed);
            currentConfidence.store(0.0f, std::memory_order_relaxed);
        }

        if (oscFreq > 0.0f)
            oscillator.setFrequency(oscFreq);

        float oscSample = oscillator.nextSample();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* channelData = buffer.getWritePointer(ch);
            float dry = channelData[i];
            float wet = dry * oscSample;
            channelData[i] = dry * (1.0f - mix) + wet * mix;
        }
    }
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
