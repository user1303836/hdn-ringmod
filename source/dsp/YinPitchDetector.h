#pragma once

#include <juce_core/juce_core.h>
#include <vector>

struct PitchResult
{
    float frequency = 0.0f;
    float confidence = 0.0f;
};

class YinPitchDetector
{
public:
    void prepare(double sampleRate);
    void feedSample(float sample);
    PitchResult getResult() const;

private:
    void analyse();

    double sr = 44100.0;
    int windowSize = 2048;
    int halfWindow = 1024;

    std::vector<float> buffer;
    int writePos = 0;
    bool bufferFull = false;

    std::vector<float> diff;
    std::vector<float> cmndf;

    PitchResult lastResult;

    static constexpr float threshold = 0.15f;
};
