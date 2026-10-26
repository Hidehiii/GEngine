#requires -Version 7.2
param([Parameter(Mandatory)][string]$ProjectRoot, [Parameter(Mandatory)][string]$Pack)
$ErrorActionPreference = 'Stop'
$ProjectRoot = [IO.Path]::GetFullPath($ProjectRoot)
$entries = @()
foreach ($name in 'Config.ini', 'Assets', 'Licenses') {
    $path = Join-Path $ProjectRoot $name
    if (-not (Test-Path -LiteralPath $path)) { continue }
    $pending = [Collections.Generic.Stack[string]]::new(); $pending.Push($path)
    while ($pending.Count) {
        $item = Get-Item -LiteralPath $pending.Pop() -Force
        $relative = [IO.Path]::GetRelativePath($ProjectRoot, $item.FullName).Replace('\', '/')
        if ($relative -imatch '^Assets/Cache(/|$)') { continue }
        if ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) { throw "Linked resource rejected: $relative" }
        if ($item.PSIsContainer) { foreach ($child in Get-ChildItem -LiteralPath $item.FullName -Force) { $pending.Push($child.FullName) }; continue }
        if ($item.Length -gt 8GB) { throw 'Resource exceeds the 8 GiB entry limit.' }
        $entries += @{ Path = $relative; Source = $item.FullName; Size = $item.Length }
    }
}
# A default config allows an otherwise empty native project to be packaged.
$defaultConfig = -not ($entries | Where-Object Path -eq 'Config.ini')
if ($defaultConfig) {
    $bytes=[Text.Encoding]::UTF8.GetBytes("GraphicsAPI: 3`nFramesInFlight: 2`nVSync: 1`nCommandBufferCount: 8`nShaderCacheDirectory: Assets/Cache/Shaders/`nLongPressThresholdMs: 500`nDoubleClickThresholdMs: 500`nWindowManagerAPI: 0`n")
    $entries += @{ Path='Config.ini'; Bytes=$bytes; Size=$bytes.Length }
}
if ($entries.Count -gt 100000 -or ($entries | Measure-Object -Property Size -Sum).Sum -gt 32GB) { throw 'Resource pack limits exceeded.' }
$entries = @($entries | Sort-Object Path -Culture '')
$stream = [IO.File]::Open($Pack, 'CreateNew', 'Write', 'None')
$writer = [IO.BinaryWriter]::new($stream, [Text.UTF8Encoding]::new($false), $false)
try {
    $writer.Write([Text.Encoding]::ASCII.GetBytes('GEPAK001')); $writer.Write([uint32]$entries.Count)
    foreach ($entry in $entries) {
        $pathBytes = [Text.Encoding]::UTF8.GetBytes($entry.Path)
        if ($pathBytes.Length -gt 4096) { throw 'Resource path exceeds 4096 bytes.' }
        $source = if ($entry.ContainsKey('Bytes')) { [IO.MemoryStream]::new([byte[]]$entry.Bytes) } else { [IO.File]::Open($entry.Source, 'Open', 'Read', 'Read') }
        try {
            $hash = [Security.Cryptography.SHA256]::HashData($source); $source.Position = 0
            $writer.Write([uint32]$pathBytes.Length); $writer.Write([uint64]$source.Length); $writer.Write($hash); $writer.Write($pathBytes)
            $source.CopyTo($stream)
        } finally { $source.Dispose() }
    }
    $writer.Flush(); $stream.Flush($true)
} finally { $writer.Dispose() }
