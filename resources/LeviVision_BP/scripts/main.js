import { world, system } from "@minecraft/server";

const NIGHT_VISION_DURATION_TICKS = 200;
const REFRESH_INTERVAL_TICKS = 100;

function applyNightVision() {
    for (const player of world.getAllPlayers()) {
        try {
            player.addEffect("night_vision", NIGHT_VISION_DURATION_TICKS, {
                amplifier: 0,
                showParticles: false,
            });
        } catch (e) {
            // Player not fully spawned yet; retry next interval.
        }
    }
}

system.runInterval(applyNightVision, REFRESH_INTERVAL_TICKS);
