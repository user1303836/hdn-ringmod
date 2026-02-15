#include "YinPitchDetector.h"
#include <algorithm>
#include <cmath>

void YinPitchDetector::prepare(double sampleRate)
{
    decimation = (sampleRate > 50000.0) ? 4 : 2;
    decimationCounter = 0;
    analysisSR = sampleRate / decimation;

    windowSize = 2048;
    halfWindow = windowSize / 2;

    buffer.assign(static_cast<size_t>(windowSize), 0.0f);
    diff.resize(static_cast<size_t>(halfWindow));
    cmndf.resize(static_cast<size_t>(halfWindow));

    writePos = 0;
    bufferFull = false;
    lastResult = {};
}

void YinPitchDetector::feedSample(float sample)
{
    if (++decimationCounter < decimation)
        return;
    decimationCounter = 0;

    buffer[static_cast<size_t>(writePos)] = sample;
    ++writePos;

    if (writePos >= windowSize)
    {
        writePos = 0;
        bufferFull = true;
        analyse();
    }
}

PitchResult YinPitchDetector::getResult() const
{
    return lastResult;
}

void YinPitchDetector::analyse()
{
    if (!bufferFull)
        return;

    auto n = static_cast<size_t>(halfWindow);

    // Step 1: Difference function
    for (size_t tau = 0; tau < n; ++tau)
    {
        float sum = 0.0f;
        for (size_t j = 0; j < n; ++j)
        {
            float d = buffer[j] - buffer[j + tau];
            sum += d * d;
        }
        diff[tau] = sum;
    }

    // Step 2: Cumulative mean normalized difference function
    cmndf[0] = 1.0f;
    float runningSum = 0.0f;

    for (size_t tau = 1; tau < n; ++tau)
    {
        runningSum += diff[tau];
        if (runningSum > 0.0f)
            cmndf[tau] = diff[tau] * static_cast<float>(tau) / runningSum;
        else
            cmndf[tau] = 1.0f;
    }

    // Step 3: Absolute threshold
    size_t tauEstimate = 0;
    for (size_t tau = 2; tau < n; ++tau)
    {
        if (cmndf[tau] < threshold)
        {
            while (tau + 1 < n && cmndf[tau + 1] < cmndf[tau])
                ++tau;
            tauEstimate = tau;
            break;
        }
    }

    if (tauEstimate == 0)
    {
        lastResult = { 0.0f, 0.0f };
        return;
    }

    // Step 4: Parabolic interpolation
    float betterTau = static_cast<float>(tauEstimate);

    if (tauEstimate > 0 && tauEstimate < n - 1)
    {
        float s0 = cmndf[tauEstimate - 1];
        float s1 = cmndf[tauEstimate];
        float s2 = cmndf[tauEstimate + 1];
        float denom = 2.0f * (2.0f * s1 - s2 - s0);
        if (std::abs(denom) > 1e-12f)
            betterTau += (s0 - s2) / denom;
    }

    float freq = static_cast<float>(analysisSR) / betterTau;
    float conf = 1.0f - cmndf[tauEstimate];

    if (freq < 20.0f || freq > 5000.0f)
    {
        lastResult = { 0.0f, 0.0f };
        return;
    }

    lastResult = { freq, std::clamp(conf, 0.0f, 1.0f) };
}
