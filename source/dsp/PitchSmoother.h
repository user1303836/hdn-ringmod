#pragma once

#include <algorithm>
#include <cmath>

class PitchSmoother
{
public:
    inline void prepare(double sampleRate)
    {
        sr = std::max(1.0f, static_cast<float>(sampleRate));
        hasValue = false;
        smoothed = 0.0f;
        recomputeAlpha();
    }

    inline void setSmoothingAmount(float amount01)
    {
        if (amount01 == smoothingAmount)
            return;
        smoothingAmount = amount01;
        recomputeAlpha();
    }

    inline void setSensitivity(float sensitivity01)
    {
        sensitivityThreshold = 1.0f - sensitivity01;
    }

    inline float process(float detectedFreq, float confidence)
    {
        if (detectedFreq <= 0.0f || confidence < sensitivityThreshold)
            return hasValue ? std::exp2(smoothed) : 0.0f;

        float logFreq = std::log2(detectedFreq);

        if (!hasValue)
        {
            smoothed = logFreq;
            hasValue = true;
            return std::exp2(smoothed);
        }

        float delta = std::abs(logFreq - smoothed);
        float effectiveAlpha = delta > 0.08f ? 1.0f : alpha;

        smoothed += effectiveAlpha * (logFreq - smoothed);
        return std::exp2(smoothed);
    }

private:
    inline void recomputeAlpha()
    {
        float tau = smoothingAmount * 0.05f;
        if (tau < 1e-6f)
            alpha = 1.0f;
        else
            alpha = 1.0f - std::exp(-1.0f / (sr * tau));
    }

    float sr = 44100.0f;
    float smoothingAmount = 0.5f;
    float smoothed = 0.0f;
    float alpha = 0.5f;
    float sensitivityThreshold = 0.5f;
    bool hasValue = false;
};
