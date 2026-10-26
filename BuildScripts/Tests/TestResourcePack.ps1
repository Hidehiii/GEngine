#requires -Version 7.2
param([Parameter(Mandatory)][string]$PackageRoot, [Parameter(Mandatory)][string]$ProjectRoot)
$ErrorActionPreference='Stop'
$scripts=Split-Path -Parent $PSScriptRoot
$PackageRoot=[IO.Path]::GetFullPath($PackageRoot); $ProjectRoot=[IO.Path]::GetFullPath($ProjectRoot)
$manifest=Get-Content (Join-Path $PackageRoot '.gengine-output.json') -Raw | ConvertFrom-Json
$name=$manifest.name
$root=Join-Path ([IO.Path]::GetTempPath()) ('GEnginePackTest-'+[guid]::NewGuid().ToString('N'))
$native=Join-Path $root 'game'; $source=Join-Path $root 'source'
[IO.Directory]::CreateDirectory($native) | Out-Null; [IO.Directory]::CreateDirectory($source) | Out-Null
function Assert([bool]$Condition,[string]$Message) { if (-not $Condition) { throw $Message } }
function Run-Game {
    $info=[Diagnostics.ProcessStartInfo]::new((Join-Path $native ($name+'.exe')))
    $info.UseShellExecute=$false; $info.CreateNoWindow=$true; $info.WorkingDirectory=$root
    $info.RedirectStandardOutput=$true; $info.RedirectStandardError=$true
    foreach ($key in @($info.Environment.Keys)) { if ($key -like 'GENGINE_*' -or $key -like 'VK_*' -or $key -like 'MONO_*') { $info.Environment.Remove($key) | Out-Null } }
    $info.Environment['PATH']="$native;$env:SystemRoot/System32;$env:SystemRoot"; $info.Environment['GENGINE_EXAMPLE_FRAME_LIMIT']='120'
    $process=[Diagnostics.Process]::Start($info)
    try {
        $stdout=$process.StandardOutput.ReadToEndAsync(); $stderr=$process.StandardError.ReadToEndAsync()
        if (-not $process.WaitForExit(45000)) { $process.Kill($true); $process.WaitForExit(); throw 'Packaged game timed out.' }
        @{Code=$process.ExitCode; Out=$stdout.Result; Error=$stderr.Result}
    } finally { $process.Dispose() }
}
$pack=Join-Path $native ($name+'.gpak')
function Write-TestPack([string[]]$Paths,[string]$Payload='X',[bool]$WrongHash=$false) {
    $writer=[IO.BinaryWriter]::new([IO.File]::Open($pack,'Create','Write','None'))
    try {
        $writer.Write([Text.Encoding]::ASCII.GetBytes('GEPAK001')); $writer.Write([uint32]$Paths.Count)
        foreach ($path in $Paths) {
            $bytes=[Text.Encoding]::UTF8.GetBytes($path); $data=[Text.Encoding]::UTF8.GetBytes($Payload)
            $hash=[Security.Cryptography.SHA256]::HashData($data); if ($WrongHash) { $hash[0]=$hash[0] -bxor 1 }
            $writer.Write([uint32]$bytes.Length); $writer.Write([uint64]$data.Length); $writer.Write($hash); $writer.Write($bytes); $writer.Write($data)
        }
    } finally { $writer.Dispose() }
}
try {
    foreach ($file in Get-ChildItem -LiteralPath $PackageRoot -File | Where-Object Extension -in '.exe','.dll') {
        New-Item -ItemType HardLink -Path (Join-Path $native $file.Name) -Target $file.FullName | Out-Null
    }
    foreach ($file in Get-ChildItem -LiteralPath (Join-Path $ProjectRoot 'Assets') -Recurse -File) {
        $relative=[IO.Path]::GetRelativePath($ProjectRoot,$file.FullName)
        if ($relative.Replace('\','/') -imatch '^Assets/Cache(/|$)') { continue }
        $target=Join-Path $source $relative; [IO.Directory]::CreateDirectory((Split-Path -Parent $target)) | Out-Null
        [IO.File]::Copy($file.FullName,$target)
    }
    [IO.File]::WriteAllText((Join-Path $source 'Assets/资源.txt'),'Unicode resource path and content 中文')
    foreach ($backend in @(@('OpenGL',1),@('Vulkan',2),@('D3D12',3))) {
        $config=[IO.File]::ReadAllText((Join-Path $ProjectRoot 'Config.ini')) -replace '(?m)^GraphicsAPI:\s*\d+',("GraphicsAPI: "+$backend[1])
        [IO.File]::WriteAllText((Join-Path $source 'Config.ini'),$config)
        if (Test-Path -LiteralPath $pack) { Remove-Item -LiteralPath $pack }
        & (Join-Path $scripts 'WriteResourcePack.ps1') -ProjectRoot $source -Pack $pack
        $run=Run-Game
        Assert ($run.Code -eq 0 -and -not $run.Error -and ([regex]::Matches($run.Out,'readback passed after 119 frames')).Count -eq 3 -and $run.Out.Contains('requesting normal shutdown')) "$($backend[0]) native resource pack run failed: $($run.Error)"
        Write-Host "PASS: $($backend[0]) native .gpak, three readbacks, exit 0, no source/SDK PATH or PowerShell launcher"
    }
    Remove-Item -LiteralPath $pack
    Remove-Item -LiteralPath (Join-Path $source 'Config.ini')
    & (Join-Path $scripts 'WriteResourcePack.ps1') -ProjectRoot $source -Pack $pack
    $defaults=Run-Game
    Assert ($defaults.Code -eq 0 -and -not $defaults.Error -and ([regex]::Matches($defaults.Out,'readback passed after 119 frames')).Count -eq 3) 'Generated default game configuration failed.'
    Write-Host 'PASS: project without Config.ini uses a complete generated configuration'
    Remove-Item -LiteralPath $pack
    $missing=Run-Game; Assert ($missing.Code -ne 0 -and $missing.Error.Contains('missing')) 'Missing required pack was accepted.'
    [IO.File]::WriteAllText($pack,'invalid-version'); $invalid=Run-Game
    Assert ($invalid.Code -ne 0 -and $invalid.Error.Contains('version')) 'Invalid archive version was accepted.'
    Write-TestPack @('Assets/data.bin') -WrongHash $true
    $corrupt=Run-Game; Assert ($corrupt.Code -ne 0 -and $corrupt.Error.Contains('checksum')) 'Corrupted resource was accepted.'
    Write-TestPack @('Config.ini','config.INI')
    $duplicate=Run-Game; Assert ($duplicate.Code -ne 0 -and $duplicate.Error.Contains('Duplicate')) 'Case-colliding resources were accepted.'
    $escape='escaped-'+[guid]::NewGuid().ToString('N')+'.bin'
    Write-TestPack @('../'+$escape)
    $traversal=Run-Game; Assert ($traversal.Code -ne 0 -and $traversal.Error.Contains('Unsafe') -and -not (Test-Path -LiteralPath (Join-Path ([IO.Path]::GetTempPath()) "GEngineGames/$escape"))) 'Path traversal was accepted.'
    Write-TestPack @('Assets/data.bin')
    $stream=[IO.File]::Open($pack,'Open','Write'); $stream.SetLength($stream.Length-1); $stream.Dispose()
    $truncated=Run-Game; Assert ($truncated.Code -ne 0 -and $truncated.Error.Contains('Truncated')) 'Truncated archive was accepted.'
    Write-Host 'PASS: missing pack, version, checksum, duplicate paths, traversal and truncation rejected before game startup'
} finally {
    $resolved=[IO.Path]::GetFullPath($root)
    if ((Split-Path -Parent $resolved) -ne ([IO.Path]::GetTempPath()).TrimEnd('\','/') -or (Split-Path -Leaf $resolved) -notlike 'GEnginePackTest-*') { throw 'Refusing unsafe test cleanup.' }
    Remove-Item -LiteralPath $resolved -Recurse -Force
}
