#pragma once

/// Central place for memory hooks / signatures used by visual modules.
/// Implementations are version-sensitive (libminecraftpe.so).
namespace levivision::hooks {

/// Install all hooks required by currently enabled modules.
bool install();

/// Remove all installed hooks.
void uninstall();

/// Whether hooks are currently active.
bool isInstalled();

/// Re-apply module state after hooks are installed (e.g. after enable).
void syncModules();

} // namespace levivision::hooks
