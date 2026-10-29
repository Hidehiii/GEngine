# Focused regression checks

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
