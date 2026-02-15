#include <catch2/catch_test_macros.hpp>
#include "ParameterIDs.h"
#include <cstring>

TEST_CASE("ParameterIDs: all IDs are non-empty")
{
    REQUIRE(std::strlen(ParameterIDs::mix) > 0);
    REQUIRE(std::strlen(ParameterIDs::rateMultiplier) > 0);
    REQUIRE(std::strlen(ParameterIDs::manualRate) > 0);
    REQUIRE(std::strlen(ParameterIDs::mode) > 0);
    REQUIRE(std::strlen(ParameterIDs::smoothing) > 0);
    REQUIRE(std::strlen(ParameterIDs::sensitivity) > 0);
    REQUIRE(std::strlen(ParameterIDs::waveform) > 0);
}

TEST_CASE("ParameterIDs: IDs have expected values")
{
    REQUIRE(std::strcmp(ParameterIDs::mix, "mix") == 0);
    REQUIRE(std::strcmp(ParameterIDs::rateMultiplier, "rateMultiplier") == 0);
    REQUIRE(std::strcmp(ParameterIDs::manualRate, "manualRate") == 0);
    REQUIRE(std::strcmp(ParameterIDs::mode, "mode") == 0);
    REQUIRE(std::strcmp(ParameterIDs::smoothing, "smoothing") == 0);
    REQUIRE(std::strcmp(ParameterIDs::sensitivity, "sensitivity") == 0);
    REQUIRE(std::strcmp(ParameterIDs::waveform, "waveform") == 0);
}

TEST_CASE("ParameterIDs: all IDs are unique")
{
    const char* ids[] = {
        ParameterIDs::mix,
        ParameterIDs::rateMultiplier,
        ParameterIDs::manualRate,
        ParameterIDs::mode,
        ParameterIDs::smoothing,
        ParameterIDs::sensitivity,
        ParameterIDs::waveform
    };

    constexpr int count = 7;
    for (int i = 0; i < count; ++i)
        for (int j = i + 1; j < count; ++j)
            REQUIRE(std::strcmp(ids[i], ids[j]) != 0);
}
