#include "PluginEditor.h"

WidenerEditor::WidenerEditor(WidenerProcessor& p)
    : juce::AudioProcessorEditor(&p), processorRef(p)
{
    setLookAndFeel(&lookAndFeel);

    titleLabel.setText("MONTAGEM WIDENER", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setFont(juce::Font(juce::FontOptions(26.0f, juce::Font::bold)));
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("STEREO SPACE  /  ONE-KNOB WIDTH", juce::dontSendNotification);
    subtitleLabel.setJustificationType(juce::Justification::centred);
    subtitleLabel.setColour(juce::Label::textColourId, WidenerLookAndFeel::textDim);
    subtitleLabel.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::plain)));
    addAndMakeVisible(subtitleLabel);

    amountSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    amountSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    amountSlider.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f,
                                      juce::MathConstants<float>::pi * 2.8f, true);
    addAndMakeVisible(amountSlider);

    amountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "amount", amountSlider);

    amountValueLabel.setJustificationType(juce::Justification::centred);
    amountValueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    amountValueLabel.setFont(juce::Font(juce::FontOptions(20.0f, juce::Font::bold)));
    addAndMakeVisible(amountValueLabel);

    footerLabel.setText("AMOUNT", juce::dontSendNotification);
    footerLabel.setJustificationType(juce::Justification::centred);
    footerLabel.setColour(juce::Label::textColourId, WidenerLookAndFeel::textDim);
    footerLabel.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    addAndMakeVisible(footerLabel);

    amountSlider.onValueChange = [this] { updateValueLabel(); repaint(); };
    updateValueLabel();

    brandLabel.setText("@montagem.widener", juce::dontSendNotification);
    brandLabel.setJustificationType(juce::Justification::centredRight);
    brandLabel.setColour(juce::Label::textColourId, WidenerLookAndFeel::textDim.withAlpha(0.5f));
    brandLabel.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::plain)));
    addAndMakeVisible(brandLabel);

    setResizable(false, false);
    setSize(480, 360);
}

WidenerEditor::~WidenerEditor()
{
    setLookAndFeel(nullptr);
}

void WidenerEditor::updateValueLabel()
{
    const int pct = (int)std::round(amountSlider.getValue() * 100.0);
    amountValueLabel.setText(juce::String(pct) + "%", juce::dontSendNotification);
}

void WidenerEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    juce::ColourGradient bgGradient(WidenerLookAndFeel::bg.brighter(0.03f), bounds.getCentre(),
                                     WidenerLookAndFeel::bg.darker(0.15f), bounds.getBottomLeft(), true);
    g.setGradientFill(bgGradient);
    g.fillAll();

    // Decoration is a pair of horizontal bars growing outward from the knob,
    // one per channel. Deliberately simpler than the earlier dot+arc design:
    // movement is purely horizontal, so there is no possible way for it to
    // grow into the title/subtitle text above, at any amount value -- no
    // curve math, no clamping, nothing that can look uneven at the extremes.
    const float amount = (float)amountSlider.getValue();
    const auto colour = WidenerLookAndFeel::red.interpolatedWith(WidenerLookAndFeel::green, amount);

    auto knobArea = amountSlider.getBounds().toFloat();
    const float centreY = knobArea.getCentreY();
    const float barThickness = 4.0f;
    const float barGap = 16.0f; // clearance from the knob's outer edge
    // Window is 480px wide, knob right edge sits at x=330 (centred, 180px knob).
    // Budget to the window edge: barGap(16) + barLength + dotRadius(5) +
    // half the L/R label width(14) must stay under 480-330=150, with margin.
    // 130 was too long (label clipped off the right edge at amount=1.0) --
    // measured, not assumed, after the first version shipped that bug.
    const float barLength = juce::jmap(amount, 0.0f, 1.0f, 10.0f, 100.0f);

    auto drawWingBar = [&](bool onRight)
    {
        const float innerX = onRight ? knobArea.getRight() + barGap : knobArea.getX() - barGap;
        const float outerX = onRight ? innerX + barLength : innerX - barLength;
        const float x = juce::jmin(innerX, outerX);
        const float w = std::abs(outerX - innerX);

        g.setColour(colour.withAlpha(0.85f));
        g.fillRoundedRectangle(x, centreY - barThickness * 0.5f, w, barThickness, barThickness * 0.5f);

        const float dotRadius = 5.0f;
        g.fillEllipse(outerX - dotRadius, centreY - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);

        g.setColour(juce::Colours::white.withAlpha(0.85f));
        g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
        g.drawText(onRight ? "R" : "L",
                    juce::Rectangle<float>(outerX - 14.0f, centreY + 12.0f, 28.0f, 16.0f),
                    juce::Justification::centred);
    };

    drawWingBar(false);
    drawWingBar(true);
}

void WidenerEditor::resized()
{
    auto area = getLocalBounds().reduced(16);

    titleLabel.setBounds(area.removeFromTop(36));
    subtitleLabel.setBounds(area.removeFromTop(20));

    area.removeFromTop(8);
    brandLabel.setBounds(area.removeFromBottom(14));
    footerLabel.setBounds(area.removeFromBottom(18));
    amountValueLabel.setBounds(area.removeFromBottom(28));

    const int knobSize = 180;
    juce::Rectangle<int> knobArea(0, 0, knobSize, knobSize);
    knobArea.setCentre(area.getCentre());
    amountSlider.setBounds(knobArea);
}
