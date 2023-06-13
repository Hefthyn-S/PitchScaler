#pragma once

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>

enum Direction {
    Input,
    Output
};

enum
{
    fftOrder  = 11,             // [1]
    fftSize   = 1 << fftOrder,  // [2]
    scopeSize = 512 // [3]
};

class SpectrumAnalyzerComponent: public juce::AudioAppComponent, private juce::Timer {
public:
    SpectrumAnalyzerComponent();
    ~SpectrumAnalyzerComponent() override;
    void paint(juce::Graphics& g) override;
    void addBuffer(const juce::AudioBuffer<float>& buffer, Direction dir);
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override {}
    void timerCallback() override;
    void pushNextSampleIntoFifo (float sample, Direction dir) noexcept;
    void drawNextFrameOfSpectrum(std::array<float, 2 * fftSize>& data, std::array<float, scopeSize>& scope);
    void drawFrame (juce::Graphics& g);
    void prepareToPlay (int, double) override {}
    void releaseResources() override          {}

private:
    juce::dsp::FFT forwardFFT;                      // [4]
    juce::dsp::WindowingFunction<float> window;     // [5]

    std::array<float, fftSize> fifo_input;                           // [6]
    std::array<float, 2 * fftSize> fftData_input;                    // [7]
    std::array<float, fftSize>  fifo_output;                           // [6]
    std::array<float, 2 * fftSize> fftData_output;                    // [7]
    std::array<float, scopeSize> scopeData_input;                    // [10]
    std::array<float, scopeSize> scopeData_output;                    // [10]

    unsigned int fifoIndex_input = 0;                              // [8]
    unsigned int fifoIndex_output = 0;                              // [8]
    bool nextFFTBlockReady_input = false;                 // [9]
    bool nextFFTBlockReady_output = false;                 // [9]
};
