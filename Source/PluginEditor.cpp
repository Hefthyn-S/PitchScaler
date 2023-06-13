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
    spectrumAnalyzer(std::make_shared<SpectrumAnalyzerComponent>()),
    octaveShiftSlider(*audioProcessor.apvts.getParameter("Octave Shift"), "Oct", 3.14f * 5/4 ,3.14f * 11/4),
    semiTomeShiftSlider(*audioProcessor.apvts.getParameter("Semitone Shift"), "Semi",0.f,6.28f),
    centShiftSlider(*audioProcessor.apvts.getParameter("Cent Shift"), "Cent", 0.f, 6.28f),
    formantOctaveSlider(*audioProcessor.apvts.getParameter("Octave Formant"), "Oct", 3.14f * 5 / 4, 3.14f * 11 / 4),
    formantSemitoneSlider(*audioProcessor.apvts.getParameter("Semitone Formant"), "Semi", 0.f, 6.28f),
    formantCentSlider(*audioProcessor.apvts.getParameter("Cent Formant"), "Cent", 0.f, 6.28f),
    drySlider(*audioProcessor.apvts.getParameter("Dry Amount"), "%", 3.14f * 5 / 4, 3.14f * 11 / 4),
    wetSlider(*audioProcessor.apvts.getParameter("Wet Amount"), "%", 3.14f * 5 / 4, 3.14f * 11 / 4),
    octaveShiftSliderAttachment(audioProcessor.apvts, "Octave Shift", octaveShiftSlider),
    semiTomeShiftSliderAttachment(audioProcessor.apvts, "Semitone Shift", semiTomeShiftSlider),
    centShiftSliderAttachment(audioProcessor.apvts, "Cent Shift", centShiftSlider),
    formantOctaveSliderAttachment(audioProcessor.apvts, "Octave Formant", formantOctaveSlider),
    formantSemitoneSliderAttachment(audioProcessor.apvts, "Semitone Formant", formantSemitoneSlider),
    formantCentSliderAttachment(audioProcessor.apvts, "Cent Formant", formantCentSlider),
    drySliderAttachment(audioProcessor.apvts, "Dry Amount", drySlider),
    wetSliderAttachment(audioProcessor.apvts, "Wet Amount", wetSlider),
	crispynessSliderAttachment(audioProcessor.apvts, "Crispyness", crispynessSlider),
    formantToggleAttachment(audioProcessor.apvts,"Formant Toggle",formantToggle)
{
    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }
    setSize (700, 500);

    sliderEditor();


    
    



    formantToggle.onClick = [this]()
    {
        octaveShiftSlider.setEnabled(!formantToggle.getToggleState());
        semiTomeShiftSlider.setEnabled(!formantToggle.getToggleState());
        centShiftSlider.setEnabled(!formantToggle.getToggleState());
        formantOctaveSlider.setEnabled(formantToggle.getToggleState());
        formantSemitoneSlider.setEnabled(formantToggle.getToggleState());
        formantCentSlider.setEnabled(formantToggle.getToggleState());
    };

    octaveShiftSlider.onValueChange = [this]()
    {
        semiTomeShiftSlider.setTextHidden(true);
        centShiftSlider.setTextHidden(true);
    };

    semiTomeShiftSlider.onValueChange = [this]()
    {

        if (semiTomeShiftSlider.getValue() == m_SemitoneMax && m_SemiToneValue == 0)
        {
            octaveShiftSlider.setValue(octaveShiftSlider.getValue() - 1);
        }
        else if (semiTomeShiftSlider.getValue() == 0 && m_SemiToneValue == m_SemitoneMax)
        {
            octaveShiftSlider.setValue(octaveShiftSlider.getValue() + 1);
        }
        m_SemiToneValue = semiTomeShiftSlider.getValue();
        sliderValueManipulator();
        semiTomeShiftSlider.setTextHidden(false);
        centShiftSlider.setTextHidden(true);
    };

    centShiftSlider.onValueChange = [this]()
    {
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
        sliderValueManipulator();
        centShiftSlider.setTextHidden(false);
    };

    formantOctaveSlider.onValueChange = [this]()
    {
        formantSemitoneSlider.setTextHidden(true);
        formantCentSlider.setTextHidden(true);
    };

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
        sliderValueFormantManipulator();
        formantSemitoneSlider.setTextHidden(false);
        formantCentSlider.setTextHidden(true);
    };

    formantCentSlider.onValueChange = [this]()
    {
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
        sliderValueFormantManipulator();
        formantCentSlider.setTextHidden(false);
    };

}

PitchScalerAudioProcessorEditor::~PitchScalerAudioProcessorEditor()
{
}


//==============================================================================
void PitchScalerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    auto backgroundImage = juce::ImageCache::getFromFile(juce::File::getCurrentWorkingDirectory().getChildFile("../../Assets/background.png"));
    g.drawImage(backgroundImage, getLocalBounds().toFloat());


    
}



void PitchScalerAudioProcessorEditor::resized()
{
    const float sliderSpacing{ 6.f };
    auto bounds = getLocalBounds();

    // set bounds for the spectrumAnalyzer
    auto spectrumArea = bounds.removeFromTop( bounds.getHeight() * 0.33).reduced(20);
    spectrumAnalyzer->setBounds(spectrumArea);

    // set bounds for dry/wet sliders
    auto dryWetArea = bounds.removeFromLeft(bounds.getWidth() * 0.333);
    auto dryArea = dryWetArea.removeFromBottom(dryWetArea.getHeight() * 0.5);
    auto wetArea = dryWetArea;
    dryArea = dryArea.reduced(sliderSpacing);
    drySlider.setBounds(dryArea);
    wetArea = wetArea.reduced(sliderSpacing);
    wetSlider.setBounds(wetArea.toNearestInt());

    //set bounds for crispiness slider
    auto crispynessArea = bounds.removeFromRight(bounds.getWidth() * 0.5);
    auto toggleButtonArea = crispynessArea.removeFromLeft(crispynessArea.getWidth() * 0.2);
    crispynessSlider.setBounds(crispynessArea);
    formantToggle.setBounds(toggleButtonArea);


    //defining areas for the shiftsliders
    auto shiftArea = bounds.removeFromTop(bounds.getHeight() * 0.5);
    auto center = shiftArea.getCentre();
    shiftArea.setWidth(std::min(shiftArea.getWidth(), shiftArea.getHeight()));
    shiftArea.setHeight(std::min(shiftArea.getWidth(), shiftArea.getHeight()));
    shiftArea.setCentre(center);
    
    shiftArea = shiftArea.reduced(sliderSpacing);
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
    
    formantArea = formantArea.reduced(sliderSpacing);
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
        &*spectrumAnalyzer,
        &octaveShiftSlider,
        &semiTomeShiftSlider,
        &centShiftSlider,
        &formantOctaveSlider,
        &formantSemitoneSlider,
        &formantCentSlider,
        &drySlider,
        &wetSlider,
        &crispynessSlider,
        &formantToggle

    };
}

void PitchScalerAudioProcessorEditor::sliderEditor()
{
    crispynessSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    crispynessSlider.setTextBoxStyle(juce::Slider::NoTextBox,true,1,1);


    formantOctaveSlider.setEnabled(formantToggle.getToggleState());
    formantSemitoneSlider.setEnabled(formantToggle.getToggleState());
    formantCentSlider.setEnabled(formantToggle.getToggleState());

    
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
        m_AtCentLimit = centShiftSlider.getValue() < 10;

    }
    else if (octaveShiftSlider.getValue() == 2 && semiTomeShiftSlider.getValue() == m_SemitoneMax)
    {
        m_AtLimit = true;
        m_AtCentLimit = centShiftSlider.getValue() > m_CentMax - 10;
    }
    else
    {
        m_AtLimit = false;
    }

    semiTomeShiftSlider.setRotaryParameters(0.f, 6.28f, m_AtLimit);
    centShiftSlider.setRotaryParameters(0.f, 6.28f, m_AtCentLimit);


    
    

}

void PitchScalerAudioProcessorEditor::sliderValueFormantManipulator()
{
    
    //formant manipulation
    if (formantOctaveSlider.getValue() == -2 && formantSemitoneSlider.getValue() == 0)
    {
        m_AtFormantLimit = true;
        m_AtFormantCentLimit = formantCentSlider.getValue() < 10;

    }
    else if (formantOctaveSlider.getValue() == 2 && formantSemitoneSlider.getValue() == m_SemitoneMax)
    {
        m_AtFormantLimit = true;
        m_AtFormantCentLimit = formantCentSlider.getValue() > m_CentMax - 10;
    }
    else
    {
        m_AtFormantLimit = false;
    }


    formantSemitoneSlider.setRotaryParameters(0.f, 6.28f, m_AtFormantLimit);
    formantCentSlider.setRotaryParameters(0.f, 6.28f, m_AtFormantCentLimit);




}

std::shared_ptr<SpectrumAnalyzerComponent> PitchScalerAudioProcessorEditor::getSpectrumAnalyzerComponent() {
    return spectrumAnalyzer;
}
