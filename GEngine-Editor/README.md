# GEngine Editor

Open a `.gproject`, then choose **Project > Import resource...**. Select an image
(PNG/JPEG/TGA/PPM), FBX mesh, TTF/OTF font, shader or HLSL include and choose a
folder inside the project's `Assets` directory (create `Assets` in the folder
picker for a project that does not have it). Import creates `<name>.gasset`,
preserves the source and selects the result in Assets. Existing names are rejected;
rename the source or choose another folder. Files are limited to 64 MiB. Import is
asynchronous; failures appear in Console and the status bar. Cancel either picker
to leave files unchanged. Import only packages source bytes; external mesh/shader
dependencies and format decoding are not validated. No Python is required.

Regression: `ctest --test-dir build/qt-editor/windows-x86_64/Release -R EditorImport --output-on-failure`
(use the Debug build directory for Debug). After import, also check the visible
selection/Inspector and source preservation. Import does not restart the viewport;
resize/render/shutdown behavior remains the existing Worker contract.

Console preserves engine log colors (trace white, debug cyan, info green, warning
bright yellow, error bright red, critical bright white on red). The editor worker
sends color codes over stderr; the Qt Console renders them without adding codes
to copied text. Host diagnostics stay neutral, HTML is literal, and restart resets
partial/color state while retaining history. History is capped at 300 text blocks
and each line at 16 Ki characters. This is a log viewer, not a terminal emulator.

## Live engine scene viewport (E4.1)

The independent Qt Host now starts a GEngine render child automatically. It
displays a Scene through the shared engine SceneRenderer and streams engine logs
into Console. Frame requests run independently of heartbeat at up to about 30 FPS.
The target follows visible viewport size and DPI, capped to fit 1280x720 while
preserving aspect ratio. Hide the viewport to pause frame requests. RGBA8 frames
are split into at most 64 KiB packets, with one request/frame in flight; stale
resize results are discarded. Synchronous GPU readback remains in the child,
so achieved FPS depends on backend, target size and pipe throughput.

File > Open scene for rendering selects a `.gscene` and restarts the child with
that document. Scenes now use GameObject base data and ImageComponent,
StaticMeshComponent and CameraComponent. Image components sample image assets;
mesh/image asset paths and material shader/name round-trip through the serializer.
The Worker supplies its default image material only when none is assigned.
Automatic camera passes and arbitrary material-property serialization remain
unavailable. Removed script/physics component keys fail explicitly in Console.
Scene transforms are not editable from the Host.
File > Show engine sample scene restores two real scene objects (red quad and
moving cyan quad); its label identifies it as the sample. A static supported
file is provided at `Worker/Assets/Scenes/ColorScene.gscene`.
Worker runtime resources use `.gasset`, and scenes use `.gscene`. The Host's
bundled UI font is also deployed as `.gasset` and decoded to a temporary TTF for
Qt font registration.
Full Scene editing, camera/lighting, Play/Gizmo and asynchronous readback remain
open. The Host can edit/undo/redo and atomically save a
separate `.gedraft` note document even after the child crashes.

Build the native workflow tool first (VS C++ tools, CMake and Python are required):

```bat
BuildScripts\GenerateProjectsCMake.bat vs2022
cmake --build build\cmake-vs2022\windows-x86_64 --config Release --target GEngineTools --parallel 6
dist\cmake-vs2022\windows-x86_64\Release\GEngineTools\GEngineTools.exe qt-sdk --configuration Release
dist\cmake-vs2022\windows-x86_64\Release\GEngineTools\GEngineTools.exe qt-editor --configuration Release --test
dist\cmake-vs2022\windows-x86_64\Release\GEngineTools\GEngineTools.exe run-qt-editor Release CMake
```

The full `qt-editor` command installs the official Mono 6.10.0.87 SDK before
building the editor/engine targets. `--protocol-only` does not download it.

RunQtEditor selects the newest executable for the requested configuration across
CMake and Premake outputs. Use `RunQtEditor.bat Release Premake` (or `CMake`)
to select one explicitly; add `-PrintPath` to inspect the selection without launching.
No arguments means Release/Auto. A missing selected build returns an error.

If Python is not on PATH, pass `--python` with its executable's absolute path to
`qt-sdk`. The native tool initializes VS developer tools in an isolated child
environment and uses bundled Ninja. SDK artifacts go to
`build/qt6/windows-x86_64/Release` and `dist/qt6/windows-x86_64/Release`; upstream
source stays clean. Repeat with Debug for its matching SDK/Host. Do not build
the same configuration concurrently.

For Premake, after building the SDK run
`GEngineTools.exe qt-editor --configuration Release --premake --test` (or Debug). It generates
optional Qt targets with `--qt6-sdk=<root-containing-Debug-and-Release>`, then
builds/deploys them. `--qt-sdk <configuration-prefix>` overrides the CMake SDK;
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

Expected: menus, docked hierarchy/draft/console, two scene quads in Viewport and
engine startup/frame logs. Use Engine > Start / restart worker, then inject native crash, hang
or malformed pipe data. Host remains responsive, retains draft/history, allows
Save and can stop/restart its child. Engine Ready follows device and pipeline
initialization. File > New/Open/Save draft handles `.gedraft`; scene opening is read-only.
The worker's own presentation window is hidden. Change GraphicsAPI in its deployed
Config.ini (1=OpenGL, 2=Vulkan, 3=D3D12), then restart; the legacy editor's local
configuration is unaffected. Host resizing replaces the child render target;
old GPU use completes before replacement through the preceding readback.

All six panels, including Viewport, Assets and Inspector, can dock in any area, float, tab together
and close without losing contents. Reopen panels from View; View > Reset layout
restores the default arrangement. The worker toolbar can also move/float/hide.
Layout state version 2 saves these positions; incompatible older states reset
to defaults without touching the legacy editor's imgui.ini.

`GEngineTools.exe qt-editor --protocol-only --configuration Debug --test` checks protocol/history
without Qt. Full `--test` also runs the real engine check using the deployed worker's
current backend configuration and writes engine-tests.log. It runs the diagnostic Host offscreen (CTest timeout 35 seconds),
covering missing child, handshake/image, native crash, hang, corrupt pipe,
restart, save failure and draft round trip. Tests use temporary files and do not
overwrite user layout. To exercise the real engine, run Host with
`--self-test --engine-check --worker <absolute-EngineWorker-exe>` from any directory.
It checks at least 12 GPU frames, expected scene pixels, target resize,
pause/resume, opening a scene and returning to the sample, engine logs and the
same crash/hang/restart/save sequence (120-second internal deadline). Add
`--self-test-image <absolute-png>` to capture the first working preview.

Verification evidence for this milestone is recorded in TODO/EditorArchitecture.md:
Debug/Release SDK and both generators' builds/tests, plus a native Windows Release
recovery run. The normal Qt window image was reviewed for layout and readable text.
`python Tests/EditorSceneViewport.py <worker.exe> --backend 1|2|3 --logs <folder>`
checks continuous transform-dependent pixels without heartbeat, full-size chunking,
40 target replacements (including odd dimensions), pause without credit, independent
heartbeat, a Unicode scene path, image orientation and explicit unsupported-component
failure. It preserves the deployed backend setting and records memory/handle samples.
Full Scene/Play rendering and manual input/resize checks remain open.

## Document tabs (E1.2)

The default docking layout shows Hierarchy/Assets on the left, Viewport/Console
in the middle and Documents/Inspector as a tab group on the right. Actual dock
group tabs use dark browser-style rounded tabs, an active highlight, X to hide
a panel and + at the end to add/reopen a panel in that group. There is no extra
global navigation strip. Adding an existing panel moves it into the selected group.
Drag dock titles or group tabs to float, split, group and redock panels;
double-click a dock title to toggle floating. Single panels keep their dock title.
Layout version 4 resets older saved layouts once; new dock arrangements persist.
The native system title bar remains. Qt saves group tab order with its dock layout.
Dock tabs use compact vertical spacing and 20x20 controls (about 26 logical pixels high).
The separate document strip lives inside Documents.
Use + / Ctrl+T / Ctrl+N to add a draft; File > Open draft opens a file. Documents
shows the active document below its document strip. Dock panels are shared.
Drag tabs to reorder; each keeps its own text, undo/redo history, path and `*`
dirty marker. Save applies to the active tab. Opening an already-open path
focuses its existing tab and preserves unsaved edits. Save As cannot overwrite
a file belonging to another open tab. Up to 32 draft tabs can be open.

Close a tab with its X or Ctrl+W. Save/Discard/Cancel is handled for that tab;
closing the last one creates a fresh draft. Exiting confirms every dirty tab
before stopping the worker. Cancel retains all tabs and restores the previous
active tab; saves already completed during confirmation remain saved.
Worker failure preserves inactive documents too. Documents can float as one
dock; individual tabs cannot detach into separate windows yet. Tabs edit
`.gedraft` notes, not `.gscene`, and are not restored after restarting the editor.

## Project workspace (E1.1)

The Assets tree lists every directory but only `.gasset` and `.gscene` files;
raw source formats must be imported before they appear in Assets.

Use File > Open project folder to browse a directory in Assets. Select a file or
folder to see its relative path, type, size and modification time in Inspector.
Both panels dock/float/close and reopen through View; Reset layout places Assets
on the left and tabs Inspector with the draft. This is read-only browsing: no
resource execution, import, component editing or .gscene loading occurs.

An invalid project path preserves the current project and selection. Switching
or closing projects clears selection while retaining the draft, undo history
and independent engine preview. Deleted selections are cleared; a missing root
closes the project. The authoritative selection is a Core SelectionService entry
containing the project generation and a project-relative resource ID; Qt controls
only project that state. Symlinks are hidden and canonical outside-root selections
are rejected. Project paths are not saved between editor runs yet.

The recovery check creates temporary Unicode project folders and exercises open,
failed open, model selection/metadata, sibling-root rejection, file/root removal,
switch/close, six-panel docking and preservation through a worker crash. Symlink
creation is checked when the host permits it, otherwise logged as skipped.
Manual file-dialog interaction, large/network directories and long-run watcher
memory still need separate checks. Opening a project does not render its assets;
the viewport remains the independent GPU triangle preview.

## Legacy editor removed

The ImGui editor, its private panels/GizmoController, assets, Mono deployment,
old launcher and build target were removed on 2026-10-25. Git history retains
that implementation. Only the Qt Host/Worker editor above is supported now.
Engine/game Dear ImGui and the engine EditorCamera remain available. The unused
ImGuizmo source submodule is retained in the shared dependency repository but
has no build consumers. Scene/Play/Qt Gizmo migration is still incomplete.

## Project files and packaging

Use File > New project or Open project for `.gproject` files. Each project stores
its default scene in `settings.defaultScene`, relative to the project folder.
Project > Project settings lets you browse/save that scene; Cancel leaves settings
unchanged. Opening a project (including `--project` or the first `.gproject` argument)
automatically opens its default scene and replaces the previous project's scene.
New projects create `Assets/Scenes/Default.gscene` as an empty scene. Legacy projects
without settings and projects whose scene is missing show a blank viewport with
a diagnostic; configure a scene to resume, or open a scene manually. Unsupported
scene components still report Worker errors under the E4.1 capability limits.
Closing a project clears its scene. Manual scene opening does not change the saved
default. Settings are project metadata and contain **no packaging information**.
Project > Package project
collects output directory, executable name and optional logo image. It automatically
builds engine/game C++ sources in Release, converts the logo to an embedded Windows
icon, creates a .gpak and publishes EXE/pack/runtime DLLs directly to that folder.
Console reports build progress/errors; Cancel stops the owned compiler tree. These
three values are local preferences and never rewrite the project file.
Opening a plain folder remains a read-only browsing operation.

`--project-settings-self-test` checks the settings dialog's Save/Cancel/error path,
failed atomic save, path validation, project switching, missing/legacy descriptors,
and new-project defaults. Add `--engine-check --worker <EngineWorker.exe>` for real
scene loading; a startup `--project <file>` or first `.gproject` argument also checks
that the initial project scene was delivered. Native export metadata compatibility
is checked with `python Tests/ProjectSettingsMetadata.py <GEngineTools.exe> --logs <folder>`.

Open `Example/FrameGraphTriangle/FrameGraphTriangle.gproject` for a ready project
description; no prebuilt EXE is required. New projects receive a minimal src/Game.cpp.
`--project <file>`
or a first `.gproject` argument opens one at startup; no OS file association is
installed. Assets/Licenses are merged into an uncompressed, unencrypted
.gpak, excluding Assets/Cache. The native EXE validates/mounts it and reads assets
on demand. Config/Config.ini and Cache/Shaders/<API>/ are stored directly under
the game root, together with Logs/ and UserData/. They persist between launches
and are preserved by subsequent packaging; Config.ini is never inside the archive.
It runs without a PowerShell launcher. C#,
asset conversion and automatic native dependency discovery remain open.
See [the export guide](../Documentation/Guides/GameExport.md).
