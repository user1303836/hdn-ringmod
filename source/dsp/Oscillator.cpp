#include "Oscillator.h"

void Oscillator::prepare(double sampleRate)
{
    sr = sampleRate;
    phase = 0.0;
    updateIncrement();
}

void Oscillator::setFrequency(float hz)
{
    freq = hz;
    updateIncrement();
}

void Oscillator::setWaveform(Waveform w)
{
    waveform = w;
}

float Oscillator::nextSample()
{
    float out = 0.0f;

    switch (waveform)
    {
        case Waveform::Sine:
            out = static_cast<float>(std::sin(phase * 2.0 * 3.14159265358979323846));
            break;
        case Waveform::Triangle:
            out = static_cast<float>(2.0 * std::abs(2.0 * phase - 1.0) - 1.0);
            break;
        case Waveform::Square:
            out = (phase < 0.5) ? 1.0f : -1.0f;
            break;
        case Waveform::Saw:
            out = static_cast<float>(2.0 * phase - 1.0);
            break;
    }

    phase += phaseIncrement;
    if (phase >= 1.0)
        phase -= 1.0;

    return out;
}

void Oscillator::updateIncrement()
{
    phaseIncrement = static_cast<double>(freq) / sr;
}
