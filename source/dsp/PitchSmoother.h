#pragma once

#include <cmath>

class PitchSmoother
{
public:
    inline void prepare(double sampleRate)
    {
        sr = static_cast<float>(sampleRate);
        recomputeAlpha();
    }

    inline void setSmoothingAmount(float amount01)
    {
        smoothingAmount = amount01;
        recomputeAlpha();
    }

    inline void setSensitivity(float sensitivity01)
    {
        sensitivityThreshold = 1.0f - sensitivity01;
    }

    inline float process(float detectedFreq, float confidence)
    {
        if (confidence < sensitivityThreshold)
            return smoothed;

        if (!hasValue)
        {
            smoothed = detectedFreq;
            hasValue = true;
            return smoothed;
        }

        smoothed += alpha * (detectedFreq - smoothed);
        return smoothed;
    }

private:
    inline void recomputeAlpha()
    {
        float tau = smoothingAmount * 0.2f;
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
