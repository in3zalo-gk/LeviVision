#pragma once

#include <string>
#include <string_view>

#include <pl/Config.hpp>

struct LeviVisionConfig {
    int version = 1;
    bool nightVision = false;
    bool xray = false;
};

nlohmann::json makeDefaultConfigJson();
nlohmann::json makeConfigSchemaJson();

template <>
struct pl::config::Schema<LeviVisionConfig> {
    static constexpr std::string_view title = "LeviVision Config";
    static constexpr std::string_view description =
        "Persistent settings for LeviVision (Night Vision / X-Ray).";

    static constexpr pl::config::FieldSchema field(std::string_view name) {
        if (name == "version")
            return {.title = "Version", .readOnly = true};
        if (name == "nightVision")
            return {.title = "Night Vision",
                    .description = "Preference toggle. Pick the matching variant via the "
                                   "gear icon next to RedstoneTechShader in Global Resources."};
        if (name == "xray")
            return {.title = "X-Ray",
                    .description = "Preference toggle. Pick the matching variant via the "
                                   "gear icon next to RedstoneTechShader in Global Resources."};
        return {};
    }
};
