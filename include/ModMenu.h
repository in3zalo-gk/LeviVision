#pragma once

class ModMenu {
public:
    /// Register main module + config entries (toggles).
    static bool registerModules();

    /// Unregister main module.
    static bool unregisterModules();
};
