#include "YinPitchDetector.h"
#include <algorithm>
#include <cmath>
#include <thread>
#include <chrono>

class YinPitchDetector::AnalysisThread : public juce::Thread
{
public:
    AnalysisThread(YinPitchDetector& owner)
        : Thread("PitchAnalysis"), o(owner) {}

    void run() override
    {
        while (!threadShouldExit())
        {
            int ready = o.fifo.getNumReady();
            if (ready == 0)
            {
                wait(1);
                continue;
            }

            int start1, size1, start2, size2;
            o.fifo.prepareToRead(ready, start1, size1, start2, size2);

            for (int i = 0; i < size1; ++i)
                processSample(o.fifoBuffer[static_cast<size_t>(start1 + i)]);
            for (int i = 0; i < size2; ++i)
                processSample(o.fifoBuffer[static_cast<size_t>(start2 + i)]);

            o.fifo.finishedRead(size1 + size2);
        }
    }

private:
    void processSample(float sample)
    {
        if (!o.decimator.processSample(sample))
            return;
        float decimated = o.decimator.getOutput();

        o.buffer[static_cast<size_t>(o.writePos)] = decimated;
        if (++o.writePos >= o.windowSize) o.writePos = 0;
        ++o.hopCounter;

        if (!o.windowFilled)
        {
            if (o.writePos == 0)
                o.windowFilled = true;
            else
                return;
        }

        if (o.hopCounter >= o.hopSize)
        {
            o.hopCounter = 0;
            o.analyse();
            o.atomicFreq.store(o.lastResult.frequency, std::memory_order_relaxed);
            o.atomicConf.store(o.lastResult.confidence, std::memory_order_relaxed);
        }
    }

    YinPitchDetector& o;
};

YinPitchDetector::YinPitchDetector() = default;

YinPitchDetector::~YinPitchDetector()
{
    if (analysisThread)
    {
        analysisThread->signalThreadShouldExit();
        analysisThread->notify();
        analysisThread->waitForThreadToExit(1000);
    }
}

void YinPitchDetector::prepare(double sampleRate)
{
    if (analysisThread)
    {
        analysisThread->signalThreadShouldExit();
        analysisThread->notify();
        analysisThread->waitForThreadToExit(1000);
        analysisThread.reset();
    }

    double decimatedSR = sampleRate / 2.0;
    decimator.reset();

    analysisSR = decimatedSR;

    halfWindow = static_cast<int>(std::ceil(decimatedSR / 60.0));
    windowSize = 2 * halfWindow;
    hopSize = static_cast<int>(std::ceil(decimatedSR * 0.012));

    fftOrder = static_cast<int>(std::ceil(std::log2(2.0 * windowSize)));
    fftSize = 1 << fftOrder;
    fft = std::make_unique<juce::dsp::FFT>(fftOrder);
    fftInput.resize(static_cast<size_t>(fftSize * 2), 0.0f);
    fftOutput.resize(static_cast<size_t>(fftSize * 2), 0.0f);

    buffer.assign(static_cast<size_t>(windowSize), 0.0f);
    linearBuffer.resize(static_cast<size_t>(windowSize));
    diff.resize(static_cast<size_t>(halfWindow));
    cmndf.resize(static_cast<size_t>(halfWindow));

    int fifoSize = std::max(8192, static_cast<int>(sampleRate * 2.5));
    fifo.setTotalSize(fifoSize);
    fifoBuffer.resize(static_cast<size_t>(fifoSize));

    writePos = 0;
    hopCounter = 0;
    windowFilled = false;
    lastResult = {};
    atomicFreq.store(0.0f, std::memory_order_relaxed);
    atomicConf.store(0.0f, std::memory_order_relaxed);

    analysisThread = std::make_unique<AnalysisThread>(*this);
    analysisThread->startThread(juce::Thread::Priority::normal);
}

void YinPitchDetector::flushForTest()
{
    if (analysisThread)
        analysisThread->notify();
    while (fifo.getNumReady() > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void YinPitchDetector::analyse()
{
    auto n = static_cast<size_t>(halfWindow);

    int tail = windowSize - writePos;
    std::copy_n(buffer.data() + writePos, tail, linearBuffer.data());
    std::copy_n(buffer.data(), writePos, linearBuffer.data() + tail);

    juce::FloatVectorOperations::clear(fftInput.data(), fftSize * 2);
    for (size_t i = 0; i < n; ++i)
        fftInput[i] = linearBuffer[i];
    fft->performRealOnlyForwardTransform(fftInput.data());

    juce::FloatVectorOperations::clear(fftOutput.data(), fftSize * 2);
    for (int i = 0; i < windowSize; ++i)
        fftOutput[static_cast<size_t>(i)] = linearBuffer[static_cast<size_t>(i)];
    fft->performRealOnlyForwardTransform(fftOutput.data());

    for (int k = 0; k < fftSize; ++k)
    {
        float aRe = fftInput[static_cast<size_t>(2 * k)];
        float aIm = fftInput[static_cast<size_t>(2 * k + 1)];
        float bRe = fftOutput[static_cast<size_t>(2 * k)];
        float bIm = fftOutput[static_cast<size_t>(2 * k + 1)];
        fftInput[static_cast<size_t>(2 * k)]     = aRe * bRe + aIm * bIm;
        fftInput[static_cast<size_t>(2 * k + 1)] = aRe * bIm - aIm * bRe;
    }

    fft->performRealOnlyInverseTransform(fftInput.data());

    float powerTerm0 = 0.0f;
    for (size_t j = 0; j < n; ++j)
        powerTerm0 += linearBuffer[j] * linearBuffer[j];

    float powerTermTau = powerTerm0;

    diff[0] = 0.0f;
    for (size_t tau = 1; tau < n; ++tau)
    {
        powerTermTau += linearBuffer[n + tau - 1] * linearBuffer[n + tau - 1]
                      - linearBuffer[tau - 1] * linearBuffer[tau - 1];
        diff[tau] = powerTerm0 + powerTermTau - 2.0f * fftInput[tau];
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
        float minVal = 1.0f;
        for (size_t tau = 2; tau < n; ++tau)
        {
            if (cmndf[tau] < minVal)
            {
                minVal = cmndf[tau];
                tauEstimate = tau;
            }
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
