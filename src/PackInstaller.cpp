#include "PackInstaller.h"

#include <pl/Mod.hpp>

#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>
#include <vector>

namespace levivision::packs {

namespace {

namespace fs = std::filesystem;

constexpr std::string_view kLeviVisionRpUuid = "c4f8a21e-9b3d-4e6a-8f01-2d7c5b9e4a10";
constexpr std::string_view kLeviVisionRpVersion = "1, 1, 0";
constexpr std::string_view kUtilityHudUuid = "3971dfda-876e-4a2f-90b1-7fe2ea5346b4";
constexpr std::string_view kUtilityHudVersion = "1, 0, 0";
constexpr std::string_view kShaderUuid = "a1cf2ab9-3834-455e-8658-c10ccfb403a4";
constexpr std::string_view kShaderVersion = "1, 1, 0";

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

std::vector<fs::path> candidateComMojangDirs(const fs::path &modDir) {
    std::vector<fs::path> candidates;

    // Confirmed real path on LeviLauncher (found via device inspection):
    candidates.emplace_back(
        "/storage/emulated/0/Android/media/org.levimc.launcher/minecraft/_shared/internal/"
        "games/com.mojang");

    fs::path cursor = modDir;
    for (int i = 0; i < 8 && cursor.has_parent_path(); ++i) {
        cursor = cursor.parent_path();
        candidates.push_back(cursor / "files" / "games" / "com.mojang");
        candidates.push_back(cursor / "games" / "com.mojang");
        candidates.push_back(cursor / "com.mojang");
    }

    candidates.emplace_back(
        "/storage/emulated/0/Android/data/org.levimc.launcher/files/games/com.mojang");
    candidates.emplace_back(
        "/storage/emulated/0/Android/data/com.mojang.minecraftpe/files/games/com.mojang");
    candidates.emplace_back("/data/data/org.levimc.launcher/files/games/com.mojang");

    return candidates;
}

fs::path findComMojangDirImpl(const fs::path &modDir, pl::log::Logger &logger) {
    for (const auto &candidate : candidateComMojangDirs(modDir)) {
        std::error_code ec;
        const bool found = fs::exists(candidate / "resource_packs", ec) && !ec;
        logger.info("  checking: {} -> {}", candidate.string(), found ? "FOUND" : "no");
        if (found) {
            logger.info("Found com.mojang at: {}", candidate.string());
            return candidate;
        }
    }
    logger.warn("Could not locate the game's com.mojang folder automatically.");
    return {};
}

void registerGlobalResourcePack(const fs::path &comMojangDir, std::string_view uuid,
                                std::string_view version, std::string_view label,
                                pl::log::Logger &logger) {
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

    if (content.find(uuid) != std::string::npos) {
        logger.info("{} already registered in global_resource_packs.json", label);
        return;
    }

    const std::string entry =
        "{\"pack_id\":\"" + std::string(uuid) + "\",\"version\":[" + std::string(version) + "]}";

    std::string updated;
    auto firstBracket = content.find('[');
    if (firstBracket == std::string::npos) {
        updated = "[" + entry + "]";
    } else {
        auto lastBracket = content.rfind(']');
        if (lastBracket == std::string::npos || lastBracket < firstBracket) {
            updated = "[" + entry + "]";
        } else {
            std::string inner = content.substr(firstBracket + 1, lastBracket - firstBracket - 1);
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
    logger.info("{} registered in global_resource_packs.json", label);
}

void unregisterGlobalResourcePack(const fs::path &comMojangDir, std::string_view uuid,
                                  std::string_view label, pl::log::Logger &logger) {
    const fs::path jsonPath = comMojangDir / "minecraftpe" / "global_resource_packs.json";
    if (!fs::exists(jsonPath))
        return;

    std::ifstream in(jsonPath, std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    std::string content = ss.str();
    in.close();

    auto uuidPos = content.find(uuid);
    if (uuidPos == std::string::npos)
        return;

    auto objStart = content.rfind('{', uuidPos);
    auto objEnd = content.find('}', uuidPos);
    if (objStart == std::string::npos || objEnd == std::string::npos) {
        logger.warn("unregisterGlobalResourcePack: unexpected JSON shape, leaving file as-is");
        return;
    }

    size_t eraseStart = objStart;
    size_t eraseEnd = objEnd + 1;
    if (eraseEnd < content.size() && content[eraseEnd] == ',') {
        eraseEnd += 1;
    } else {
        while (eraseStart > 0 && std::isspace(static_cast<unsigned char>(content[eraseStart - 1])))
            --eraseStart;
        if (eraseStart > 0 && content[eraseStart - 1] == ',')
            --eraseStart;
    }

    content.erase(eraseStart, eraseEnd - eraseStart);

    std::ofstream out(jsonPath, std::ios::binary | std::ios::trunc);
    if (!out) {
        logger.warn("Could not write global_resource_packs.json (unregister)");
        return;
    }
    out << content;
    logger.info("{} removed from global_resource_packs.json", label);
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
    const fs::path hudSource = bundledResources / "UtilityHUD";
    const fs::path shaderSource = bundledResources / "RedstoneTechShader";

    const fs::path comMojang = findComMojangDirImpl(native->getModDir(), logger);
    if (comMojang.empty())
        return false;

    bool rpOk = copyPackDir(rpSource, comMojang / "resource_packs" / "LeviVision_RP", logger);
    bool bpOk = copyPackDir(bpSource, comMojang / "behavior_packs" / "LeviVision_BP", logger);
    bool hudOk = copyPackDir(hudSource, comMojang / "resource_packs" / "UtilityHUD", logger);
    bool shaderOk = copyPackDir(shaderSource, comMojang / "resource_packs" / "RedstoneTechShader",
                                logger);

    if (rpOk) {
        registerGlobalResourcePack(comMojang, kLeviVisionRpUuid, kLeviVisionRpVersion,
                                   "LeviVision RP", logger);
    }
    if (hudOk) {
        registerGlobalResourcePack(comMojang, kUtilityHudUuid, kUtilityHudVersion, "UtilityHUD",
                                   logger);
        logger.info("UtilityHUD (chunk border/hitbox) auto-installed and activated.");
    }
    if (shaderOk) {
        registerGlobalResourcePack(comMojang, kShaderUuid, kShaderVersion, "RedstoneTechShader",
                                   logger);
        logger.info("RedstoneTechShader auto-installed. Use the gear icon next to it in "
                    "Global Resources to pick X-ray / NightVision / LightOverlay variants.");
    }
    if (bpOk) {
        logger.info("LeviVision BP (Night Vision) auto-installed. Activate it once per world "
                    "you host: World Settings -> Behavior Packs -> LeviVision BP -> Activate.");
    }

    return rpOk || bpOk || hudOk || shaderOk;
}

std::filesystem::path findGameComMojangDir() {
    auto *native = pl::mod::NativeMod::current();
    if (!native)
        return {};
    return findComMojangDirImpl(native->getModDir(), native->getLogger());
}

bool setResourcePackActive(bool active) {
    auto *native = pl::mod::NativeMod::current();
    if (!native)
        return false;

    auto &logger = native->getLogger();
    const fs::path comMojang = findComMojangDirImpl(native->getModDir(), logger);
    if (comMojang.empty())
        return false;

    if (active) {
        registerGlobalResourcePack(comMojang, kLeviVisionRpUuid, kLeviVisionRpVersion,
                                   "LeviVision RP", logger);
    } else {
        unregisterGlobalResourcePack(comMojang, kLeviVisionRpUuid, "LeviVision RP", logger);
    }
    return true;
}

} // namespace levivision::packs
