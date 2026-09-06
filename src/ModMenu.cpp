#include "ModMenu.h"

#include "LeviVision.h"
#include "PackInstaller.h"
#include "Utils.h"

#include <pl/Mod.hpp>
#include <pl/ModMenu.hpp>

namespace {

using pl::modmenu::ConfigType;
using pl::modmenu::ModuleBuilder;

void applyVariant(const LeviVisionConfig &cfg) {
    using levivision::packs::Variant;
    // Mutually exclusive: only one shader material can be active at a time.
    // X-Ray takes priority if both are somehow on.
    if (cfg.xray) {
        levivision::packs::setActiveVariant(Variant::XRay);
    } else if (cfg.nightVision) {
        levivision::packs::setActiveVariant(Variant::NightVision);
    } else {
        levivision::packs::setActiveVariant(Variant::None);
    }
}

void onModuleToggle(std::string_view /*moduleId*/, bool enabled) {
    auto &mod = LeviVision::instance();
    auto &cfg = mod.config();
    if (!enabled) {
        levivision::packs::setActiveVariant(levivision::packs::Variant::None);
        return;
    }
    applyVariant(cfg);
}

void onConfigChanged(std::string_view /*moduleId*/, std::string_view key,
                     std::string_view value) {
    auto &mod = LeviVision::instance();
    auto &cfg = mod.config();
    auto &logger = mod.getSelf().getLogger();

    if (key == "nightVision") {
        cfg.nightVision = levivision::utils::parseBool(value, cfg.nightVision);
        if (cfg.nightVision)
            cfg.xray = false; // mutually exclusive
        logger.info("nightVision = {}", cfg.nightVision);
    } else if (key == "xray") {
        cfg.xray = levivision::utils::parseBool(value, cfg.xray);
        if (cfg.xray)
            cfg.nightVision = false; // mutually exclusive
        logger.info("xray = {}", cfg.xray);
    } else {
        logger.warn("Unknown config key: {}", key);
        return;
    }

    applyVariant(cfg);
    mod.saveConfig();
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
            .description("X-Ray and Night Vision, switched directly from this menu (no manual "
                          "steps in the game needed).")
            .defaultEnabled(true)
            .onToggle(onModuleToggle)
            .onConfigChanged(onConfigChanged)
            .config("xray", "X-Ray", ConfigType::Toggle,
                    levivision::utils::boolToString(cfg.xray))
            .config("nightVision", "Night Vision", ConfigType::Toggle,
                    levivision::utils::boolToString(cfg.nightVision))
            .registerModule();

    if (!ok) {
        native->getLogger().error("Failed to register Mod Menu module.");
        return false;
    }

    // Apply whatever was loaded from config.json on startup.
    applyVariant(cfg);

    native->getLogger().info("Mod Menu module registered.");
    return true;
}

bool ModMenu::unregisterModules() {
    pl::modmenu::unregisterModule("levivision.main");
    return true;
}
