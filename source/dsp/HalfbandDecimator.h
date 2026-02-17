#pragma once

#include <array>

class HalfbandDecimator
{
public:
    void reset()
    {
        delayLine.fill(0.0f);
        phase = 0;
    }

    bool processSample(float input)
    {
        for (int i = numTaps - 1; i > 0; --i)
            delayLine[static_cast<size_t>(i)] = delayLine[static_cast<size_t>(i - 1)];
        delayLine[0] = input;

        if (++phase >= 2)
        {
            phase = 0;
            lastOutput = 0.0f;
            for (int i = 0; i < numTaps; ++i)
                lastOutput += coeffs[static_cast<size_t>(i)] * delayLine[static_cast<size_t>(i)];
            return true;
        }
        return false;
    }

    float getOutput() const { return lastOutput; }

private:
    static constexpr int numTaps = 7;
    static constexpr std::array<float, 7> coeffs = {
        -0.03125f, 0.0f, 0.28125f, 0.5f, 0.28125f, 0.0f, -0.03125f
    };
    std::array<float, 7> delayLine {};
    float lastOutput = 0.0f;
    int phase = 0;
};
