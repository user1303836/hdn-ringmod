#include "YinPitchDetector.h"
#include <algorithm>
#include <cmath>

void YinPitchDetector::prepare(double sampleRate)
{
    analysisSR = sampleRate;

    halfWindow = static_cast<int>(std::ceil(sampleRate / 70.0));
    windowSize = 2 * halfWindow;
    hopSize = static_cast<int>(std::ceil(sampleRate * 0.003));

    buffer.assign(static_cast<size_t>(windowSize), 0.0f);
    linearBuffer.resize(static_cast<size_t>(windowSize));
    diff.resize(static_cast<size_t>(halfWindow));
    cmndf.resize(static_cast<size_t>(halfWindow));

    writePos = 0;
    hopCounter = 0;
    windowFilled = false;
    lastResult = {};
}

void YinPitchDetector::feedSample(float sample)
{
    buffer[static_cast<size_t>(writePos)] = sample;
    writePos = (writePos + 1) % windowSize;
    ++hopCounter;

    if (!windowFilled)
    {
        if (writePos == 0)
            windowFilled = true;
        else
            return;
    }

    if (hopCounter >= hopSize)
    {
        hopCounter = 0;
        analyse();
    }
}

PitchResult YinPitchDetector::getResult() const
{
    return lastResult;
}

void YinPitchDetector::analyse()
{
    auto n = static_cast<size_t>(halfWindow);

    for (int i = 0; i < windowSize; ++i)
        linearBuffer[static_cast<size_t>(i)] = buffer[static_cast<size_t>((writePos + i) % windowSize)];

    for (size_t tau = 0; tau < n; ++tau)
    {
        float sum = 0.0f;
        for (size_t j = 0; j < n; ++j)
        {
            float d = linearBuffer[j] - linearBuffer[j + tau];
            sum += d * d;
        }
        diff[tau] = sum;
    }

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

    if (betterTau < 1.0f)
    {
        lastResult = { 0.0f, 0.0f };
        return;
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
