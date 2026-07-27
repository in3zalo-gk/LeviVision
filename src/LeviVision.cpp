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

LeviVision::LeviVision() = default;

pl::mod::NativeMod &LeviVision::getSelf() const {
    return *mSelf;
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
        mSelf->getLogger().error("Failed to save config.json");
        return false;
    }
    return true;
}

bool LeviVision::reloadConfig() {
    if (!mConfigFile)
        return false;

    if (!mConfigFile->load()) {
        mSelf->getLogger().error("Failed to reload config.json");
        return false;
    }

    mConfigValue = mConfigFile->value();
    return true;
}

bool LeviVision::load() {
    mSelf = pl::mod::NativeMod::current();
    if (mSelf == nullptr) {
        // Never abort the host process — log through a standalone logger and
        // bail out of load() gracefully. The preloader can retry/report this
        // as a failed mod load instead of taking the whole game down.
        pl::log::Logger::getOrCreate("LeviVision")
            .error("NativeMod::current() was null during load(); aborting mod load safely.");
        return false;
    }

    auto &logger = mSelf->getLogger();

    logger.info("====================================");
    logger.info("  LeviVision v1.0.0");
    logger.info("  Author: Say");
    logger.info("====================================");

    std::error_code ec;
    std::filesystem::create_directories(mSelf->getDataDir(), ec);
    if (ec) {
        logger.error("Failed to create data dir {}: {}", mSelf->getDataDir().string(),
                     ec.message());
        return false;
    }

    std::filesystem::create_directories(mSelf->getConfigDir(), ec);
    if (ec) {
        logger.error("Failed to create config dir {}: {}", mSelf->getConfigDir().string(),
                     ec.message());
        return false;
    }

    mConfigFile.emplace();

    if (!mConfigFile->load()) {
        // Do not abort mod load: keep in-memory defaults and continue.
        // Returning false here can make the preloader treat the mod as failed
        // and may destabilize startup depending on launcher version.
        logger.warn("Typed config load/save failed; using in-memory defaults.");
        mConfigValue = LeviVisionConfig{};
    } else {
        mConfigValue = mConfigFile->value();
    }

    if (mConfigValue.version < 1)
        mConfigValue.version = 1;

    logger.info("Config loaded (NV={} XRay={} Glow={} Outline={} RD={} TP={} GS={})",
                mConfigValue.nightVision, mConfigValue.xray, mConfigValue.glowOres,
                mConfigValue.outline, mConfigValue.renderDistance, mConfigValue.transparency,
                mConfigValue.glowStrength);

    return true;
}

bool LeviVision::enable() {
    if (mSelf == nullptr) {
        pl::log::Logger::getOrCreate("LeviVision").error("enable() called before a successful load().");
        return false;
    }
    auto &logger = mSelf->getLogger();
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
    if (mSelf == nullptr)
        return true;
    auto &logger = mSelf->getLogger();
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
    if (mSelf != nullptr) {
        mSelf->getLogger().info("Unloading LeviVision...");
    }
    mConfigFile.reset();
    return true;
}
