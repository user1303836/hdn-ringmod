#include "Oscillator.h"
#include <algorithm>
#include <array>

static constexpr double twoPi = 6.283185307179586476925;

static const float* getSineTable()
{
    static constexpr int size = 2048;
    static const auto table = [] {
        std::array<float, size + 1> t {};
        for (int i = 0; i <= size; ++i)
            t[static_cast<size_t>(i)] = static_cast<float>(std::sin(twoPi * static_cast<double>(i) / size));
        return t;
    }();
    return table.data();
}

void Oscillator::prepare(double sampleRate)
{
    sr = std::max(1.0, sampleRate);
    phase = 0.0;
    updateIncrement();
}

void Oscillator::setFrequency(float hz)
{
    if (hz == freq)
        return;
    freq = hz;
    updateIncrement();
}

void Oscillator::setWaveform(Waveform w)
{
    waveform = w;
}

double Oscillator::polyBLEP(double t, double dt)
{
    if (dt <= 0.0)
        return 0.0;
    if (t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.0;
    }
    if (t > 1.0 - dt)
    {
        t = (t - 1.0) / dt;
        return t * t + t + t + 1.0;
    }
    return 0.0;
}

float Oscillator::nextSample()
{
    float out = 0.0f;

    switch (waveform)
    {
        case Waveform::Sine:
        {
            const float* table = getSineTable();
            double idx = phase * 2048.0;
            auto i0 = static_cast<int>(idx);
            float frac = static_cast<float>(idx - i0);
            out = table[i0] + frac * (table[i0 + 1] - table[i0]);
            break;
        }
        case Waveform::Triangle:
            out = static_cast<float>(2.0 * std::abs(2.0 * phase - 1.0) - 1.0);
            break;
        case Waveform::Square:
            out = (phase < 0.5) ? 1.0f : -1.0f;
            out += static_cast<float>(polyBLEP(phase, phaseIncrement));
            out -= static_cast<float>(polyBLEP(std::fmod(phase + 0.5, 1.0), phaseIncrement));
            break;
        case Waveform::Saw:
            out = static_cast<float>(2.0 * phase - 1.0);
            out -= static_cast<float>(polyBLEP(phase, phaseIncrement));
            break;
    }

    phase += phaseIncrement;
    while (phase >= 1.0)
        phase -= 1.0;
    while (phase < 0.0)
        phase += 1.0;

    return out;
}

void Oscillator::updateIncrement()
{
    phaseIncrement = static_cast<double>(freq) / sr;
}
