#include "GameSettings.h"

#include "PackInstaller.h"

#include <pl/Mod.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace levivision::gamesettings {

namespace {

namespace fs = std::filesystem;

// Absurdly high on purpose: turns caves/night fully bright, well beyond
// what the in-game slider allows (normally clamped to ~1.0).
constexpr const char *kExtremeGammaValue = "1000000.0";
constexpr const char *kNormalGammaValue = "1.0";
// Confirmed real Bedrock options.txt key (NOT "gamma:", which is the
// Java Edition key name).
constexpr const char *kGammaKey = "gfx_gamma:";

} // namespace

bool setExtremeGamma(bool enabled) {
    auto *native = pl::mod::NativeMod::current();
    if (!native)
        return false;

    auto &logger = native->getLogger();
    const fs::path comMojang = levivision::packs::findGameComMojangDir();
    if (comMojang.empty()) {
        logger.warn("setExtremeGamma: com.mojang folder not found, cannot edit options.txt");
        return false;
    }

    const fs::path optionsPath = comMojang / "minecraftpe" / "options.txt";

    std::error_code ec;
    fs::create_directories(optionsPath.parent_path(), ec);

    std::string content;
    if (fs::exists(optionsPath)) {
        std::ifstream in(optionsPath, std::ios::binary);
        std::ostringstream ss;
        ss << in.rdbuf();
        content = ss.str();
    }

    const std::string targetValue = enabled ? kExtremeGammaValue : kNormalGammaValue;
    const std::string newLine = std::string(kGammaKey) + targetValue;

    // Replace an existing "gfx_gamma:<value>" line, or append one.
    std::istringstream lines(content);
    std::ostringstream rebuilt;
    std::string line;
    bool replaced = false;
    while (std::getline(lines, line)) {
        if (line.rfind(kGammaKey, 0) == 0) {
            rebuilt << newLine << "\n";
            replaced = true;
        } else {
            rebuilt << line << "\n";
        }
    }
    if (!replaced) {
        rebuilt << newLine << "\n";
    }

    std::ofstream out(optionsPath, std::ios::binary | std::ios::trunc);
    if (!out) {
        logger.error("setExtremeGamma: could not write {}", optionsPath.string());
        return false;
    }
    out << rebuilt.str();

    logger.info("Gamma set to {} in {}", targetValue, optionsPath.string());
    logger.info("Fully close and reopen Minecraft for this to take effect "
                "(options.txt is only read at startup).");
    return true;
}

} // namespace levivision::gamesettings
