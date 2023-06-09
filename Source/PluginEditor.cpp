/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
PitchScalerAudioProcessorEditor::PitchScalerAudioProcessorEditor (PitchScalerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    octaveShiftSliderAttachment(audioProcessor.apvts, "Octave Shift", octaveShiftSlider),
    semiTomeShiftSliderAttachment(audioProcessor.apvts, "Semitone Shift", semiTomeShiftSlider),
    centShiftSliderAttachment(audioProcessor.apvts, "Cent Shift", centShiftSlider),
    formantOctaveSliderAttachment(audioProcessor.apvts, "Octave Formant", formantOctaveSlider),
    formantSemitoneSliderAttachment(audioProcessor.apvts, "Semitone Formant", formantSemitoneSlider),
    formantCentSliderAttachment(audioProcessor.apvts, "Cent Formant", formantCentSlider),
    drySliderAttachment(audioProcessor.apvts, "Dry Amount", drySlider),
    wetSliderAttachment(audioProcessor.apvts, "Wet Amount", wetSlider),
    crispynessSliderAttachment(audioProcessor.apvts, "Crispyness", crispynessSlider)
{
    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }
    setSize (600, 400);

    siderEditor();
    
}

PitchScalerAudioProcessorEditor::~PitchScalerAudioProcessorEditor()
{
}


//==============================================================================
void PitchScalerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    
     
    sliderValueManipulator();
}



void PitchScalerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // set bounds for response area
    auto responseArea = bounds.removeFromTop( bounds.getHeight() * 0.33);




    // set bounds for dry/wet sliders
    auto dryWetArea = bounds.removeFromLeft(bounds.getWidth() * 0.333);
    auto dryArea = dryWetArea.removeFromBottom(dryWetArea.getHeight() * 0.5);
    auto wetArea = dryWetArea;
    drySlider.setBounds(dryArea);
    wetSlider.setBounds(wetArea);

    //set bounds for crispiness slider
    auto crispynessArea = bounds.removeFromRight(bounds.getWidth() * 0.5);
    crispynessSlider.setBounds(crispynessArea);
    

    //defining areas for the shiftsliders
    auto shiftArea = bounds.removeFromTop(bounds.getHeight() * 0.5);
    shiftArea.setLeft(shiftArea.getX() + shiftArea.getWidth() * 0.5f - shiftArea.getHeight() * 0.5f);
    shiftArea.setWidth(std::min(shiftArea.getWidth(), shiftArea.getHeight()));
    octaveShiftSlider.setBounds(shiftArea);
    auto radius = std::min(shiftArea.getWidth(), shiftArea.getHeight()) * 0.5f;
    auto byReducer = radius - 0.75f * radius;

    auto semitoneShiftArea = shiftArea.reduced(byReducer);
    semiTomeShiftSlider.setBounds(semitoneShiftArea);
    radius = std::min(semitoneShiftArea.getWidth(), semitoneShiftArea.getHeight()) * 0.5f;
    byReducer = radius - 0.66f * radius;

    auto centShiftArea = semitoneShiftArea.reduced(byReducer);
    centShiftSlider.setBounds(centShiftArea);


    //defining areas for the formantsliders
    auto formantArea = bounds;

    formantArea.setLeft(formantArea.getX() + formantArea.getWidth() * 0.5f - formantArea.getHeight() * 0.5f);
    formantArea.setWidth(std::min(formantArea.getWidth(), formantArea.getHeight()));
    formantOctaveSlider.setBounds(formantArea);
    radius = std::min(formantArea.getWidth(), formantArea.getHeight()) * 0.5f;
    byReducer = radius - 0.75f * radius;

    auto formantSemitoneArea = formantArea.reduced(byReducer);
    formantSemitoneSlider.setBounds(formantSemitoneArea);
    radius = std::min(formantSemitoneArea.getWidth(), formantSemitoneArea.getHeight()) * 0.5f;
    byReducer = radius - 0.66f * radius;

    auto formantCentArea = formantSemitoneArea.reduced(byReducer);
    formantCentSlider.setBounds(formantCentArea);



   
}

std::vector<juce::Component*> PitchScalerAudioProcessorEditor::getComps()
{
    return
    {
        &octaveShiftSlider,
        &semiTomeShiftSlider,
        &centShiftSlider,
        &formantOctaveSlider,
        &formantSemitoneSlider,
        &formantCentSlider,
        &drySlider,
        &wetSlider,
        &crispynessSlider

    };
}

void PitchScalerAudioProcessorEditor::siderEditor()
{
    crispynessSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    

    
    semiTomeShiftSlider.setRotaryParameters(0.f, 6.28f, m_AtLimit);
    centShiftSlider.setRotaryParameters(0.f, 6.28f, m_AtCentLimit);
    formantSemitoneSlider.setRotaryParameters(0.f, 6.28f, m_AtFormantLimit);
    formantCentSlider.setRotaryParameters(0.f, 6.28f, m_AtFormantCentLimit);


    octaveShiftSlider.setDoubleClickReturnValue(true, 0);
    semiTomeShiftSlider.setDoubleClickReturnValue(true, 0);
    centShiftSlider.setDoubleClickReturnValue(true, 0);
    formantOctaveSlider.setDoubleClickReturnValue(true, 0);
    formantSemitoneSlider.setDoubleClickReturnValue(true, 0);
    formantCentSlider.setDoubleClickReturnValue(true, 0);
    drySlider.setDoubleClickReturnValue(true, 0);
    wetSlider.setDoubleClickReturnValue(true, 1);
    crispynessSlider.setDoubleClickReturnValue(true, 1.5);

}
void PitchScalerAudioProcessorEditor::sliderValueManipulator()
{
    //shifter manipulation
    if (octaveShiftSlider.getValue() == -2 && semiTomeShiftSlider.getValue() == 0)
    {
        m_AtLimit = true;
        if (centShiftSlider.getValue() < 10)
        {
            m_AtCentLimit = true;
        }
        else
        {
            m_AtCentLimit = false;
        }

    }
    else if (octaveShiftSlider.getValue() == 2 && semiTomeShiftSlider.getValue() == m_SemitoneMax)
    {
        m_AtLimit = true;
        if (centShiftSlider.getValue() > m_CentMax - 10)
        {
            m_AtCentLimit = true;
        }
        else
        {
            m_AtCentLimit = false;
        }
    }
    else
    {
        m_AtLimit = false;
    }

    semiTomeShiftSlider.setRotaryParameters(0.f, 6.28f, m_AtLimit);
    centShiftSlider.setRotaryParameters(0.f, 6.28f, m_AtCentLimit);
    

    semiTomeShiftSlider.onValueChange = [this]()
    {
        /*semiTomeShiftSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow,
            true,
            10,
            10);*/
        if (semiTomeShiftSlider.getValue() == m_SemitoneMax && m_SemiToneValue == 0)
        {
            octaveShiftSlider.setValue(octaveShiftSlider.getValue() - 1);
        }
        else if (semiTomeShiftSlider.getValue() == 0 && m_SemiToneValue == m_SemitoneMax)
        {
            octaveShiftSlider.setValue(octaveShiftSlider.getValue() + 1);
        }
        m_SemiToneValue = semiTomeShiftSlider.getValue();
    };

    centShiftSlider.onValueChange = [this]()
    {
        std::cout << centShiftSlider.getValue() << std::endl;
        if (round(centShiftSlider.getValue()) > m_CentMax - 10 && m_CentValue < 10)
        {
            if (semiTomeShiftSlider.getValue() == 0 && octaveShiftSlider.getValue() != -2)
            {
                semiTomeShiftSlider.setValue(m_SemitoneMax);
            }
            else {
                semiTomeShiftSlider.setValue((static_cast <int> (semiTomeShiftSlider.getValue() - 1)) % 13);
            }
        }
        else if (round(centShiftSlider.getValue()) < 10 && m_CentValue > m_CentMax - 10)
        {
            semiTomeShiftSlider.setValue((static_cast <int> (semiTomeShiftSlider.getValue() + 1)) % 13);

        }
        m_CentValue = round(centShiftSlider.getValue());
    };
    
    //formant manipulation
    if (formantOctaveSlider.getValue() == -2 && formantSemitoneSlider.getValue() == 0)
    {
        m_AtFormantLimit = true;
        if (formantCentSlider.getValue() < 10)
        {
            m_AtFormantCentLimit = true;
        }
        else
        {
            m_AtFormantCentLimit = false;
        }

    }
    else if (formantOctaveSlider.getValue() == 2 && formantSemitoneSlider.getValue() == m_SemitoneMax)
    {
        m_AtFormantLimit = true;
        if (formantCentSlider.getValue() > m_CentMax - 10)
        {
            m_AtFormantCentLimit = true;
        }
        else
        {
            m_AtFormantCentLimit = false;
        }
    }
    else
    {
        m_AtFormantLimit = false;
    }


    formantSemitoneSlider.setRotaryParameters(0.f, 6.28f, m_AtFormantLimit);
    formantCentSlider.setRotaryParameters(0.f, 6.28f, m_AtFormantCentLimit);


    formantSemitoneSlider.onValueChange = [this]()
    {
        if (formantSemitoneSlider.getValue() == m_SemitoneMax && m_SemiFormantValue == 0)
        {
            formantOctaveSlider.setValue(formantOctaveSlider.getValue() - 1);
        }
        else if (formantSemitoneSlider.getValue() == 0 && m_SemiFormantValue == m_SemitoneMax)
        {
            formantOctaveSlider.setValue(formantOctaveSlider.getValue() + 1);
        }
        m_SemiFormantValue = formantSemitoneSlider.getValue();
    };

    formantCentSlider.onValueChange = [this]()
    {
        std::cout << formantCentSlider.getValue() << std::endl;
        if (round(formantCentSlider.getValue()) > m_CentMax - 10 && m_CentFormantValue < 10)
        {
            if (formantSemitoneSlider.getValue() == 0 && formantOctaveSlider.getValue() != -2)
            {
                formantSemitoneSlider.setValue(m_SemitoneMax);
            }
            else {
                formantSemitoneSlider.setValue((static_cast <int> (formantSemitoneSlider.getValue() - 1)) % 13);
            }
        }
        else if (round(formantCentSlider.getValue()) < 10 && m_CentFormantValue > m_CentMax - 10)
        {
            formantSemitoneSlider.setValue((static_cast <int> (formantSemitoneSlider.getValue() + 1)) % 13);

        }
        m_CentFormantValue = round(formantCentSlider.getValue());
    };
    
    
}