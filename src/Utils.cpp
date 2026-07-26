#include "Utils.h"

#include <cctype>
#include <charconv>
#include <string>

namespace levivision::utils {

bool parseBool(std::string_view value, bool fallback) {
    if (value.empty())
        return fallback;

    std::string lower;
    lower.reserve(value.size());
    for (char c : value)
        lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));

    if (lower == "true" || lower == "1" || lower == "yes" || lower == "on")
        return true;
    if (lower == "false" || lower == "0" || lower == "no" || lower == "off")
        return false;
    return fallback;
}

int parseInt(std::string_view value, int fallback) {
    if (value.empty())
        return fallback;

    int result = fallback;
    const auto *begin = value.data();
    const auto *end = value.data() + value.size();
    auto [ptr, ec] = std::from_chars(begin, end, result);
    if (ec != std::errc{} || ptr != end)
        return fallback;
    return result;
}

int clampInt(int value, int minValue, int maxValue) {
    if (value < minValue)
        return minValue;
    if (value > maxValue)
        return maxValue;
    return value;
}

std::string boolToString(bool value) {
    return value ? "true" : "false";
}

std::string intToString(int value) {
    return std::to_string(value);
}

} // namespace levivision::utils
