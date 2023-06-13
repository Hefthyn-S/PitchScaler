#pragma once

#include "JuceHeader.h"


//code from video: https://www.youtube.com/watch?v=i_Iq4_Kd7Rc

struct LookAndFeel : juce::LookAndFeel_V4
{
    LookAndFeel():m_TextHidden(false)
    {}
    void drawRotarySlider(juce::Graphics&,
        int x, int y, int width, int height,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider&) override;
    void setTextHidden(bool state);
private:
    bool m_TextHidden;
};



class RotarySliderWithLabels : public juce::Slider
{
public:
    RotarySliderWithLabels(juce::RangedAudioParameter& rap, const juce::String& unitSuffix, const float startAngle, const float endAngle) :
        juce::Slider(juce::Slider::SliderStyle::Rotary,
            juce::Slider::TextEntryBoxPosition::NoTextBox),
        m_Param(&rap),
        m_Suffix(unitSuffix),
        m_StartAngle(startAngle),
        m_EndAngle(endAngle)
    {
        setLookAndFeel(&m_Lnf);
    }

    ~RotarySliderWithLabels()
    {
        setLookAndFeel(nullptr);
    }


    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const { return 14; }
    juce::String getDisplayString() const;
    void setTextHidden(bool state);

private:
    LookAndFeel m_Lnf;
    juce::RangedAudioParameter* m_Param;
    juce::String m_Suffix;
    const float m_StartAngle, m_EndAngle;
};

