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
    pl::mod::NativeMod &mSelf;
    LeviVisionConfig mConfigValue{};
    std::optional<pl::config::ConfigFile<LeviVisionConfig>> mConfigFile;
};
