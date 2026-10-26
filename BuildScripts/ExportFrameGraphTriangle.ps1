#requires -Version 7.2
[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')][string]$Configuration = 'Release',
    [ValidateSet('CMake', 'Premake')][string]$Generator = 'CMake',
    [Parameter(Mandatory)][string]$Output,
    [switch]$Preview
)
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$generatorFolder = if ($Generator -eq 'CMake') { 'cmake-vs2022' } else { 'premake-vs2022' }
$build = Join-Path $repoRoot "dist/$generatorFolder/windows-x86_64/$Configuration/FrameGraphTriangle"
if (-not (Test-Path -LiteralPath (Join-Path $build 'FrameGraphTriangle.exe'))) { throw "Build FrameGraphTriangle first: $build" }
$example = Join-Path $repoRoot 'Example/FrameGraphTriangle'
$files = @(
    @{ id = 'game'; root = 'build'; source = 'FrameGraphTriangle.exe'; path = 'FrameGraphTriangle.exe'; role = 'native' },
    @{ id = 'config'; root = 'example'; source = 'Config.ini'; path = 'Config.ini'; role = 'config' },
    @{ id = 'triangle-shader'; root = 'example'; source = 'Assets/Shaders/FrameGraphTriangle.shader'; path = 'Assets/Shaders/FrameGraphTriangle.shader'; role = 'asset' },
    @{ id = 'color-shader'; root = 'example'; source = 'Assets/Shaders/FrameGraphColor.shader'; path = 'Assets/Shaders/FrameGraphColor.shader'; role = 'asset' },
    @{ id = 'launcher'; root = 'scripts'; source = 'StartExportedGame.ps1'; path = 'LaunchGame.ps1'; role = 'native' }
)
# Keep the current build's runtime set intact; this is not PE import discovery.
foreach ($dll in (Get-ChildItem -LiteralPath $build -Filter '*.dll' -File | Sort-Object Name)) {
    $files += @{ id = "dll:$($dll.Name)"; root = 'build'; source = $dll.Name; path = $dll.Name; role = 'native' }
}
if (-not ($files | Where-Object { $_.path -eq 'GEngine.dll' })) { throw 'GEngine.dll is missing from the build output.' }
$files[0].dependencies = @($files | Where-Object { $_.id -ne 'game' } | ForEach-Object { $_.id })
$spec = [ordered]@{ schemaVersion = 1; enableScripting = $false
    roots = @{ build = $build; example = $example; scripts = $PSScriptRoot }
    entrypoint = 'game'; include = @(); files = $files }
$temporaryManifest = Join-Path ([IO.Path]::GetTempPath()) ('gengine-export-' + [guid]::NewGuid().ToString('N') + '.json')
try {
    $spec | ConvertTo-Json -Depth 20 | Set-Content -LiteralPath $temporaryManifest -Encoding utf8
    $result = & (Join-Path $PSScriptRoot 'ExportGame.ps1') -Manifest $temporaryManifest -Output $Output -Preview:$Preview
    $result | ConvertTo-Json -Depth 20
} finally { Remove-Item -LiteralPath $temporaryManifest -Force }
