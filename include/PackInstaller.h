#pragma once

#include <filesystem>

namespace levivision::packs {

/// Copies the bundled LeviVision_RP / LeviVision_BP folders (shipped inside
/// the mod's own resources/ directory, see getResourceDir()) into the
/// running game's com.mojang resource_packs / behavior_packs folders, and
/// activates the resource pack globally when possible.
///
/// This never touches game memory or the game process — it only performs
/// filesystem copies, so a failure here (e.g. the game folder isn't found)
/// can only mean "the packs aren't auto-installed", never a crash.
///
/// Returns true if at least the resource pack (X-Ray / Glow Ores) was
/// installed successfully. The behavior pack (Night Vision) still requires
/// a one-time manual activation per world (Global Resources only applies
/// automatically to resource packs; Mojang requires explicit per-world
/// opt-in for behavior packs).
bool installBundledPacks();

/// Activates or deactivates the LeviVision resource pack (X-Ray/Glow Ores
/// textures) in global_resource_packs.json. Call with true when either
/// X-Ray or Glow Ores is enabled, false when both are off.
bool setResourcePackActive(bool active);

/// Locates the running game's com.mojang folder (contains resource_packs/,
/// minecraftpe/options.txt, etc). Returns an empty path if it could not be
/// found. Shared with GameSettings.cpp so both use identical detection logic.
std::filesystem::path findGameComMojangDir();

} // namespace levivision::packs
