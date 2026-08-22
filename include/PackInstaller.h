#pragma once

#include <filesystem>

namespace levivision::packs {

bool installBundledPacks();

bool setResourcePackActive(bool active);

std::filesystem::path findGameComMojangDir();

} // namespace levivision::packs
