#include "SpectrumAnalyzerComponent.h"

SpectrumAnalyzerComponent::SpectrumAnalyzerComponent(): forwardFFT(fftOrder), window(fftSize, juce::dsp::WindowingFunction<float>::hann) {
    setOpaque(false);
    setAudioChannels(2, 0);
    startTimerHz(30);
    setSize(700, 500);
    scopeData_input.fill(0);
    scopeData_output.fill(0);
}
SpectrumAnalyzerComponent::~SpectrumAnalyzerComponent()
{
    shutdownAudio();
}

void SpectrumAnalyzerComponent::paint(juce::Graphics& g) {
    g.setColour(juce::Colour::fromFloatRGBA(0.f, 0.f, 0.f, 0.8f));
    g.fillRect(g.getClipBounds());
    g.setColour(juce::Colours::white);
    drawFrame(g);
}

void SpectrumAnalyzerComponent::addBuffer(const juce::AudioBuffer<float>& buffer, Direction dir) {
    if (buffer.getNumChannels() > 0)
    {
        auto* channelData = buffer.getReadPointer(0);
        for (auto i = 0; i < buffer.getNumSamples(); ++i) {
            pushNextSampleIntoFifo (channelData[i], dir);
        }
    }
}

void SpectrumAnalyzerComponent::timerCallback() {
    if (nextFFTBlockReady_input)
    {
        drawNextFrameOfSpectrum(fftData_input, scopeData_input);
        nextFFTBlockReady_input = false;
    }
    if (nextFFTBlockReady_output)
    {
        drawNextFrameOfSpectrum(fftData_output, scopeData_output);
        nextFFTBlockReady_output = false;
    }
    repaint();
}

void SpectrumAnalyzerComponent::pushNextSampleIntoFifo (float sample, Direction dir) noexcept
{
    // if the fifo contains enough data, set a flag to say
    // that the next frame should now be rendered..
    std::array<float, fftSize> & queue = dir == Input ? fifo_input : fifo_output;
    std::array<float, 2 * fftSize> & data = dir == Input ? fftData_input : fftData_output;
    unsigned int& index = dir == Input ? fifoIndex_input : fifoIndex_output;
    bool& nextFFTBlockReady = dir == Input ? nextFFTBlockReady_input : nextFFTBlockReady_output;

    if (index == fftSize)               // [11]
    {
        if (! nextFFTBlockReady)            // [12]
        {
            data.fill(0);
            std::copy(queue.begin(), queue.end(), data.begin());
            nextFFTBlockReady = true;
        }

        index = 0;
    }
    queue[index++] = sample;             // [12]
}

void SpectrumAnalyzerComponent::drawNextFrameOfSpectrum(std::array<float, 2 * fftSize>& data, std::array<float, scopeSize>& scope)
{
    // first apply a windowing function to our data
    window.multiplyWithWindowingTable (data.data(), fftSize);       // [1]

    // then render our FFT data..
    forwardFFT.performFrequencyOnlyForwardTransform (data.data());  // [2]

    auto mindB = -100.0f;
    auto maxdB =    0.0f;

    for (int i = 0; i < scopeSize; ++i)                         // [3]
    {
        auto skewedProportionX = 1.0f - std::exp (std::log (1.0f - (float) i / (float) scopeSize) * 0.2f);
        auto fftDataIndex = juce::jlimit (0, fftSize / 2, (int) (skewedProportionX * (float) fftSize * 0.5f));
        auto level = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (data[fftDataIndex])
                                               - juce::Decibels::gainToDecibels ((float) fftSize)),
                                 mindB, maxdB, 0.0f, 1.0f);

        scope[i] = level;                                   // [4]
    }
}

void SpectrumAnalyzerComponent::drawFrame (juce::Graphics& g)
{
    for (int i = 1; i < scopeSize; ++i)
    {
        auto width  = getLocalBounds().getWidth();
        auto height = getLocalBounds().getHeight();

        g.setColour(juce::Colours::white);
        g.drawLine ({ (float) juce::jmap (i - 1, 0, scopeSize - 1, 0, width),
            juce::jmap (scopeData_input[i - 1], 0.0f, 1.0f, (float) height, 0.0f),
            (float) juce::jmap (i,     0, scopeSize - 1, 0, width),
            juce::jmap (scopeData_input[i],     0.0f, 1.0f, (float) height, 0.0f) });

        g.setColour(juce::Colours::green);
        g.drawLine ({ (float) juce::jmap (i - 1, 0, scopeSize - 1, 0, width),
            juce::jmap (scopeData_output[i - 1], 0.0f, 1.0f, (float) height, 0.0f),
            (float) juce::jmap (i,     0, scopeSize - 1, 0, width),
            juce::jmap (scopeData_output[i],     0.0f, 1.0f, (float) height, 0.0f) });
    }
}
