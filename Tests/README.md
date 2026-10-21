# Focused regression checks

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
