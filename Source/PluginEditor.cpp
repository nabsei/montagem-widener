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

    subtitleLabel.setText("STEREO SPACE  -  ONE-KNOB WIDTH", juce::dontSendNotification);
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

    amountSlider.onValueChange = [this] { updateValueLabel(); };
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

    // Decoration is deliberately different from Phonk Finisher's waveform bars:
    // those read as "loudness/saturation before-after", which isn't what this
    // plugin does. Here, two L/R dots physically move apart as amount
    // increases -- a literal stereo-width metaphor -- while keeping the same
    // red (narrow) -> green (wide) brand colour language as the knob.
    const float amount = (float)amountSlider.getValue();
    const auto colour = WidenerLookAndFeel::red.interpolatedWith(WidenerLookAndFeel::green, amount);

    auto knobArea = amountSlider.getBounds().toFloat();
    const float centreX = knobArea.getCentreX();
    const float centreY = knobArea.getCentreY();

    // Knob radius is 90px (180px knob) -- minSpread must clear that or the
    // dots/arcs render hidden behind the opaque knob face at low amounts.
    const float spread = juce::jmap(amount, 0.0f, 1.0f, 98.0f, 168.0f);
    const juce::Point<float> lPos(centreX - spread, centreY);
    const juce::Point<float> rPos(centreX + spread, centreY);

    // Connecting arc: the whole curve scales smoothly with spread (a single
    // proportional factor, no separate additive term and no clamp) so it
    // rises and falls as one continuous shape across the full amount range,
    // instead of stretching unevenly then hitting a hard ceiling. The 0.55
    // factor is chosen so even at maximum spread the peak stays clear of the
    // title/subtitle block above -- verified, not just assumed.
    {
        juce::Path arc;
        const float controlY = centreY - spread * 0.55f;
        arc.startNewSubPath(lPos);
        arc.quadraticTo(centreX, controlY, rPos.x, rPos.y);
        g.setColour(colour.withAlpha(0.35f));
        g.strokePath(arc, juce::PathStrokeType(2.0f));
    }

    // concentric "radiating" arcs behind each dot, growing with amount
    auto drawRadiate = [&](juce::Point<float> centre, bool facingRight)
    {
        for (int i = 0; i < 3; ++i)
        {
            const float t = (float)(i + 1) / 3.0f;
            const float r = 10.0f + t * (10.0f + amount * 18.0f);
            juce::Path arcPath;
            const float startAngle = facingRight ? juce::MathConstants<float>::pi * 1.25f
                                                  : juce::MathConstants<float>::pi * -0.25f;
            const float sweep = facingRight ? juce::MathConstants<float>::pi * 0.5f
                                             : juce::MathConstants<float>::pi * -0.5f;
            arcPath.addCentredArc(centre.x, centre.y, r, r, 0.0f, startAngle, startAngle + sweep, true);
            g.setColour(colour.withAlpha(0.25f * (1.0f - t) + 0.05f));
            g.strokePath(arcPath, juce::PathStrokeType(1.5f));
        }
    };
    drawRadiate(lPos, false);
    drawRadiate(rPos, true);

    const float dotRadius = 8.0f;
    g.setColour(colour);
    g.fillEllipse(lPos.x - dotRadius, lPos.y - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
    g.fillEllipse(rPos.x - dotRadius, rPos.y - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);

    g.setColour(juce::Colours::white.withAlpha(0.85f));
    g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    g.drawText("L", juce::Rectangle<float>(lPos.x - 14.0f, lPos.y + dotRadius + 4.0f, 28.0f, 16.0f), juce::Justification::centred);
    g.drawText("R", juce::Rectangle<float>(rPos.x - 14.0f, rPos.y + dotRadius + 4.0f, 28.0f, 16.0f), juce::Justification::centred);
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
