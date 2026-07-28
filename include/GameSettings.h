#pragma once

namespace levivision::gamesettings {

/// Writes an extreme gamma value into the game's options.txt, unlocking
/// brightness far beyond the in-game slider's normal maximum. This is a
/// plain text-file edit (com.mojang/minecraftpe/options.txt) - no game
/// memory is touched, so it cannot crash the game, and it works on any
/// server since brightness/gamma is a purely client-side render setting.
bool setExtremeGamma(bool enabled);

} // namespace levivision::gamesettings
