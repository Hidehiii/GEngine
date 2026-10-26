#requires -Version 7.2
param([Parameter(Mandatory)][string]$NativeRoot, [Parameter(Mandatory)][string]$Pack,
    [Parameter(Mandatory)][string]$Name, [Parameter(Mandatory)][string]$Output)
$ErrorActionPreference='Stop'
$Output=[IO.Path]::GetFullPath($Output).TrimEnd('\','/')
$parent=Split-Path -Parent $Output
if (-not $parent) { throw 'Cannot publish to a filesystem root.' }
function Assert-NoLinks([string]$Path) {
    while ($Path) {
        if ((Test-Path -LiteralPath $Path) -and ((Get-Item -LiteralPath $Path -Force).Attributes -band [IO.FileAttributes]::ReparsePoint)) { throw "Linked output path rejected: $Path" }
        $Path=[IO.Path]::GetDirectoryName($Path)
    }
}
Assert-NoLinks $Output
$sources=@{ ($Name+'.exe')=(Join-Path $NativeRoot ($Name+'.exe')); ($Name+'.gpak')=$Pack }
foreach ($dll in Get-ChildItem -LiteralPath $NativeRoot -File -Filter '*.dll') { $sources[$dll.Name]=$dll.FullName }
$records=@()
foreach ($path in ($sources.Keys | Sort-Object -Culture '')) {
    $records += [ordered]@{path=$path; size=(Get-Item -LiteralPath $sources[$path]).Length; sha256=(Get-FileHash -LiteralPath $sources[$path]).Hash.ToLowerInvariant()}
}
$manifest=([ordered]@{schemaVersion=1; name=$Name; files=$records} | ConvertTo-Json -Depth 10 -Compress)+"`n"
$previous=@{}
$marker=Join-Path $Output '.gengine-output.json'
if ([IO.Directory]::Exists($Output)) {
    $items=@(Get-ChildItem -LiteralPath $Output -Force)
    if ($items.Count) {
        if (-not [IO.File]::Exists($marker)) { throw 'Output contains unmanaged files; choose an empty folder.' }
        $old=Get-Content -LiteralPath $marker -Raw | ConvertFrom-Json -AsHashtable
        if ($old.schemaVersion -ne 1) { throw 'Unknown output manifest version.' }
        foreach ($file in $old.files) {
            if ($file.path -ne [IO.Path]::GetFileName($file.path) -or $file.path -match '[:/\\]') { throw 'Invalid old output manifest.' }
            $previous[$file.path]=$file
        }
        foreach ($item in $items) {
            if ($item.PSIsContainer -or ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) -or
                ($item.Name -ne '.gengine-output.json' -and -not $previous.ContainsKey($item.Name))) { throw 'Output contains unmanaged files; refusing replacement.' }
        }
        foreach ($file in $old.files) {
            $path=Join-Path $Output $file.path
            if (-not [IO.File]::Exists($path) -or (Get-FileHash -LiteralPath $path).Hash.ToLowerInvariant() -cne $file.sha256) { throw 'Published output was modified; choose a new destination.' }
        }
        if ([IO.File]::ReadAllText($marker) -ceq $manifest) { return [ordered]@{output=$Output; executable=(Join-Path $Output ($Name+'.exe')); resourcePack=(Join-Path $Output ($Name+'.gpak')); copyFiles=0} }
    }
}
[IO.Directory]::CreateDirectory($parent) | Out-Null
$lockId=[Convert]::ToHexString([Security.Cryptography.SHA256]::HashData([Text.Encoding]::UTF8.GetBytes($Output.ToLowerInvariant())))
$lock=[IO.FileStream]::new((Join-Path $parent ".gengine-publish-$lockId.lock"),'CreateNew','Write','None',4096,[IO.FileOptions]::DeleteOnClose)
$stage=Join-Path $parent ('.gengine-publish-'+[guid]::NewGuid().ToString('N'))
$backup=$null; $copied=0
try {
    [IO.Directory]::CreateDirectory($stage) | Out-Null
    foreach ($file in $records) {
        $target=Join-Path $stage $file.path
        if ($previous.ContainsKey($file.path) -and $previous[$file.path].sha256 -ceq $file.sha256) { New-Item -ItemType HardLink -Path $target -Target (Join-Path $Output $file.path) | Out-Null }
        else { [IO.File]::Copy($sources[$file.path],$target,$false); $copied++ }
        if ((Get-FileHash -LiteralPath $target).Hash.ToLowerInvariant() -cne $file.sha256) { throw 'Publish integrity check failed.' }
    }
    [IO.File]::WriteAllText((Join-Path $stage '.gengine-output.json'),$manifest,[Text.UTF8Encoding]::new($false))
    if ([IO.Directory]::Exists($Output)) {
        $backup=$Output+'.previous-'+[guid]::NewGuid().ToString('N')
        [IO.Directory]::Move($Output,$backup)
    }
    try { [IO.Directory]::Move($stage,$Output); $stage=$null }
    catch { if ($backup -and -not (Test-Path -LiteralPath $Output)) { [IO.Directory]::Move($backup,$Output); $backup=$null }; throw }
    [ordered]@{output=$Output; executable=(Join-Path $Output ($Name+'.exe')); resourcePack=(Join-Path $Output ($Name+'.gpak')); copyFiles=$copied; previousOutput=$backup}
} finally {
    if ($stage -and [IO.Directory]::Exists($stage)) {
        Assert-NoLinks $stage
        if ((Split-Path -Parent ([IO.Path]::GetFullPath($stage))) -ne $parent -or (Split-Path -Leaf $stage) -notlike '.gengine-publish-*') { throw 'Refusing unsafe staging cleanup.' }
        Remove-Item -LiteralPath $stage -Recurse -Force
    }
    $lock.Dispose()
}
