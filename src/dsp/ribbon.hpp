#pragma once
#include <algorithm>
#include <cmath>
#include <array>

namespace clonotribe {

class Ribbon {
public:
    enum class Mode { Key = 0, Narrow = 1, Wide = 2 };

    Ribbon() = default;

    void setMode(int m) noexcept {
        mode = static_cast<Mode>(std::clamp(m, 0, 2));
    }
    void setOctave(float oct) noexcept { octave = oct; }
    void setTouching(bool t) noexcept { touching = t; }
    void setPosition(float pos) noexcept { position = std::clamp(pos, 0.0f, 1.0f); }
    float getPosition() const noexcept { return position; }

    [[nodiscard]] float getCV() const noexcept {
        // Map position to CV depending on mode
        switch (mode) {
            case Mode::Key: {
                int step = static_cast<int>(position * 12.0f);
                return (static_cast<float>(step) / 12.0f) + octave;
            }
            case Mode::Narrow:
                return position * 2.0f + octave - 1.0f;
            case Mode::Wide:
                return position * 5.0f + octave - 2.5f;
            default:
                return 0.0f;
        }
    }
    [[nodiscard]] float getGate() const noexcept {
        return touching ? 10.0f : 0.0f;
    }
    [[nodiscard]] float getGateTimeMod() const noexcept {
        // 0.0f (short) to 1.0f (long)
        return position;
    }
    [[nodiscard]] float getVolumeAutomation() const noexcept {
        // -1.0f (min) to +1.0f (max)
        return (position - 0.5f) * 2.0f;
    }
    [[nodiscard]] float getDrumRollIntensity() const noexcept {
        // 0.0f (min) to 1.0f (max)
        return position;
    }
    bool touching = false;
private:
    Mode mode = Mode::Key;
    float octave = 0.0f;
    float position = 0.0f;
};
}