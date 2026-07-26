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

    // Native brightness / fog hooks go here once signatures are resolved
    // for the target Minecraft version (see Hooks.cpp).
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
