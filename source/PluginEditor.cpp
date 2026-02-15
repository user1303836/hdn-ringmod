#include "PluginEditor.h"
#include "ParameterIDs.h"
#include "NoteNames.h"

HdnRingmodAudioProcessorEditor::HdnRingmodAudioProcessorEditor(HdnRingmodAudioProcessor& p)
    : AudioProcessorEditor(p), processorRef(p)
{
    auto setupSlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& text)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        addAndMakeVisible(slider);

        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
    };

    setupSlider(mixSlider, mixLabel, "Mix");
    setupSlider(rateMultSlider, rateMultLabel, "Rate Mult");
    setupSlider(manualRateSlider, manualRateLabel, "Manual Rate");
    setupSlider(smoothingSlider, smoothingLabel, "Smoothing");
    setupSlider(sensitivitySlider, sensitivityLabel, "Sensitivity");

    mixAttach        = std::make_unique<SliderAttachment>(p.apvts, ParameterIDs::mix, mixSlider);
    rateMultAttach   = std::make_unique<SliderAttachment>(p.apvts, ParameterIDs::rateMultiplier, rateMultSlider);
    manualRateAttach = std::make_unique<SliderAttachment>(p.apvts, ParameterIDs::manualRate, manualRateSlider);
    smoothingAttach  = std::make_unique<SliderAttachment>(p.apvts, ParameterIDs::smoothing, smoothingSlider);
    sensitivityAttach = std::make_unique<SliderAttachment>(p.apvts, ParameterIDs::sensitivity, sensitivitySlider);

    auto setupCombo = [&](juce::ComboBox& box, juce::Label& label, const juce::String& text,
                           const juce::String& paramID)
    {
        if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(p.apvts.getParameter(paramID)))
            box.addItemList(param->choices, 1);
        addAndMakeVisible(box);

        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centredRight);
        addAndMakeVisible(label);
    };

    setupCombo(modeBox, modeLabel, "Mode", ParameterIDs::mode);
    setupCombo(waveformBox, waveformLabel, "Waveform", ParameterIDs::waveform);

    modeAttach     = std::make_unique<ComboBoxAttachment>(p.apvts, ParameterIDs::mode, modeBox);
    waveformAttach = std::make_unique<ComboBoxAttachment>(p.apvts, ParameterIDs::waveform, waveformBox);

    pitchReadout.setJustificationType(juce::Justification::centred);
    pitchReadout.setFont(juce::FontOptions(20.0f));
    pitchReadout.setText("--", juce::dontSendNotification);
    addAndMakeVisible(pitchReadout);

    setSize(600, 400);
    startTimerHz(30);
}

HdnRingmodAudioProcessorEditor::~HdnRingmodAudioProcessorEditor()
{
    stopTimer();
    mixAttach.reset();
    rateMultAttach.reset();
    manualRateAttach.reset();
    smoothingAttach.reset();
    sensitivityAttach.reset();
    modeAttach.reset();
    waveformAttach.reset();
}

void HdnRingmodAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(22.0f));
    g.drawText("HDN Ring Modulator", 0, 8, getWidth(), 30, juce::Justification::centred);
}

void HdnRingmodAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    area.removeFromTop(40);
    auto pitchArea = area.removeFromTop(30);
    pitchReadout.setBounds(pitchArea);

    area.removeFromTop(10);

    auto sliderArea = area.removeFromTop(200);
    int sliderWidth = sliderArea.getWidth() / 5;

    auto placeSlider = [&](juce::Slider& slider, juce::Label& label)
    {
        auto col = sliderArea.removeFromLeft(sliderWidth);
        label.setBounds(col.removeFromTop(20));
        slider.setBounds(col);
    };

    placeSlider(mixSlider, mixLabel);
    placeSlider(rateMultSlider, rateMultLabel);
    placeSlider(manualRateSlider, manualRateLabel);
    placeSlider(smoothingSlider, smoothingLabel);
    placeSlider(sensitivitySlider, sensitivityLabel);

    area.removeFromTop(10);

    auto comboArea = area.removeFromTop(30);
    int comboWidth = comboArea.getWidth() / 2;

    auto leftCombo = comboArea.removeFromLeft(comboWidth).reduced(10, 0);
    modeLabel.setBounds(leftCombo.removeFromLeft(70));
    modeBox.setBounds(leftCombo);

    auto rightCombo = comboArea.reduced(10, 0);
    waveformLabel.setBounds(rightCombo.removeFromLeft(80));
    waveformBox.setBounds(rightCombo);
}

void HdnRingmodAudioProcessorEditor::timerCallback()
{
    float hz = processorRef.currentPitchHz.load(std::memory_order_relaxed);
    float conf = processorRef.currentConfidence.load(std::memory_order_relaxed);

    if (hz > 0.0f && conf > 0.1f)
    {
        auto note = NoteNames::fromFrequency(hz);
        auto text = juce::String(note) + "  " + juce::String(hz, 1) + " Hz";
        pitchReadout.setText(text, juce::dontSendNotification);
    }
    else
    {
        pitchReadout.setText("--", juce::dontSendNotification);
    }
}
