#include "WidenerProcessor.h"
#include "PluginEditor.h"

WidenerProcessor::WidenerProcessor()
    : AudioProcessor(BusesProperties()
                          .withInput("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "STATE", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout WidenerProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "amount", "Amount", juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    return { params.begin(), params.end() };
}

void WidenerProcessor::prepareToPlay(double sampleRate, int)
{
    lastSampleRate = sampleRate;

    sideLowCoef = 1.0f - std::exp(-2.0f * juce::MathConstants<float>::pi * 150.0f / (float)sampleRate);
    sideLowState = 0.0f;

    delayBufferSize = (int)(0.05 * sampleRate) + 4;
    delayBuffer.assign((size_t)delayBufferSize, 0.0f);
    delayWritePos = 0;

    amountSmoothed.reset(sampleRate, 0.03);
    amountSmoothed.setCurrentAndTargetValue(*apvts.getRawParameterValue("amount"));
}

void WidenerProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    if (buffer.getNumChannels() < 2)
        return;

    amountSmoothed.setTargetValue(*apvts.getRawParameterValue("amount"));
    const float amount01 = amountSmoothed.skip(buffer.getNumSamples());

    const float widthGain = juce::jmap(amount01, 0.0f, 1.0f, 1.0f, 2.0f);
    const float maxDelayMs = 10.0f;
    const float delaySamples = juce::jmap(amount01, 0.0f, 1.0f, 0.0f, maxDelayMs) * 0.001f * (float)lastSampleRate;
    const float haasMix = amount01 * 0.3f;

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const float mid = (left[i] + right[i]) * 0.5f;
        const float side = (left[i] - right[i]) * 0.5f;

        sideLowState += sideLowCoef * (side - sideLowState);
        const float sideLow = sideLowState;
        const float sideHigh = side - sideLow;

        delayBuffer[(size_t)delayWritePos] = sideHigh;

        float readPos = (float)delayWritePos - delaySamples;
        while (readPos < 0.0f)
            readPos += (float)delayBufferSize;
        const int readIdx0 = (int)readPos % delayBufferSize;
        const int readIdx1 = (readIdx0 + 1) % delayBufferSize;
        const float frac = readPos - std::floor(readPos);
        const float delayedSideHigh = delayBuffer[(size_t)readIdx0] * (1.0f - frac) + delayBuffer[(size_t)readIdx1] * frac;

        delayWritePos = (delayWritePos + 1) % delayBufferSize;

        const float sideOut = sideLow + sideHigh * widthGain + delayedSideHigh * haasMix;

        left[i] = mid + sideOut;
        right[i] = mid - sideOut;
    }
}

juce::AudioProcessorEditor* WidenerProcessor::createEditor()
{
    return new WidenerEditor(*this);
}

void WidenerProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
        if (auto xml = state.createXml())
            copyXmlToBinary(*xml, destData);
}

void WidenerProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}
