#pragma once

class NightVision {
public:
    static void enable();
    static void disable();
    static void setEnabled(bool enabled);
    static void update();
    static bool isEnabled();

    /// Apply runtime effect based on current config (called after load/enable).
    static void applyFromConfig(bool enabled);

private:
    static bool mEnabled;
};
