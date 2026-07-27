#include "Hooks.h"

#include "GlowOres.h"
#include "NightVision.h"
#include "XRay.h"

#include <pl/Mod.hpp>
#include <pl/memory/Hook.hpp>
#include <pl/memory/Signature.hpp>

namespace levivision::hooks {
namespace {

bool gInstalled = false;

// Example signature placeholders — replace with real patterns for your
// exact Minecraft Bedrock build (libminecraftpe.so). Patterns change often.
//
// constexpr auto kExamplePattern = "?? ?? ?? ?? F4 4F 01 A9";
//
// void *resolveGameSymbol(std::string_view pattern) {
//     return pl::memory::resolveSignature(pattern, "libminecraftpe.so");
// }

} // namespace

bool install() {
    if (gInstalled)
        return true;

    auto *mod = pl::mod::NativeMod::current();
    auto &logger = mod ? mod->getLogger() : pl::log::Logger::getOrCreate("LeviVision");

    // Hooks are optional when using the resource-pack path for X-Ray / Glow.
    // Install native hooks here when you have stable signatures, e.g.:
    //
    // auto *addr = pl::memory::resolveSignature("...", "libminecraftpe.so");
    // if (!addr) {
    //     logger.warn("Signature not found; running without native render hooks.");
    //     gInstalled = true;
    //     return true;
    // }
    // pl::memory::hook(addr, ...);

    logger.info("Hooks: resource-pack / soft mode (no binary patches installed).");
    gInstalled = true;
    syncModules();
    return true;
}

void uninstall() {
    if (!gInstalled)
        return;

    // Unhook / restore originals here when binary hooks are used.
    gInstalled = false;

    if (auto *mod = pl::mod::NativeMod::current()) {
        mod->getLogger().info("Hooks uninstalled.");
    }
}

bool isInstalled() {
    return gInstalled;
}

void syncModules() {
    NightVision::update();
    XRay::update();
    GlowOres::update();
}

} // namespace levivision::hooks
