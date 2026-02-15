#pragma once

class PitchSmoother
{
public:
    inline void setSmoothingAmount(float amount01)
    {
        alpha = 1.0f - amount01;
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
    float smoothed = 0.0f;
    float alpha = 0.5f;
    float sensitivityThreshold = 0.5f;
    bool hasValue = false;
};
