#include "RotarySlidersWithLabels.h"

//code from video: https://www.youtube.com/watch?v=i_Iq4_Kd7Rc


void LookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x,
    int y,
    int width,
    int height,
    float sliderPosProportional,
    float rotaryStartAngle,
    float rotaryEndAngle,
    juce::Slider& slider)
{
    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, height);

    auto enabled = slider.isEnabled();


    if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        auto darkRed = Colour::fromFloatRGBA(0.7f, 0.f, 0.f, 1.0f);

        auto center = bounds.getCentre();
        Path p;

        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(20);

        p.addRoundedRectangle(r, 2.f);

        jassert(rotaryStartAngle < rotaryEndAngle);

        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));


        if (m_TextHidden == false)
        {

            g.setColour(enabled ? Colours::aqua : Colours::darkgrey);
            g.fillEllipse(bounds);

            g.setFont(rswl->getTextHeight());
            auto text = rswl->getDisplayString();
            auto strWidth = g.getCurrentFont().getStringWidth(text);

            r.setSize(strWidth, rswl->getTextHeight() + 2);
            r.setCentre(bounds.getCentre());

            g.setColour(enabled ? Colours::aquamarine : Colours::darkgrey);
            g.fillRect(r);

            auto textRect = r.reduced(1);
            g.setColour(enabled ? darkRed : Colours::lightgrey);
            g.drawFittedText(text, textRect.toNearestInt(), juce::Justification::centred, 1);
        }

        g.setColour(enabled ? darkRed : Colours::grey);
        g.drawEllipse(bounds, 1.f);

        g.fillPath(p);

        
    }
}

void LookAndFeel::setTextHidden(bool state)
{
    m_TextHidden = state;
}


//==============================================================================
void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;

    auto range = getRange();

    auto sliderBounds = getSliderBounds();

    g.drawRect(sliderBounds);
    getLookAndFeel().drawRotarySlider(g,
        sliderBounds.getX(),
        sliderBounds.getY(),
        sliderBounds.getWidth(),
        sliderBounds.getHeight(),
        jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
        m_StartAngle,
        m_EndAngle,
        *this);
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

    size -= getTextHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);

    return r;

}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(m_Param))
        return choiceParam->getCurrentChoiceName();

    juce::String str;

    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(m_Param))
    {
        float val = getValue();
        str = juce::String(val, 0);
    }
    else
    {
        jassertfalse; //this shouldn't happen!
    }

    if (m_Suffix.isNotEmpty())
    {
        str << " ";
        str << m_Suffix;
    }

    return str;
}

void RotarySliderWithLabels::setTextHidden(bool state)
{
    m_Lnf.setTextHidden(state);
}

