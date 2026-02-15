#include <catch2/catch_test_macros.hpp>
#include "ParameterIDs.h"
#include <cstring>

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
