#pragma once

#include <optional>

#include <pl/Config.hpp>
#include <pl/Mod.hpp>

#include "Config.h"

class LeviVision {
public:
    static LeviVision &instance();

    LeviVision();

    [[nodiscard]] pl::mod::NativeMod &getSelf() const;

    bool load();
    bool enable();
    bool disable();
    bool unload();

    /// Access typed config (valid after successful load()).
    [[nodiscard]] LeviVisionConfig &config();
    [[nodiscard]] const LeviVisionConfig &config() const;

    /// Persist current config to disk.
    bool saveConfig();

    /// Reload config from disk into memory.
    bool reloadConfig();

private:
    // Resolved lazily in load(), NOT in the constructor: PL_REGISTER_MOD may
    // materialize this singleton before the preloader has finished setting
    // the "current mod" pointer. Touching NativeMod::current() too early was
    // causing the whole game process to crash on launch.
    pl::mod::NativeMod *mSelf = nullptr;
    LeviVisionConfig mConfigValue{};
    std::optional<pl::config::ConfigFile<LeviVisionConfig>> mConfigFile;
};
