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

    float m = Px(10);
    ImVec2 size(io.DisplaySize.x - 2 * m, io.DisplaySize.y - 2 * m);
    if (BeginWindow("Fluent Menu", open, ImVec2(m, m), size)) {
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
            ToggleCard(Icon::Eye, "Enable ESP", "Master switch for every overlay element", &s.esp);
            GroupHeader("Elements");
            if (BeginSection(Icon::Eye, "Players", "Boxes, skeleton, lines and info labels", true)) {
                ToggleCard(Icon::None, "Boxes", "Bounding box around each target", &s.boxes);
                ComboCard(Icon::None, "Box style", "Shape of the bounding box", &s.boxStyle, boxStyles, 3);
                ToggleCard(Icon::None, "Skeleton", "Bone lines", &s.skeleton);
                ToggleCard(Icon::None, "Snap lines", "Line from screen edge to target", &s.lines);
                ToggleCard(Icon::None, "Names", "", &s.names);
                ToggleCard(Icon::None, "Health", "Health bar next to the box", &s.health);
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
            SliderIntCard(Icon::Gear, "Max distance", "Targets farther than this are hidden", &s.maxDist, 50, 1000);
        } else if (s.tab == 1) {
            PageTitle("Aim");
            ToggleCard(Icon::Target, "Enable aim assist", "Master switch", &s.aim);
            KeyBindCard(Icon::Key, "Hotkey", "Hold to activate", &s.aimKey);
            GroupHeader("Targeting");
            SliderFloatCard(Icon::Target, "FOV", "Field of view in degrees", &s.fov, 1.f, 180.f, "%.0f");
            SliderFloatCard(Icon::Target, "Smoothing", "Higher is slower and smoother", &s.smooth, 1.f, 20.f, "%.1f");
            ComboCard(Icon::Target, "Target bone", "", &s.bone, bones, 4);
            ComboCard(Icon::Target, "Priority", "How the target is chosen", &s.priority, prios, 3);
            MultiComboCard(Icon::Info, "Target groups", "", layerNames, s.targets, 3);
            GroupHeader("FOV circle");
            ToggleCard(Icon::Eye, "Show FOV circle", "Draws the aim radius on screen", &s.showFov);
            ColorCard(Icon::Sun, "Circle color", "", s.fovCol);
        } else if (s.tab == 2) {
            PageTitle("Memory");
            ValueCard(Icon::Chip, "Driver status", "Kernel driver connection", s.driverLoaded ? "Connected" : "Not loaded", s.driverLoaded);
            if (ButtonCard(Icon::Chip, "Driver", "Reload or unload the driver", s.driverLoaded ? "Unload" : "Load", s.driverLoaded ? Kind::Danger : Kind::Accent)) {
                s.driverLoaded = !s.driverLoaded;
                PushToast(s.driverLoaded ? "Driver loaded" : "Driver unloaded", s.driverLoaded ? ToastType::Success : ToastType::Info);
            }
            GroupHeader("Values");
            SliderIntCard(Icon::Gear, "Integer value", "Click the number for exact input", &s.valueA, 0, 1000);
            SliderFloatCard(Icon::Gear, "Float value", "", &s.valueB, 0.f, 10.f, "%.2f");
            CardBegin(Icon::Key, "Address", "Hex address");
            ImGui::SetCursorScreenPos(CardCtrl(Px(220), Px(32)));
            InputText("##addr", s.addr, sizeof(s.addr), 220.f, "0x0000000");
            CardEnd();
        } else {
            PageTitle("Settings");
            GroupHeader("Appearance");
            ComboCard(Icon::Globe, "Language", "Interface language", &s.lang, langs, 3);
            ComboCard(Icon::Sun, "Theme", "Dark or light, with a smooth transition", &s.theme, themes, 2);
            if (SliderFloatCard(Icon::Gear, "UI scale", "Applied when you release the slider", &s.pendingScale, 0.75f, 1.75f, "%.2f")) s.scaleDirty = true;
            if (s.scaleDirty && !ImGui::IsMouseDown(0)) { s.uiScale = s.pendingScale; SetScale(s.uiScale); s.scaleDirty = false; }
            ToggleCard(Icon::Gear, "Reduce animations", "Turns off motion effects", &s.reduceMotion);
            GroupHeader("General");
            ToggleCard(Icon::Speaker, "Sound", "Interface sounds", &s.sound);
            ToggleCard(Icon::Gear, "Show FPS", "Counter in the corner", &s.showFps);
            if (ButtonCard(Icon::Eye, "Screenshot", "Save the current frame", "Capture")) PushToast("Screenshot saved", ToastType::Success);
            GroupHeader("Configs");
            CardBegin(Icon::Info, "Config name", "Used when saving or loading");
            ImGui::SetCursorScreenPos(CardCtrl(Px(220), Px(32)));
            InputText("##cfg", s.cfgName, sizeof(s.cfgName), 220.f, "name");
            CardEnd();
            ImGui::Dummy(ImVec2(0, Px(4)));
            if (Button("Save", Kind::Accent)) PushToast("Config saved", ToastType::Success);
            Tooltip("Write the current settings to the named config");
            ImGui::SameLine();
            if (Button("Load")) PushToast("Config loaded");
            ImGui::SameLine();
            if (Button("Reset", Kind::Danger)) PushToast("Settings reset to defaults", ToastType::Error);
            Spacer(24);
        }
        EndPage();
        EndContent();
    }
    EndWindow();

    if (s.showFps) {
        char b[32]; snprintf(b, sizeof(b), "%.0f FPS", io.Framerate);
        ImGui::GetForegroundDrawList()->AddText(Ctx().fontSmall, FontPx(Ctx().fontSmall), ImVec2(io.DisplaySize.x - Px(70), Px(6)), U32(Pal().textSecondary), b);
    }
    RenderToasts();
}

} // namespace Demo
