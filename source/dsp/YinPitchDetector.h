#pragma once

#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <memory>
#include <vector>
#include "HalfbandDecimator.h"

struct PitchResult
{
    float frequency = 0.0f;
    float confidence = 0.0f;
};

class YinPitchDetector
{
public:
    YinPitchDetector();
    ~YinPitchDetector();

    void prepare(double sampleRate);

    inline void feedSample(float sample)
    {
        int start1, size1, start2, size2;
        fifo.prepareToWrite(1, start1, size1, start2, size2);
        if (size1 > 0)
            fifoBuffer[static_cast<size_t>(start1)] = sample;
        fifo.finishedWrite(size1 + size2);
    }

    inline PitchResult getResult() const
    {
        return { atomicFreq.load(std::memory_order_relaxed),
                 atomicConf.load(std::memory_order_relaxed) };
    }

    void flushForTest();

private:
    void analyse();

    class AnalysisThread;
    friend class AnalysisThread;

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

    juce::AbstractFifo fifo { 0 };
    std::vector<float> fifoBuffer;

    std::unique_ptr<AnalysisThread> analysisThread;

    std::atomic<float> atomicFreq { 0.0f };
    std::atomic<float> atomicConf { 0.0f };

    HalfbandDecimator decimator;
};
