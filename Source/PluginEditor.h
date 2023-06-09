/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::Rotary,
        juce::Slider::TextEntryBoxPosition::NoTextBox)
    {

    }
};

//==============================================================================
/**
*/

class PitchScalerAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    PitchScalerAudioProcessorEditor (PitchScalerAudioProcessor&);
    ~PitchScalerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    
    

private:
    void siderEditor();
    void sliderValueManipulator();
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PitchScalerAudioProcessor& audioProcessor;


    CustomRotarySlider octaveShiftSlider,
        semiTomeShiftSlider,
        centShiftSlider,
        formantOctaveSlider,
        formantSemitoneSlider,
        formantCentSlider,
        drySlider,
        wetSlider;
    juce::Slider crispynessSlider;

    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;

    Attachment octaveShiftSliderAttachment,
        semiTomeShiftSliderAttachment,
        centShiftSliderAttachment,
        formantOctaveSliderAttachment,
        formantSemitoneSliderAttachment,
        formantCentSliderAttachment,
        drySliderAttachment,
        wetSliderAttachment,
        crispynessSliderAttachment;


    std::vector<juce::Component*> getComps();

    bool m_AtLimit{ false };
    bool m_AtFormantLimit{ false };
    bool m_AtCentLimit{ false };
    bool m_AtFormantCentLimit{ false };
    int m_SemiToneValue{};
    int m_CentValue{};
    int m_SemiFormantValue{};
    int m_CentFormantValue{};
    const int m_SemitoneMax{12};
    const int m_CentMax{100};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PitchScalerAudioProcessorEditor)
};