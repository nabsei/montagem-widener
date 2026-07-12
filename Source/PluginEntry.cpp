#include "WidenerProcessor.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WidenerProcessor();
}
