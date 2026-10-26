#requires -Version 7.2
$ErrorActionPreference='Stop'
$publish=Join-Path (Split-Path -Parent $PSScriptRoot) 'PublishProject.ps1'
$root=Join-Path ([IO.Path]::GetTempPath()) ('GEnginePublishTest-'+[guid]::NewGuid().ToString('N'))
$native=Join-Path $root 'native'; $output=Join-Path $root 'output'; $pack=Join-Path $root 'Game.gpak'
function Assert([bool]$Value,[string]$Message) { if (-not $Value) { throw $Message } }
function Expect-Failure([scriptblock]$Action) { $failed=$false; try { & $Action | Out-Null } catch { $failed=$true }; Assert $failed 'Expected publication failure.' }
[IO.Directory]::CreateDirectory($native) | Out-Null
try {
    [IO.File]::WriteAllText((Join-Path $native 'Game.exe'),'exe'); [IO.File]::WriteAllText((Join-Path $native 'GEngine.dll'),'dll'); [IO.File]::WriteAllText($pack,'pack1')
    $first=& $publish -NativeRoot $native -Pack $pack -Name Game -Output $output
    $marker=Join-Path $output '.gengine-output.json'; $time=(Get-Item -LiteralPath $marker).LastWriteTimeUtc.Ticks
    $repeat=& $publish -NativeRoot $native -Pack $pack -Name Game -Output $output
    Assert ($repeat.copyFiles -eq 0 -and (Get-Item -LiteralPath $marker).LastWriteTimeUtc.Ticks -eq $time) 'Repeat was not zero copy.'
    [IO.File]::WriteAllText($pack,'pack2')
    $changed=& $publish -NativeRoot $native -Pack $pack -Name Game -Output $output
    Assert ($changed.copyFiles -eq 1 -and [IO.File]::ReadAllText((Join-Path $changed.previousOutput 'Game.gpak')) -eq 'pack1') 'Changed resource or old backup incorrect.'
    [IO.File]::WriteAllText((Join-Path $output 'user.txt'),'user')
    Expect-Failure { & $publish -NativeRoot $native -Pack $pack -Name Game -Output $output }
    Assert ([IO.File]::ReadAllText((Join-Path $output 'user.txt')) -eq 'user') 'User file was modified.'
    Remove-Item -LiteralPath (Join-Path $output 'user.txt')
    [IO.File]::WriteAllText((Join-Path $output 'Game.gpak'),'user change')
    Expect-Failure { & $publish -NativeRoot $native -Pack $pack -Name Game -Output $output }
    $unmanaged=Join-Path $root 'unmanaged'; [IO.Directory]::CreateDirectory($unmanaged) | Out-Null
    [IO.File]::WriteAllText((Join-Path $unmanaged 'keep'),'user')
    Expect-Failure { & $publish -NativeRoot $native -Pack $pack -Name Game -Output $unmanaged }
    Assert ([IO.File]::ReadAllText((Join-Path $unmanaged 'keep')) -eq 'user') 'Unmanaged output damaged.'
    Write-Host 'PASS: flat output, repeat/timestamps, single-file update, preserved backup, user files and modified/unmanaged output protection'
} finally {
    $full=[IO.Path]::GetFullPath($root)
    if ((Split-Path -Parent $full) -ne ([IO.Path]::GetTempPath()).TrimEnd('\','/') -or (Split-Path -Leaf $full) -notlike 'GEnginePublishTest-*') { throw 'Refusing unsafe test cleanup.' }
    Remove-Item -LiteralPath $full -Recurse -Force
}
