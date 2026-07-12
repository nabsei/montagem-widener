#pragma once
#include "WidenerLookAndFeel.h"
#include "WidenerProcessor.h"

class WidenerEditor : public juce::AudioProcessorEditor
{
public:
    explicit WidenerEditor(WidenerProcessor&);
    ~WidenerEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    WidenerProcessor& processorRef;
    WidenerLookAndFeel lookAndFeel;

    juce::Slider amountSlider;
    juce::Label amountValueLabel;
    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::Label footerLabel;
    juce::Label brandLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> amountAttachment;

    void updateValueLabel();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WidenerEditor)
};
