#pragma once

class ModMenu {
public:
    /// Register main module + config entries (toggles / sliders).
    static bool registerModules();

    /// Unregister main module.
    static bool unregisterModules();

    /// Register floating "LV" button.
    static bool registerButtons();

    /// Unregister floating button.
    static bool unregisterButtons();

    /// Push current config values into runtime modules (no UI).
    static void applyConfigToModules();
};
