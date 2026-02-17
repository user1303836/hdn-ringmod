#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "dsp/YinPitchDetector.h"
#include <cmath>

static constexpr double twoPi = 6.283185307179586476925;
static constexpr int decimationFactor = 2;

static int computeWindowSize(double sampleRate)
{
    double decimatedSR = sampleRate / decimationFactor;
    int halfWindow = static_cast<int>(std::ceil(decimatedSR / 60.0));
    return 2 * halfWindow;
}

static int computeHopSize(double sampleRate)
{
    double decimatedSR = sampleRate / decimationFactor;
    return static_cast<int>(std::ceil(decimatedSR * 0.012));
}

static void feedSine(YinPitchDetector& yin, double sampleRate, float freq, int numSamples)
{
    double phase = 0.0;
    double inc = twoPi * static_cast<double>(freq) / sampleRate;

    for (int i = 0; i < numSamples; ++i)
    {
        float sample = static_cast<float>(std::sin(phase));
        yin.feedSample(sample);
        phase += inc;
    }
    yin.flushForTest();
}

static void feedHarmonicComplex(YinPitchDetector& yin, double sampleRate, float freq, int numSamples, float amplitude)
{
    double phase = 0.0;
    double inc = twoPi * static_cast<double>(freq) / sampleRate;

    for (int i = 0; i < numSamples; ++i)
    {
        double s = std::sin(phase)
                 + 0.8 * std::sin(2.0 * phase)
                 + 0.6 * std::sin(3.0 * phase)
                 + 0.5 * std::sin(4.0 * phase)
                 + 0.3 * std::sin(5.0 * phase);
        yin.feedSample(static_cast<float>(s * static_cast<double>(amplitude)));
        phase += inc;
    }
    yin.flushForTest();
}

static int feedSineUntilDetection(YinPitchDetector& yin, double sampleRate, float freq, int maxSamples)
{
    double phase = 0.0;
    double inc = twoPi * static_cast<double>(freq) / sampleRate;
    int hopOriginal = computeHopSize(sampleRate) * decimationFactor;
    int totalFed = 0;

    while (totalFed < maxSamples)
    {
        int toFeed = std::min(hopOriginal, maxSamples - totalFed);
        for (int i = 0; i < toFeed; ++i)
        {
            float sample = static_cast<float>(std::sin(phase));
            yin.feedSample(sample);
            phase += inc;
        }
        totalFed += toFeed;
        yin.flushForTest();

        if (yin.getResult().frequency > 0.0f)
            return totalFed;
    }
    return maxSamples;
}

TEST_CASE("YIN: detects 440 Hz sine at 44100 Hz")
{
    YinPitchDetector yin;
    yin.prepare(44100.0);

    feedSine(yin, 44100.0, 440.0f, 44100);

    auto result = yin.getResult();
    REQUIRE(result.frequency > 0.0f);
    REQUIRE_THAT(static_cast<double>(result.frequency),
                 Catch::Matchers::WithinRel(440.0, 0.01));
    REQUIRE(result.confidence > 0.5f);
}

TEST_CASE("YIN: detects 220 Hz sine at 44100 Hz")
{
    YinPitchDetector yin;
    yin.prepare(44100.0);

    feedSine(yin, 44100.0, 220.0f, 44100);

    auto result = yin.getResult();
    REQUIRE(result.frequency > 0.0f);
    REQUIRE_THAT(static_cast<double>(result.frequency),
                 Catch::Matchers::WithinRel(220.0, 0.01));
    REQUIRE(result.confidence > 0.5f);
}

TEST_CASE("YIN: detects E2 (82.4 Hz) at 44100 Hz")
{
    YinPitchDetector yin;
    yin.prepare(44100.0);

    feedSine(yin, 44100.0, 82.4f, 44100);

    auto result = yin.getResult();
    REQUIRE(result.frequency > 0.0f);
    REQUIRE_THAT(static_cast<double>(result.frequency),
                 Catch::Matchers::WithinRel(82.4, 0.03));
}

TEST_CASE("YIN: detects 880 Hz sine at 48000 Hz")
{
    YinPitchDetector yin;
    yin.prepare(48000.0);

    feedSine(yin, 48000.0, 880.0f, 48000);

    auto result = yin.getResult();
    REQUIRE(result.frequency > 0.0f);
    REQUIRE_THAT(static_cast<double>(result.frequency),
                 Catch::Matchers::WithinRel(880.0, 0.03));
    REQUIRE(result.confidence > 0.5f);
}

TEST_CASE("YIN: detects pitch at 96000 Hz sample rate")
{
    YinPitchDetector yin;
    yin.prepare(96000.0);

    feedSine(yin, 96000.0, 440.0f, 96000);

    auto result = yin.getResult();
    REQUIRE(result.frequency > 0.0f);
    REQUIRE_THAT(static_cast<double>(result.frequency),
                 Catch::Matchers::WithinRel(440.0, 0.03));
    REQUIRE(result.confidence > 0.5f);
}

TEST_CASE("YIN: returns zero for silence")
{
    YinPitchDetector yin;
    yin.prepare(44100.0);

    for (int i = 0; i < 44100; ++i)
        yin.feedSample(0.0f);
    yin.flushForTest();

    auto result = yin.getResult();
    REQUIRE(result.frequency == 0.0f);
}

TEST_CASE("YIN: prepare resets state")
{
    YinPitchDetector yin;
    yin.prepare(44100.0);

    feedSine(yin, 44100.0, 440.0f, 44100);
    REQUIRE(yin.getResult().frequency > 0.0f);

    yin.prepare(44100.0);
    REQUIRE(yin.getResult().frequency == 0.0f);
}

TEST_CASE("YIN: rejects out-of-range frequencies")
{
    YinPitchDetector yin;
    yin.prepare(44100.0);

    feedSine(yin, 44100.0, 10.0f, 44100);

    auto result = yin.getResult();
    REQUIRE(result.frequency == 0.0f);
}

TEST_CASE("YIN: confidence is clamped to [0, 1]")
{
    YinPitchDetector yin;
    yin.prepare(44100.0);

    feedSine(yin, 44100.0, 440.0f, 44100);

    auto result = yin.getResult();
    REQUIRE(result.confidence >= 0.0f);
    REQUIRE(result.confidence <= 1.0f);
}

TEST_CASE("YIN: first detection within one window fill")
{
    YinPitchDetector yin;
    yin.prepare(44100.0);

    int winOriginal = computeWindowSize(44100.0) * decimationFactor;
    int hopOriginal = computeHopSize(44100.0) * decimationFactor;
    int samplesNeeded = feedSineUntilDetection(yin, 44100.0, 440.0f, 44100);

    REQUIRE(samplesNeeded <= winOriginal + hopOriginal);
    REQUIRE(yin.getResult().frequency > 0.0f);
}

TEST_CASE("YIN: detects pitch change after silence")
{
    YinPitchDetector yin;
    yin.prepare(44100.0);

    int winOriginal = computeWindowSize(44100.0) * decimationFactor;
    int hopOriginal = computeHopSize(44100.0) * decimationFactor;

    for (int i = 0; i < winOriginal * 2; ++i)
        yin.feedSample(0.0f);
    yin.flushForTest();

    REQUIRE(yin.getResult().frequency == 0.0f);

    int samplesNeeded = feedSineUntilDetection(yin, 44100.0, 440.0f, 44100);

    REQUIRE(samplesNeeded <= winOriginal + hopOriginal);

    auto result = yin.getResult();
    REQUIRE(result.frequency > 0.0f);
    REQUIRE_THAT(static_cast<double>(result.frequency),
                 Catch::Matchers::WithinRel(440.0, 0.03));
}

TEST_CASE("YIN: tracks frequency sweep across hop intervals")
{
    YinPitchDetector yin;
    yin.prepare(44100.0);

    int winOriginal = computeWindowSize(44100.0) * decimationFactor;

    feedSine(yin, 44100.0, 440.0f, winOriginal + 4000);

    auto resultBefore = yin.getResult();
    REQUIRE(resultBefore.frequency > 0.0f);
    REQUIRE_THAT(static_cast<double>(resultBefore.frequency),
                 Catch::Matchers::WithinRel(440.0, 0.03));

    feedSine(yin, 44100.0, 880.0f, winOriginal + 4000);

    auto resultAfter = yin.getResult();
    REQUIRE(resultAfter.frequency > 0.0f);
    REQUIRE_THAT(static_cast<double>(resultAfter.frequency),
                 Catch::Matchers::WithinRel(880.0, 0.05));
}

TEST_CASE("YIN: fallback detects harmonically complex low signal")
{
    YinPitchDetector yin;
    yin.prepare(44100.0);

    feedHarmonicComplex(yin, 44100.0, 82.4f, 44100, 0.15f);

    auto result = yin.getResult();
    REQUIRE(result.frequency > 0.0f);
    REQUIRE(result.frequency > 40.0f);
    REQUIRE(result.frequency < 200.0f);
}

TEST_CASE("YIN: silence still returns zero with fallback")
{
    YinPitchDetector yin;
    yin.prepare(44100.0);

    for (int i = 0; i < 44100; ++i)
        yin.feedSample(0.0f);
    yin.flushForTest();

    auto result = yin.getResult();
    REQUIRE(result.frequency == 0.0f);
    REQUIRE(result.confidence == 0.0f);
}

TEST_CASE("YIN: threshold path still preferred for clean signals")
{
    YinPitchDetector yin;
    yin.prepare(44100.0);

    feedSine(yin, 44100.0, 440.0f, 44100);

    auto result = yin.getResult();
    REQUIRE(result.confidence > 0.8f);
    REQUIRE_THAT(static_cast<double>(result.frequency),
                 Catch::Matchers::WithinRel(440.0, 0.01));
}
