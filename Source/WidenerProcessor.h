#pragma once
#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

// One-button stereo widener, companion to Phonk Finisher: a single "Amount"
// macro drives mid-side widening (bass-protected) plus a short Haas delay
// for perceived space. Designed to sit after a finisher/saturator in the
// chain without smearing the mono low end (808/sub).
//
// NOTE: this is the public / portfolio version. The tuned constants (width
// crossover frequency, gain curve, delay time range, mix levels) used in
// the shipped build are simplified/omitted here -- this file demonstrates
// the JUCE architecture, not the production calibration.
class WidenerProcessor : public juce::AudioProcessor
{
public:
    WidenerProcessor();
    ~WidenerProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Montagem Widener"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.05; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Mid-side width, protected below ~150Hz: the side channel's low content
    // passes through unboosted (mono bass stays exactly as recorded/panned),
    // only content above the crossover gets widened. Keeps an 808/sub solid
    // in mono while still opening up the top end.
    float sideLowState = 0.0f;
    float sideLowCoef = 0.0f;

    // Short Haas-effect delay, fed from the isolated high side signal only
    // (never the full mid/side mix) so it can't reintroduce L/R difference
    // into the bass. Purely a perceptual space cue -- not a full reverb --
    // so it stays predictable and doesn't wash out a busy mix.
    std::vector<float> delayBuffer;
    int delayWritePos = 0;
    int delayBufferSize = 0;

    double lastSampleRate = 44100.0;

    // Smooths "amount" itself so width/delay coefficients ramp over ~30ms
    // instead of jumping block-to-block (see FinisherProcessor for why).
    juce::SmoothedValue<float> amountSmoothed;
};
