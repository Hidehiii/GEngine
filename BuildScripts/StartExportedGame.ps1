#requires -Version 7.2
[CmdletBinding()]
param(
    [string]$StateRoot = '',
    [ValidateSet('Default', 'OpenGL', 'Vulkan', 'D3D12')][string]$GraphicsAPI = 'Default',
    [ValidateRange(0, 10000000)][int]$FrameLimit = 0,
    [ValidateRange(0, 3600)][int]$TimeoutSeconds = 0
)
$ErrorActionPreference = 'Stop'
function Assert-NoLinks([string]$Path) {
    $cursor = [IO.Path]::GetFullPath($Path)
    while ($cursor) {
        if ((Test-Path -LiteralPath $cursor) -and
            ((Get-Item -LiteralPath $cursor -Force).Attributes -band [IO.FileAttributes]::ReparsePoint)) {
            throw "Links/reparse points are not accepted: $cursor"
        }
        $cursor = [IO.Path]::GetDirectoryName($cursor)
    }
}
function Get-PackagePath([string]$Root, [string]$Relative) {
    if (-not $Relative -or $Relative.Contains('\') -or $Relative -match '(^/|:|(^|/)\.\.?(/|$))') { throw 'Invalid package path.' }
    $path = [IO.Path]::GetFullPath((Join-Path $Root $Relative))
    if (-not $path.StartsWith($Root.TrimEnd('\', '/') + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
        throw 'Package path escapes its root.'
    }
    Assert-NoLinks $path
    return $path
}
$release = [IO.Path]::GetFullPath($PSScriptRoot)
$releaseId = Split-Path -Leaf $release
if ($releaseId -cnotmatch '^[a-f0-9]{64}$') { throw 'Launch this script from a published release directory.' }
$manifestPath = Join-Path $release 'package-manifest.json'
Assert-NoLinks $manifestPath
if ((Get-FileHash -LiteralPath $manifestPath -Algorithm SHA256).Hash.ToLowerInvariant() -cne $releaseId) { throw 'Package manifest hash mismatch.' }
$package = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json -AsHashtable
if ($package.schemaVersion -ne 1 -or $package.enableScripting -isnot [bool] -or $package.enableScripting) { throw 'Unsupported package schema or scripting mode.' }
foreach ($record in $package.files) {
    $source = Get-PackagePath $release $record.path
    if (-not [IO.File]::Exists($source) -or
        (Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash.ToLowerInvariant() -cne $record.sha256) {
        throw "Package payload damaged: $($record.path)"
    }
}
$entrypoint = Get-PackagePath $release $package.entrypoint
if (@($package.files | Where-Object { $_.path -ceq $package.entrypoint -and $_.role -ceq 'native' }).Count -ne 1) {
    throw 'Invalid executable entrypoint.'
}
if (-not $StateRoot) { $StateRoot = Join-Path ([Environment]::GetFolderPath('LocalApplicationData')) 'GEngine/ExportedGames' }
$StateRoot = [IO.Path]::GetFullPath($StateRoot)
Assert-NoLinks $StateRoot
if ($StateRoot.Equals($release, [StringComparison]::OrdinalIgnoreCase) -or
    $StateRoot.StartsWith($release + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Runtime state must be outside the published release.'
}
$working = Join-Path $StateRoot "$releaseId/$([guid]::NewGuid().ToString('N'))"
[IO.Directory]::CreateDirectory($working) | Out-Null
foreach ($record in $package.files) {
    if ($record.role -in 'asset', 'config') {
        $target = Get-PackagePath $working $record.path
        [IO.Directory]::CreateDirectory((Split-Path -Parent $target)) | Out-Null
        [IO.File]::Copy((Get-PackagePath $release $record.path), $target, $false)
    }
}
if ($GraphicsAPI -ne 'Default') {
    $config = Join-Path $working 'Config.ini'
    $value = @{ OpenGL = 1; Vulkan = 2; D3D12 = 3 }[$GraphicsAPI]
    $text = [IO.File]::ReadAllText($config)
    if ($text -notmatch '(?m)^GraphicsAPI:\s*\d+\s*$') { throw 'No GraphicsAPI setting in packaged Config.ini.' }
    [IO.File]::WriteAllText($config, ($text -replace '(?m)^GraphicsAPI:\s*\d+\s*$', "GraphicsAPI: $value"))
}
$info = [Diagnostics.ProcessStartInfo]::new()
$info.FileName = $entrypoint
$info.WorkingDirectory = $working
$info.UseShellExecute = $false
$info.CreateNoWindow = $true
$info.RedirectStandardOutput = $true
$info.RedirectStandardError = $true
foreach ($key in @($info.Environment.Keys)) {
    if ($key -like 'GENGINE_*' -or $key -like 'VK_*' -or $key -like 'MONO_*') { $info.Environment.Remove($key) | Out-Null }
}
# System graphics runtimes remain available, but developer SDK DLL paths do not.
$info.Environment['PATH'] = "$release;$env:SystemRoot/System32;$env:SystemRoot"
if ($FrameLimit) { $info.Environment['GENGINE_EXAMPLE_FRAME_LIMIT'] = [string]$FrameLimit }
$process = [Diagnostics.Process]::new()
$process.StartInfo = $info
$timedOut = $false
$started = $false
try {
    if (-not $process.Start()) { throw 'Game process failed to start.' }
    $started = $true
    $stdout = $process.StandardOutput.ReadToEndAsync()
    $stderr = $process.StandardError.ReadToEndAsync()
    if ($TimeoutSeconds -and -not $process.WaitForExit($TimeoutSeconds * 1000)) {
        $timedOut = $true
        $process.Kill($true)
    }
    $process.WaitForExit()
    [IO.File]::WriteAllText((Join-Path $working 'stdout.log'), $stdout.GetAwaiter().GetResult())
    [IO.File]::WriteAllText((Join-Path $working 'stderr.log'), $stderr.GetAwaiter().GetResult())
    $result = [ordered]@{ releaseId = $releaseId; exitCode = $process.ExitCode; timedOut = $timedOut; workingDirectory = $working }
    $result | ConvertTo-Json | Write-Output
    if ($timedOut -or $process.ExitCode -ne 0) { throw "Packaged game failed; see $working" }
} finally {
    if ($started -and -not $process.HasExited) { $process.Kill($true); $process.WaitForExit() }
    $process.Dispose()
}
