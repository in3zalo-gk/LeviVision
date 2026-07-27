#pragma once

#include <string>
#include <string_view>

namespace levivision::utils {

/// Parse bool from Mod Menu string values ("true" / "false" / "1" / "0").
bool parseBool(std::string_view value, bool fallback = false);

/// Parse integer from Mod Menu string values.
int parseInt(std::string_view value, int fallback = 0);

/// Clamp integer into [min, max].
int clampInt(int value, int minValue, int maxValue);

/// Convert bool to Mod Menu default string.
std::string boolToString(bool value);

/// Convert int to string.
std::string intToString(int value);

} // namespace levivision::utils
