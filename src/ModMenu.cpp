#include "ModMenu.h"

#include "GlowOres.h"
#include "LeviVision.h"
#include "NightVision.h"
#include "Utils.h"
#include "XRay.h"

#include <pl/Mod.hpp>
#include <pl/ModMenu.hpp>

namespace {

using pl::modmenu::ButtonBehavior;
using pl::modmenu::ButtonBuilder;
using pl::modmenu::ButtonEvent;
using pl::modmenu::ConfigType;
using pl::modmenu::ModuleBuilder;

void onModuleToggle(std::string_view /*moduleId*/, bool enabled) {
    auto &mod = LeviVision::instance();
    auto &cfg = mod.config();

    if (!enabled) {
        // Master module off: disable visual effects but keep saved toggles.
        NightVision::disable();
        XRay::disable();
        GlowOres::disable();
        mod.getSelf().getLogger().info("LeviVision module disabled.");
        return;
    }

    NightVision::applyFromConfig(cfg.nightVision);
    XRay::applyFromConfig(cfg.xray, cfg.transparency, cfg.renderDistance, cfg.outline);
    GlowOres::applyFromConfig(cfg.glowOres, cfg.glowStrength, cfg.renderDistance, cfg.outline);
    mod.getSelf().getLogger().info("LeviVision module enabled.");
}

void onConfigChanged(std::string_view /*moduleId*/, std::string_view key,
                     std::string_view value) {
    auto &mod = LeviVision::instance();
    auto &cfg = mod.config();
    auto &logger = mod.getSelf().getLogger();

    if (key == "nightVision") {
        cfg.nightVision = levivision::utils::parseBool(value, cfg.nightVision);
        NightVision::applyFromConfig(cfg.nightVision);
        logger.info("Config nightVision = {}", cfg.nightVision);
    } else if (key == "xray") {
        cfg.xray = levivision::utils::parseBool(value, cfg.xray);
        XRay::applyFromConfig(cfg.xray, cfg.transparency, cfg.renderDistance, cfg.outline);
        logger.info("Config xray = {}", cfg.xray);
    } else if (key == "glowOres") {
        cfg.glowOres = levivision::utils::parseBool(value, cfg.glowOres);
        GlowOres::applyFromConfig(cfg.glowOres, cfg.glowStrength, cfg.renderDistance,
                                  cfg.outline);
        logger.info("Config glowOres = {}", cfg.glowOres);
    } else if (key == "outline") {
        cfg.outline = levivision::utils::parseBool(value, cfg.outline);
        XRay::setOutline(cfg.outline);
        GlowOres::setOutline(cfg.outline);
        logger.info("Config outline = {}", cfg.outline);
    } else if (key == "renderDistance") {
        cfg.renderDistance =
            levivision::utils::clampInt(levivision::utils::parseInt(value, cfg.renderDistance),
                                       16, 512);
        XRay::setRenderDistance(cfg.renderDistance);
        GlowOres::setRenderDistance(cfg.renderDistance);
        logger.info("Config renderDistance = {}", cfg.renderDistance);
    } else if (key == "transparency") {
        cfg.transparency =
            levivision::utils::clampInt(levivision::utils::parseInt(value, cfg.transparency), 0,
                                       100);
        XRay::setTransparency(cfg.transparency);
        logger.info("Config transparency = {}", cfg.transparency);
    } else if (key == "glowStrength") {
        cfg.glowStrength =
            levivision::utils::clampInt(levivision::utils::parseInt(value, cfg.glowStrength), 0,
                                       100);
        GlowOres::setStrength(cfg.glowStrength);
        logger.info("Config glowStrength = {}", cfg.glowStrength);
    } else {
        logger.warn("Unknown config key: {}", key);
        return;
    }

    mod.saveConfig();
}

// Currently unused: onEvent callback for the floating button, which is
// temporarily disabled (see registerButtons() below).
[[maybe_unused]] void onFloatingButton(std::string_view /*buttonId*/, ButtonEvent event, float /*value*/) {
    if (event != ButtonEvent::Click)
        return;

    auto &mod = LeviVision::instance();
    auto &cfg = mod.config();

    // Quick cycle: all off -> NV -> XRay -> Glow -> all visual off
    const bool any = cfg.nightVision || cfg.xray || cfg.glowOres;
    if (!any) {
        cfg.nightVision = true;
    } else if (cfg.nightVision && !cfg.xray && !cfg.glowOres) {
        cfg.nightVision = false;
        cfg.xray = true;
    } else if (cfg.xray && !cfg.glowOres) {
        cfg.xray = false;
        cfg.glowOres = true;
    } else {
        cfg.nightVision = false;
        cfg.xray = false;
        cfg.glowOres = false;
    }

    NightVision::applyFromConfig(cfg.nightVision);
    XRay::applyFromConfig(cfg.xray, cfg.transparency, cfg.renderDistance, cfg.outline);
    GlowOres::applyFromConfig(cfg.glowOres, cfg.glowStrength, cfg.renderDistance, cfg.outline);
    mod.saveConfig();

    mod.getSelf().getLogger().info("LV button: NV={} XRay={} Glow={}", cfg.nightVision, cfg.xray,
                                   cfg.glowOres);
}

} // namespace

bool ModMenu::registerModules() {
    auto *native = pl::mod::NativeMod::current();
    if (!native)
        return false;

    auto &cfg = LeviVision::instance().config();

    const bool ok =
        ModuleBuilder("levivision.main", "LeviVision")
            .modId(native->getId())
            .description("Night Vision, X-Ray e Glow Ores para Minecraft Bedrock.")
            .defaultEnabled(true)
            .onToggle(onModuleToggle)
            .onConfigChanged(onConfigChanged)
            .config("nightVision", "Night Vision", ConfigType::Toggle,
                    levivision::utils::boolToString(cfg.nightVision))
            .config("xray", "X-Ray", ConfigType::Toggle,
                    levivision::utils::boolToString(cfg.xray))
            .config("glowOres", "Glow Ores", ConfigType::Toggle,
                    levivision::utils::boolToString(cfg.glowOres))
            .config("outline", "Outline", ConfigType::Toggle,
                    levivision::utils::boolToString(cfg.outline))
            .config("transparency", "Transparency", ConfigType::SliderInt,
                    levivision::utils::intToString(cfg.transparency), "0", "100")
            .config("glowStrength", "Glow Strength", ConfigType::SliderInt,
                    levivision::utils::intToString(cfg.glowStrength), "0", "100")
            .config("renderDistance", "Render Distance", ConfigType::SliderInt,
                    levivision::utils::intToString(cfg.renderDistance), "16", "512")
            .registerModule();

    if (!ok) {
        native->getLogger().error("Failed to register Mod Menu module.");
        return false;
    }

    native->getLogger().info("Mod Menu module registered.");
    return true;
}

bool ModMenu::unregisterModules() {
    pl::modmenu::unregisterModule("levivision.main");
    return true;
}

bool ModMenu::registerButtons() {
    // DISABLED: the floating quick-toggle button was crashing the launcher
    // with SIGSEGV inside pl::modmenu::registerButton() (confirmed via
    // xCrash tombstone — memcpy inside libpreloader.so's internal button
    // registration path). The main config module (Night Vision / X-Ray /
    // Glow Ores toggles) does not go through this code path and is
    // unaffected. Re-enable once the exact cause is confirmed against the
    // real preloader-android headers/source.
    (void)0;
    return true;

    /*
    auto *native = pl::mod::NativeMod::current();
    if (!native)
        return false;

    const bool ok = ButtonBuilder("levivision.button", "LeviVision Quick")
                        .moduleId("levivision.main")
                        .modId(native->getId())
                        .label("LV")
                        .behavior(ButtonBehavior::Click)
                        .defaultVisible(true)
                        .onEvent(onFloatingButton)
                        .registerButton();

    if (!ok) {
        native->getLogger().error("Failed to register floating button.");
        return false;
    }

    native->getLogger().info("Floating button LV registered.");
    return true;
    */
}

bool ModMenu::unregisterButtons() {
    pl::modmenu::unregisterButton("levivision.button");
    return true;
}

void ModMenu::applyConfigToModules() {
    auto &cfg = LeviVision::instance().config();
    NightVision::applyFromConfig(cfg.nightVision);
    XRay::applyFromConfig(cfg.xray, cfg.transparency, cfg.renderDistance, cfg.outline);
    GlowOres::applyFromConfig(cfg.glowOres, cfg.glowStrength, cfg.renderDistance, cfg.outline);
}
