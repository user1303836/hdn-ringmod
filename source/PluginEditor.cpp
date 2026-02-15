#include "PluginEditor.h"

HdnRingmodAudioProcessorEditor::HdnRingmodAudioProcessorEditor(HdnRingmodAudioProcessor& p)
    : AudioProcessorEditor(p), processorRef(p)
{
    setSize(600, 400);
}

void HdnRingmodAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawText("HDN Ring Modulator", getLocalBounds(), juce::Justification::centred);
}

void HdnRingmodAudioProcessorEditor::resized()
{
}
