#include "PackInstaller.h"

#include <pl/Mod.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>
#include <vector>

namespace levivision::packs {

namespace {

namespace fs = std::filesystem;

constexpr std::string_view kRpUuid = "c4f8a21e-9b3d-4e6a-8f01-2d7c5b9e4a10";
constexpr std::string_view kRpVersion = "1, 1, 0";

// Recursively copy `from` into `to`, overwriting existing files.
bool copyPackDir(const fs::path &from, const fs::path &to, pl::log::Logger &logger) {
    std::error_code ec;
    if (!fs::exists(from, ec) || ec) {
        logger.warn("Pack source missing: {}", from.string());
        return false;
    }

    fs::create_directories(to, ec);
    if (ec) {
        logger.error("Could not create {}: {}", to.string(), ec.message());
        return false;
    }

    fs::copy(from, to,
              fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
    if (ec) {
        logger.error("Copy failed {} -> {}: {}", from.string(), to.string(), ec.message());
        return false;
    }
    return true;
}

// Walk upward from the mod's own install directory looking for a
// "com.mojang" folder that already contains resource_packs/. This works
// regardless of exactly how the launcher lays out its app-data directory,
// since it's anchored to the mod's own (documented) getModDir().
std::vector<fs::path> candidateComMojangDirs(const fs::path &modDir) {
    std::vector<fs::path> candidates;

    fs::path cursor = modDir;
    for (int i = 0; i < 8 && cursor.has_parent_path(); ++i) {
        cursor = cursor.parent_path();
        candidates.push_back(cursor / "files" / "games" / "com.mojang");
        candidates.push_back(cursor / "games" / "com.mojang");
        candidates.push_back(cursor / "com.mojang");
    }

    // Common Android external-storage layouts, tried as a fallback.
    candidates.emplace_back(
        "/storage/emulated/0/Android/data/org.levimc.launcher/files/games/com.mojang");
    candidates.emplace_back(
        "/storage/emulated/0/Android/data/com.mojang.minecraftpe/files/games/com.mojang");
    candidates.emplace_back("/data/data/org.levimc.launcher/files/games/com.mojang");

    return candidates;
}

std::filesystem::path findComMojangDirImpl(const fs::path &modDir, pl::log::Logger &logger) {
    for (const auto &candidate : candidateComMojangDirs(modDir)) {
        std::error_code ec;
        const bool found = fs::exists(candidate / "resource_packs", ec) && !ec;
        logger.info("  checking: {} -> {}", candidate.string(), found ? "FOUND" : "no");
        if (found) {
            logger.info("Found com.mojang at: {}", candidate.string());
            return candidate;
        }
    }
    logger.warn("Could not locate the game's com.mojang folder automatically. "
                "Packs were not auto-installed; import the .mcpack files manually.");
    return {};
}

// Best-effort: add our resource pack's uuid/version to global_resource_packs.json
// so it is active without the user opening Global Resources manually.
// This is plain text manipulation (no JSON dependency) and is safe to skip
// on any anomaly - worst case the user just activates the pack by hand once.
void registerGlobalResourcePack(const fs::path &comMojangDir, pl::log::Logger &logger) {
    const fs::path jsonPath = comMojangDir / "minecraftpe" / "global_resource_packs.json";

    std::error_code ec;
    fs::create_directories(jsonPath.parent_path(), ec);
    if (ec) {
        logger.warn("Could not prepare minecraftpe/ dir for global_resource_packs.json");
        return;
    }

    std::string content;
    if (fs::exists(jsonPath)) {
        std::ifstream in(jsonPath, std::ios::binary);
        std::ostringstream ss;
        ss << in.rdbuf();
        content = ss.str();
    }

    if (content.find(kRpUuid) != std::string::npos) {
        logger.info("LeviVision RP already registered in global_resource_packs.json");
        return;
    }

    const std::string entry = "{\"pack_id\":\"" + std::string(kRpUuid) + "\",\"version\":[" +
                               std::string(kRpVersion) + "]}";

    std::string updated;
    auto firstBracket = content.find('[');
    if (firstBracket == std::string::npos) {
        // Missing or empty file: create a fresh array.
        updated = "[" + entry + "]";
    } else {
        auto lastBracket = content.rfind(']');
        if (lastBracket == std::string::npos || lastBracket < firstBracket) {
            updated = "[" + entry + "]";
        } else {
            std::string inner = content.substr(firstBracket + 1, lastBracket - firstBracket - 1);
            // Trim whitespace to check if array is empty.
            auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };
            std::string trimmed = inner;
            while (!trimmed.empty() && isSpace(static_cast<unsigned char>(trimmed.front())))
                trimmed.erase(trimmed.begin());
            while (!trimmed.empty() && isSpace(static_cast<unsigned char>(trimmed.back())))
                trimmed.pop_back();

            if (trimmed.empty()) {
                updated = content.substr(0, firstBracket + 1) + entry +
                          content.substr(lastBracket);
            } else {
                updated = content.substr(0, lastBracket) + "," + entry +
                          content.substr(lastBracket);
            }
        }
    }

    std::ofstream out(jsonPath, std::ios::binary | std::ios::trunc);
    if (!out) {
        logger.warn("Could not write global_resource_packs.json");
        return;
    }
    out << updated;
    logger.info("LeviVision RP registered in global_resource_packs.json");
}

} // namespace

bool installBundledPacks() {
    auto *native = pl::mod::NativeMod::current();
    if (!native)
        return false;

    auto &logger = native->getLogger();
    const fs::path bundledResources = native->getResourceDir();
    const fs::path rpSource = bundledResources / "LeviVision_RP";
    const fs::path bpSource = bundledResources / "LeviVision_BP";

    const fs::path comMojang = findComMojangDirImpl(native->getModDir(), logger);
    if (comMojang.empty())
        return false;

    bool rpOk = copyPackDir(rpSource, comMojang / "resource_packs" / "LeviVision_RP", logger);
    bool bpOk = copyPackDir(bpSource, comMojang / "behavior_packs" / "LeviVision_BP", logger);

    if (rpOk) {
        registerGlobalResourcePack(comMojang, logger);
        logger.info("LeviVision RP (X-Ray/Glow) auto-installed and activated globally.");
    }
    if (bpOk) {
        logger.info("LeviVision BP (Night Vision) auto-installed. Activate it once per world "
                    "you host: World Settings -> Behavior Packs -> LeviVision BP -> Activate.");
    }

    return rpOk || bpOk;
}

std::filesystem::path findGameComMojangDir() {
    auto *native = pl::mod::NativeMod::current();
    if (!native)
        return {};
    return findComMojangDirImpl(native->getModDir(), native->getLogger());
}

} // namespace levivision::packs
