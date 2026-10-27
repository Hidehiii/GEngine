# Focused regression checks

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
read image/FBX/YAML/include assets and owned background bytes, and keep Assets
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
