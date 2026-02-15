# HDN Ring Modulator

[![Build](https://github.com/user1303836/hdn-ringmod/actions/workflows/build.yml/badge.svg)](https://github.com/user1303836/hdn-ringmod/actions/workflows/build.yml)

A pitch-tracking ring modulator audio plugin (VST3/AU/Standalone) built with JUCE 8.

![funny_picture](img/Screenshot%202026-02-14%20at%208.30.52%20PM.png)

## Requirements

- CMake 3.25+
- C++20 compiler
- **macOS**: Xcode command line tools, Ninja (`brew install ninja`)
- **Windows**: Visual Studio 2022 (MSVC)

## Building

```bash
git clone --recursive https://github.com/user1303836/hdn-ringmod.git
cd hdn-ringmod
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The VST3 plugin will be in `build/HdnRingmod_artefacts/Release/VST3/`.

To build with tests:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DHDN_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build --build-config Release --output-on-failure
```

## Parameters

| Parameter | Range | Default | Description |
|---|---|---|---|
| Mix | 0 - 100% | 50% | Dry/wet blend |
| Rate Multiplier | 0.1 - 8.0x | 1.0x | Multiplier applied to tracked pitch |
| Manual Rate | 20 - 5000 Hz | 440 Hz | Fixed oscillator frequency (Manual mode) |
| Mode | Pitch Track / Manual | Pitch Track | Pitch source selection |
| Smoothing | 0 - 100% | 50% | Pitch tracking smoothing amount |
| Sensitivity | 0 - 100% | 50% | Pitch detection confidence threshold |
| Waveform | Sine / Triangle / Square / Saw | Sine | Ring modulator oscillator shape |

## How It Works

In **Pitch Track** mode, the plugin detects the pitch of the incoming audio using the YIN algorithm, then ring-modulates the signal with an oscillator locked to that pitch (multiplied by the Rate Multiplier). The wet signal is confidence-gated so the effect fades in as pitch detection becomes more certain.

In **Manual** mode, the oscillator runs at a fixed frequency set by the Manual Rate knob.

## License

GPLv3. See [LICENSE](LICENSE).
