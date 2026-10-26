#requires -Version 7.2
param([Parameter(Mandatory)][string]$ProjectRoot, [Parameter(Mandatory)][string]$Name)
$ErrorActionPreference = 'Stop'
$engine = $PSScriptRoot
while ($engine -and -not (Test-Path -LiteralPath (Join-Path $engine 'BuildScripts/CMake/CMakeLists.txt'))) { $engine = Split-Path -Parent $engine }
if (-not $engine) { throw 'The editor requires a GEngine source SDK to compile projects.' }
if ($Name -notmatch '^[\p{L}\p{N}_-][\p{L}\p{N}_ .-]{0,100}$' -or $Name.EndsWith('.') -or $Name.EndsWith(' ')) { throw 'Project name is not a valid game filename.' }
if (-not (Test-Path -LiteralPath (Join-Path $ProjectRoot 'src'))) { throw 'Project src/ directory is missing.' }
$identity = [Convert]::ToHexString([Security.Cryptography.SHA256]::HashData([Text.Encoding]::UTF8.GetBytes($ProjectRoot.ToLowerInvariant()))).ToLowerInvariant()
$cache = Join-Path ([IO.Path]::GetTempPath()) "GEngineBuilds/$identity"
[IO.Directory]::CreateDirectory($cache) | Out-Null
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vs = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vs) { throw 'Visual Studio C++ build tools are required.' }
$vcvars = Join-Path $vs 'VC/Auxiliary/Build/vcvars64.bat'
$cmake = (Get-Command cmake -ErrorAction Stop).Source
$ninja = Join-Path $vs 'Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe'
$environment = [Collections.Generic.Dictionary[string,string]]::new([StringComparer]::OrdinalIgnoreCase)
$setup = [Diagnostics.ProcessStartInfo]::new($env:ComSpec)
$setup.Arguments = '/d /s /c "call "' + $vcvars + '" >nul && set"'
$setup.UseShellExecute = $false; $setup.CreateNoWindow = $true; $setup.RedirectStandardOutput = $true; $setup.RedirectStandardError = $true
$setup.Environment.Clear()
foreach ($entry in [Environment]::GetEnvironmentVariables().GetEnumerator()) { $environment[$entry.Key] = $entry.Value }
foreach ($key in $environment.Keys) { $setup.Environment[$key] = $environment[$key] }
$setupProcess = [Diagnostics.Process]::Start($setup)
try {
    $out = $setupProcess.StandardOutput.ReadToEndAsync(); $err = $setupProcess.StandardError.ReadToEndAsync(); $setupProcess.WaitForExit()
    if ($setupProcess.ExitCode -ne 0) { throw ('C++ environment setup failed: ' + $err.Result) }
    foreach ($line in ($out.Result -split "`r?`n")) { $equals = $line.IndexOf('='); if ($equals -gt 0) { $environment[$line.Substring(0,$equals)] = $line.Substring($equals+1) } }
} finally { $setupProcess.Dispose() }
function Invoke-Compile([string[]]$Arguments, [string]$LogName) {
    $info = [Diagnostics.ProcessStartInfo]::new($cmake); $info.UseShellExecute=$false; $info.CreateNoWindow=$true
    $info.RedirectStandardOutput=$true; $info.RedirectStandardError=$true; $info.Environment.Clear()
    foreach ($key in $environment.Keys) { $info.Environment[$key]=$environment[$key] }
    foreach ($argument in $Arguments) { $info.ArgumentList.Add($argument) }
    $process = [Diagnostics.Process]::new(); $process.StartInfo=$info
    $started=$false
    $log = [IO.StreamWriter]::new((Join-Path $cache $LogName), $false)
    try {
        if (-not $process.Start()) { throw 'Could not start compiler.' }
        $started=$true
        $stdout=$process.StandardOutput.ReadLineAsync(); $stderr=$process.StandardError.ReadLineAsync()
        while ($null -ne $stdout -or $null -ne $stderr) {
            foreach ($streamName in 'stdout','stderr') {
                $task = Get-Variable -Name $streamName -ValueOnly
                if ($null -ne $task -and $task.IsCompleted) {
                    $line=$task.GetAwaiter().GetResult()
                    if ($null -eq $line) { Set-Variable -Name $streamName -Value $null }
                    else {
                        $log.WriteLine($line); [Console]::Error.WriteLine($line)
                        $next=if ($streamName -eq 'stdout') { $process.StandardOutput.ReadLineAsync() } else { $process.StandardError.ReadLineAsync() }
                        Set-Variable -Name $streamName -Value $next
                    }
                }
            }
            if (-not $process.HasExited) { [void]$process.WaitForExit(50) }
        }
        $process.WaitForExit()
        if ($process.ExitCode -ne 0) { throw "Compilation failed; see $cache/$LogName" }
    } finally {
        if ($started -and -not $process.HasExited) { $process.Kill($true); $process.WaitForExit() }
        $log.Dispose(); $process.Dispose()
    }
}
Invoke-Compile @('-S', (Join-Path $engine 'BuildScripts/Project'), '-B', $cache, '-G', 'Ninja',
    "-DCMAKE_MAKE_PROGRAM=$($ninja.Replace('\','/'))", '-DCMAKE_BUILD_TYPE=Release', "-DGE_ENGINE_ROOT=$($engine.Replace('\','/'))", "-DGE_PROJECT_ROOT=$($ProjectRoot.Replace('\','/'))",
    "-DGE_PROJECT_NAME=$Name", "-DGE_DIST_ROOT=$($cache.Replace('\','/'))/native") 'configure.log'
Invoke-Compile @('--build', $cache, '--target', 'GProjectGame', '--parallel', '2') 'build.log'
Join-Path $cache 'native/Release/Game'
