#pragma once

#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

class HdnRingmodAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        private juce::Timer
{
public:
    explicit HdnRingmodAudioProcessorEditor(HdnRingmodAudioProcessor&);
    ~HdnRingmodAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    HdnRingmodAudioProcessor& processorRef;

    juce::Slider mixSlider, rateMultSlider, manualRateSlider, smoothingSlider, sensitivitySlider;
    juce::Label mixLabel, rateMultLabel, manualRateLabel, smoothingLabel, sensitivityLabel;

    juce::ComboBox modeBox, waveformBox;
    juce::Label modeLabel, waveformLabel;

    juce::Label pitchReadout;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment> mixAttach, rateMultAttach, manualRateAttach,
                                       smoothingAttach, sensitivityAttach;
    std::unique_ptr<ComboBoxAttachment> modeAttach, waveformAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HdnRingmodAudioProcessorEditor)
};
