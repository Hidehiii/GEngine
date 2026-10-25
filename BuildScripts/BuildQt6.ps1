param(
    [ValidateSet('Debug', 'Release')][string]$Configuration = 'Release',
    [ValidateRange(1, 32)][int]$Jobs = 6,
    [string]$Python = 'python',
    [switch]$ConfigureOnly
)
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$sourcePath = Join-Path $repoRoot 'GEngine/GEngine-ThirdParty/qtbase'
$buildPath = Join-Path $repoRoot "build/qt6/windows-x86_64/$Configuration"
$installPath = Join-Path $repoRoot "dist/qt6/windows-x86_64/$Configuration"
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vsPath = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) { throw 'Visual Studio C++ tools are required.' }
$vcvars = Join-Path $vsPath 'VC/Auxiliary/Build/vcvars64.bat'
$ninja = Join-Path $vsPath 'Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe'
$cmake = (Get-Command cmake -ErrorAction Stop).Source
$pythonPath = (Get-Command $Python -ErrorAction Stop).Source
if (-not (Test-Path "$sourcePath/CMakeLists.txt")) { throw 'Initialize the qtbase submodule first.' }
New-Item -ItemType Directory -Force -Path $buildPath | Out-Null
$configureArgs = @('-S', $sourcePath, '-B', $buildPath, '-G', 'Ninja',
    "-DCMAKE_MAKE_PROGRAM=$ninja", "-DCMAKE_BUILD_TYPE=$Configuration",
    "-DCMAKE_INSTALL_PREFIX=$installPath", "-DPython3_EXECUTABLE=$pythonPath",
    '-DQT_BUILD_TESTS=OFF', '-DQT_BUILD_EXAMPLES=OFF', '-DFEATURE_opengl=ON',
    '-DFEATURE_vulkan=OFF', '-DFEATURE_sql=OFF', '-DFEATURE_network=OFF',
    '-DFEATURE_dbus=OFF', '-DFEATURE_printsupport=OFF', '-DFEATURE_testlib=OFF',
    '-DFEATURE_concurrent=OFF')
function Invoke-QtBuildCommand([string[]]$Arguments, [string]$LogName) {
    $quoted = ($Arguments | ForEach-Object { '"' + $_ + '"' }) -join ' '
    $processInfo = New-Object System.Diagnostics.ProcessStartInfo
    $processInfo.FileName = $env:ComSpec
    $processInfo.Arguments = '/d /s /c "call "' + $vcvars + '" >nul && "' + $cmake + '" ' + $quoted + '"'
    $processInfo.WorkingDirectory = $repoRoot
    $processInfo.UseShellExecute = $false
    $processInfo.CreateNoWindow = $true
    $processInfo.RedirectStandardOutput = $true
    $processInfo.RedirectStandardError = $true
    $processInfo.EnvironmentVariables.Clear()
    $taskPath = $env:Path
    foreach ($entry in [Environment]::GetEnvironmentVariables().GetEnumerator()) {
        if ($entry.Key -ine 'Path') { $processInfo.EnvironmentVariables[$entry.Key] = $entry.Value }
    }
    $processInfo.EnvironmentVariables['Path'] = $taskPath
    $process = New-Object System.Diagnostics.Process
    $process.StartInfo = $processInfo
    [void]$process.Start()
    $outTask = $process.StandardOutput.ReadToEndAsync()
    $errTask = $process.StandardError.ReadToEndAsync()
    $process.WaitForExit()
    $logPath = Join-Path $buildPath $LogName
    [IO.File]::WriteAllText($logPath, $outTask.Result + $errTask.Result)
    $resultCode = $process.ExitCode
    $process.Dispose()
    Write-Host "$LogName : exit $resultCode ($logPath)"
    if ($resultCode -ne 0) { throw "Qt build failed. Inspect $logPath" }
}
Invoke-QtBuildCommand $configureArgs 'configure.log'
if (-not $ConfigureOnly) {
    Invoke-QtBuildCommand @('--build', $buildPath, '--parallel', "$Jobs") 'build.log'
    Invoke-QtBuildCommand @('--install', $buildPath) 'install.log'
    Write-Host "Qt SDK installed: $installPath"
}
