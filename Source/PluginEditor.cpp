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

    brandLabel.setText("Bumpin Audio", juce::dontSendNotification);
    brandLabel.setJustificationType(juce::Justification::centredRight);
    brandLabel.setColour(juce::Label::textColourId, WidenerLookAndFeel::textDim.withAlpha(0.5f));
    brandLabel.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::plain)));
    addAndMakeVisible(brandLabel);

    setResizable(true, true);
    setResizeLimits(360, 280, 900, 700);
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
    const auto colour = WidenerLookAndFeel::cyan.interpolatedWith(WidenerLookAndFeel::magenta, amount);

    auto knobArea = amountSlider.getBounds().toFloat();
    const float centreY = knobArea.getCentreY();
    const float barThickness = 4.0f;
    const float barGap = 16.0f; // clearance from the knob's outer edge
    // Computed from the actual window/knob size rather than a hardcoded
    // budget for one fixed window size -- the window is resizable and the
    // knob scales with it (see resized()), so a fixed max length here would
    // reintroduce the exact "label clipped off the edge" bug this decoration
    // was rewritten to avoid once already.
    const float budgetDotRadius = 5.0f; // matches drawWingBar's own dotRadius below
    const float halfLabelWidth = 14.0f;
    const float decorBudget = barGap + budgetDotRadius + halfLabelWidth + 6.0f; // + small margin
    const float availableToEdge = bounds.getWidth() * 0.5f - knobArea.getWidth() * 0.5f;
    const float maxBarLength = juce::jmax(10.0f, availableToEdge - decorBudget);
    const float barLength = juce::jmap(amount, 0.0f, 1.0f, 10.0f, maxBarLength);

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

    const int knobSize = juce::jlimit(120, 260, juce::jmin(area.getWidth(), area.getHeight()) - 40);
    juce::Rectangle<int> knobArea(0, 0, knobSize, knobSize);
    knobArea.setCentre(area.getCentre());
    amountSlider.setBounds(knobArea);
}
