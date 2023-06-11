#pragma once

#include <vector>
#include "common/RingBuffer.h"

struct Settings
{
	float octaves_value;
	float semitones_value;
	float cents_value;
	float wet_value;
    float dry_value;
	float crispness_value;
	bool formant_value;
};

namespace RubberBand {
	class RubberBandStretcher;
}

class PitchShifter
{
public:
	PitchShifter(int sampleRate, size_t channels);
	~PitchShifter();

    void processBlock(Settings& settings, int num_samples, std::vector<float*> channel_pointers);


protected:
	enum {
		LatencyPort = 0,
		CentsPort = 1,
		SemitonesPort = 2,
		OctavesPort = 3,
		CrispnessPort = 4,
		FormantPort = 5,
		WetDryPort = 6,
		InputPort1 = 7,
		OutputPort1 = 8,
		PortCountMono = OutputPort1 + 1,
		InputPort2 = 9,
		OutputPort2 = 10,
		PortCountStereo = OutputPort2 + 1
	};

    void loadSettings(Settings& settings);
    void activateImpl();
    void runImpl(uint32_t count);
    void runImpl(uint32_t count, uint32_t offset);
    void updateRatio();
    void updateCrispness();
    void updateFormant();

    int getLatency() const;

    float** m_input;
    float** m_output;
    float* m_latency;
    float* m_cents;
    float* m_semitones;
    float* m_octaves;
    float* m_crispness;
    bool m_formant;
    float* m_wet;
    float* m_dry;
    double m_ratio;
    double m_prevRatio;
    int m_currentCrispness;

    size_t m_blockSize;
    size_t m_reserve;
    size_t m_bufsize;
    size_t m_minfill;

    RubberBand::RubberBandStretcher* m_stretcher;
    RubberBand::RingBuffer<float>** m_outputBuffer;
    RubberBand::RingBuffer<float>** m_delayMixBuffer;
    float** m_scratch;
    float** m_inptrs;

    int m_sampleRate;
    size_t m_channels;

private:

};

