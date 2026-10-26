#requires -Version 7.2
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$WorkRoot,
    [ValidateSet('Debug', 'Release')][string]$Configuration = 'Release',
    [ValidateSet('CMake', 'Premake')][string]$Generator = 'CMake'
)
$ErrorActionPreference = 'Stop'
$scripts = Split-Path -Parent $PSScriptRoot
$WorkRoot = [IO.Path]::GetFullPath($WorkRoot)
if (Test-Path -LiteralPath $WorkRoot) { throw 'Use a new isolated work directory; evidence will be retained.' }
$output = Join-Path $WorkRoot 'export'
$first = & (Join-Path $scripts 'ExportFrameGraphTriangle.ps1') -Configuration $Configuration -Generator $Generator -Output $output | ConvertFrom-Json
$repeat = & (Join-Path $scripts 'ExportFrameGraphTriangle.ps1') -Configuration $Configuration -Generator $Generator -Output $output | ConvertFrom-Json
if ($repeat.copyFiles -ne 0 -or $repeat.copyBytes -ne 0 -or $repeat.releaseId -cne $first.releaseId) { throw 'Repeat export was not zero-copy.' }
$launcher = Join-Path $output "releases/$($first.releaseId)/LaunchGame.ps1"
$results = @()
foreach ($backend in 'OpenGL', 'Vulkan', 'D3D12') {
    $run = & $launcher -StateRoot (Join-Path $WorkRoot 'state') -GraphicsAPI $backend -FrameLimit 120 -TimeoutSeconds 45 | ConvertFrom-Json
    $stdout = [IO.File]::ReadAllText((Join-Path $run.workingDirectory 'stdout.log'))
    $stderr = [IO.File]::ReadAllText((Join-Path $run.workingDirectory 'stderr.log'))
    if ($run.exitCode -ne 0 -or $run.timedOut -or $stderr -or
        ([regex]::Matches($stdout, 'readback passed after 119 frames')).Count -ne 3 -or
        -not $stdout.Contains('FrameGraphTriangle completed 120 frames; requesting normal shutdown.') -or
        $stdout -match '(?im)\b(error|VUID-|assertion failed)\b') { throw "Packaged $backend validation failed: $($run.workingDirectory)" }
    $results += [ordered]@{ backend = $backend; configuration = $Configuration; exitCode = $run.exitCode; readbacks = 3; workingDirectory = $run.workingDirectory }
}
$afterRun = & (Join-Path $scripts 'ExportFrameGraphTriangle.ps1') -Configuration $Configuration -Generator $Generator -Output $output | ConvertFrom-Json
if ($afterRun.copyFiles -ne 0 -or $afterRun.releaseId -cne $first.releaseId) { throw 'Game execution mutated published payloads.' }
$evidence = [ordered]@{ releaseId = $first.releaseId; files = $first.files; initialCopyBytes = $first.copyBytes; repeatCopyBytes = $repeat.copyBytes; postRunCopyBytes = $afterRun.copyBytes; results = $results }
$evidence | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath (Join-Path $WorkRoot 'verification.json') -Encoding utf8
$evidence | ConvertTo-Json -Depth 10
