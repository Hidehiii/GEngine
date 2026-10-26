#requires -Version 7.2
$ErrorActionPreference = 'Stop'
$exporter = Join-Path $PSScriptRoot '../ExportGame.ps1'
$testRoot = Join-Path ([IO.Path]::GetTempPath()) ('GEngineExport-' + [guid]::NewGuid().ToString('N'))
$inputRoot = Join-Path $testRoot 'inputs'
$output = Join-Path $testRoot 'package'
$manifest = Join-Path $testRoot 'export.json'
function Assert([bool]$Condition, [string]$Message) { if (-not $Condition) { throw "FAIL: $Message" } }
function Assert-Fails([scriptblock]$Action) {
    $failed = $false
    try { & $Action | Out-Null } catch { $failed = $true }
    Assert $failed 'Expected operation to fail'
}
function Save-Manifest { $spec | ConvertTo-Json -Depth 20 | Set-Content -LiteralPath $manifest -Encoding utf8 }
[IO.Directory]::CreateDirectory($inputRoot) | Out-Null
try {
    foreach ($name in 'game.exe', 'scene.json', 'texture.bin', 'unused.bin') {
        [IO.File]::WriteAllText((Join-Path $inputRoot $name), $name)
    }
    $spec = @{ schemaVersion = 1; enableScripting = $false; roots = @{ build = 'inputs' }; entrypoint = 'game'; include = @(); files = @(
        @{ id = 'game'; root = 'build'; source = 'game.exe'; path = 'Game.exe'; role = 'native'; dependencies = @('scene') },
        @{ id = 'scene'; root = 'build'; source = 'scene.json'; path = 'Assets/scene.json'; dependencies = @('texture') },
        @{ id = 'texture'; root = 'build'; source = 'texture.bin'; path = 'Assets/texture.bin'; dependencies = @('scene') },
        @{ id = 'unused'; root = 'build'; source = 'unused.bin'; path = 'Assets/unused.bin' }
    ) }
    Save-Manifest
    $preview = & $exporter -Manifest $manifest -Output $output -Preview
    Assert ($preview.copyFiles -eq 3 -and -not (Test-Path -LiteralPath $output)) 'Preview/closure/cycle'
    $first = & $exporter -Manifest $manifest -Output $output
    $release = Join-Path $output "releases/$($first.releaseId)"
    Assert (-not (Test-Path -LiteralPath (Join-Path $release 'Assets/unused.bin'))) 'Unreachable resource excluded'
    $timestamps = @{}
    Get-ChildItem -LiteralPath $output -Recurse -File | ForEach-Object { $timestamps[$_.FullName] = $_.LastWriteTimeUtc.Ticks }
    $again = & $exporter -Manifest $manifest -Output $output
    Assert ($again.copyFiles -eq 0 -and $again.copyBytes -eq 0) 'Repeat zero copy'
    foreach ($file in $timestamps.Keys) { Assert ((Get-Item -LiteralPath $file).LastWriteTimeUtc.Ticks -eq $timestamps[$file]) 'Repeat timestamp preserved' }
    [IO.File]::WriteAllText((Join-Path $inputRoot 'texture.bin'), 'changed texture')
    $changed = & $exporter -Manifest $manifest -Output $output
    Assert ($changed.copyFiles -eq 1 -and $changed.updated -eq 1 -and $changed.reused -eq 2) 'Single resource incremental copy'
    Assert ([IO.File]::ReadAllText((Join-Path $release 'Assets/texture.bin')) -eq 'texture.bin') 'Old release intact'
    $pointer = Join-Path $output 'current.json'
    $pointerBefore = [IO.File]::ReadAllText($pointer)
    [IO.File]::WriteAllText((Join-Path $output 'user.txt'), 'user')
    [IO.File]::WriteAllText((Join-Path $inputRoot 'texture.bin'), 'second change')
    $pointerLock = [IO.File]::Open($pointer, 'Open', 'Read', 'Read')
    try { Assert-Fails { & $exporter -Manifest $manifest -Output $output } } finally { $pointerLock.Dispose() }
    Assert ([IO.File]::ReadAllText($pointer) -eq $pointerBefore) 'Pointer failure retains old publication'
    Assert (-not (Test-Path -LiteralPath (Join-Path $output '.export.lock'))) 'Lock cleaned after failure'
    Assert (@(Get-ChildItem -LiteralPath $output -Filter '.current-*').Count -eq 0) 'Pointer temporary cleaned'
    $recovered = & $exporter -Manifest $manifest -Output $output
    Assert ($recovered.copyFiles -eq 0) 'Retry reuses completed orphan release'
    $lock = Join-Path $output '.export.lock'
    [IO.File]::WriteAllText($lock, 'other')
    Assert-Fails { & $exporter -Manifest $manifest -Output $output }
    Assert ([IO.File]::ReadAllText($lock) -eq 'other') 'Concurrent lock preserved'
    Remove-Item -LiteralPath $lock
    foreach ($bad in '../escape', '/absolute', 'C:/absolute', 'Assets/CON', 'Assets/a.', 'Assets/a:b', 'Assets\bad', 'game.EXE', 'Game.exe/child', 'package-manifest.json/child') {
        $spec.files[1].path = $bad; Save-Manifest
        Assert-Fails { & $exporter -Manifest $manifest -Output $output }
    }
    $spec.files[1].path = 'Assets/scene.json'
    $spec.files[0].dependencies = @('missing'); Save-Manifest
    Assert-Fails { & $exporter -Manifest $manifest -Output $output }
    $spec.files[0].dependencies = @('scene'); $spec.enableScripting = $true; Save-Manifest
    Assert-Fails { & $exporter -Manifest $manifest -Output $output }
    $spec.enableScripting = $false; Save-Manifest
    Assert-Fails { & $exporter -Manifest $manifest -Output (Join-Path $inputRoot 'out') }
    $link = Join-Path $testRoot 'linked-input'
    New-Item -ItemType Junction -Path $link -Target $inputRoot | Out-Null
    $spec.roots.build = 'linked-input'; Save-Manifest
    Assert-Fails { & $exporter -Manifest $manifest -Output $output }
    Remove-Item -LiteralPath $link -Force
    $spec.roots.build = 'inputs'; $spec.files[0].dependencies = @(); $spec.include = @('unused'); Save-Manifest
    $removed = & $exporter -Manifest $manifest -Output $output
    Assert ($removed.removed -eq 2 -and $removed.added -eq 1) 'Removed resources/new dynamic root'
    Assert ([IO.File]::ReadAllText((Join-Path $output 'user.txt')) -eq 'user') 'User file retained'
    Assert (Test-Path -LiteralPath $release) 'History retained'
    $damaged = Join-Path $output "releases/$($removed.releaseId)/Game.exe"
    [IO.File]::WriteAllText($damaged, 'damage')
    Assert-Fails { & $exporter -Manifest $manifest -Output $output }
    Write-Host 'PASS: preview, closure/cycles, incremental copy, timestamps, pointer failure/retry, locks, invalid paths, dependencies, scripting rejection, overlap, junction rejection, removals, history, user files, corruption.'
} finally {
    $resolved = [IO.Path]::GetFullPath($testRoot)
    $tempParent = [IO.Path]::GetFullPath([IO.Path]::GetTempPath()).TrimEnd('\', '/')
    if ((Split-Path -Parent $resolved) -ne $tempParent -or (Split-Path -Leaf $resolved) -notlike 'GEngineExport-*') {
        throw 'Refusing cleanup outside the isolated test directory.'
    }
    Remove-Item -LiteralPath $resolved -Recurse -Force
}
