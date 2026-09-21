# Focused regression checks

## Public logging header

`LoggingHeader.cpp` includes the public engine logging header first and checks
formatting without an engine PCH or DLL. On MSVC Debug it also requires the
normal iterator debug level of 2; do not disable iterator checks to pass this
test. This covers the historical spdlog/fmt checked_array_iterator build failure.

From the repository root in the selected Visual Studio x64 developer prompt:

```bat
if not exist .integration\logging-test mkdir .integration\logging-test
cl /nologo /std:c++17 /EHsc /MDd /D_DEBUG /DGE_DEBUG /I GEngine\include /I GEngine\vendor\spdlog\include Tests\LoggingHeader.cpp /Fo.integration\logging-test\Debug.obj /Fe.integration\logging-test\Debug.exe
if errorlevel 1 exit /b 1
.integration\logging-test\Debug.exe
```

For Release, replace `/MDd /D_DEBUG /DGE_DEBUG` with
`/MD /DNDEBUG /DGE_RELEASE` and use `Release.obj`/`Release.exe` output names.
Expected result: compiler and executable exit code 0. Existing vendored fmt
deprecation warnings are distinct from missing-type errors. This focused check
does not replace an engine/example build or the three-backend rendering tests.
