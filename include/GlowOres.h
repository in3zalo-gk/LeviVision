#pragma once

class GlowOres {
public:
    static void enable();
    static void disable();
    static void setEnabled(bool enabled);
    static void update();
    static bool isEnabled();

    static void setStrength(int percent);
    static void setRenderDistance(int blocks);
    static void setOutline(bool enabled);

    static int strength();
    static int renderDistance();
    static bool outlineEnabled();

    static void applyFromConfig(bool enabled, int strength, int renderDistance, bool outline);

private:
    static bool mEnabled;
    static bool mOutline;
    static int mStrength;
    static int mRenderDistance;
};
