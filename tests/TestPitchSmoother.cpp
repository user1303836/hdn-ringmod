#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "dsp/PitchSmoother.h"
#include <cmath>

TEST_CASE("PitchSmoother: returns 0 before any valid input")
{
    PitchSmoother smoother;
    smoother.prepare(44100.0);
    smoother.setSensitivity(0.5f);

    float result = smoother.process(0.0f, 0.0f);
    REQUIRE(result == 0.0f);
}

TEST_CASE("PitchSmoother: converges to steady-state frequency")
{
    PitchSmoother smoother;
    smoother.prepare(44100.0);
    smoother.setSmoothingAmount(0.1f);
    smoother.setSensitivity(1.0f);

    float lastOutput = 0.0f;
    for (int i = 0; i < 44100; ++i)
        lastOutput = smoother.process(440.0f, 1.0f);

    REQUIRE_THAT(static_cast<double>(lastOutput),
                 Catch::Matchers::WithinAbs(440.0, 1.0));
}

TEST_CASE("PitchSmoother: holds last frequency when confidence drops")
{
    PitchSmoother smoother;
    smoother.prepare(44100.0);
    smoother.setSmoothingAmount(0.0f);
    smoother.setSensitivity(0.5f);

    float output = smoother.process(440.0f, 1.0f);
    REQUIRE_THAT(static_cast<double>(output),
                 Catch::Matchers::WithinAbs(440.0, 0.1));

    float held = smoother.process(0.0f, 0.0f);
    REQUIRE_THAT(static_cast<double>(held),
                 Catch::Matchers::WithinAbs(440.0, 0.1));
}

TEST_CASE("PitchSmoother: prepare resets state")
{
    PitchSmoother smoother;
    smoother.prepare(44100.0);
    smoother.setSmoothingAmount(0.0f);
    smoother.setSensitivity(1.0f);

    smoother.process(440.0f, 1.0f);

    smoother.prepare(44100.0);
    float result = smoother.process(0.0f, 0.0f);
    REQUIRE(result == 0.0f);
}

TEST_CASE("PitchSmoother: sensitivity threshold gates low-confidence input")
{
    PitchSmoother smoother;
    smoother.prepare(44100.0);
    smoother.setSmoothingAmount(0.0f);
    smoother.setSensitivity(0.5f);

    float result = smoother.process(440.0f, 0.3f);
    REQUIRE(result == 0.0f);
}

TEST_CASE("PitchSmoother: zero smoothing passes frequency through immediately")
{
    PitchSmoother smoother;
    smoother.prepare(44100.0);
    smoother.setSmoothingAmount(0.0f);
    smoother.setSensitivity(1.0f);

    float result = smoother.process(440.0f, 1.0f);
    REQUIRE_THAT(static_cast<double>(result),
                 Catch::Matchers::WithinAbs(440.0, 0.1));
}

TEST_CASE("PitchSmoother: smoothing slows convergence")
{
    PitchSmoother fast, slow;
    fast.prepare(44100.0);
    slow.prepare(44100.0);

    fast.setSmoothingAmount(0.1f);
    fast.setSensitivity(1.0f);
    slow.setSmoothingAmount(0.9f);
    slow.setSensitivity(1.0f);

    fast.process(220.0f, 1.0f);
    slow.process(220.0f, 1.0f);

    float fastOut = 0.0f, slowOut = 0.0f;
    for (int i = 0; i < 1000; ++i)
    {
        fastOut = fast.process(440.0f, 1.0f);
        slowOut = slow.process(440.0f, 1.0f);
    }

    REQUIRE(std::abs(fastOut - 440.0f) < std::abs(slowOut - 440.0f));
}

TEST_CASE("PitchSmoother: works in log-frequency domain (octave-uniform)")
{
    PitchSmoother smoother;
    smoother.prepare(44100.0);
    smoother.setSmoothingAmount(0.5f);
    smoother.setSensitivity(1.0f);

    smoother.process(220.0f, 1.0f);

    float output = 0.0f;
    for (int i = 0; i < 4410; ++i)
        output = smoother.process(440.0f, 1.0f);

    REQUIRE(output > 220.0f);
    REQUIRE(output < 440.0f);
}
