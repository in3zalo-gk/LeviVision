#include "NightVision.h"

#include "GameSettings.h"

#include <pl/Mod.hpp>

bool NightVision::mEnabled = false;

void NightVision::enable() {
    setEnabled(true);
}

void NightVision::disable() {
    setEnabled(false);
}

void NightVision::setEnabled(bool enabled) {
    if (mEnabled == enabled)
        return;

    mEnabled = enabled;

    if (auto *mod = pl::mod::NativeMod::current()) {
        mod->getLogger().info("Night Vision: {}", enabled ? "ON" : "OFF");
    }

    // NOTE: the actual Night Vision effect is delivered two ways:
    //  1) Extreme gamma override in options.txt (setExtremeGamma below) -
    //     real, immediate, works on any server, but only applies after a
    //     full game restart (options.txt is read once at startup).
    //  2) The companion "LeviVision_BP" behavior pack (Script API), which
    //     only works in worlds this device hosts.
    levivision::gamesettings::setExtremeGamma(enabled);
    update();
}

void NightVision::update() {
    if (!mEnabled)
        return;

    // Per-frame or periodic night-vision maintenance if needed.
}

bool NightVision::isEnabled() {
    return mEnabled;
}

void NightVision::applyFromConfig(bool enabled) {
    setEnabled(enabled);
}
