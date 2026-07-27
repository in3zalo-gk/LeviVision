#include "XRay.h"

#include "Utils.h"

#include <pl/Mod.hpp>

bool XRay::mEnabled = false;
bool XRay::mOutline = true;
int XRay::mTransparency = 90;
int XRay::mRenderDistance = 128;

void XRay::enable() {
    setEnabled(true);
}

void XRay::disable() {
    setEnabled(false);
}

void XRay::setEnabled(bool enabled) {
    if (mEnabled == enabled)
        return;

    mEnabled = enabled;

    if (auto *mod = pl::mod::NativeMod::current()) {
        mod->getLogger().info("X-Ray: {}", enabled ? "ON" : "OFF");
    }

    // Prefer resource-pack path (LeviVision_RP) for transparent stone + visible ores.
    // Native RenderDragon material overrides can be layered via Hooks + MaterialBinLoader.
    update();
}

void XRay::update() {
    if (!mEnabled)
        return;

    // Optional: refresh alpha / material state if hooked.
}

bool XRay::isEnabled() {
    return mEnabled;
}

void XRay::setTransparency(int percent) {
    mTransparency = levivision::utils::clampInt(percent, 0, 100);
    update();
}

void XRay::setRenderDistance(int blocks) {
    mRenderDistance = levivision::utils::clampInt(blocks, 16, 512);
    update();
}

void XRay::setOutline(bool enabled) {
    mOutline = enabled;
    update();
}

int XRay::transparency() {
    return mTransparency;
}

int XRay::renderDistance() {
    return mRenderDistance;
}

bool XRay::outlineEnabled() {
    return mOutline;
}

void XRay::applyFromConfig(bool enabled, int transparency, int renderDistance, bool outline) {
    setTransparency(transparency);
    setRenderDistance(renderDistance);
    setOutline(outline);
    setEnabled(enabled);
}
