// LeviVision - Night Vision via official Script API.
// Runs only in worlds this device hosts (singleplayer or a server you own).
// Does not use cheats/commands, so it never disables achievements.
import { world, system } from "@minecraft/server";

const NIGHT_VISION_DURATION_TICKS = 200; // 10s; refreshed continuously below
const REFRESH_INTERVAL_TICKS = 100; // every 5s

function applyNightVision() {
    for (const player of world.getAllPlayers()) {
        try {
            player.addEffect("night_vision", NIGHT_VISION_DURATION_TICKS, {
                amplifier: 0,
                showParticles: false,
            });
        } catch (e) {
            // Player may not be fully spawned yet; ignore and retry next interval.
        }
    }
}

system.runInterval(applyNightVision, REFRESH_INTERVAL_TICKS);
