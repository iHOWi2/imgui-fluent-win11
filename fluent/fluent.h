// fluent.h - Windows 11 (Fluent Design) theme + widgets for Dear ImGui.
// Header-only. Visual layer only: no application logic lives here.
// Needs Dear ImGui >= 1.91 (uses imgui_internal.h). Include after your ImGui backend headers.
//
// Usage:
//   Fluent::Init(dpiScale);                 // once, after ImGui::CreateContext()
//   each frame: ImGui::NewFrame(); Fluent::NewFrame(); ...widgets...; Fluent::RenderToasts();
#pragma once
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui.h"
#include "imgui_internal.h"
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace Fluent {

enum class Theme { Dark, Light };
enum class Kind { Standard, Accent, Danger };
enum class ToastType { Info, Success, Error };
enum class Icon { None, Eye, Target, Chip, Gear, Minimize, Close, Chevron, Check, Info, Sun, Globe, Speaker, Key };

// ----------------------------------------------------------------------------------------------
// Palette (values follow the WinUI 3 / Windows 11 Fluent tokens)
// ----------------------------------------------------------------------------------------------
inline ImVec4 RGBA(int r, int g, int b, float a = 1.f) { return ImVec4(r / 255.f, g / 255.f, b / 255.f, a); }

struct Palette {
    ImVec4 bg, card, cardHover, control, controlHover, controlActive, subtle, stroke, strokeStrong;
    ImVec4 text, textSecondary, textDisabled, accent, accentHover, accentActive, onAccent;
    ImVec4 danger, dangerHover, popup, shadow;
};

inline Palette DarkPalette() {
    Palette p;
    p.bg = RGBA(32, 32, 32);
    p.card = RGBA(255, 255, 255, 0.0512f);
    p.cardHover = RGBA(255, 255, 255, 0.0837f);
    p.control = RGBA(255, 255, 255, 0.0605f);
    p.controlHover = RGBA(255, 255, 255, 0.0837f);
    p.controlActive = RGBA(255, 255, 255, 0.0326f);
    p.subtle = RGBA(255, 255, 255, 0.0605f);
    p.stroke = RGBA(255, 255, 255, 0.07f);
    p.strokeStrong = RGBA(255, 255, 255, 0.5442f);
    p.text = RGBA(255, 255, 255, 1.f);
    p.textSecondary = RGBA(255, 255, 255, 0.786f);
    p.textDisabled = RGBA(255, 255, 255, 0.3628f);
    p.accent = RGBA(76, 194, 255);
    p.accentHover = RGBA(76, 194, 255, 0.9f);
    p.accentActive = RGBA(76, 194, 255, 0.8f);
    p.onAccent = RGBA(0, 0, 0, 0.9f);
    p.danger = RGBA(196, 43, 28);
    p.dangerHover = RGBA(196, 43, 28, 0.9f);
    p.popup = RGBA(44, 44, 44);
    p.shadow = RGBA(0, 0, 0, 0.38f);
    return p;
}

inline Palette LightPalette() {
    Palette p;
    p.bg = RGBA(243, 243, 243);
    p.card = RGBA(255, 255, 255, 0.7f);
    p.cardHover = RGBA(249, 249, 249, 0.85f);
    p.control = RGBA(255, 255, 255, 0.7f);
    p.controlHover = RGBA(249, 249, 249, 0.5f);
    p.controlActive = RGBA(249, 249, 249, 0.3f);
    p.subtle = RGBA(0, 0, 0, 0.0373f);
    p.stroke = RGBA(0, 0, 0, 0.0578f);
    p.strokeStrong = RGBA(0, 0, 0, 0.6063f);
    p.text = RGBA(0, 0, 0, 0.8956f);
    p.textSecondary = RGBA(0, 0, 0, 0.6063f);
    p.textDisabled = RGBA(0, 0, 0, 0.3614f);
    p.accent = RGBA(0, 103, 192);
    p.accentHover = RGBA(0, 103, 192, 0.9f);
    p.accentActive = RGBA(0, 103, 192, 0.8f);
    p.onAccent = RGBA(255, 255, 255, 1.f);
    p.danger = RGBA(196, 43, 28);
    p.dangerHover = RGBA(196, 43, 28, 0.9f);
    p.popup = RGBA(252, 252, 252);
    p.shadow = RGBA(0, 0, 0, 0.2f);
    return p;
}

inline Palette MixPalette(const Palette& a, const Palette& b, float t) {
    Palette r;
    const ImVec4* pa = (const ImVec4*)&a;
    const ImVec4* pb = (const ImVec4*)&b;
    ImVec4* pr = (ImVec4*)&r;
    for (size_t i = 0; i < sizeof(Palette) / sizeof(ImVec4); i++) pr[i] = ImLerp(pa[i], pb[i], t);
    return r;
}

// ----------------------------------------------------------------------------------------------
// Context / helpers
// ----------------------------------------------------------------------------------------------
struct ToastItem { std::string text; ToastType type; float born; };

struct Context {
    Theme theme = Theme::Dark;
    float themeT = 0.f;                 // 0 = dark, 1 = light (animated)
    Palette pal = DarkPalette();
    float dpi = 1.f, userScale = 1.f;   // dpi = device density, userScale = "UI scale" setting
    bool reduceMotion = false;
    ImFont *fontBody = nullptr, *fontSmall = nullptr, *fontSub = nullptr, *fontTitle = nullptr;
    float time = 0.f;
    bool collapsed = false;
    // navigation indicator
    float indY = 0.f; bool indInit = false;
    // page transition
    int pageTab = -1; float pageT = 1.f;
    // scroll area
    float scrollAlpha = 0.f; bool scrollHot = false; float scrollVel = 0.f; bool flung = false;
    // card layout
    ImVec2 cardStart, cardSize;
    std::vector<ToastItem> toasts;
};

inline Context& Ctx() { static Context c; return c; }
inline float Px(float v) { return v * Ctx().dpi * Ctx().userScale; }
inline const Palette& Pal() { return Ctx().pal; }

inline float Sat(float x) { return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); }
inline float EaseOut(float t) { float u = 1.f - Sat(t); return 1.f - u * u * u; }
inline ImU32 U32(ImVec4 c, float a = 1.f) { c.w *= a; return ImGui::ColorConvertFloat4ToU32(c); }
inline ImGuiID Sub(ImGuiID id, int k) { return id ^ (0x9E3779B1u * (ImGuiID)(k + 1)); }

// Frame-rate independent smoothing stored per widget id. Honors the "reduce animations" setting.
inline float Animate(ImGuiID id, float target, float speed = 16.f, float init = -1.f) {
    ImGuiStorage* st = ImGui::GetStateStorage();
    float v = st->GetFloat(id, init < 0.f ? target : init);
    if (Ctx().reduceMotion) v = target;
    else {
        v += (target - v) * (1.f - expf(-speed * ImGui::GetIO().DeltaTime));
        if (fabsf(target - v) < 0.001f) v = target;
    }
    st->SetFloat(id, v);
    return v;
}

inline float FontPx(ImFont* f) { return f->FontSize * ImGui::GetIO().FontGlobalScale; }
inline ImVec2 TextSize(ImFont* f, const char* t) { return f->CalcTextSizeA(FontPx(f), FLT_MAX, 0.f, t); }
inline void DrawLabel(ImDrawList* dl, ImFont* f, ImVec2 p, ImU32 col, const char* t) { dl->AddText(f, FontPx(f), p, col, t); }

struct Hit { bool pressed = false, hovered = false, held = false; };
inline Hit InteractRaw(const ImRect& bb, ImGuiID id) {
    Hit h;
    if (!ImGui::ItemAdd(bb, id)) return h;
    h.pressed = ImGui::ButtonBehavior(bb, id, &h.hovered, &h.held);
    return h;
}
inline Hit Interact(const ImRect& bb, ImGuiID id) { ImGui::ItemSize(bb); return InteractRaw(bb, id); }

// ----------------------------------------------------------------------------------------------
// Setup / per-frame
// ----------------------------------------------------------------------------------------------
// Loads fonts (Cyrillic included). Pass a Segoe UI Variable .ttf as fontPath to match Windows 11 exactly.
inline void Init(float dpi, const char* fontPath = nullptr) {
    Context& c = Ctx();
    c.dpi = dpi;
    ImGuiIO& io = ImGui::GetIO();
    const char* cands[] = { fontPath, "/system/fonts/Roboto-Regular.ttf", "/system/fonts/Roboto[wdth,wght].ttf",
                            "/system/fonts/DroidSans.ttf", "C:/Windows/Fonts/segoeui.ttf" };
    auto load = [&](float px) -> ImFont* {
        for (const char* p : cands) {
            if (!p) continue;
            FILE* f = fopen(p, "rb");
            if (!f) continue;
            fclose(f);
            ImFontConfig cfg;
            cfg.OversampleH = 2; cfg.OversampleV = 2;
            return io.Fonts->AddFontFromFileTTF(p, px * dpi, &cfg, io.Fonts->GetGlyphRangesCyrillic());
        }
        return io.Fonts->AddFontDefault();
    };
    c.fontBody = load(14.f);
    c.fontSmall = load(12.f);
    c.fontSub = load(18.f);
    c.fontTitle = load(28.f);
    io.FontDefault = c.fontBody;
}

inline void SetTheme(Theme t) { Ctx().theme = t; }
inline void SetScale(float s) { Ctx().userScale = s < 0.6f ? 0.6f : (s > 2.0f ? 2.0f : s); }
inline void SetReduceMotion(bool b) { Ctx().reduceMotion = b; }

// Call right after ImGui::NewFrame().
inline void NewFrame() {
    Context& c = Ctx();
    ImGuiIO& io = ImGui::GetIO();
    c.time += io.DeltaTime;
    float target = c.theme == Theme::Light ? 1.f : 0.f;
    if (c.reduceMotion) c.themeT = target;
    else {
        c.themeT += (target - c.themeT) * (1.f - expf(-9.f * io.DeltaTime));   // smooth theme cross-fade
        if (fabsf(target - c.themeT) < 0.002f) c.themeT = target;
    }
    c.pal = MixPalette(DarkPalette(), LightPalette(), c.themeT);
    const Palette& P = c.pal;
    io.FontGlobalScale = c.userScale;

    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding = Px(8); s.ChildRounding = Px(8); s.PopupRounding = Px(8); s.FrameRounding = Px(4);
    s.ScrollbarRounding = Px(4); s.ScrollbarSize = Px(4); s.GrabRounding = Px(4);
    s.WindowBorderSize = 0; s.ChildBorderSize = 0; s.PopupBorderSize = 1; s.FrameBorderSize = 0;
    s.WindowPadding = ImVec2(Px(12), Px(12));
    s.FramePadding = ImVec2(Px(12), Px(7));
    s.ItemSpacing = ImVec2(Px(8), Px(6));
    s.ItemInnerSpacing = ImVec2(Px(8), Px(4));
    ImVec4* col = s.Colors;
    col[ImGuiCol_Text] = P.text;
    col[ImGuiCol_TextDisabled] = P.textDisabled;
    col[ImGuiCol_WindowBg] = ImVec4(0, 0, 0, 0);
    col[ImGuiCol_ChildBg] = ImVec4(0, 0, 0, 0);
    col[ImGuiCol_PopupBg] = P.popup;
    col[ImGuiCol_Border] = ImLerp(P.stroke, ImVec4(0, 0, 0, 0.25f), 0.f);
    col[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
    col[ImGuiCol_FrameBg] = P.control;
    col[ImGuiCol_FrameBgHovered] = P.controlHover;
    col[ImGuiCol_FrameBgActive] = P.controlActive;
    col[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0);
    col[ImGuiCol_TextSelectedBg] = ImVec4(P.accent.x, P.accent.y, P.accent.z, 0.35f);
    col[ImGuiCol_NavHighlight] = ImVec4(0, 0, 0, 0);
}

// ----------------------------------------------------------------------------------------------
// Line icons (drawn with vector primitives: always crisp at any scale, no icon font needed)
// ----------------------------------------------------------------------------------------------
inline void DrawIcon(ImDrawList* dl, Icon ic, ImVec2 c, float s, ImU32 col) {
    const float th = ImMax(1.f, s * 0.085f);
    switch (ic) {
    case Icon::Eye: {
        const int N = 14; ImVec2 pts[2 * N + 2]; int n = 0;
        float w = s * 0.5f, h = s * 0.3f;
        for (int i = 0; i <= N; i++) { float u = -1.f + 2.f * i / N; pts[n++] = ImVec2(c.x + u * w, c.y - h * (1.f - u * u)); }
        for (int i = N - 1; i >= 1; i--) { float u = -1.f + 2.f * i / N; pts[n++] = ImVec2(c.x + u * w, c.y + h * (1.f - u * u)); }
        dl->AddPolyline(pts, n, col, ImDrawFlags_Closed, th);
        dl->AddCircle(c, s * 0.14f, col, 0, th);
        break; }
    case Icon::Target:
        dl->AddCircle(c, s * 0.38f, col, 0, th);
        dl->AddCircleFilled(c, s * 0.07f, col);
        for (int k = 0; k < 4; k++) {
            float a = k * IM_PI * 0.5f, ca = cosf(a), sa = sinf(a);
            dl->AddLine(ImVec2(c.x + ca * s * 0.26f, c.y + sa * s * 0.26f), ImVec2(c.x + ca * s * 0.5f, c.y + sa * s * 0.5f), col, th);
        }
        break;
    case Icon::Chip:
        dl->AddRect(ImVec2(c.x - s * 0.26f, c.y - s * 0.26f), ImVec2(c.x + s * 0.26f, c.y + s * 0.26f), col, s * 0.06f, 0, th);
        dl->AddRectFilled(ImVec2(c.x - s * 0.09f, c.y - s * 0.09f), ImVec2(c.x + s * 0.09f, c.y + s * 0.09f), col, s * 0.02f);
        for (int k = -1; k <= 1; k += 2) {
            float o = k * s * 0.12f;
            dl->AddLine(ImVec2(c.x + o, c.y - s * 0.26f), ImVec2(c.x + o, c.y - s * 0.44f), col, th);
            dl->AddLine(ImVec2(c.x + o, c.y + s * 0.26f), ImVec2(c.x + o, c.y + s * 0.44f), col, th);
            dl->AddLine(ImVec2(c.x - s * 0.26f, c.y + o), ImVec2(c.x - s * 0.44f, c.y + o), col, th);
            dl->AddLine(ImVec2(c.x + s * 0.26f, c.y + o), ImVec2(c.x + s * 0.44f, c.y + o), col, th);
        }
        break;
    case Icon::Gear:
        dl->AddCircle(c, s * 0.15f, col, 0, th);
        dl->AddCircle(c, s * 0.32f, col, 0, th);
        for (int k = 0; k < 8; k++) {
            float a = k * IM_PI * 0.25f, ca = cosf(a), sa = sinf(a);
            dl->AddLine(ImVec2(c.x + ca * s * 0.32f, c.y + sa * s * 0.32f), ImVec2(c.x + ca * s * 0.46f, c.y + sa * s * 0.46f), col, th * 1.8f);
        }
        break;
    case Icon::Minimize:
        dl->AddLine(ImVec2(c.x - s * 0.5f, c.y), ImVec2(c.x + s * 0.5f, c.y), col, th);
        break;
    case Icon::Close:
        dl->AddLine(ImVec2(c.x - s * 0.5f, c.y - s * 0.5f), ImVec2(c.x + s * 0.5f, c.y + s * 0.5f), col, th);
        dl->AddLine(ImVec2(c.x + s * 0.5f, c.y - s * 0.5f), ImVec2(c.x - s * 0.5f, c.y + s * 0.5f), col, th);
        break;
    case Icon::Chevron: {
        ImVec2 pts[3] = { ImVec2(c.x - s * 0.3f, c.y - s * 0.14f), ImVec2(c.x, c.y + s * 0.16f), ImVec2(c.x + s * 0.3f, c.y - s * 0.14f) };
        dl->AddPolyline(pts, 3, col, 0, th);
        break; }
    case Icon::Check: {
        ImVec2 pts[3] = { ImVec2(c.x - s * 0.3f, c.y + s * 0.02f), ImVec2(c.x - s * 0.08f, c.y + s * 0.24f), ImVec2(c.x + s * 0.32f, c.y - s * 0.22f) };
        dl->AddPolyline(pts, 3, col, 0, th * 1.2f);
        break; }
    case Icon::Info:
        dl->AddCircle(c, s * 0.46f, col, 0, th);
        dl->AddLine(ImVec2(c.x, c.y - s * 0.02f), ImVec2(c.x, c.y + s * 0.24f), col, th);
        dl->AddCircleFilled(ImVec2(c.x, c.y - s * 0.2f), th * 0.75f, col);
        break;
    case Icon::Sun:
        dl->AddCircle(c, s * 0.17f, col, 0, th);
        for (int k = 0; k < 8; k++) {
            float a = k * IM_PI * 0.25f, ca = cosf(a), sa = sinf(a);
            dl->AddLine(ImVec2(c.x + ca * s * 0.3f, c.y + sa * s * 0.3f), ImVec2(c.x + ca * s * 0.46f, c.y + sa * s * 0.46f), col, th);
        }
        break;
    case Icon::Globe: {
        dl->AddCircle(c, s * 0.46f, col, 0, th);
        dl->AddLine(ImVec2(c.x - s * 0.46f, c.y), ImVec2(c.x + s * 0.46f, c.y), col, th);
        const int N = 20; ImVec2 pts[N];
        for (int i = 0; i < N; i++) { float a = i * 2.f * IM_PI / N; pts[i] = ImVec2(c.x + cosf(a) * s * 0.2f, c.y + sinf(a) * s * 0.46f); }
        dl->AddPolyline(pts, N, col, ImDrawFlags_Closed, th);
        break; }
    case Icon::Speaker: {
        ImVec2 pts[6] = { ImVec2(c.x - s * 0.38f, c.y - s * 0.1f), ImVec2(c.x - s * 0.22f, c.y - s * 0.1f), ImVec2(c.x - s * 0.02f, c.y - s * 0.28f),
                          ImVec2(c.x - s * 0.02f, c.y + s * 0.28f), ImVec2(c.x - s * 0.22f, c.y + s * 0.1f), ImVec2(c.x - s * 0.38f, c.y + s * 0.1f) };
        dl->AddPolyline(pts, 6, col, ImDrawFlags_Closed, th);
        dl->PathArcTo(ImVec2(c.x + s * 0.02f, c.y), s * 0.24f, -0.9f, 0.9f);
        dl->PathStroke(col, 0, th);
        break; }
    case Icon::Key:
        dl->AddRect(ImVec2(c.x - s * 0.46f, c.y - s * 0.26f), ImVec2(c.x + s * 0.46f, c.y + s * 0.26f), col, s * 0.08f, 0, th);
        for (int k = -1; k <= 1; k++) dl->AddCircleFilled(ImVec2(c.x + k * s * 0.22f, c.y - s * 0.06f), th * 0.6f, col);
        dl->AddLine(ImVec2(c.x - s * 0.22f, c.y + s * 0.12f), ImVec2(c.x + s * 0.22f, c.y + s * 0.12f), col, th);
        break;
    default: break;
    }
}

// ----------------------------------------------------------------------------------------------
// Window frame: Mica-like surface, rounded corners, hairline border, soft shadow, title bar
// ----------------------------------------------------------------------------------------------
// Always pair with EndWindow(). Returns true when the content area is visible.
inline bool BeginWindow(const char* title, bool* open, ImVec2 pos, ImVec2 size) {
    Context& c = Ctx();
    const Palette& P = c.pal;
    ImGuiIO& io = ImGui::GetIO();
    const float th = Px(40);
    ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(size.x, c.collapsed ? th : size.y), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    bool vis = ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
                                              ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
    ImGui::PopStyleVar(2);

    // title-bar drag + caption buttons (hit-test first so the window can move before we draw)
    ImVec2 p = ImGui::GetWindowPos(), sz = ImGui::GetWindowSize();
    const float bw = Px(46);
    ImRect dragBB(p, ImVec2(p.x + sz.x - 2 * bw, p.y + th));
    Hit drag = InteractRaw(dragBB, ImGui::GetID("##fl_drag"));
    if (drag.held && ImGui::IsMouseDragging(0, 0.f)) ImGui::SetWindowPos(ImGui::GetWindowPos() + io.MouseDelta);
    p = ImGui::GetWindowPos();
    ImRect minBB(ImVec2(p.x + sz.x - 2 * bw, p.y), ImVec2(p.x + sz.x - bw, p.y + th));
    ImRect clsBB(ImVec2(p.x + sz.x - bw, p.y), ImVec2(p.x + sz.x, p.y + th));
    Hit hMin = InteractRaw(minBB, ImGui::GetID("##fl_min"));
    Hit hCls = InteractRaw(clsBB, ImGui::GetID("##fl_cls"));
    if (hMin.pressed) c.collapsed = !c.collapsed;
    if (hCls.pressed && open) *open = false;

    const float r = Px(8);
    ImDrawList* bgl = ImGui::GetBackgroundDrawList();
    for (int i = 12; i >= 1; i--) {                                   // soft shadow
        float k = 1.f - i / 12.f;
        float ex = Px(i * 1.6f);
        bgl->AddRectFilled(ImVec2(p.x - ex, p.y - ex + Px(6)), ImVec2(p.x + sz.x + ex, p.y + sz.y + ex + Px(6)),
                           U32(P.shadow, 0.14f * k * k + 0.01f), r + ex);
    }
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(p, p + sz, U32(P.bg), r);                      // Mica base
    ImVec4 tint = ImLerp(ImVec4(P.accent.x, P.accent.y, P.accent.z, 0.06f), ImVec4(P.accent.x, P.accent.y, P.accent.z, 0.05f), 0.5f);
    dl->AddRectFilledMultiColor(ImVec2(p.x + r, p.y + 1), ImVec2(p.x + sz.x - r, p.y + ImMin(sz.y, Px(160))),
                                U32(tint), U32(tint), U32(tint, 0.f), U32(tint, 0.f)); // subtle luminosity tint
    dl->AddRect(p, p + sz, U32(P.strokeStrong, 0.22f), r, 0, 1.f);

    // caption buttons
    if (hMin.hovered) dl->AddRectFilled(minBB.Min, minBB.Max, U32(P.subtle, hMin.held ? 0.6f : 1.f));
    if (hCls.hovered) dl->AddRectFilled(clsBB.Min, clsBB.Max, U32(P.danger, hCls.held ? 0.8f : 1.f), r, ImDrawFlags_RoundCornersTopRight);
    DrawIcon(dl, Icon::Minimize, minBB.GetCenter(), Px(10), U32(P.text));
    DrawIcon(dl, Icon::Close, clsBB.GetCenter(), Px(10), U32(hCls.hovered ? RGBA(255, 255, 255) : P.text));
    DrawIcon(dl, Icon::Gear, ImVec2(p.x + Px(20), p.y + th * 0.5f), Px(14), U32(P.text));
    DrawLabel(dl, c.fontSmall, ImVec2(p.x + Px(36), p.y + (th - FontPx(c.fontSmall)) * 0.5f), U32(P.text), title);

    ImGui::SetCursorPos(ImVec2(0, th));
    return vis && !c.collapsed;
}
inline void EndWindow() { ImGui::End(); }

// ----------------------------------------------------------------------------------------------
// Navigation (left pane, animated selection pill) and content area
// ----------------------------------------------------------------------------------------------
inline void BeginNav(float width = 220.f) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(Px(10), Px(6)));
    ImGui::BeginChild("##fl_nav", ImVec2(Px(width), 0), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar();
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(Px(8), Px(4)));
}
inline void EndNav() { ImGui::PopStyleVar(); ImGui::EndChild(); }

inline bool NavItem(Icon ic, const char* label, int* sel, int idx) {
    Context& c = Ctx(); const Palette& P = c.pal;
    ImGuiID id = ImGui::GetID(label);
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x, h = Px(40);
    ImRect bb(p, ImVec2(p.x + w, p.y + h));
    Hit hit = Interact(bb, id);
    bool changed = false;
    if (hit.pressed && *sel != idx) { *sel = idx; changed = true; }
    bool selected = (*sel == idx);
    float hv = Animate(Sub(id, 0), hit.hovered ? 1.f : 0.f), sl = Animate(Sub(id, 1), selected ? 1.f : 0.f);
    float pr = Animate(Sub(id, 2), hit.held ? 1.f : 0.f, 24.f);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float fill = ImMax(sl, hv * 0.8f) * (1.f - 0.4f * pr);
    dl->AddRectFilled(bb.Min, bb.Max, U32(P.subtle, fill), Px(4));
    if (selected) {                                                    // sliding accent indicator
        float target = bb.GetCenter().y - ImGui::GetWindowPos().y;
        if (!c.indInit || c.reduceMotion) { c.indY = target; c.indInit = true; }
        float dy = target - c.indY;
        c.indY += dy * (1.f - expf(-18.f * ImGui::GetIO().DeltaTime));
        float ih = Px(16) + ImMin(fabsf(dy) * 0.6f, Px(18));          // stretches while travelling
        float cy = ImGui::GetWindowPos().y + c.indY;
        dl->AddRectFilled(ImVec2(bb.Min.x, cy - ih * 0.5f), ImVec2(bb.Min.x + Px(3), cy + ih * 0.5f), U32(P.accent), Px(2));
    }
    float cy = bb.GetCenter().y;
    DrawIcon(dl, ic, ImVec2(p.x + Px(24), cy), Px(18), U32(P.text));
    DrawLabel(dl, c.fontBody, ImVec2(p.x + Px(44), cy - FontPx(c.fontBody) * 0.5f), U32(P.text), label);
    return changed;
}

// Scrollable content with thin auto-hiding scrollbar, touch drag + fling scrolling.
inline void BeginContent() {
    Context& c = Ctx(); const Palette& P = c.pal;
    float dt = ImGui::GetIO().DeltaTime;
    c.scrollAlpha += ((c.scrollHot ? 1.f : 0.f) - c.scrollAlpha) * (1.f - expf(-10.f * dt));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, U32(P.strokeStrong, 0.7f * c.scrollAlpha));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, U32(P.strokeStrong, 0.9f * c.scrollAlpha));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, U32(P.strokeStrong, c.scrollAlpha));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(Px(28), Px(8)));
    ImGui::BeginChild("##fl_content", ImVec2(0, 0), ImGuiChildFlags_AlwaysUseWindowPadding);
    ImGui::PopStyleVar();
    c.scrollHot = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && (ImGui::IsMouseDown(0) || ImGui::GetIO().MouseWheel != 0.f || c.scrollHot);
    if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)) c.scrollHot = false;
}
inline void EndContent() {
    Context& c = Ctx(); ImGuiIO& io = ImGui::GetIO();
    float dt = ImMax(io.DeltaTime, 1e-4f);
    if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(0, Px(6)) && !ImGui::IsAnyItemActive()) {
        ImGui::SetScrollY(ImGui::GetScrollY() - io.MouseDelta.y);
        c.scrollVel = -io.MouseDelta.y / dt; c.flung = true; c.scrollHot = true;
    } else if (c.flung) {
        if (ImGui::IsMouseDown(0)) c.flung = false;
        else {
            ImGui::SetScrollY(ImGui::GetScrollY() + c.scrollVel * dt);
            c.scrollVel *= expf(-4.f * dt);
            if (fabsf(c.scrollVel) < 20.f) c.flung = false;
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleColor(3);
}

// Smooth content switch: fade + slide-up whenever the tab index changes.
inline void BeginPage(int tab) {
    Context& c = Ctx();
    if (tab != c.pageTab) { c.pageTab = tab; c.pageT = c.reduceMotion ? 1.f : 0.f; }
    c.pageT = Sat(c.pageT + ImGui::GetIO().DeltaTime / 0.24f);
    float e = EaseOut(c.pageT);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImMax(e, 0.001f));
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (1.f - e) * Px(22));
}
inline void EndPage() { ImGui::PopStyleVar(); }

inline void PageTitle(const char* text) {
    Context& c = Ctx();
    ImGui::PushFont(c.fontTitle);
    ImGui::TextUnformatted(text);
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0, Px(6)));
}
inline void GroupHeader(const char* text) {
    Context& c = Ctx();
    ImGui::Dummy(ImVec2(0, Px(6)));
    ImGui::PushFont(c.fontSub);
    ImGui::TextUnformatted(text);
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0, Px(2)));
}
inline void Spacer(float h) { ImGui::Dummy(ImVec2(0, Px(h))); }

// ----------------------------------------------------------------------------------------------
// Primitive controls
// ----------------------------------------------------------------------------------------------
inline bool Toggle(const char* sid, bool* v) {
    const Palette& P = Pal();
    ImGuiID id = ImGui::GetID(sid);
    ImVec2 p = ImGui::GetCursorScreenPos(), sz(Px(40), Px(20));
    Hit h = Interact(ImRect(p, p + sz), id);
    if (h.pressed) *v = !*v;
    float t = Animate(Sub(id, 0), *v ? 1.f : 0.f, 18.f), hv = Animate(Sub(id, 1), h.hovered ? 1.f : 0.f), hd = Animate(Sub(id, 2), h.held ? 1.f : 0.f, 24.f);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float r = sz.y * 0.5f;
    dl->AddRectFilled(p, p + sz, U32(P.accent, t * (1.f - 0.15f * hd)), r);
    dl->AddRect(p, p + sz, U32(P.strokeStrong, 1.f - t), r, 0, ImMax(1.f, Px(1)));
    float tr = Px(5.f) + Px(1.f) * hv + Px(1.f) * hd;                  // thumb grows on hover / press
    float x = ImLerp(p.x + Px(10), p.x + sz.x - Px(10), t);
    dl->AddCircleFilled(ImVec2(x, p.y + r), tr, U32(ImLerp(P.textSecondary, P.onAccent, t)));
    return h.pressed;
}

inline bool Checkbox(const char* label, bool* v) {
    Context& c = Ctx(); const Palette& P = c.pal;
    const char* end = ImGui::FindRenderedTextEnd(label);
    std::string txt(label, end);
    ImGuiID id = ImGui::GetID(label);
    ImVec2 p = ImGui::GetCursorScreenPos();
    float box = Px(20), tw = TextSize(c.fontBody, txt.c_str()).x;
    ImRect bb(p, ImVec2(p.x + box + (tw > 0 ? Px(8) + tw : 0), p.y + Px(24)));
    Hit h = Interact(bb, id);
    if (h.pressed) *v = !*v;
    float t = Animate(Sub(id, 0), *v ? 1.f : 0.f, 20.f), hv = Animate(Sub(id, 1), h.hovered ? 1.f : 0.f), pr = Animate(Sub(id, 2), h.held ? 1.f : 0.f, 24.f);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 a(p.x, p.y + (Px(24) - box) * 0.5f), b(a.x + box, a.y + box);
    dl->AddRectFilled(a, b, U32(P.control, (1.f - t) * (0.6f + 0.4f * hv)), Px(4));
    dl->AddRectFilled(a, b, U32(P.accent, t * (1.f - 0.2f * pr)), Px(4));
    dl->AddRect(a, b, U32(P.strokeStrong, 1.f - t), Px(4), 0, ImMax(1.f, Px(1)));
    if (t > 0.01f) {                                                  // animated check stroke
        ImVec2 c0(a.x + box * 0.24f, a.y + box * 0.52f), c1(a.x + box * 0.43f, a.y + box * 0.72f), c2(a.x + box * 0.77f, a.y + box * 0.30f);
        float k = t * 2.f; ImVec2 pts[3] = { c0, c1, c2 }; int n = 3;
        if (k < 1.f) { pts[1] = ImLerp(c0, c1, k); n = 2; } else pts[2] = ImLerp(c1, c2, k - 1.f);
        dl->AddPolyline(pts, n, U32(P.onAccent), 0, ImMax(1.5f, Px(1.6f)));
    }
    if (tw > 0) DrawLabel(dl, c.fontBody, ImVec2(a.x + box + Px(8), p.y + (Px(24) - FontPx(c.fontBody)) * 0.5f), U32(P.text), txt.c_str());
    return h.pressed;
}

inline bool Button(const char* label, Kind kind = Kind::Standard, ImVec2 size = ImVec2(0, 0)) {
    Context& c = Ctx(); const Palette& P = c.pal;
    const char* end = ImGui::FindRenderedTextEnd(label);
    std::string txt(label, end);
    ImGuiID id = ImGui::GetID(label);
    ImVec2 ts = TextSize(c.fontBody, txt.c_str());
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = size.x > 0 ? size.x : ts.x + Px(32), h = size.y > 0 ? size.y : Px(32);
    Hit hit = Interact(ImRect(p, ImVec2(p.x + w, p.y + h)), id);
    float hv = Animate(Sub(id, 0), hit.hovered ? 1.f : 0.f, 18.f), pr = Animate(Sub(id, 1), hit.held ? 1.f : 0.f, 26.f);
    ImVec4 base, hover, active, txtc;
    switch (kind) {
    case Kind::Accent: base = P.accent; hover = P.accentHover; active = P.accentActive; txtc = P.onAccent; break;
    case Kind::Danger: base = P.danger; hover = P.dangerHover; active = ImVec4(P.danger.x, P.danger.y, P.danger.z, 0.8f); txtc = RGBA(255, 255, 255); break;
    default: base = P.control; hover = P.controlHover; active = P.controlActive; txtc = P.text; break;
    }
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(p, ImVec2(p.x + w, p.y + h), U32(ImLerp(ImLerp(base, hover, hv), active, pr)), Px(4));
    dl->AddRect(p, ImVec2(p.x + w, p.y + h), U32(P.stroke, kind == Kind::Standard ? 1.f : 0.5f), Px(4));
    ImVec4 tc = txtc; tc.w *= 1.f - 0.2f * pr;
    DrawLabel(dl, c.fontBody, ImVec2(p.x + (w - ts.x) * 0.5f, p.y + (h - ts.y) * 0.5f), U32(tc), txt.c_str());
    return hit.pressed;
}

// Slider with accent track, round thumb with inner dot, value box on the right.
// Click (or double click the track) the value box for precise keyboard input.
inline bool SliderCore(const char* sid, float* v, float mn, float mx, const char* fmt, bool isInt, float trackW) {
    Context& c = Ctx(); const Palette& P = c.pal;
    ImGuiID id = ImGui::GetID(sid);
    ImVec2 p = ImGui::GetCursorScreenPos();
    float h = Px(24), gap = Px(12), valW = Px(60);
    ImVec2 after = ImVec2(p.x, p.y + h + ImGui::GetStyle().ItemSpacing.y);
    ImGui::ItemSize(ImVec2(trackW + gap + valW, h));
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float thumbR = Px(10);
    ImRect tb(p, ImVec2(p.x + trackW, p.y + h));
    Hit th = InteractRaw(tb, id);
    bool changed = false;
    if (th.held) {
        float t = Sat((ImGui::GetIO().MousePos.x - (tb.Min.x + thumbR)) / (trackW - 2 * thumbR));
        float nv = mn + t * (mx - mn);
        if (isInt) nv = roundf(nv);
        if (nv != *v) { *v = nv; changed = true; }
    }
    float tn = Sat((*v - mn) / (mx - mn));
    float tt = Animate(Sub(id, 0), tn, 30.f), hv = Animate(Sub(id, 1), (th.hovered || th.held) ? 1.f : 0.f), pr = Animate(Sub(id, 2), th.held ? 1.f : 0.f, 24.f);
    float cy = p.y + h * 0.5f, x0 = p.x + thumbR, x1 = p.x + trackW - thumbR, tx = ImLerp(x0, x1, tt);
    float tr = Px(2);
    dl->AddRectFilled(ImVec2(x0, cy - tr), ImVec2(x1, cy + tr), U32(P.strokeStrong, 0.5f), tr);
    dl->AddRectFilled(ImVec2(x0, cy - tr), ImVec2(tx, cy + tr), U32(P.accent), tr);
    dl->AddCircleFilled(ImVec2(tx, cy), thumbR, U32(P.popup));
    dl->AddCircle(ImVec2(tx, cy), thumbR, U32(P.stroke, 2.f));
    dl->AddCircleFilled(ImVec2(tx, cy), Px(4.f) + Px(1.5f) * hv - Px(1.5f) * pr, U32(P.accent));

    // value box / precise input
    ImVec2 vp(p.x + trackW + gap, p.y);
    ImRect vb(vp, ImVec2(vp.x + valW, vp.y + h));
    ImGuiStorage* st = ImGui::GetStateStorage();
    ImGuiID eid = Sub(id, 9);
    static char buf[32]; static bool focus = false; static int editFrame = 0;
    bool editing = st->GetBool(eid, false);
    if (!editing) {
        Hit vh = InteractRaw(vb, Sub(id, 8));
        dl->AddRectFilled(vb.Min, vb.Max, U32(P.control, vh.hovered ? 1.4f : 1.f), Px(4));
        dl->AddRect(vb.Min, vb.Max, U32(P.stroke), Px(4));
        char tmp[32]; snprintf(tmp, sizeof(tmp), isInt ? "%d" : fmt, isInt ? (int)*v : *v);
        ImVec2 ts = TextSize(c.fontBody, tmp);
        DrawLabel(dl, c.fontBody, ImVec2(vb.Min.x + (valW - ts.x) * 0.5f, vb.Min.y + (h - ts.y) * 0.5f), U32(P.text), tmp);
        if (vh.pressed || (th.hovered && ImGui::IsMouseDoubleClicked(0))) {
            st->SetBool(eid, true); focus = true; editFrame = ImGui::GetFrameCount();
            snprintf(buf, sizeof(buf), isInt ? "%d" : "%g", isInt ? (int)*v : *v);
        }
    } else {
        ImVec2 save = ImGui::GetCursorScreenPos();
        ImGui::SetCursorScreenPos(vp);
        ImGui::SetNextItemWidth(valW);
        if (focus) { ImGui::SetKeyboardFocusHere(); focus = false; }
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(Px(6), (h - FontPx(c.fontBody)) * 0.5f));
        bool done = ImGui::InputText("##edit", buf, sizeof(buf), ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);
        ImGui::PopStyleVar();
        bool active = ImGui::IsItemActive();
        if (done) {
            float nv = (float)atof(buf); nv = nv < mn ? mn : (nv > mx ? mx : nv);
            if (isInt) nv = roundf(nv);
            if (nv != *v) { *v = nv; changed = true; }
        }
        if (done || (!active && ImGui::GetFrameCount() > editFrame + 2)) st->SetBool(eid, false);
        ImGui::SetCursorScreenPos(save);
    }
    ImGui::SetCursorScreenPos(after);
    return changed;
}
inline bool SliderFloat(const char* sid, float* v, float mn, float mx, const char* fmt = "%.1f", float width = 220.f) { return SliderCore(sid, v, mn, mx, fmt, false, Px(width)); }
inline bool SliderInt(const char* sid, int* v, int mn, int mx, float width = 220.f) {
    float f = (float)*v; bool ch = SliderCore(sid, &f, (float)mn, (float)mx, "%d", true, Px(width)); if (ch) *v = (int)f; return ch;
}

// Selectable row used inside combo popups (single select = accent bar, multi select = check box).
inline bool PopupItem(const char* label, bool selected, bool multi = false) {
    Context& c = Ctx(); const Palette& P = c.pal;
    ImGuiID id = ImGui::GetID(label);
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x, h = Px(36);
    Hit hit = Interact(ImRect(p, ImVec2(p.x + w, p.y + h)), id);
    float hv = Animate(Sub(id, 0), hit.hovered ? 1.f : 0.f), sl = Animate(Sub(id, 1), selected ? 1.f : 0.f);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(p, ImVec2(p.x + w, p.y + h), U32(P.subtle, ImMax(hv, multi ? 0.f : sl * 0.8f)), Px(4));
    float tx = p.x + Px(12);
    if (multi) {
        float b = Px(18); ImVec2 a(p.x + Px(10), p.y + (h - b) * 0.5f);
        dl->AddRectFilled(a, ImVec2(a.x + b, a.y + b), U32(P.accent, sl), Px(4));
        dl->AddRect(a, ImVec2(a.x + b, a.y + b), U32(P.strokeStrong, 1.f - sl), Px(4), 0, ImMax(1.f, Px(1)));
        if (sl > 0.01f) DrawIcon(dl, Icon::Check, ImVec2(a.x + b * 0.5f, a.y + b * 0.5f), b * 0.9f, U32(P.onAccent, sl));
        tx = p.x + Px(38);
    } else if (sl > 0.01f) {
        float bh = Px(16) * sl;
        dl->AddRectFilled(ImVec2(p.x + Px(2), p.y + h * 0.5f - bh * 0.5f), ImVec2(p.x + Px(5), p.y + h * 0.5f + bh * 0.5f), U32(P.accent), Px(2));
    }
    DrawLabel(dl, c.fontBody, ImVec2(tx, p.y + (h - FontPx(c.fontBody)) * 0.5f), U32(P.text), label);
    return hit.pressed;
}

inline bool ComboButton(const char* sid, const char* text, float width, bool open, ImRect* outBB) {
    Context& c = Ctx(); const Palette& P = c.pal;
    ImGuiID id = ImGui::GetID(sid);
    ImVec2 p = ImGui::GetCursorScreenPos(); float h = Px(32);
    ImRect bb(p, ImVec2(p.x + width, p.y + h));
    Hit hit = Interact(bb, id);
    float hv = Animate(Sub(id, 0), hit.hovered ? 1.f : 0.f), pr = Animate(Sub(id, 1), hit.held ? 1.f : 0.f, 24.f), op = Animate(Sub(id, 2), open ? 1.f : 0.f);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(bb.Min, bb.Max, U32(ImLerp(ImLerp(P.control, P.controlHover, hv), P.controlActive, pr)), Px(4));
    dl->AddRect(bb.Min, bb.Max, U32(P.stroke), Px(4));
    DrawLabel(dl, c.fontBody, ImVec2(p.x + Px(12), p.y + (h - FontPx(c.fontBody)) * 0.5f), U32(P.text), text);
    DrawIcon(dl, Icon::Chevron, ImVec2(bb.Max.x - Px(16), p.y + h * 0.5f + Px(1) * hv - Px(1) * op), Px(12), U32(P.textSecondary));
    *outBB = bb;
    return hit.pressed;
}

inline void PopupBegin(const char* sid, const ImRect& anchor, float width, bool* open) {}

inline bool BeginDropdown(const char* sid, const ImRect& bb, float width) {
    ImGuiID id = ImGui::GetID(sid);
    ImGuiStorage* st = ImGui::GetStateStorage();
    if (!ImGui::IsPopupOpen(sid)) st->SetFloat(Sub(id, 5), 0.f);
    float a = Animate(Sub(id, 5), 1.f, 22.f, 0.f);
    ImGui::SetNextWindowPos(ImVec2(bb.Min.x, bb.Max.y + Px(4) - (1.f - a) * Px(8)));
    ImGui::SetNextWindowSize(ImVec2(width, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImMax(a, 0.001f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(Px(4), Px(4)));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(Px(4), Px(2)));
    ImGui::PushStyleColor(ImGuiCol_Border, Pal().stroke);
    bool ok = ImGui::BeginPopup(sid);
    if (!ok) { ImGui::PopStyleColor(); ImGui::PopStyleVar(3); }
    return ok;
}
inline void EndDropdown() { ImGui::EndPopup(); ImGui::PopStyleColor(); ImGui::PopStyleVar(3); }

inline bool Combo(const char* sid, int* cur, const char* const* items, int n, float width = 180.f) {
    float w = Px(width); ImRect bb; bool changed = false;
    bool open = ImGui::IsPopupOpen(sid);
    if (ComboButton(sid, items[*cur], w, open, &bb)) ImGui::OpenPopup(sid);
    if (BeginDropdown(sid, bb, w)) {
        for (int i = 0; i < n; i++)
            if (PopupItem(items[i], i == *cur)) { *cur = i; changed = true; ImGui::CloseCurrentPopup(); }
        EndDropdown();
    }
    return changed;
}

// Multi-select dropdown: list with check boxes, stays open while ticking.
inline bool MultiCombo(const char* sid, const char* const* items, bool* sel, int n, float width = 180.f) {
    float w = Px(width); ImRect bb; bool changed = false;
    std::string sum; int cnt = 0;
    for (int i = 0; i < n; i++) if (sel[i]) { cnt++; if (cnt <= 2) sum += (cnt > 1 ? ", " : "") + std::string(items[i]); }
    if (cnt == 0) sum = "None"; else if (cnt > 2) sum = std::to_string(cnt) + " selected";
    if (ComboButton(sid, sum.c_str(), w, ImGui::IsPopupOpen(sid), &bb)) ImGui::OpenPopup(sid);
    if (BeginDropdown(sid, bb, w)) {
        for (int i = 0; i < n; i++) if (PopupItem(items[i], sel[i], true)) { sel[i] = !sel[i]; changed = true; }
        EndDropdown();
    }
    return changed;
}

inline void Checker(ImDrawList* dl, ImVec2 a, ImVec2 b) {
    float s = Px(6);
    dl->PushClipRect(a, b, true);
    dl->AddRectFilled(a, b, IM_COL32(255, 255, 255, 255));
    for (float y = a.y, r = 0; y < b.y; y += s, r++)
        for (float x = a.x + ((int)r % 2) * s; x < b.x; x += s * 2) dl->AddRectFilled(ImVec2(x, y), ImVec2(x + s, y + s), IM_COL32(204, 204, 204, 255));
    dl->PopClipRect();
}

// RGBA colour swatch + picker popup (saturation/value square, hue bar, alpha bar).
inline bool ColorEdit(const char* sid, float col[4]) {
    Context& c = Ctx(); const Palette& P = c.pal;
    ImGuiID id = ImGui::GetID(sid);
    ImGuiStorage* st = ImGui::GetStateStorage();
    ImVec2 p = ImGui::GetCursorScreenPos(); ImVec2 sz(Px(56), Px(28));
    Hit hit = Interact(ImRect(p, p + sz), id);
    float hv = Animate(Sub(id, 0), hit.hovered ? 1.f : 0.f);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    Checker(dl, p, p + sz);
    dl->AddRectFilled(p, p + sz, ImGui::ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], col[3])), Px(4));
    dl->AddRect(p, p + sz, U32(P.strokeStrong, 0.3f + 0.4f * hv), Px(4), 0, ImMax(1.f, Px(1)));
    ImGuiID hid = Sub(id, 20), sid2 = Sub(id, 21), vid = Sub(id, 22);
    if (hit.pressed) {
        float h, s, v; ImGui::ColorConvertRGBtoHSV(col[0], col[1], col[2], h, s, v);
        st->SetFloat(hid, h); st->SetFloat(sid2, s); st->SetFloat(vid, v);
        ImGui::OpenPopup(sid);
    }
    bool changed = false;
    if (BeginDropdown(sid, ImRect(p, p + sz), Px(256))) {
        float h = st->GetFloat(hid, 0), s = st->GetFloat(sid2, 0), v = st->GetFloat(vid, 1);
        ImDrawList* d = ImGui::GetWindowDrawList();
        float W = ImGui::GetContentRegionAvail().x - Px(8), sq = W, bar = Px(18);
        ImVec2 o = ImGui::GetCursorScreenPos(); o.x += Px(4);
        // saturation / value square
        ImRect svb(o, ImVec2(o.x + W, o.y + sq));
        ImGui::ItemSize(ImVec2(W, sq));
        Hit sh = InteractRaw(svb, Sub(id, 30));
        if (sh.held) { s = Sat((ImGui::GetIO().MousePos.x - svb.Min.x) / W); v = 1.f - Sat((ImGui::GetIO().MousePos.y - svb.Min.y) / sq); changed = true; }
        float hr, hg, hb; ImGui::ColorConvertHSVtoRGB(h, 1, 1, hr, hg, hb);
        ImU32 hc = ImGui::ColorConvertFloat4ToU32(ImVec4(hr, hg, hb, 1));
        d->AddRectFilledMultiColor(svb.Min, svb.Max, IM_COL32_WHITE, hc, hc, IM_COL32_WHITE);
        d->AddRectFilledMultiColor(svb.Min, svb.Max, IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0), IM_COL32_BLACK, IM_COL32_BLACK);
        ImVec2 sp(svb.Min.x + s * W, svb.Min.y + (1.f - v) * sq);
        d->AddCircle(sp, Px(7), IM_COL32_WHITE, 0, Px(2)); d->AddCircle(sp, Px(8.5f), IM_COL32(0, 0, 0, 90), 0, Px(1));
        // hue bar
        ImGui::Dummy(ImVec2(0, Px(2)));
        ImVec2 ho = ImGui::GetCursorScreenPos(); ho.x += Px(4);
        ImRect hb2(ho, ImVec2(ho.x + W, ho.y + bar));
        ImGui::ItemSize(ImVec2(W, bar));
        Hit hh = InteractRaw(hb2, Sub(id, 31));
        if (hh.held) { h = Sat((ImGui::GetIO().MousePos.x - hb2.Min.x) / W); changed = true; }
        for (int i = 0; i < 6; i++) {
            float r0, g0, b0, r1, g1, b1; ImGui::ColorConvertHSVtoRGB(i / 6.f, 1, 1, r0, g0, b0); ImGui::ColorConvertHSVtoRGB((i + 1) / 6.f, 1, 1, r1, g1, b1);
            ImU32 c0 = ImGui::ColorConvertFloat4ToU32(ImVec4(r0, g0, b0, 1)), c1 = ImGui::ColorConvertFloat4ToU32(ImVec4(r1, g1, b1, 1));
            d->AddRectFilledMultiColor(ImVec2(hb2.Min.x + W * i / 6.f, hb2.Min.y), ImVec2(hb2.Min.x + W * (i + 1) / 6.f, hb2.Max.y), c0, c1, c1, c0);
        }
        d->AddCircleFilled(ImVec2(hb2.Min.x + h * W, hb2.GetCenter().y), bar * 0.5f, IM_COL32_WHITE);
        d->AddCircle(ImVec2(hb2.Min.x + h * W, hb2.GetCenter().y), bar * 0.5f, IM_COL32(0, 0, 0, 90), 0, Px(1));
        // alpha bar
        ImGui::Dummy(ImVec2(0, Px(2)));
        ImVec2 ao = ImGui::GetCursorScreenPos(); ao.x += Px(4);
        ImRect ab(ao, ImVec2(ao.x + W, ao.y + bar));
        ImGui::ItemSize(ImVec2(W, bar));
        Hit ah = InteractRaw(ab, Sub(id, 32));
        if (ah.held) { col[3] = Sat((ImGui::GetIO().MousePos.x - ab.Min.x) / W); changed = true; }
        Checker(d, ab.Min, ab.Max);
        ImGui::ColorConvertHSVtoRGB(h, s, v, hr, hg, hb);
        ImU32 c0 = ImGui::ColorConvertFloat4ToU32(ImVec4(hr, hg, hb, 0)), c1 = ImGui::ColorConvertFloat4ToU32(ImVec4(hr, hg, hb, 1));
        d->AddRectFilledMultiColor(ab.Min, ab.Max, c0, c1, c1, c0);
        d->AddCircleFilled(ImVec2(ab.Min.x + col[3] * W, ab.GetCenter().y), bar * 0.5f, IM_COL32_WHITE);
        d->AddCircle(ImVec2(ab.Min.x + col[3] * W, ab.GetCenter().y), bar * 0.5f, IM_COL32(0, 0, 0, 90), 0, Px(1));
        char hex[40]; snprintf(hex, sizeof(hex), "#%02X%02X%02X   A %d%%", (int)(hr * 255 + .5f), (int)(hg * 255 + .5f), (int)(hb * 255 + .5f), (int)(col[3] * 100 + .5f));
        ImGui::Dummy(ImVec2(0, Px(4)));
        ImGui::PushStyleColor(ImGuiCol_Text, P.textSecondary);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + Px(4));
        ImGui::TextUnformatted(hex);
        ImGui::PopStyleColor();
        if (changed) { col[0] = hr; col[1] = hg; col[2] = hb; st->SetFloat(hid, h); st->SetFloat(sid2, s); st->SetFloat(vid, v); }
        EndDropdown();
    }
    return changed;
}

// Key bind button: click, then press any key (Esc cancels, Backspace clears).
inline bool KeyBind(const char* sid, int* key) {
    Context& c = Ctx(); const Palette& P = c.pal;
    ImGuiID id = ImGui::GetID(sid);
    ImGuiStorage* st = ImGui::GetStateStorage();
    bool wait = st->GetBool(id, false);
    ImVec2 p = ImGui::GetCursorScreenPos(); ImVec2 sz(Px(132), Px(32));
    Hit hit = Interact(ImRect(p, p + sz), id);
    bool changed = false;
    if (hit.pressed && !wait) { st->SetBool(id, true); wait = true; }
    else if (wait) {
        if (ImGui::IsKeyPressed(ImGuiKey_Escape, false)) { st->SetBool(id, false); wait = false; }
        else if (ImGui::IsKeyPressed(ImGuiKey_Backspace, false)) { *key = 0; changed = true; st->SetBool(id, false); wait = false; }
        else for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_NamedKey_END; k++) {
            if (k == ImGuiKey_MouseLeft || k == ImGuiKey_MouseWheelX || k == ImGuiKey_MouseWheelY) continue;
            if (ImGui::IsKeyPressed((ImGuiKey)k, false)) { *key = k; changed = true; st->SetBool(id, false); wait = false; break; }
        }
    }
    float hv = Animate(Sub(id, 0), hit.hovered ? 1.f : 0.f), w = Animate(Sub(id, 1), wait ? 1.f : 0.f);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(p, p + sz, U32(ImLerp(P.control, P.controlHover, hv)), Px(4));
    float pulse = 0.55f + 0.45f * sinf(c.time * 6.f);
    dl->AddRect(p, p + sz, U32(ImLerp(P.stroke, P.accent, w), 1.f), Px(4), 0, ImMax(1.f, Px(1)) + w * Px(1) * pulse);
    const char* txt = wait ? "Press a key..." : (*key == 0 ? "None" : ImGui::GetKeyName((ImGuiKey)*key));
    ImVec2 ts = TextSize(c.fontBody, txt);
    DrawLabel(dl, c.fontBody, ImVec2(p.x + (sz.x - ts.x) * 0.5f, p.y + (sz.y - ts.y) * 0.5f), U32(wait ? P.accent : P.text), txt);
    return changed;
}

// Text box with the Windows 11 accent underline on focus.
inline bool InputText(const char* sid, char* buf, size_t size, float width = 0.f, const char* hint = "") {
    Context& c = Ctx(); const Palette& P = c.pal;
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, Px(4));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(Px(12), (Px(32) - FontPx(c.fontBody)) * 0.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, ImMax(1.f, Px(1)));
    ImGui::PushStyleColor(ImGuiCol_Border, P.stroke);
    ImGui::SetNextItemWidth(width > 0 ? Px(width) : ImGui::GetContentRegionAvail().x);
    bool ch = ImGui::InputTextWithHint(sid, hint, buf, size);
    ImGuiID id = ImGui::GetItemID();
    float a = Animate(Sub(id, 40), ImGui::IsItemActive() ? 1.f : 0.f, 20.f);
    ImVec2 mn = ImGui::GetItemRectMin(), mx = ImGui::GetItemRectMax();
    ImGui::GetWindowDrawList()->AddLine(ImVec2(mn.x + Px(4), mx.y - Px(1)), ImVec2(mx.x - Px(4), mx.y - Px(1)), U32(P.accent, a), ImMax(2.f, Px(2)));
    ImGui::PopStyleColor(); ImGui::PopStyleVar(3);
    return ch;
}

// Win11-style tooltip; call right after the item it describes.
inline void Tooltip(const char* text) {
    if (!ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay)) return;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(Px(10), Px(7)));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, Px(6));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, ImMax(1.f, Px(1)));
    ImGui::PushStyleColor(ImGuiCol_Border, Pal().stroke);
    if (ImGui::BeginTooltip()) { ImGui::PushFont(Ctx().fontSmall); ImGui::TextUnformatted(text); ImGui::PopFont(); ImGui::EndTooltip(); }
    ImGui::PopStyleColor(); ImGui::PopStyleVar(3);
}

// ----------------------------------------------------------------------------------------------
// Settings cards (icon + title + description + control on the right), sections
// ----------------------------------------------------------------------------------------------
inline void CardBegin(Icon ic, const char* title, const char* desc) {
    Context& c = Ctx(); const Palette& P = c.pal;
    ImGui::PushID(title);
    bool hasDesc = desc && *desc;
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImVec2 sz(ImGui::GetContentRegionAvail().x, Px(hasDesc ? 64.f : 52.f));
    c.cardStart = p; c.cardSize = sz;
    float hv = Animate(ImGui::GetID("##cardhv"), ImGui::IsMouseHoveringRect(p, p + sz) && ImGui::IsWindowHovered() ? 1.f : 0.f, 14.f);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(p, p + sz, U32(ImLerp(P.card, P.cardHover, hv)), Px(4));
    dl->AddRect(p, p + sz, U32(P.stroke), Px(4));
    float tx = p.x + Px(16);
    if (ic != Icon::None) { DrawIcon(dl, ic, ImVec2(p.x + Px(32), p.y + sz.y * 0.5f), Px(20), U32(P.text)); tx = p.x + Px(58); }
    float bh = FontPx(c.fontBody), sh = FontPx(c.fontSmall);
    if (hasDesc) {
        float total = bh + Px(2) + sh, y = p.y + (sz.y - total) * 0.5f;
        DrawLabel(dl, c.fontBody, ImVec2(tx, y), U32(P.text), title);
        DrawLabel(dl, c.fontSmall, ImVec2(tx, y + bh + Px(2)), U32(P.textSecondary), desc);
    } else DrawLabel(dl, c.fontBody, ImVec2(tx, p.y + (sz.y - bh) * 0.5f), U32(P.text), title);
}
inline ImVec2 CardCtrl(float w, float h) { Context& c = Ctx(); return ImVec2(c.cardStart.x + c.cardSize.x - Px(16) - w, c.cardStart.y + (c.cardSize.y - h) * 0.5f); }
inline void CardEnd() { Context& c = Ctx(); ImGui::SetCursorScreenPos(c.cardStart); ImGui::Dummy(c.cardSize); ImGui::PopID(); }

inline bool ToggleCard(Icon ic, const char* title, const char* desc, bool* v) {
    CardBegin(ic, title, desc); ImGui::SetCursorScreenPos(CardCtrl(Px(40), Px(20)));
    bool ch = Toggle("##t", v); CardEnd(); return ch;
}
inline bool SliderFloatCard(Icon ic, const char* title, const char* desc, float* v, float mn, float mx, const char* fmt = "%.1f") {
    CardBegin(ic, title, desc); ImGui::SetCursorScreenPos(CardCtrl(Px(220 + 12 + 60), Px(24)));
    bool ch = SliderFloat("##s", v, mn, mx, fmt); CardEnd(); return ch;
}
inline bool SliderIntCard(Icon ic, const char* title, const char* desc, int* v, int mn, int mx) {
    CardBegin(ic, title, desc); ImGui::SetCursorScreenPos(CardCtrl(Px(220 + 12 + 60), Px(24)));
    bool ch = SliderInt("##s", v, mn, mx); CardEnd(); return ch;
}
inline bool ComboCard(Icon ic, const char* title, const char* desc, int* cur, const char* const* items, int n) {
    CardBegin(ic, title, desc); ImGui::SetCursorScreenPos(CardCtrl(Px(180), Px(32)));
    bool ch = Combo("##c", cur, items, n); CardEnd(); return ch;
}
inline bool MultiComboCard(Icon ic, const char* title, const char* desc, const char* const* items, bool* sel, int n) {
    CardBegin(ic, title, desc); ImGui::SetCursorScreenPos(CardCtrl(Px(180), Px(32)));
    bool ch = MultiCombo("##m", items, sel, n); CardEnd(); return ch;
}
inline bool ColorCard(Icon ic, const char* title, const char* desc, float col[4]) {
    CardBegin(ic, title, desc); ImGui::SetCursorScreenPos(CardCtrl(Px(56), Px(28)));
    bool ch = ColorEdit("##col", col); CardEnd(); return ch;
}
inline bool KeyBindCard(Icon ic, const char* title, const char* desc, int* key) {
    CardBegin(ic, title, desc); ImGui::SetCursorScreenPos(CardCtrl(Px(132), Px(32)));
    bool ch = KeyBind("##k", key); CardEnd(); return ch;
}
inline bool ButtonCard(Icon ic, const char* title, const char* desc, const char* btn, Kind kind = Kind::Standard) {
    float w = TextSize(Ctx().fontBody, btn).x + Px(32);
    CardBegin(ic, title, desc); ImGui::SetCursorScreenPos(CardCtrl(w, Px(32)));
    bool ch = Button(btn, kind); CardEnd(); return ch;
}
// Static read-only value on the right (status text, etc.)
inline void ValueCard(Icon ic, const char* title, const char* desc, const char* value, bool good = false) {
    Context& c = Ctx();
    CardBegin(ic, title, desc);
    ImVec2 ts = TextSize(c.fontBody, value); ImVec2 pos = CardCtrl(ts.x, ts.y);
    DrawLabel(ImGui::GetWindowDrawList(), c.fontBody, pos, U32(good ? c.pal.accent : c.pal.textSecondary), value);
    CardEnd();
}

// Collapsible section. If it returns true, call EndSection() after the content.
inline bool BeginSection(Icon ic, const char* title, const char* desc, bool defaultOpen = true) {
    Context& c = Ctx(); const Palette& P = c.pal;
    ImGuiID id = ImGui::GetID(title);
    ImGuiStorage* st = ImGui::GetStateStorage();
    bool open = st->GetBool(Sub(id, 50), defaultOpen);
    CardBegin(ic, title, desc);
    ImVec2 s = c.cardStart; ImVec2 sz = c.cardSize;
    ImGui::SetCursorScreenPos(s);
    Hit hit = InteractRaw(ImRect(s, s + sz), Sub(id, 51));
    if (hit.pressed) { open = !open; st->SetBool(Sub(id, 50), open); }
    float t = Animate(Sub(id, 52), open ? 1.f : 0.f, 16.f);
    float flip = 1.f - 2.f * t;                                        // chevron rotates down -> up
    ImVec2 cc(s.x + sz.x - Px(28), s.y + sz.y * 0.5f), q = ImVec2(Px(6), Px(3)) * flip;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 pts[3] = { ImVec2(cc.x - Px(5), cc.y - q.y), ImVec2(cc.x, cc.y + q.y), ImVec2(cc.x + Px(5), cc.y - q.y) };
    dl->AddPolyline(pts, 3, U32(P.textSecondary), 0, ImMax(1.f, Px(1.5f)));
    CardEnd();
    if (open) ImGui::Indent(Px(16));
    return open;
}
inline void EndSection() { ImGui::Unindent(Px(16)); }

// ----------------------------------------------------------------------------------------------
// Toasts (bottom-right, slide in / fade out)
// ----------------------------------------------------------------------------------------------
inline void PushToast(const char* text, ToastType type = ToastType::Info) {
    Context& c = Ctx(); c.toasts.push_back({ text, type, c.time });
    if (c.toasts.size() > 5) c.toasts.erase(c.toasts.begin());
}

inline void RenderToasts() {
    Context& c = Ctx(); const Palette& P = c.pal;
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    ImVec2 disp = ImGui::GetIO().DisplaySize;
    const float life = 3.5f;
    int idx = 0;
    for (size_t i = 0; i < c.toasts.size();) {
        ToastItem& t = c.toasts[i];
        float age = c.time - t.born;
        if (age > life) { c.toasts.erase(c.toasts.begin() + i); continue; }
        float in = EaseOut(age / 0.3f), out = Sat((life - age) / 0.3f);
        float w = Px(300), h = Px(56), m = Px(16);
        ImVec2 p(disp.x - w - m + (1.f - in) * (w + 2 * m), disp.y - m - (idx + 1) * (h + Px(8)) + idx * 0.f);
        float al = out;
        for (int k = 8; k >= 1; k--) {
            float ex = Px(k * 1.5f), kk = 1.f - k / 8.f;
            dl->AddRectFilled(ImVec2(p.x - ex, p.y - ex + Px(4)), ImVec2(p.x + w + ex, p.y + h + ex + Px(4)), U32(P.shadow, al * (0.12f * kk * kk + 0.01f)), Px(8) + ex);
        }
        dl->AddRectFilled(p, ImVec2(p.x + w, p.y + h), U32(P.popup, al), Px(8));
        dl->AddRect(p, ImVec2(p.x + w, p.y + h), U32(P.stroke, al * 1.5f), Px(8));
        ImVec4 ac = t.type == ToastType::Error ? P.danger : P.accent;
        ImVec2 ic(p.x + Px(28), p.y + h * 0.5f);
        dl->AddCircleFilled(ic, Px(12), U32(ac, al));
        DrawIcon(dl, t.type == ToastType::Success ? Icon::Check : (t.type == ToastType::Error ? Icon::Close : Icon::Info), ic, Px(12),
                 U32(t.type == ToastType::Error ? RGBA(255, 255, 255) : P.onAccent, al));
        DrawLabel(dl, c.fontBody, ImVec2(p.x + Px(52), p.y + (h - FontPx(c.fontBody)) * 0.5f), U32(P.text, al), t.text.c_str());
        idx++; i++;
    }
}

} // namespace Fluent
