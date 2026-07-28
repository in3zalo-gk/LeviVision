#include "NightVision.h"

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

    // NOTE: the actual Night Vision effect is delivered by the companion
    // "LeviVision_BP" behavior pack (functions/tick.json), which is
    // achievement-safe and does not require native memory hooks. This flag
    // is kept for the config UI / preference tracking and for any future
    // native hook that may complement it.
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
