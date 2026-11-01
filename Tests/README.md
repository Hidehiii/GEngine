# Focused regression checks

## Ordered HDR effects

RenderPipelineAssets.py --post exercises P3 HDR camera composition followed by
ordered exposure/Reinhard effects. Use the native RenderPipeline executable and
isolated log directories as below. The runner provides 12 command slots in its
copied config. Require 18 invalid candidates, four pre-recording invalid live
snapshots, two same-instance live changes, six replacements and 11 numeric probes
per 120 frames; 6000 frames require 349 replacements/403 probes. Checks cover
the required `Post-process pass reuse passed` preparation regression as well as
empty/global/individual bypass, each effect, order, EV extremes, sRGB toe/curve,
resize, independent parameters and stable intermediate target identities/counts.
RGBA16F capability queries are also included in FrameGraphTriangle --msaa format
metadata checks. Debug/Release 120-frame and Debug 6000-frame P3 runs passed all
four variants, with P1/P2/default/MSAA regressions in both builds. Logs/CSV:
build/post-process/final-{Debug,Release}, final-stress-Debug and named regression
directories. See the P3 TODO and example README for the precise shader contract,
bounded memory observations, launch instructions and remaining limitations.

## Pipeline asset switching

P2 adds `--stack` to this runner. Build the native RenderPipeline example and
run the same command below with `--stack --logs build/camera-stacks/Debug`.
Require 16 invalid-stack rejections, three pre-recording binding failures,
two same-instance live binding changes, seven replacements and eleven
four-quadrant probes of both raw and sampled output per 120-frame run.
The eight cases cover depth LOAD/CLEAR, ordered UI, viewport/scissor, whole-color
clear, disabled parents and 4x resized targets. `--frames 6000` repeats them for
399 replacements/403 probes. See the example README for the visible result,
native readback origin handling and verification status.
Final-source P2 Debug/Release 120-frame and Debug 6000-frame runs passed all
four variants; logs/CSV: build/camera-stacks/final-{Debug,Release} and
final-stress-Debug. P1, original C++ and rebuilt MSAA regressions also passed
both configurations; see the P2 TODO for exact evidence and remaining limits.

Build the native RenderPipeline target, then run:

```bat
python -B Tests/RenderPipelineAssets.py dist/pipeline-lifecycle/windows-x86_64/Debug/RenderPipeline/RenderPipeline.exe --logs build/pipeline-assets/Debug
```

Repeat with Release. The four isolated backend runs enable asset mode, require
18 rollback checks, seven switches and nine numeric pixel checks in 120 frames,
and reject native warnings/errors or incomplete shutdown. Use `--backend` for
one variant, or `--frames 6000` for 399 switches and 401 pixel checks. Outputs
are runtime.log/memory.csv plus the example's development round-trip GAsset.
The 120-second timeout only stops the runner-owned process. Original backend
configurations stay unchanged. See Example/RenderPipeline/README.md and the P1
TODO for schema scope, expected output and verification status.
Final CMake GE_WITH_MONO=OFF Debug/Release runs passed all four backend variants
at 120 frames, plus final Debug 6000-frame runs. Logs: build/pipeline-assets/
final-{Debug,Release} and final-stress-Debug. Invalid HLSL intentionally logs a
compiler error; the test verifies rollback and subsequent GPU output rather than
treating this expected diagnostic as a native validation failure.

## Rendering foundation and lifetime controls

Build the native FrameGraphTriangle target, then run from the repository root:

```bat
python Tests/RenderingFoundation.py dist/pipeline-lifecycle/windows-x86_64/Debug/FrameGraphTriangle/FrameGraphTriangle.exe --logs build/foundation/Debug
python Tests/RenderingFoundation.py dist/pipeline-lifecycle/windows-x86_64/Debug/FrameGraphTriangle/FrameGraphTriangle.exe --mode lifetime --logs build/foundation/lifetime
```

Replace the executable with your actual build output; Release uses the same
checks. The first command runs 120 frames each on OpenGL, Vulkan shared, D3D12
with the debug layer, and Vulkan dedicated. The second runs matched 30,000-frame
D3D12 controls with the layer on/off and replacement on/off. `--frames` overrides
the bounded count (120..60000). Config and assets are copied to isolated run
directories; original backend settings stay unchanged. Each run must pass the
attachment preflight marker, three frame-119 GPU readbacks and normal shutdown;
native validation errors and Vulkan validation warnings fail the run. Startup checks cover malformed
portable/legacy attachments, device limits and rejection before any allocation.
runtime.log and memory.csv record output, handles and private bytes; optional
native lifetime traces/live-object messages are diagnostic evidence, not leak
proof. Reports occur while context-owned objects still exist. A 120-second
timeout kills only the runner's own process. No desktop automation or elevated
system profiler configuration is used.
Trend summaries omit the first three seconds and final teardown second;
process-exit samples may show zero handles and must not be used as a plateau.

Add `--msaa` to the native matrix command to check all attachment-format
queries, supported format creation and repeated 1x/2x/4x/8x MRT/depth resolve
pixel readbacks. The runner requires three `MSAA matrix passed` records at
frames 0/30/60 in addition to the standard readbacks and exact frame-limit
shutdown. `--backend OpenGL|Vulkan|D3D12|VulkanDedicated` selects one matrix
case for focused diagnosis. See the FrameGraphTriangle README for unsupported
depth-resolve/preserved-load semantics. No UI automation is used.

The MSAA matrix passed in CMake Debug/Release on OpenGL, Vulkan shared/
dedicated and D3D12 (real debug layer), plus Debug 6000-frame runs. The final
logs contain three 24-case matrices per backend and all standard readbacks.
MSAA mode additionally rejects D3D12 warnings and GL_INVALID diagnostics.

## Scene component assets and base data

Build and run SceneRenderHelpers with GENGINE_SCENE_HELPER_FRAME_LIMIT=120.
Its ComponentChecks.cpp exercises GameObject base data/64-bit identity, the three
retained Component classes, Camera settings, shared asset/material ownership,
scene copy/duplication/hierarchy/deletion, persistent asset/material shader/name
roundtrips and explicit invalid/removed-component rejection. It also checks a
malformed FBX error without replacing the previous mesh. Run Debug and Release
on each available backend; success must include the component-check marker,
replacement markers and normal 120-frame shutdown.

Tests/EditorSceneViewport.py additionally checks actual sampled image pixels,
real Worker recording, sized targets and removed-component rejection. It uses
Python only as a test driver; component implementations and Engine Tools are C++.
Full material-property serialization, camera passes and manual window checks
are outside these regressions; see TODO/SceneAndPhysics.md.

## D3D12 cross-queue UAV ordering

After building/exporting FrameGraphTriangle:

```bat
python Tests/D3D12QueueRegression.py <packaged-root-exe> --logs build/d3d12-queue-check
```

Run against Debug and Release packages separately. This uses the real runtime
and debug layer, covering repeated executions sharing ComputeFirstColor, normal
resource replacement, no replacement, an unsubmitted graphics recording and two
synthetic resizes. Every case must pass three readbacks, exit 0, emit the active
logger marker and contain no D3D12 warnings/errors. `--stress-seconds` accepts
4-45 (default 15); every owned process has a 60-second deadline. Kernel handle
and private-byte samples are written to CSV, not treated as proof of ownership
or a flat-memory assertion. Config/Config.ini is restored on completion/failure.
Use a generated test package without concurrent users. Manual visible output,
OS resize and complete native allocation attribution remain separate checks.

## YAML GAsset envelope

The dependency-free GAsset regression compiles the production envelope parser
and validates text and Base64 payloads, expected types, schema versions, field
sets and unsupported encodings. From the repository root:

```bat
cmake -S Tests/GAsset -B build/gasset-tests -G "Visual Studio 17 2022" -A x64
cmake --build build/gasset-tests --config Release --parallel 4
ctest --test-dir build/gasset-tests -C Release --output-on-failure
```

Repeat with Debug. This check does not replace backend texture/model/shader
loading or packaged-runtime verification.

## Native resource pack

The dependency-free resource regression compiles the production Windows archive
implementation and exercises directory/archive equality, concurrent owned reads,
immutable backing handles, replacement lifetime, failed-mount preservation,
directory enumeration, read-only write rejection and malformed archives. It also
checks handle counts across 100 remounts. From the repository root:

```bat
cmake -S Tests/ResourcePack -B build/resource-pack-tests -G "Visual Studio 17 2022" -A x64
cmake --build build/resource-pack-tests --config Release --parallel 4
ctest --test-dir build/resource-pack-tests -C Release --output-on-failure
```

Repeat with Debug for the other configuration. Fixtures stay inside that build
directory; no source assets/configuration are changed.

After exporting FrameGraphTriangle, run:

```bat
python Tests/ResourcePackRuntime.py build/gpak-direct-package/DirectPack.exe --logs build/gpak-runtime-logs
```

Use a generated test package without concurrent users. The check temporarily
changes its external Config/Config.ini backend and restores original config/pack
bytes in a finally block. Config.ini must be absent from the archive. Each backend
must render 120 frames twice, reuse nonempty Cache/Shaders/<API>/ files unchanged,
and pass three readbacks,
read image/FBX/YAML/include GAsset payloads and owned background bytes, and keep Assets
absent from the game root. An edited literal HLSL include must invalidate cache.
Corrupt/missing packs and missing external configuration must fail before
Application construction. Logs record each backend independently; this does not
establish manual visual/resize behavior or long-duration memory stability.

## Public logging header

`LoggingHeader.cpp` includes the public engine logging header first and checks
formatting without an engine PCH or DLL. On MSVC Debug it also requires the
normal iterator debug level of 2; do not disable iterator checks to pass this
test. This covers the historical spdlog/fmt checked_array_iterator build failure.

From the repository root in the selected Visual Studio x64 developer prompt:

```bat
if not exist .integration\logging-test mkdir .integration\logging-test
cl /nologo /std:c++17 /EHsc /MDd /D_DEBUG /DGE_DEBUG /I GEngine\include /I GEngine\GEngine-ThirdParty\spdlog\include Tests\LoggingHeader.cpp /Fo.integration\logging-test\Debug.obj /Fe.integration\logging-test\Debug.exe
if errorlevel 1 exit /b 1
.integration\logging-test\Debug.exe
```

For Release, replace `/MDd /D_DEBUG /DGE_DEBUG` with
`/MD /DNDEBUG /DGE_RELEASE` and use `Release.obj`/`Release.exe` output names.
Expected result: compiler and executable exit code 0. Existing vendored fmt
deprecation warnings are distinct from missing-type errors. This focused check
does not replace an engine/example build or the three-backend rendering tests.

## Local log save build switch

`LocalLogSave.cpp` links the engine's `Log.cpp` directly and verifies that
`GE_ENABLE_LOCAL_LOG_SAVE=1` creates the file and `=0` keeps console logging
without creating it. Run each binary from a separate empty working directory;
the test writes `Logs/GEngine.log` only in the enabled case. Compile with the
same Debug/Release runtime flags as above, adding `GEngine/src/GEngine/Core/Log.cpp`,
`/I GEngine/src`, `/DGE_BUILD_DLL`, and `/DGE_ENABLE_LOCAL_LOG_SAVE=0` or `=1`.
This is a focused branch check, not a substitute for building the full engine.
Depth-resolve/LOAD follow-up: `RenderingFoundation.py --msaa` now additionally
requires numeric resolved-depth logs and exactly four cross-frame LOAD probes.
The example tests both depth conventions, explicit queried modes, min/max edge
reduction, preserved color/depth at frames 1/59/61/119, and target recreation at
60. Failed earlier fixture attempts remain in build/msaa-depth; use the final
verified directories recorded in TODO/Rendering/Phase4.md as acceptance evidence.
Final depth/LOAD acceptance logs: build/msaa-depth/verified-{Debug,Release} and
stress-Debug, each containing all four variants. Debug/Release totaled 1536
color cases, 1184 numeric depth probes and 432 min/max edge checks; stress added
6000 frames per variant with the same early matrices and normal shutdown.
