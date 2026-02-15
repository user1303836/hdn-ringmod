#pragma once

#include <cmath>

class Oscillator
{
public:
    enum class Waveform { Sine, Triangle, Square, Saw };

    void prepare(double sampleRate);
    void setFrequency(float hz);
    void setWaveform(Waveform w);
    float nextSample();

private:
    double sr = 44100.0;
    float freq = 440.0f;
    double phase = 0.0;
    double phaseIncrement = 0.0;
    Waveform waveform = Waveform::Sine;

    void updateIncrement();
    static double polyBLEP(double t, double dt);
};
