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

constexpr std::string_view kXRayUuid = "b5178998-b0b1-4f5e-ad7d-59f6c93379c0";
constexpr std::string_view kXRayVersion = "1, 1, 0";
constexpr const char *kXRayFolder = "LeviVision_XRay";

constexpr std::string_view kNightVisionUuid = "d8f4db04-bcd9-4f13-9871-4a9b94f370a6";
constexpr std::string_view kNightVisionVersion = "1, 1, 0";
constexpr const char *kNightVisionFolder = "LeviVision_NightVision";

std::vector<fs::path> candidateComMojangDirs(const fs::path &modDir) {
    std::vector<fs::path> candidates;

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

fs::path findComMojangDir(const fs::path &modDir, pl::log::Logger &logger) {
    for (const auto &candidate : candidateComMojangDirs(modDir)) {
        std::error_code ec;
        if (fs::exists(candidate / "resource_packs", ec) && !ec) {
            logger.info("Found com.mojang at: {}", candidate.string());
            return candidate;
        }
    }
    logger.warn("Could not locate the game's com.mojang folder automatically.");
    return {};
}

bool copyPackIfMissing(const fs::path &source, const fs::path &dest, pl::log::Logger &logger) {
    std::error_code ec;
    if (fs::exists(dest / "manifest.json", ec)) {
        logger.info("{} already installed, skipping copy.", dest.string());
        return true;
    }
    if (!fs::exists(source, ec) || ec) {
        logger.warn("Bundled pack source missing: {}", source.string());
        return false;
    }
    fs::create_directories(dest, ec);
    if (ec) {
        logger.error("Could not create {}: {}", dest.string(), ec.message());
        return false;
    }
    fs::copy(source, dest, fs::copy_options::recursive | fs::copy_options::overwrite_existing,
              ec);
    if (ec) {
        logger.error("Copy failed {} -> {}: {}", source.string(), dest.string(), ec.message());
        return false;
    }
    logger.info("Installed {} -> {}", source.string(), dest.string());
    return true;
}

void registerPack(const fs::path &comMojangDir, std::string_view uuid, std::string_view version,
                  pl::log::Logger &logger) {
    const fs::path jsonPath = comMojangDir / "minecraftpe" / "global_resource_packs.json";

    std::error_code ec;
    fs::create_directories(jsonPath.parent_path(), ec);
    if (ec)
        return;

    std::string content;
    if (fs::exists(jsonPath)) {
        std::ifstream in(jsonPath, std::ios::binary);
        std::ostringstream ss;
        ss << in.rdbuf();
        content = ss.str();
    }

    if (content.find(uuid) != std::string::npos)
        return;

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
            updated = trimmed.empty()
                          ? content.substr(0, firstBracket + 1) + entry + content.substr(lastBracket)
                          : content.substr(0, lastBracket) + "," + entry + content.substr(lastBracket);
        }
    }

    std::ofstream out(jsonPath, std::ios::binary | std::ios::trunc);
    if (!out)
        return;
    out << updated;
    logger.info("Registered {} in global_resource_packs.json", uuid);
}

void unregisterPack(const fs::path &comMojangDir, std::string_view uuid,
                    pl::log::Logger &logger) {
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
    if (objStart == std::string::npos || objEnd == std::string::npos)
        return;

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
    if (!out)
        return;
    out << content;
    logger.info("Unregistered {} from global_resource_packs.json", uuid);
}

} // namespace

bool installBundledPacks() {
    auto *native = pl::mod::NativeMod::current();
    if (!native)
        return false;

    auto &logger = native->getLogger();
    const fs::path comMojang = findComMojangDir(native->getModDir(), logger);
    if (comMojang.empty())
        return false;

    const fs::path bundled = native->getResourceDir();
    bool a = copyPackIfMissing(bundled / kXRayFolder, comMojang / "resource_packs" / kXRayFolder,
                               logger);
    bool b = copyPackIfMissing(bundled / kNightVisionFolder,
                               comMojang / "resource_packs" / kNightVisionFolder, logger);
    return a || b;
}

bool setActiveVariant(Variant variant) {
    auto *native = pl::mod::NativeMod::current();
    if (!native)
        return false;

    auto &logger = native->getLogger();
    const fs::path comMojang = findComMojangDir(native->getModDir(), logger);
    if (comMojang.empty())
        return false;

    if (variant == Variant::XRay) {
        registerPack(comMojang, kXRayUuid, kXRayVersion, logger);
        unregisterPack(comMojang, kNightVisionUuid, logger);
    } else if (variant == Variant::NightVision) {
        unregisterPack(comMojang, kXRayUuid, logger);
        registerPack(comMojang, kNightVisionUuid, kNightVisionVersion, logger);
    } else {
        unregisterPack(comMojang, kXRayUuid, logger);
        unregisterPack(comMojang, kNightVisionUuid, logger);
    }
    return true;
}

} // namespace levivision::packs
