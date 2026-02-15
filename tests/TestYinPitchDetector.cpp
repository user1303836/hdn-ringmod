#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "dsp/YinPitchDetector.h"
#include <cmath>

static constexpr double twoPi = 6.283185307179586476925;

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
}

TEST_CASE("YIN: detects 440 Hz sine at 44100 Hz")
{
    YinPitchDetector yin;
    yin.prepare(44100.0);

    feedSine(yin, 44100.0, 440.0f, 44100);

    auto result = yin.getResult();
    REQUIRE(result.frequency > 0.0f);
    REQUIRE_THAT(static_cast<double>(result.frequency),
                 Catch::Matchers::WithinAbs(440.0, 3.0));
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
                 Catch::Matchers::WithinAbs(220.0, 3.0));
    REQUIRE(result.confidence > 0.5f);
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
