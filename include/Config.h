#pragma once

#include <string>
#include <string_view>

#include <pl/Config.hpp>

struct LeviVisionConfig {
    int version = 1;
    bool nightVision = false;
    bool xray = false;
    bool glowOres = false;
    bool outline = true;
    int renderDistance = 128;
    int transparency = 90;
    int glowStrength = 100;
};

nlohmann::json makeDefaultConfigJson();
nlohmann::json makeConfigSchemaJson();

template <>
struct pl::config::Schema<LeviVisionConfig> {
    static constexpr std::string_view title = "LeviVision Config";
    static constexpr std::string_view description =
        "Persistent settings for LeviVision visual modules.";

    static constexpr pl::config::FieldSchema field(std::string_view name) {
        if (name == "version")
            return {.title = "Version", .readOnly = true};
        if (name == "nightVision")
            return {.title = "Night Vision",
                    .description = "Enables full brightness / night vision effect."};
        if (name == "xray")
            return {.title = "X-Ray",
                    .description = "Makes non-ore blocks transparent to reveal ores."};
        if (name == "glowOres")
            return {.title = "Glow Ores",
                    .description = "Applies emissive highlight to ore blocks."};
        if (name == "outline")
            return {.title = "Outline",
                    .description = "Draws outline around highlighted ores."};
        if (name == "renderDistance")
            return {.title = "Render Distance",
                    .description = "Block radius used by X-Ray / Glow scanning.",
                    .minimum = 16,
                    .maximum = 512};
        if (name == "transparency")
            return {.title = "Transparency",
                    .description = "Opacity of non-ore blocks when X-Ray is enabled (0-100).",
                    .minimum = 0,
                    .maximum = 100};
        if (name == "glowStrength")
            return {.title = "Glow Strength",
                    .description = "Intensity of the emissive ore highlight (0-100).",
                    .minimum = 0,
                    .maximum = 100};
        return {};
    }
};
