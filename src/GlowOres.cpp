#include "GlowOres.h"

#include "Utils.h"

#include <pl/Mod.hpp>

bool GlowOres::mEnabled = false;
bool GlowOres::mOutline = true;
int GlowOres::mStrength = 100;
int GlowOres::mRenderDistance = 128;

void GlowOres::enable() {
    setEnabled(true);
}

void GlowOres::disable() {
    setEnabled(false);
}

void GlowOres::setEnabled(bool enabled) {
    if (mEnabled == enabled)
        return;

    mEnabled = enabled;

    if (auto *mod = pl::mod::NativeMod::current()) {
        mod->getLogger().info("Glow Ores: {}", enabled ? "ON" : "OFF");
    }

    // Emissive ore look is primarily driven by the companion resource pack
    // (textures + materials). Native glow strength can modulate material
    // parameters once RenderDragon hooks are in place.
    update();
}

void GlowOres::update() {
    if (!mEnabled)
        return;
}

bool GlowOres::isEnabled() {
    return mEnabled;
}

void GlowOres::setStrength(int percent) {
    mStrength = levivision::utils::clampInt(percent, 0, 100);
    update();
}

void GlowOres::setRenderDistance(int blocks) {
    mRenderDistance = levivision::utils::clampInt(blocks, 16, 512);
    update();
}

void GlowOres::setOutline(bool enabled) {
    mOutline = enabled;
    update();
}

int GlowOres::strength() {
    return mStrength;
}

int GlowOres::renderDistance() {
    return mRenderDistance;
}

bool GlowOres::outlineEnabled() {
    return mOutline;
}

void GlowOres::applyFromConfig(bool enabled, int strength, int renderDistance, bool outline) {
    setStrength(strength);
    setRenderDistance(renderDistance);
    setOutline(outline);
    setEnabled(enabled);
}
