[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')][string]$Configuration = 'Release',
    [ValidateSet('Auto', 'CMake', 'Premake')][string]$Generator = 'Auto',
    [switch]$PrintPath
)

$ErrorActionPreference = 'Stop'
try {
    $repoRoot = Split-Path $PSScriptRoot -Parent
    $toolIds = switch ($Generator) {
        'CMake' { 'qt-editor' }
        'Premake' { 'premake-vs2022' }
        default { 'qt-editor'; 'premake-vs2022' }
    }
    $candidates = @(foreach ($toolId in $toolIds) {
        $candidate = Join-Path $repoRoot "dist/$toolId/windows-x86_64/$Configuration/GEngine-EditorHost/GEngine-EditorHost.exe"
        if (Test-Path -LiteralPath $candidate -PathType Leaf) { Get-Item -LiteralPath $candidate }
    })
    $selected = $candidates | Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 1
    if (-not $selected) {
        throw "No $Configuration editor build found for $Generator. Run BuildScripts/BuildQtEditor.ps1 first."
    }
    if ($PrintPath) { $selected.FullName; exit 0 }
    Write-Host "Starting $($selected.FullName)"
    Push-Location $selected.DirectoryName
    try {
        & $selected.FullName
        $editorExitCode = $LASTEXITCODE
    } finally { Pop-Location }
    exit $editorExitCode
} catch {
    Write-Error $_ -ErrorAction Continue
    exit 1
}
