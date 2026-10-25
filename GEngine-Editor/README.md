# GEngine Editor

## New Qt editor: engine preview prototype

The independent Qt Host now starts a GEngine render child automatically. It
displays a real GPU-rendered cyan triangle and streams engine logs into Console.
This first connection uses a fixed 160x90 RGBA8 preview at roughly 2 FPS, with
synchronous GPU readback confined to the child. Scene/Play/Gizmo and `.GEScene`
editing remain future work. The Host can edit/undo/redo and atomically save a
separate `.gedraft` note document even after the child crashes.

From the repository root in PowerShell (VS C++ tools, CMake and Python required):

```powershell
.\BuildScripts\BuildQt6.ps1 -Configuration Release
.\BuildScripts\BuildQtEditor.ps1 -Configuration Release -Test
.\GEngine-Editor\RunQtEditor.bat Release
```

If Python is not on PATH, pass `-Python` with its executable's absolute path to
BuildQt6.ps1. The script initializes VS developer tools in an isolated child
environment and uses bundled Ninja. SDK artifacts go to
`build/qt6/windows-x86_64/Release` and `dist/qt6/windows-x86_64/Release`; upstream
source stays clean. Repeat with Debug for its matching SDK/Host. Do not build
the same configuration concurrently.

For Premake, after building the SDK run
`BuildQtEditor.ps1 -Configuration Release -Premake -Test` (or Debug). It generates
optional Qt targets with `--qt6-sdk=<root-containing-Debug-and-Release>`, then
builds/deploys them. `-QtSdk <configuration-prefix>` overrides the CMake SDK;
Premake derives its paired SDK root from the prefix's parent. Default engine/game
builds still need no Qt. Root CMake also has opt-in GE_BUILD_QT_EDITOR=ON.

CMake/Ninja output:
`dist/qt-editor/windows-x86_64/<config>/GEngine-EditorHost/GEngine-EditorHost.exe`.
Its EngineWorker is in the adjacent target directory; `--worker <exe>` can
override that owned child executable. `--self-test` selects the separate CPU
DiagnosticWorker unless overridden. Premake uses standard
`dist/premake-vs2022/windows-x86_64/<config>/<target>` paths. Qt libraries,
Windows/offscreen plugins and the existing Noto font/OFL deploy beside Host;
the machine's matching MSVC runtime is required.

Expected: menus, docked hierarchy/draft/console, cyan GPU triangle in Viewport and
engine startup/frame logs. Use Engine > Start / restart worker, then inject native crash, hang
or malformed pipe data. Host remains responsive, retains draft/history, allows
Save and can stop/restart its child. Engine Ready follows device and pipeline
initialization. File > New/Open/Save handles `.gedraft`; `.GEScene` is unsupported.
The worker's own presentation window is hidden. Change GraphicsAPI in its deployed
Config.ini (1=OpenGL, 2=Vulkan, 3=D3D12), then restart; the legacy editor's local
configuration is unaffected. Host resizing scales the fixed preview; it does not
resize the child render target yet.

All four panels, including Viewport, can dock in any area, float, tab together
and close without losing contents. Reopen panels from View; View > Reset layout
restores the default arrangement. The worker toolbar can also move/float/hide.
Layout state version 2 saves these positions; incompatible older states reset
to defaults without touching the legacy editor's imgui.ini.

`BuildQtEditor.ps1 -ProtocolOnly -Configuration Debug -Test` checks protocol/history
without Qt. Full `-Test` also runs the real engine check using the deployed worker's
current backend configuration and writes engine-tests.log. It runs the diagnostic Host offscreen (CTest timeout 35 seconds),
covering missing child, handshake/image, native crash, hang, corrupt pipe,
restart, save failure and draft round trip. Tests use temporary files and do not
overwrite user layout. To exercise the real engine, run Host with
`--self-test --engine-check --worker <absolute-EngineWorker-exe>` from any directory.
It checks at least two GPU frames, expected triangle pixels, engine logs and the
same crash/hang/restart/save sequence (120-second internal deadline). Add
`--self-test-image <absolute-png>` to capture the first working preview.

Verification evidence for this milestone is recorded in TODO/EditorArchitecture.md:
Debug/Release SDK and both generators' builds/tests, plus a native Windows Release
recovery run. The normal Qt window image was reviewed for layout and readable text.
The fixed preview has separate OpenGL/Vulkan/D3D12 Debug/Release checks. Full
Scene/Play rendering and manual input/resize checks remain open.

## Legacy editor removed

The ImGui editor, its private panels/GizmoController, assets, Mono deployment,
old launcher and build target were removed on 2026-10-25. Git history retains
that implementation. Only the Qt Host/Worker editor above is supported now.
Engine/game Dear ImGui and the engine EditorCamera remain available. The unused
ImGuizmo source submodule is retained in the shared dependency repository but
has no build consumers. Scene/Play/Qt Gizmo migration is still incomplete.
