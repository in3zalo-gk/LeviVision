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

LeviVision::LeviVision()
    : mSelf([]() -> pl::mod::NativeMod & {
          auto *current = pl::mod::NativeMod::current();
          // Preloader must set current() before PL_REGISTER_MOD materializes the instance.
          // Dereferencing null here would abort the whole game process.
          if (current == nullptr) {
              // Fall back is impossible for a reference member; log via android if needed.
              // Returning a dangling ref is worse — abort early with a clear path.
              __builtin_trap();
          }
          return *current;
      }()) {}

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

    // Use explicit paths so ConfigFile does not depend on defaultConfigPath()
    // resolving NativeMod::current() at an unexpected time (empty path => load fails).
    mConfigFile.emplace(
        LeviVisionConfig{},
        mSelf.getConfigDir() / "config.json",
        mSelf.getConfigDir() / "config.schema.json");

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
