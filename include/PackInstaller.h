#pragma once

namespace levivision::packs {

/// Copies both bundled packs (LeviVision_XRay, LeviVision_NightVision) into
/// the game's resource_packs folder, ONLY on first run (skips if already
/// installed, to avoid blocking the main thread with heavy file I/O on
/// every launch - this is what caused an ANR before). Does NOT activate
/// either of them.
bool installBundledPacks();

enum class Variant { None, XRay, NightVision };

/// Activates exactly one variant (or none) in global_resource_packs.json,
/// deactivating the other. This is real, code-driven switching - no manual
/// gear-icon step needed.
bool setActiveVariant(Variant variant);

} // namespace levivision::packs
