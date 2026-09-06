#include "LeviVision.h"

#include "ModMenu.h"
#include "PackInstaller.h"

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
        pl::log::Logger::getOrCreate("LeviVision")
            .error("NativeMod::current() was null during load(); aborting mod load safely.");
        return false;
    }

    auto &logger = mSelf->getLogger();

    logger.info("====================================");
    logger.info("  LeviVision v2.0.0 (X-Ray + Night Vision)");
    logger.info("  Author: Say");
    logger.info("====================================");

    std::error_code ec;
    std::filesystem::create_directories(mSelf->getDataDir(), ec);
    if (ec) {
        logger.error("Failed to create data dir: {}", ec.message());
        return false;
    }

    std::filesystem::create_directories(mSelf->getConfigDir(), ec);
    if (ec) {
        logger.error("Failed to create config dir: {}", ec.message());
        return false;
    }

    mConfigFile.emplace();

    if (!mConfigFile->load()) {
        logger.warn("Config load/save failed; using in-memory defaults.");
        mConfigValue = LeviVisionConfig{};
    } else {
        mConfigValue = mConfigFile->value();
    }

    if (mConfigValue.version < 1)
        mConfigValue.version = 1;

    logger.info("Config loaded (NV={} XRay={})", mConfigValue.nightVision, mConfigValue.xray);
    return true;
}

bool LeviVision::enable() {
    if (mSelf == nullptr) {
        pl::log::Logger::getOrCreate("LeviVision").error("enable() called before load().");
        return false;
    }
    auto &logger = mSelf->getLogger();
    logger.info("Enabling LeviVision...");

    if (!ModMenu::registerModules()) {
        logger.warn("Mod Menu module registration failed.");
    }

    // NOTE: floating button intentionally NOT registered here.
    // pl::modmenu::registerButton() caused a confirmed SIGSEGV crash
    // (verified via xCrash tombstone). Do not re-enable without a fresh
    // crash log confirming it is safe.

    if (!levivision::packs::installBundledPacks()) {
        logger.warn("Could not auto-install the bundled shader pack; "
                    "see previous log lines for details.");
    }

    logger.info("LeviVision enabled. Open Global Resources and tap the gear icon next to "
                "RedstoneTechShader to pick X-Ray or Night Vision.");
    return true;
}

bool LeviVision::disable() {
    if (mSelf == nullptr)
        return true;
    mSelf->getLogger().info("Disabling LeviVision...");
    ModMenu::unregisterModules();
    saveConfig();
    return true;
}

bool LeviVision::unload() {
    if (mSelf != nullptr) {
        mSelf->getLogger().info("Unloading LeviVision...");
    }
    mConfigFile.reset();
    return true;
}
