#pragma once

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

    double analysisSR = 44100.0;
    int windowSize = 1024;
    int halfWindow = 512;
    int hopSize = 128;

    std::vector<float> buffer;
    std::vector<float> linearBuffer;
    int writePos = 0;
    int hopCounter = 0;
    int sampleCount = 0;

    std::vector<float> diff;
    std::vector<float> cmndf;

    PitchResult lastResult;

    static constexpr float threshold = 0.15f;
};
