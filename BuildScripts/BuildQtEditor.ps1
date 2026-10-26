param(
    [ValidateSet('Debug', 'Release')][string]$Configuration = 'Release',
    [ValidateRange(1, 32)][int]$Jobs = 6,
    [string]$QtSdk = '',
    [switch]$Premake,
    [switch]$ProtocolOnly,
    [switch]$Test
)
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
if (-not $ProtocolOnly) {
    & (Join-Path $PSScriptRoot 'BuildMonoSdk.ps1')
}
if (-not $QtSdk) { $QtSdk = Join-Path $repoRoot "dist/qt6/windows-x86_64/$Configuration" }
$buildPath = Join-Path $repoRoot "build/qt-editor/windows-x86_64/$Configuration"
if ($Premake) { $buildPath = Join-Path $repoRoot "build/premake-vs2022/windows-x86_64/$Configuration/qt-editor-logs" }
if ($ProtocolOnly) { $buildPath += '-protocol' }
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vsPath = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) { throw 'Visual Studio C++ tools are required.' }
$vcvars = Join-Path $vsPath 'VC/Auxiliary/Build/vcvars64.bat'
$ninja = Join-Path $vsPath 'Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe'
$cmake = (Get-Command cmake -ErrorAction Stop).Source
New-Item -ItemType Directory -Force -Path $buildPath | Out-Null
function Invoke-EditorCommand([string]$Tool, [string[]]$Arguments, [string]$LogName, [bool]$Offscreen = $false) {
    $quoted = ($Arguments | ForEach-Object { '"' + $_ + '"' }) -join ' '
    $info = New-Object System.Diagnostics.ProcessStartInfo
    $info.FileName = $env:ComSpec
    $info.Arguments = '/d /s /c "call "' + $vcvars + '" >nul && "' + $Tool + '" ' + $quoted + '"'
    $info.WorkingDirectory = $repoRoot
    $info.UseShellExecute = $false
    $info.CreateNoWindow = $true
    $info.RedirectStandardOutput = $true
    $info.RedirectStandardError = $true
    $info.EnvironmentVariables.Clear()
    $taskPath = $env:Path
    foreach ($entry in [Environment]::GetEnvironmentVariables().GetEnumerator()) {
        if ($entry.Key -ine 'Path') { $info.EnvironmentVariables[$entry.Key] = $entry.Value }
    }
    $info.EnvironmentVariables['Path'] = $taskPath
    if ($Offscreen) { $info.EnvironmentVariables['QT_QPA_PLATFORM'] = 'offscreen' }
    $process = New-Object System.Diagnostics.Process
    $process.StartInfo = $info
    [void]$process.Start()
    $stdout = $process.StandardOutput.ReadToEndAsync()
    $stderr = $process.StandardError.ReadToEndAsync()
    $process.WaitForExit()
    $logPath = Join-Path $buildPath $LogName
    [IO.File]::WriteAllText($logPath, $stdout.Result + $stderr.Result)
    $code = $process.ExitCode
    $process.Dispose()
    Write-Host "$LogName : exit $code ($logPath)"
    if ($code -ne 0) { throw "Editor command failed. Inspect $logPath" }
}
$protocolFlag = if ($ProtocolOnly) { 'ON' } else { 'OFF' }
if ($Premake) {
    if ($ProtocolOnly) { throw 'Use the CMake protocol-only mode without -Premake.' }
    $premakeTool = Join-Path $PSScriptRoot 'Premake/premake5.exe'
    $sdkRoot = Split-Path -Parent $QtSdk
    Invoke-EditorCommand $premakeTool @("--qt6-sdk=$sdkRoot", 'vs2022') 'generate.log'
    $msbuild = Join-Path $vsPath 'MSBuild/Current/Bin/MSBuild.exe'
    Invoke-EditorCommand $msbuild @("$PSScriptRoot/Premake/GEngine-EditorEngineWorker.vcxproj",
        "/p:Configuration=$Configuration", '/p:Platform=x64', '/m:4', '/nologo', '/v:minimal') 'engine-worker-build.log'
    foreach ($target in @('GEngine-EditorDiagnosticWorker','GEngine-EditorHost','GEngine-EditorProtocolTests')) {
        Invoke-EditorCommand $msbuild @("$PSScriptRoot/Premake/$target.vcxproj", "/p:Configuration=$Configuration",
            '/p:Platform=x64', '/m:4', '/nologo', '/v:minimal') "$target-build.log"
    }
    if ($Test) {
        $outputRoot = Join-Path $repoRoot "dist/premake-vs2022/windows-x86_64/$Configuration"
        Invoke-EditorCommand "$outputRoot/GEngine-EditorProtocolTests/GEngine-EditorProtocolTests.exe" @() 'protocol-tests.log'
        Invoke-EditorCommand "$outputRoot/GEngine-EditorHost/GEngine-EditorHost.exe" @('--self-test') 'recovery-tests.log' $true
        Invoke-EditorCommand "$outputRoot/GEngine-EditorHost/GEngine-EditorHost.exe" @('--self-test', '--engine-check',
            '--worker', "$outputRoot/GEngine-EditorEngineWorker/GEngine-EditorEngineWorker.exe") 'engine-tests.log' $true
    }
    return
}
Invoke-EditorCommand $cmake @('-S', "$PSScriptRoot/Editor", '-B', $buildPath, '-G', 'Ninja',
    "-DCMAKE_MAKE_PROGRAM=$ninja", "-DCMAKE_BUILD_TYPE=$Configuration", "-DCMAKE_PREFIX_PATH=$QtSdk",
    "-DGE_EDITOR_PROTOCOL_ONLY=$protocolFlag") 'configure.log'
Invoke-EditorCommand $cmake @('--build', $buildPath, '--parallel', "$Jobs") 'build.log'
if (-not $ProtocolOnly) {
    $engineBuild = Join-Path $repoRoot 'build/cmake-vs2022/windows-x86_64'
    Invoke-EditorCommand $cmake @('-S', "$PSScriptRoot/CMake", '-B', $engineBuild, '-G', 'Visual Studio 17 2022',
        '-A', 'x64', '-DGE_TOOL_ID=cmake-vs2022') 'engine-configure.log'
    Invoke-EditorCommand $cmake @('--build', $engineBuild, '--config', $Configuration,
        '--target', 'GEngine-EditorEngineWorker', '--parallel', "$Jobs") 'engine-build.log'
    $engineOutput = Join-Path $repoRoot "dist/cmake-vs2022/windows-x86_64/$Configuration/GEngine-EditorEngineWorker"
    $editorOutput = Join-Path $repoRoot "dist/qt-editor/windows-x86_64/$Configuration/GEngine-EditorEngineWorker"
    New-Item -ItemType Directory -Force -Path $editorOutput | Out-Null
    foreach ($artifact in Get-ChildItem -LiteralPath $engineOutput) {
        if ($artifact.Name -eq 'Config.ini' -and (Test-Path (Join-Path $editorOutput 'Config.ini'))) { continue }
        Copy-Item -LiteralPath $artifact.FullName -Destination $editorOutput -Recurse -Force
    }
}
if ($Test) {
    $ctest = Join-Path (Split-Path $cmake) 'ctest.exe'
    Invoke-EditorCommand $ctest @('--test-dir', $buildPath, '--output-on-failure', '-C', $Configuration) 'tests.log'
    if (-not $ProtocolOnly) {
        $outputRoot = Join-Path $repoRoot "dist/qt-editor/windows-x86_64/$Configuration"
        Invoke-EditorCommand "$outputRoot/GEngine-EditorHost/GEngine-EditorHost.exe" @('--self-test', '--engine-check',
            '--worker', "$outputRoot/GEngine-EditorEngineWorker/GEngine-EditorEngineWorker.exe") 'engine-tests.log' $true
    }
}
