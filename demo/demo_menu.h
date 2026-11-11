// demo_menu.h - sample menu built on fluent.h. UI only: every value here is a placeholder
// that you replace with your existing menu variables; no feature logic is implemented.
#pragma once
#include "fluent.h"

namespace Demo {
using namespace Fluent;

struct State {
    int tab = 0;
    // Visuals / ESP
    bool esp = true, boxes = true, skeleton = true, lines = false, names = true, health = true, distance = false;
    int boxStyle = 0;
    float boxCol[4] = { 0.30f, 0.76f, 1.00f, 1.00f }, skelCol[4] = { 1.00f, 1.00f, 1.00f, 0.85f }, lineCol[4] = { 1.00f, 0.35f, 0.35f, 0.80f };
    float thickness = 1.5f; int maxDist = 300;
    bool layers[3] = { true, true, false };
    // Aim
    bool aim = false, showFov = true;
    int aimKey = 0, bone = 0, priority = 0;
    float fov = 90.f, smooth = 5.f; float fovCol[4] = { 1.f, 1.f, 1.f, 0.6f };
    bool targets[3] = { true, true, false };
    // Memory
    bool driverLoaded = true; int valueA = 100; float valueB = 1.0f; char addr[32] = "";
    // Settings
    int lang = 0, theme = 0; float uiScale = 1.f, pendingScale = 1.f; bool scaleDirty = false;
    bool sound = true, showFps = true, reduceMotion = false;
    char cfgName[32] = "default";
};

inline void Render(State& s, bool* open) {
    ImGuiIO& io = ImGui::GetIO();
    SetTheme(s.theme == 1 ? Theme::Light : Theme::Dark);
    SetReduceMotion(s.reduceMotion);

    // Window takes 58% x 66% of the screen (always kept inside 30..70%), centred on first show.
    ImVec2 size = WindowSize(0.58f, 0.66f);
    if (BeginWindow("Fluent Menu", open, CenterPos(size), size)) {
        BeginNav();
        NavItem(Icon::Eye, "Visuals", &s.tab, 0);
        NavItem(Icon::Target, "Aim", &s.tab, 1);
        NavItem(Icon::Chip, "Memory", &s.tab, 2);
        NavItem(Icon::Gear, "Settings", &s.tab, 3);
        EndNav();
        ImGui::SameLine(0, 0);

        BeginContent();
        BeginPage(s.tab);
        static const char* boxStyles[] = { "2D box", "Corner box", "Filled" };
        static const char* bones[] = { "Head", "Neck", "Chest", "Pelvis" };
        static const char* prios[] = { "Closest", "Lowest health", "Crosshair" };
        static const char* layerNames[] = { "Enemies", "Teammates", "Bots" };
        static const char* langs[] = { "English", "\xD0\xA0\xD1\x83\xD1\x81\xD1\x81\xD0\xBA\xD0\xB8\xD0\xB9", "Deutsch" };
        static const char* themes[] = { "Dark", "Light" };

        if (s.tab == 0) {
            PageTitle("Visuals / ESP");
            ToggleCard(Icon::Eye, "Enable ESP", "Master switch for all overlays", &s.esp);
            GroupHeader("Elements");
            if (BeginSection(Icon::Eye, "Players", "Boxes, skeleton, lines, labels", true)) {
                ToggleCard(Icon::None, "Boxes", "", &s.boxes);
                ComboCard(Icon::None, "Box style", "", &s.boxStyle, boxStyles, 3);
                ToggleCard(Icon::None, "Skeleton", "", &s.skeleton);
                ToggleCard(Icon::None, "Snap lines", "", &s.lines);
                ToggleCard(Icon::None, "Names", "", &s.names);
                ToggleCard(Icon::None, "Health", "", &s.health);
                ToggleCard(Icon::None, "Distance", "", &s.distance);
                EndSection();
            }
            MultiComboCard(Icon::Info, "Show for", "Which groups are drawn", layerNames, s.layers, 3);
            GroupHeader("Colors");
            ColorCard(Icon::Sun, "Box color", "With transparency", s.boxCol);
            ColorCard(Icon::Sun, "Skeleton color", "", s.skelCol);
            ColorCard(Icon::Sun, "Line color", "", s.lineCol);
            GroupHeader("Rendering");
            SliderFloatCard(Icon::Gear, "Line thickness", "In pixels", &s.thickness, 0.5f, 5.f, "%.1f");
            SliderIntCard(Icon::Gear, "Max distance", "Farther targets are hidden", &s.maxDist, 50, 1000);
        } else if (s.tab == 1) {
            PageTitle("Aim");
            ToggleCard(Icon::Target, "Enable aim assist", "Master switch", &s.aim);
            KeyBindCard(Icon::Key, "Hotkey", "Hold to activate", &s.aimKey);
            GroupHeader("Targeting");
            SliderFloatCard(Icon::Target, "FOV", "Degrees", &s.fov, 1.f, 180.f, "%.0f");
            SliderFloatCard(Icon::Target, "Smoothing", "Higher is smoother", &s.smooth, 1.f, 20.f, "%.1f");
            ComboCard(Icon::Target, "Target bone", "", &s.bone, bones, 4);
            ComboCard(Icon::Target, "Priority", "How the target is chosen", &s.priority, prios, 3);
            MultiComboCard(Icon::Info, "Target groups", "", layerNames, s.targets, 3);
            GroupHeader("FOV circle");
            ToggleCard(Icon::Eye, "Show FOV circle", "Draws the aim radius", &s.showFov);
            ColorCard(Icon::Sun, "Circle color", "", s.fovCol);
        } else if (s.tab == 2) {
            PageTitle("Memory");
            ValueCard(Icon::Chip, "Driver status", "Kernel driver connection", s.driverLoaded ? "Connected" : "Not loaded", s.driverLoaded);
            if (ButtonCard(Icon::Chip, "Driver", "Load or unload", s.driverLoaded ? "Unload" : "Load", s.driverLoaded ? Kind::Danger : Kind::Accent)) {
                s.driverLoaded = !s.driverLoaded;
                PushToast(s.driverLoaded ? "Driver loaded" : "Driver unloaded", s.driverLoaded ? ToastType::Success : ToastType::Info);
            }
            GroupHeader("Values");
            SliderIntCard(Icon::Gear, "Integer value", "Tap the number to type", &s.valueA, 0, 1000);
            SliderFloatCard(Icon::Gear, "Float value", "", &s.valueB, 0.f, 10.f, "%.2f");
            InputCard(Icon::Key, "Address", "Hex address", s.addr, sizeof(s.addr), "0x0000000");
        } else {
            PageTitle("Settings");
            GroupHeader("Appearance");
            ComboCard(Icon::Globe, "Language", "Interface language", &s.lang, langs, 3);
            ComboCard(Icon::Sun, "Theme", "Smooth transition", &s.theme, themes, 2);
            if (SliderFloatCard(Icon::Gear, "UI scale", "Applied on release", &s.pendingScale, 0.75f, 1.75f, "%.2f")) s.scaleDirty = true;
            if (s.scaleDirty && !ImGui::IsMouseDown(0)) { s.uiScale = s.pendingScale; SetScale(s.uiScale); s.scaleDirty = false; }
            ToggleCard(Icon::Gear, "Reduce animations", "Turns off motion effects", &s.reduceMotion);
            GroupHeader("General");
            ToggleCard(Icon::Speaker, "Sound", "Interface sounds", &s.sound);
            ToggleCard(Icon::Gear, "Show FPS", "Counter in the corner", &s.showFps);
            if (ButtonCard(Icon::Eye, "Screenshot", "Save the current frame", "Capture")) PushToast("Screenshot saved", ToastType::Success);
            GroupHeader("Configs");
            InputCard(Icon::Info, "Config name", "Used when saving or loading", s.cfgName, sizeof(s.cfgName), "name");
            ImGui::Dummy(ImVec2(0, Px(2)));
            if (Button("Save", Kind::Accent)) PushToast("Config saved", ToastType::Success);
            Tooltip("Write the current settings to the named config");
            ImGui::SameLine();
            if (Button("Load")) PushToast("Config loaded");
            ImGui::SameLine();
            if (Button("Reset", Kind::Danger)) PushToast("Settings reset to defaults", ToastType::Error);
            Spacer(16);
        }
        EndPage();
        EndContent();
    }
    EndWindow();

    if (s.showFps) {
        char b[32]; snprintf(b, sizeof(b), "%.0f FPS", io.Framerate);
        ImGui::GetForegroundDrawList()->AddText(Ctx().fontSmall, FontPx(Ctx().fontSmall), ImVec2(io.DisplaySize.x - Px(56), Px(4)), U32(Pal().textSecondary), b);
    }
    RenderToasts();
}

} // namespace Demo
