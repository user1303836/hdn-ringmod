#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "dsp/Oscillator.h"
#include <cmath>
#include <vector>

static constexpr double kSampleRate = 44100.0;

static std::vector<float> generate(Oscillator& osc, int numSamples)
{
    std::vector<float> out(static_cast<size_t>(numSamples));
    for (int i = 0; i < numSamples; ++i)
        out[static_cast<size_t>(i)] = osc.nextSample();
    return out;
}

static int countZeroCrossings(const std::vector<float>& buf)
{
    int crossings = 0;
    for (size_t i = 1; i < buf.size(); ++i)
        if ((buf[i - 1] >= 0.0f) != (buf[i] >= 0.0f))
            ++crossings;
    return crossings;
}

TEST_CASE("Oscillator: sine output stays in [-1, 1]")
{
    Oscillator osc;
    osc.prepare(kSampleRate);
    osc.setWaveform(Oscillator::Waveform::Sine);
    osc.setFrequency(440.0f);

    auto buf = generate(osc, 44100);

    for (auto s : buf)
    {
        REQUIRE(s >= -1.0f);
        REQUIRE(s <= 1.0f);
    }
}

TEST_CASE("Oscillator: all waveforms stay in bounded range")
{
    Oscillator::Waveform waveforms[] = {
        Oscillator::Waveform::Sine,
        Oscillator::Waveform::Triangle,
        Oscillator::Waveform::Square,
        Oscillator::Waveform::Saw
    };

    for (auto wf : waveforms)
    {
        DYNAMIC_SECTION("waveform " << static_cast<int>(wf))
        {
            Oscillator osc;
            osc.prepare(kSampleRate);
            osc.setWaveform(wf);
            osc.setFrequency(1000.0f);

            auto buf = generate(osc, 44100);

            for (auto s : buf)
            {
                REQUIRE(s >= -1.01f);
                REQUIRE(s <= 1.01f);
            }
        }
    }
}

TEST_CASE("Oscillator: sine frequency matches expected zero crossings")
{
    float freq = 440.0f;
    int numSamples = 44100;

    Oscillator osc;
    osc.prepare(kSampleRate);
    osc.setWaveform(Oscillator::Waveform::Sine);
    osc.setFrequency(freq);

    auto buf = generate(osc, numSamples);
    int crossings = countZeroCrossings(buf);

    float expectedCycles = freq * static_cast<float>(numSamples) / static_cast<float>(kSampleRate);
    float expectedCrossings = expectedCycles * 2.0f;

    REQUIRE(std::abs(static_cast<float>(crossings) - expectedCrossings) < 4.0f);
}

TEST_CASE("Oscillator: prepare resets phase")
{
    Oscillator osc;
    osc.prepare(kSampleRate);
    osc.setWaveform(Oscillator::Waveform::Sine);
    osc.setFrequency(440.0f);

    generate(osc, 1000);

    osc.prepare(kSampleRate);
    osc.setFrequency(440.0f);
    float after = osc.nextSample();

    REQUIRE_THAT(after, Catch::Matchers::WithinAbs(0.0, 0.01));
}

TEST_CASE("Oscillator: square wave has correct zero crossing count")
{
    float freq = 100.0f;
    int numSamples = 44100;

    Oscillator osc;
    osc.prepare(kSampleRate);
    osc.setWaveform(Oscillator::Waveform::Square);
    osc.setFrequency(freq);

    auto buf = generate(osc, numSamples);
    int crossings = countZeroCrossings(buf);

    float expectedCycles = freq * static_cast<float>(numSamples) / static_cast<float>(kSampleRate);
    float expectedCrossings = expectedCycles * 2.0f;

    REQUIRE(std::abs(static_cast<float>(crossings) - expectedCrossings) < 4.0f);
}

TEST_CASE("Oscillator: setFrequency dirty check avoids unnecessary recompute")
{
    Oscillator osc;
    osc.prepare(kSampleRate);
    osc.setFrequency(440.0f);

    generate(osc, 100);
    float sample1 = osc.nextSample();

    osc.prepare(kSampleRate);
    osc.setFrequency(440.0f);

    generate(osc, 100);
    float sample2 = osc.nextSample();

    REQUIRE_THAT(static_cast<double>(sample1),
                 Catch::Matchers::WithinAbs(static_cast<double>(sample2), 0.001));
}

TEST_CASE("Oscillator: polyBLEP guard handles zero dt")
{
    Oscillator osc;
    osc.prepare(kSampleRate);
    osc.setWaveform(Oscillator::Waveform::Square);
    osc.setFrequency(0.0f);

    float sample = osc.nextSample();
    REQUIRE(std::isfinite(sample));
}
