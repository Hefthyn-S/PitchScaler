/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PitchScalerAudioProcessor::PitchScalerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{

}

PitchScalerAudioProcessor::~PitchScalerAudioProcessor()
{
}

//==============================================================================
const juce::String PitchScalerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PitchScalerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PitchScalerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PitchScalerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PitchScalerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PitchScalerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PitchScalerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PitchScalerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PitchScalerAudioProcessor::getProgramName (int index)
{
    return {};
}

void PitchScalerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PitchScalerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    pitchShifter = std::make_unique<PitchShifter>(
        sampleRate, getTotalNumInputChannels());

}

void PitchScalerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PitchScalerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PitchScalerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.

    std::string suffix = apvts.getParameter("Formant Toggle")->getValue() ? " Formant" : " Shift";

    Settings settings {apvts.getRawParameterValue("Octave" + suffix)->load(),
    	apvts.getRawParameterValue("Semitone" + suffix)->load(),
    	apvts.getRawParameterValue("Cent" + suffix)->load(),
    	apvts.getParameter("Wet Amount")->getValue(),
        apvts.getParameter("Dry Amount")->getValue(),
		apvts.getRawParameterValue("Crispyness")->load(),
        false
     };

    std::vector<float*> channel_pointers;
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        channel_pointers.emplace_back(buffer.getWritePointer(channel));
    }

    if (auto s = spectrumAnalyzerComponent.lock()) {
        s->addBuffer(buffer, Input);
    }

    pitchShifter->processBlock(settings, buffer.getNumSamples(), channel_pointers);

    if (auto s = spectrumAnalyzerComponent.lock()) {
        s->addBuffer(buffer, Output);
    }
}

//==============================================================================
bool PitchScalerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PitchScalerAudioProcessor::createEditor()
{
    auto* editor = new PitchScalerAudioProcessorEditor (*this);
    spectrumAnalyzerComponent = editor->getSpectrumAnalyzerComponent();
    return editor;
}

//==============================================================================
void PitchScalerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void PitchScalerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        apvts.replaceState(tree);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout PitchScalerAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>("Dry Amount", "Dry Amount", juce::NormalisableRange<float>(0.f, 100.f, 1.f, 1.f), 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Wet Amount", "Wet Amount", juce::NormalisableRange<float>(0.f, 100.f, 1.f, 1.f), 100));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Octave Shift", "Octave Shift", juce::NormalisableRange<float>(-2.f, 2.f, 1.f, 1.f), 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Semitone Shift", "Semitone Shift", juce::NormalisableRange<float>(0.f, 12.f, 1.f, 1.f), 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Cent Shift", "Cent Shift", juce::NormalisableRange<float>(0.f, 100.f, 1.f, 1.f), 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Octave Formant", "Octave Formant", juce::NormalisableRange<float>(-2.f, 2.f, 1.f, 1.f), 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Semitone Formant", "Semitone Formant", juce::NormalisableRange<float>(0.f, 12.f, 1.f, 1.f), 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Cent Formant", "Cent Formant", juce::NormalisableRange<float>(0.f, 100.f, 1.f, 1.f), 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Crispyness", "Crispyness", juce::NormalisableRange<float>(0.f, 3.f, 1.f, 1.f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterBool>("Formant Toggle", "Formant Toggle", false));


    return layout;
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PitchScalerAudioProcessor();
}
