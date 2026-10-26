#requires -Version 7.2
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$Manifest,
    [Parameter(Mandatory)][string]$Output,
    [switch]$Preview
)
$ErrorActionPreference = 'Stop'

function Assert-NoLinks([string]$Path) {
    $cursor = [IO.Path]::GetFullPath($Path)
    while ($cursor) {
        if (Test-Path -LiteralPath $cursor) {
            if ((Get-Item -LiteralPath $cursor -Force).Attributes -band [IO.FileAttributes]::ReparsePoint) {
                throw "Links/reparse points are not accepted: $cursor"
            }
        }
        $parent = [IO.Path]::GetDirectoryName($cursor)
        if ($parent -eq $cursor) { break }
        $cursor = $parent
    }
}
function Get-RelativePath([object]$Value) {
    if ($Value -isnot [string] -or -not $Value -or $Value.Contains('\')) { throw 'Expected a relative path using /.' }
    foreach ($part in $Value.Split('/')) {
        if (-not $part -or $part -in '.', '..' -or $part -match '[<>:"|?*\x00-\x1f]' -or
            $part -match '[ .]$' -or $part -match '^(CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])(\.|$)') {
            throw "Invalid relative path: $Value"
        }
    }
    return $Value
}
function Get-Digest([string]$Path) { (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant() }
function Get-TextDigest([string]$Text) {
    [Convert]::ToHexString([Security.Cryptography.SHA256]::HashData([Text.Encoding]::UTF8.GetBytes($Text))).ToLowerInvariant()
}
function ConvertTo-PackageJson($Value) { (ConvertTo-Json -InputObject $Value -Depth 50 -Compress) + "`n" }
function Assert-JsonKeys($Element) {
    if ($Element.ValueKind -eq [Text.Json.JsonValueKind]::Object) {
        $names = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
        foreach ($property in $Element.EnumerateObject()) {
            if (-not $names.Add($property.Name)) { throw "Duplicate JSON key: $($property.Name)" }
            Assert-JsonKeys $property.Value
        }
    } elseif ($Element.ValueKind -eq [Text.Json.JsonValueKind]::Array) {
        foreach ($child in $Element.EnumerateArray()) { Assert-JsonKeys $child }
    }
}
function Read-PackageJson([string]$Path) {
    Assert-NoLinks $Path
    $text = [IO.File]::ReadAllText($Path)
    $document = [Text.Json.JsonDocument]::Parse($text)
    try { Assert-JsonKeys $document.RootElement } finally { $document.Dispose() }
    ConvertFrom-Json -InputObject $text -AsHashtable -Depth 50
}
function Test-Within([string]$Child, [string]$Parent) {
    $Child.Equals($Parent, [StringComparison]::OrdinalIgnoreCase) -or
        $Child.StartsWith($Parent.TrimEnd('\', '/') + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)
}
function Get-ExportPlan {
    Assert-NoLinks $Output
    $spec = Read-PackageJson $Manifest
    if ($spec.schemaVersion -ne 1 -or $spec.enableScripting -isnot [bool] -or $spec.enableScripting) {
        throw 'Schema 1 requires enableScripting: false.'
    }
    if ($spec.roots -isnot [Collections.IDictionary] -or -not $spec.roots.Count -or
        $spec.files -isnot [array] -or -not $spec.files.Count) { throw 'Nonempty roots and files are required.' }
    $roots = @{}
    foreach ($key in $spec.roots.Keys) {
        if ($spec.roots[$key] -isnot [string]) { throw 'Root paths must be strings.' }
        $path = [IO.Path]::GetFullPath($spec.roots[$key], (Split-Path -Parent $Manifest))
        Assert-NoLinks $path
        if (-not [IO.Directory]::Exists($path) -or (Test-Within $Output $path) -or (Test-Within $path $Output)) {
            throw "Missing input root or overlapping output: $path"
        }
        $roots[$key] = $path
    }
    $entries = @{}; $destinations = @{}
    foreach ($item in $spec.files) {
        if ($item.id -isnot [string] -or -not $item.id -or $entries.ContainsKey($item.id)) { throw 'Missing/duplicate file ID.' }
        $destination = Get-RelativePath $item.path
        if ($destinations.ContainsKey($destination) -or $destination -match '^package-manifest\.json(/|$)') {
            throw "Conflicting/reserved destination: $destination"
        }
        if (-not $roots.ContainsKey($item.root)) { throw "Unknown input root: $($item.root)" }
        $source = Join-Path $roots[$item.root] (Get-RelativePath $item.source)
        Assert-NoLinks $source
        if (-not [IO.File]::Exists($source)) { throw "Missing input: $source" }
        if ($item.Contains('dependencies') -and $item.dependencies -isnot [array]) { throw 'dependencies must be an array.' }
        foreach ($dependency in $item.dependencies) {
            if ($dependency -isnot [string] -or -not $dependency) { throw 'Dependencies must be file IDs.' }
        }
        $role = if ($item.Contains('role')) { $item.role } else { 'asset' }
        if ($role -cnotin 'native', 'asset', 'config', 'license') { throw "Invalid role: $role" }
        $entries[$item.id] = @{ Item = $item; Source = $source; Path = $destination; Role = $role }
        $destinations[$destination] = $item.id
    }
    foreach ($destination in $destinations.Keys) {
        $parts = $destination.Split('/')
        for ($i = 1; $i -lt $parts.Length; $i++) {
            if ($destinations.ContainsKey(($parts[0..($i - 1)] -join '/'))) { throw "File/directory conflict: $destination" }
        }
    }
    foreach ($entry in $entries.Values) {
        foreach ($dependency in $entry.Item.dependencies) {
            if (-not $entries.ContainsKey($dependency)) { throw "Missing dependency: $dependency" }
        }
    }
    if ($spec.entrypoint -isnot [string] -or -not $entries.ContainsKey($spec.entrypoint) -or
        $entries[$spec.entrypoint].Role -cne 'native') { throw 'entrypoint must identify a native file.' }
    if ($spec.Contains('include') -and $spec.include -isnot [array]) { throw 'include must be an array.' }
    $selected = @{}; $pending = [Collections.Generic.Stack[string]]::new()
    $pending.Push($spec.entrypoint)
    foreach ($id in $spec.include) {
        if ($id -isnot [string] -or -not $id) { throw 'include must contain file IDs.' }
        $pending.Push($id)
    }
    while ($pending.Count) {
        $id = $pending.Pop()
        if (-not $entries.ContainsKey($id)) { throw "Unknown include ID: $id" }
        if (-not $selected.ContainsKey($id)) {
            $selected[$id] = $true
            foreach ($dependency in $entries[$id].Item.dependencies) { $pending.Push($dependency) }
        }
    }
    $records = @(); $sources = @{}
    foreach ($id in ($selected.Keys | Sort-Object -CaseSensitive -Culture '')) {
        $entry = $entries[$id]
        $records += [ordered]@{ id = $id; path = $entry.Path; role = $entry.Role
            dependencies = @($entry.Item.dependencies | Sort-Object -Unique -CaseSensitive -Culture '')
            size = (Get-Item -LiteralPath $entry.Source).Length; sha256 = Get-Digest $entry.Source }
        $sources[$entry.Path] = $entry.Source
    }
    $package = [ordered]@{ schemaVersion = 1; enableScripting = $false; entrypoint = $entries[$spec.entrypoint].Path; files = $records }
    $json = ConvertTo-PackageJson $package
    $releaseId = Get-TextDigest $json
    $release = Join-Path $Output "releases/$releaseId"
    Assert-NoLinks $release
    $previous = @{}; $reusable = @{}
    $pointer = Join-Path $Output 'current.json'
    if (Test-Path -LiteralPath $pointer) {
        $current = Read-PackageJson $pointer
        if ($current.schemaVersion -ne 1 -or $current.releaseId -cnotmatch '^[a-f0-9]{64}$') { throw 'Invalid current pointer.' }
        $oldRelease = Join-Path $Output "releases/$($current.releaseId)"
        $oldManifest = Join-Path $oldRelease 'package-manifest.json'
        $old = Read-PackageJson $oldManifest
        if ((Get-Digest $oldManifest) -cne $current.releaseId) { throw 'Previous manifest hash mismatch.' }
        foreach ($record in $old.files) { $previous[$record.path] = $record }
        foreach ($record in $records) {
            if ($previous.ContainsKey($record.path) -and $previous[$record.path].sha256 -ceq $record.sha256) {
                $oldFile = Join-Path $oldRelease (Get-RelativePath $record.path)
                Assert-NoLinks $oldFile
                if ([IO.File]::Exists($oldFile) -and (Get-Digest $oldFile) -ceq $record.sha256) { $reusable[$record.path] = $oldFile }
            }
        }
    }
    $exists = [IO.Directory]::Exists($release)
    if ($exists) {
        $existingManifest = Join-Path $release 'package-manifest.json'
        Assert-NoLinks $existingManifest
        if ([IO.File]::ReadAllText($existingManifest) -cne $json) { throw 'Existing release manifest mismatch.' }
        foreach ($record in $records) {
            $file = Join-Path $release $record.path
            Assert-NoLinks $file
            if (-not [IO.File]::Exists($file) -or (Get-Digest $file) -cne $record.sha256) { throw "Damaged release: $file" }
        }
    }
    $stats = [ordered]@{ releaseId = $releaseId; files = $records.Count; added = 0; updated = 0; reused = 0
        removed = 0; copyFiles = 0; copyBytes = [long]0; plan = @(); removedPaths = @() }
    foreach ($record in $records) {
        $action = if (-not $previous.ContainsKey($record.path)) { 'added' }
            elseif ($previous[$record.path].sha256 -ceq $record.sha256) { 'reused' } else { 'updated' }
        $stats[$action]++
        $stats.plan += @{ path = $record.path; size = $record.size; action = $action }
        if (-not $exists -and -not $reusable.ContainsKey($record.path)) { $stats.copyFiles++; $stats.copyBytes += $record.size }
    }
    $stats.removedPaths = @($previous.Keys | Where-Object { -not $sources.ContainsKey($_) } | Sort-Object)
    $stats.removed = $stats.removedPaths.Count
    @{ PackageJson = $json; Records = $records; Sources = $sources; Reusable = $reusable; Release = $release; Stats = $stats }
}

$Manifest = [IO.Path]::GetFullPath($Manifest)
$Output = [IO.Path]::GetFullPath($Output)
$plan = Get-ExportPlan
if ($Preview) { return $plan.Stats }
[IO.Directory]::CreateDirectory($Output) | Out-Null
$lockPath = Join-Path $Output '.export.lock'
$lock = [IO.FileStream]::new($lockPath, [IO.FileMode]::CreateNew, [IO.FileAccess]::Write,
    [IO.FileShare]::None, 4096, [IO.FileOptions]::DeleteOnClose)
$staging = $null; $temporaryPointer = $null
try {
    $plan = Get-ExportPlan
    if (-not [IO.Directory]::Exists($plan.Release)) {
        $releaseRoot = Split-Path -Parent $plan.Release
        [IO.Directory]::CreateDirectory($releaseRoot) | Out-Null
        $staging = Join-Path $releaseRoot ('.staging-' + [guid]::NewGuid().ToString('N'))
        [IO.Directory]::CreateDirectory($staging) | Out-Null
        foreach ($record in $plan.Records) {
            $target = Join-Path $staging $record.path
            [IO.Directory]::CreateDirectory((Split-Path -Parent $target)) | Out-Null
            if ($plan.Reusable.ContainsKey($record.path)) {
                New-Item -ItemType HardLink -Path $target -Target $plan.Reusable[$record.path] | Out-Null
            } else {
                Assert-NoLinks $plan.Sources[$record.path]
                [IO.File]::Copy($plan.Sources[$record.path], $target, $false)
            }
            if ((Get-Digest $target) -cne $record.sha256) { throw "Input changed during export: $($record.path)" }
        }
        [IO.File]::WriteAllText((Join-Path $staging 'package-manifest.json'), $plan.PackageJson, [Text.UTF8Encoding]::new($false))
        [IO.Directory]::Move($staging, $plan.Release)
        $staging = $null
    }
    $pointer = Join-Path $Output 'current.json'
    $pointerJson = ConvertTo-PackageJson ([ordered]@{ schemaVersion = 1; releaseId = $plan.Stats.releaseId })
    Assert-NoLinks $pointer
    if (-not [IO.File]::Exists($pointer) -or [IO.File]::ReadAllText($pointer) -cne $pointerJson) {
        $temporaryPointer = Join-Path $Output ('.current-' + [guid]::NewGuid().ToString('N'))
        [IO.File]::WriteAllText($temporaryPointer, $pointerJson, [Text.UTF8Encoding]::new($false))
        if ([IO.File]::Exists($pointer)) { [IO.File]::Replace($temporaryPointer, $pointer, [System.Management.Automation.Language.NullString]::Value) }
        else { [IO.File]::Move($temporaryPointer, $pointer) }
        $temporaryPointer = $null
    }
    $plan.Stats
} finally {
    if ($staging) {
        Assert-NoLinks $staging
        if ((Split-Path -Parent ([IO.Path]::GetFullPath($staging))) -ne (Join-Path $Output 'releases') -or
            (Split-Path -Leaf $staging) -notlike '.staging-*') { throw 'Refusing cleanup outside release staging.' }
        Remove-Item -LiteralPath $staging -Recurse -Force
    }
    if ($temporaryPointer) { Remove-Item -LiteralPath $temporaryPointer -Force }
    $lock.Dispose()
    # DeleteOnClose also releases the lock after a killed exporter process.
}
