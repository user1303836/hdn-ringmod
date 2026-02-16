#pragma once

#include <juce_dsp/juce_dsp.h>
#include <memory>
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
    int windowSize = 0;
    int halfWindow = 0;
    int hopSize = 0;

    std::vector<float> buffer;
    std::vector<float> linearBuffer;
    int writePos = 0;
    int hopCounter = 0;
    bool windowFilled = false;

    std::unique_ptr<juce::dsp::FFT> fft;
    int fftOrder = 0;
    int fftSize = 0;
    std::vector<float> fftInput;
    std::vector<float> fftOutput;

    std::vector<float> diff;
    std::vector<float> cmndf;

    PitchResult lastResult;

    static constexpr float threshold = 0.15f;
};
