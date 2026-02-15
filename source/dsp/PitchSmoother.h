#pragma once

class PitchSmoother
{
public:
    void setSmoothingAmount(float amount01);
    void setSensitivity(float sensitivity01);
    float process(float detectedFreq, float confidence);

private:
    float smoothed = 0.0f;
    float alpha = 0.5f;
    float sensitivityThreshold = 0.5f;
    bool hasValue = false;
};
