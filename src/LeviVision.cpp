#include "LeviVision.h"

#include "GlowOres.h"
#include "Hooks.h"
#include "ModMenu.h"
#include "NightVision.h"
#include "XRay.h"

#include <filesystem>

LeviVision &LeviVision::instance() {
    static LeviVision mod;
    return mod;
}

LeviVision::LeviVision() : mSelf(*pl::mod::NativeMod::current()) {}

pl::mod::NativeMod &LeviVision::getSelf() const {
    return mSelf;
}

LeviVisionConfig &LeviVision::config() {
    return mConfigValue;
}

const LeviVisionConfig &LeviVision::config() const {
    return mConfigValue;
}

bool LeviVision::saveConfig() {
    if (!mConfigFile)
        return false;

    mConfigFile->value() = mConfigValue;
    if (!mConfigFile->save()) {
        mSelf.getLogger().error("Failed to save config.json");
        return false;
    }
    return true;
}

bool LeviVision::reloadConfig() {
    if (!mConfigFile)
        return false;

    if (!mConfigFile->load()) {
        mSelf.getLogger().error("Failed to reload config.json");
        return false;
    }

    mConfigValue = mConfigFile->value();
    return true;
}

bool LeviVision::load() {
    auto &logger = mSelf.getLogger();

    logger.info("====================================");
    logger.info("  LeviVision v1.0.0");
    logger.info("  Author: Say");
    logger.info("====================================");

    std::error_code ec;
    std::filesystem::create_directories(mSelf.getDataDir(), ec);
    if (ec) {
        logger.error("Failed to create data dir {}: {}", mSelf.getDataDir().string(),
                     ec.message());
        return false;
    }

    std::filesystem::create_directories(mSelf.getConfigDir(), ec);
    if (ec) {
        logger.error("Failed to create config dir {}: {}", mSelf.getConfigDir().string(),
                     ec.message());
        return false;
    }

    mConfigFile.emplace();
    if (!mConfigFile->load()) {
        logger.error("Failed to load typed config.");
        return false;
    }

    mConfigValue = mConfigFile->value();
    if (mConfigValue.version < 1)
        mConfigValue.version = 1;

    logger.info("Config loaded (NV={} XRay={} Glow={} Outline={} RD={} TP={} GS={})",
                mConfigValue.nightVision, mConfigValue.xray, mConfigValue.glowOres,
                mConfigValue.outline, mConfigValue.renderDistance, mConfigValue.transparency,
                mConfigValue.glowStrength);

    return true;
}

bool LeviVision::enable() {
    auto &logger = mSelf.getLogger();
    logger.info("Enabling LeviVision...");

    if (!ModMenu::registerModules()) {
        logger.warn("Mod Menu module registration failed.");
    }

    if (!ModMenu::registerButtons()) {
        logger.warn("Floating button registration failed.");
    }

    if (!levivision::hooks::install()) {
        logger.warn("Hook install reported failure (continuing in soft mode).");
    }

    ModMenu::applyConfigToModules();

    logger.info("LeviVision enabled.");
    return true;
}

bool LeviVision::disable() {
    auto &logger = mSelf.getLogger();
    logger.info("Disabling LeviVision...");

    NightVision::disable();
    XRay::disable();
    GlowOres::disable();

    ModMenu::unregisterButtons();
    ModMenu::unregisterModules();
    levivision::hooks::uninstall();

    saveConfig();

    logger.info("LeviVision disabled.");
    return true;
}

bool LeviVision::unload() {
    mSelf.getLogger().info("Unloading LeviVision...");
    mConfigFile.reset();
    return true;
}
