# imgui-fluent-win11

Windows 11 (Fluent Design) look for Dear ImGui, plus a landscape Android demo.

- `fluent/fluent.h`: the whole theme + widgets, header-only. Drop it next to your ImGui and include it.
- `demo/demo_menu.h`: sample Visuals / Aim / Memory / Settings menu using it (UI only, placeholder values).
- `app/`: Android NativeActivity host. `ci/build.yml` is the GitHub Actions workflow that builds the APK (move it to `.github/workflows/`).

## Integrating into an existing menu

```cpp
Fluent::Init(dpiScale);            // once, after ImGui::CreateContext()
// every frame, after ImGui::NewFrame():
Fluent::NewFrame();
if (Fluent::BeginWindow("Menu", &open, pos, size)) { /* nav + content */ }
Fluent::EndWindow();
Fluent::RenderToasts();
```

Swap your existing checkboxes, sliders and combos for the `Fluent::` equivalents. Your variables and logic stay untouched.

## Notes

- Mica/Acrylic is approximated (opaque base + luminosity tint + hairline border). A true backdrop blur needs a render-to-texture pass over your scene.
- Icons are drawn as vectors, so they stay sharp at any scale. Font defaults to the system Roboto; pass a Segoe UI Variable .ttf to `Fluent::Init` for an exact Windows match.
- Theme change cross-fades every colour. Disable all motion with `Fluent::SetReduceMotion(true)`.
