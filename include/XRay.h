#pragma once

class XRay {
public:
    static void enable();
    static void disable();
    static void setEnabled(bool enabled);
    static void update();
    static bool isEnabled();

    static void setTransparency(int percent);
    static void setRenderDistance(int blocks);
    static void setOutline(bool enabled);

    static int transparency();
    static int renderDistance();
    static bool outlineEnabled();

    static void applyFromConfig(bool enabled, int transparency, int renderDistance,
                                bool outline);

private:
    static bool mEnabled;
    static bool mOutline;
    static int mTransparency;
    static int mRenderDistance;
};
