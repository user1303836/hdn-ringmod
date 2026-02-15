#pragma once

#include <cmath>
#include <string>

namespace NoteNames
{
    inline std::string fromFrequency(float hz)
    {
        if (hz <= 0.0f)
            return "--";

        static const char* names[] = { "C", "C#", "D", "D#", "E", "F",
                                        "F#", "G", "G#", "A", "A#", "B" };

        float midiNote = 69.0f + 12.0f * std::log2(hz / 440.0f);
        int rounded = static_cast<int>(std::round(midiNote));
        int noteIndex = rounded % 12;
        if (noteIndex < 0) noteIndex += 12;
        int octave = (rounded / 12) - 1;

        return std::string(names[noteIndex]) + std::to_string(octave);
    }
}
