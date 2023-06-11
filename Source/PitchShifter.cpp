#include "PitchShifter.h"
#include "RubberBandStretcher.h"

PitchShifter::PitchShifter(int sampleRate, size_t channels)
    : m_latency(nullptr), m_cents(nullptr), m_semitones(nullptr),
    m_octaves(nullptr), m_crispness(nullptr), m_formant(nullptr),
    m_wet(nullptr), m_dry(nullptr), m_ratio(1.0), m_prevRatio(1.0),
    m_currentCrispness(-1), m_currentFormant(false), m_blockSize(1024),
    m_reserve(8192), m_bufsize(0), m_minfill(0),
    m_stretcher(new RubberBand::RubberBandStretcher(
        sampleRate, channels,
        RubberBand::RubberBandStretcher::OptionProcessRealTime |
        RubberBand::RubberBandStretcher::OptionPitchHighConsistency)),
    m_sampleRate(sampleRate), m_channels(channels) {
    m_input = new float* [m_channels];
    m_output = new float* [m_channels];

    m_outputBuffer = new RubberBand::RingBuffer<float> *[m_channels];
    m_delayMixBuffer = new RubberBand::RingBuffer<float> *[m_channels];
    m_scratch = new float* [m_channels];
    m_inptrs = new float* [m_channels];

    m_bufsize = m_blockSize + m_reserve + 8192;

    for (size_t c = 0; c < m_channels; ++c) {

        m_input[c] = 0;
        m_output[c] = 0;

        m_outputBuffer[c] = new RubberBand::RingBuffer<float>(m_bufsize);
        m_delayMixBuffer[c] = new RubberBand::RingBuffer<float>(m_bufsize);

        m_scratch[c] = new float[m_bufsize];
        for (size_t i = 0; i < m_bufsize; ++i) {
            m_scratch[c][i] = 0.f;
        }

        m_inptrs[c] = 0;
    }

    activateImpl();
}

PitchShifter::~PitchShifter()
{
    delete m_stretcher;
    for (size_t c = 0; c < m_channels; ++c) {
        delete m_outputBuffer[c];
        delete m_delayMixBuffer[c];
        delete[] m_scratch[c];
    }
    delete[] m_outputBuffer;
    delete[] m_delayMixBuffer;
    delete[] m_inptrs;
    delete[] m_scratch;
    delete[] m_output;
    delete[] m_input;
}

void PitchShifter::activateImpl()
{
    updateRatio();
    m_prevRatio = m_ratio;
    m_stretcher->reset();
    m_stretcher->setPitchScale(m_ratio);

    for (size_t c = 0; c < m_channels; ++c) {
        m_outputBuffer[c]->reset();
    }

    for (size_t c = 0; c < m_channels; ++c) {
        m_delayMixBuffer[c]->reset();
        m_delayMixBuffer[c]->zero(getLatency());
    }

    for (size_t c = 0; c < m_channels; ++c) {
        for (size_t i = 0; i < m_bufsize; ++i) {
            m_scratch[c][i] = 0.f;
        }
    }

    m_minfill = 0;

    m_stretcher->process(m_scratch, m_reserve, false);
}

void PitchShifter::runImpl(uint32_t count)
{
    for (size_t c = 0; c < m_channels; ++c) {
        m_delayMixBuffer[c]->write(m_input[c], count);
    }

    size_t offset = 0;

    // We have to break up the input into chunks like this because
    // insamples could be arbitrarily large and our output buffer is
    // of limited size

    while (offset < count) {

        size_t block = m_blockSize;
        if (offset + block > count) {
            block = count - offset;
        }

        runImpl(block, offset);

        offset += block;
    }

    float mix = 0.0;
    if (m_wet) mix = *m_wet;

    for (size_t c = 0; c < m_channels; ++c) {
            for (size_t i = 0; i < count; ++i) {
                float dry = m_delayMixBuffer[c]->readOne();
                m_output[c][i] *= (1-mix);
                m_output[c][i] += *m_dry * dry;
            }
       
    }

   
}

void PitchShifter::runImpl(uint32_t count, uint32_t offset)
{
    updateRatio();
    if (m_ratio != m_prevRatio) {
        m_stretcher->setPitchScale(m_ratio);
        m_prevRatio = m_ratio;
    }

    if (m_latency) {
        *m_latency = getLatency();
    }

    updateCrispness();
    updateFormant();

    const int samples = count;
    int processed = 0;
    size_t outTotal = 0;

    while (processed < samples) {

        // never feed more than the minimum necessary number of
        // samples at a time; ensures nothing will overflow internally
        // and we don't need to call setMaxProcessSize

        int toCauseProcessing = m_stretcher->getSamplesRequired();
        int inchunk = std::min(samples - processed, toCauseProcessing);

        for (size_t c = 0; c < m_channels; ++c) {
            m_inptrs[c] = &(m_input[c][offset + processed]);
        }

        m_stretcher->process(m_inptrs, inchunk, false);

        processed += inchunk;

        int avail = m_stretcher->available();
        int writable = m_outputBuffer[0]->getWriteSpace();

        int outchunk = avail;
        if (outchunk > writable) {
            std::cerr << "RubberBandPitchShifter::runImpl: buffer is not large enough: size = " << m_outputBuffer[0]->getSize() << ", chunk = " << outchunk << ", space = " << writable << " (buffer contains " << m_outputBuffer[0]->getReadSpace() << " unread)" << std::endl;
            outchunk = writable;
        }

        size_t actual = m_stretcher->retrieve(m_scratch, outchunk);
        outTotal += actual;

        for (size_t c = 0; c < m_channels; ++c) {
            m_outputBuffer[c]->write(m_scratch[c], actual);
        }
    }

    for (size_t c = 0; c < m_channels; ++c) {
        int toRead = m_outputBuffer[c]->getReadSpace();
        if (toRead < samples && c == 0) {
            std::cerr << "RubberBandPitchShifter::runImpl: buffer underrun: required = " << samples << ", available = " << toRead << std::endl;
        }
        int chunk = std::min(toRead, samples);
        m_outputBuffer[c]->read(&(m_output[c][offset]), chunk);
    }

    size_t fill = m_outputBuffer[0]->getReadSpace();
    if (fill < m_minfill || m_minfill == 0) {
        m_minfill = fill;
        //        cerr << "minfill = " << m_minfill << endl;
    }
}

int PitchShifter::getLatency() const
{
    return m_reserve;
}

void PitchShifter::updateRatio()
{
    double octaves = round(m_octaves ? *m_octaves : 0.0);
    if (octaves < -2.0) octaves = -2.0;
    if (octaves > 2.0) octaves = 2.0;

    double semitones = round(m_semitones ? *m_semitones : 0.0);
    if (semitones < -12.0) semitones = -12.0;
    if (semitones > 12.0) semitones = 12.0;

    double cents = round(m_cents ? *m_cents : 0.0);
    if (cents < -100.0) cents = -100.0;
    if (cents > 100.0) cents = 100.0;

    m_ratio = pow(2.0,
        octaves +
        semitones / 12.0 +
        cents / 1200.0);
}

void PitchShifter::updateCrispness()
{
    if (!m_crispness) return;

    int c = lrintf(*m_crispness);
    if (c == m_currentCrispness) return;
    if (c < 0 || c > 3) return;
    RubberBand::RubberBandStretcher* s = m_stretcher;

    switch (c) {
    case 0:
        s->setPhaseOption(RubberBand::RubberBandStretcher::OptionPhaseIndependent);
        s->setTransientsOption(RubberBand::RubberBandStretcher::OptionTransientsSmooth);
        break;
    case 1:
        s->setPhaseOption(RubberBand::RubberBandStretcher::OptionPhaseLaminar);
        s->setTransientsOption(RubberBand::RubberBandStretcher::OptionTransientsSmooth);
        break;
    case 2:
        s->setPhaseOption(RubberBand::RubberBandStretcher::OptionPhaseLaminar);
        s->setTransientsOption(RubberBand::RubberBandStretcher::OptionTransientsMixed);
        break;
    case 3:
        s->setPhaseOption(RubberBand::RubberBandStretcher::OptionPhaseLaminar);
        s->setTransientsOption(RubberBand::RubberBandStretcher::OptionTransientsCrisp);
        break;
    }

    m_currentCrispness = c;
}

void PitchShifter::updateFormant()
{
    if (!m_formant) return;

    bool f = (*m_formant > 0.5f);
    if (f == m_currentFormant) return;

    RubberBand::RubberBandStretcher* s = m_stretcher;

    s->setFormantOption(f ? RubberBand::RubberBandStretcher::OptionFormantPreserved : RubberBand::RubberBandStretcher::OptionFormantShifted);

    m_currentFormant = f;
}

void PitchShifter::loadSettings(Settings& settings)
{
	m_octaves = &settings.octaves_value;
	m_semitones = &settings.semitones_value;
	m_cents = &settings.cents_value;
	m_wet = &settings.wet_value;
    m_dry = &settings.dry_value;
	m_crispness = &settings.crispness_value;
	m_formant = &settings.formant_value;
}

void PitchShifter::processBlock(Settings& settings, int num_samples, std::vector<float*> channel_pointers)
{
	loadSettings(settings);
	for (int channel = 0; channel < channel_pointers.size(); ++channel) {
		m_input[channel] = channel_pointers[channel];
		m_output[channel] = channel_pointers[channel];
	}
	runImpl(num_samples);
}