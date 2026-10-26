param(
    [switch]$Force
)

$ErrorActionPreference = 'Stop'

$version = '6.10.0.87'
$msiName = "mono-$version-x64-0.msi"
$msiUrl = "https://download.mono-project.com/archive/6.10.0/windows-installer/$msiName"
$expectedHash = '8853A0F1CAA495CE49D0F61BA72C904D0D1DAC0C1391949157DC7182CC9D0FE4'
$repoRoot = Split-Path -Parent $PSScriptRoot
$buildPath = Join-Path $repoRoot "build/mono/windows-x86_64/$version"
$installPath = Join-Path $repoRoot "dist/mono/windows-x86_64/$version"
$sdkRoot = Join-Path $installPath 'Mono'

function Remove-WorkspaceDirectory([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path)) { return }
    $resolved = (Resolve-Path -LiteralPath $Path).Path
    $workspace = (Resolve-Path -LiteralPath $repoRoot).Path
    if (-not $resolved.StartsWith($workspace, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to remove path outside workspace: $resolved"
    }
    Remove-Item -LiteralPath $resolved -Recurse -Force
}

if ($Force) { Remove-WorkspaceDirectory $installPath }

if (Test-Path -LiteralPath (Join-Path $sdkRoot 'include/mono-2.0')) {
    Write-Host "Mono SDK already installed: $sdkRoot"
    return
}

New-Item -ItemType Directory -Force -Path $buildPath, $installPath | Out-Null
$msiPath = Join-Path $buildPath $msiName

$actualHash = $null
if (Test-Path -LiteralPath $msiPath) {
    $actualHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $msiPath).Hash
}
if ($actualHash -ne $expectedHash) {
    Write-Host "Downloading $msiUrl"
    curl.exe --ssl-no-revoke -L --retry 3 --output $msiPath $msiUrl
    if ($LASTEXITCODE -ne 0) { throw "Failed to download Mono SDK installer." }
    $actualHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $msiPath).Hash
    if ($actualHash -ne $expectedHash) {
        throw "Mono SDK hash mismatch. Expected $expectedHash, got $actualHash."
    }
}

Write-Host "Extracting $msiPath"
$process = Start-Process -FilePath msiexec.exe `
    -ArgumentList @('/a', "`"$msiPath`"", '/qn', "TARGETDIR=`"$installPath`"") `
    -WindowStyle Hidden -Wait -PassThru
if ($process.ExitCode -ne 0) {
    throw "Mono SDK administrative extraction failed with exit code $($process.ExitCode)."
}

if (-not (Test-Path -LiteralPath (Join-Path $sdkRoot 'lib/libmono-static-sgen.lib'))) {
    throw 'Mono SDK extraction did not produce libmono-static-sgen.lib.'
}

curl.exe --ssl-no-revoke -L --silent --output (Join-Path $sdkRoot 'LICENSE') `
    https://raw.githubusercontent.com/mono/mono/mono-$version/LICENSE
if ($LASTEXITCODE -ne 0) { throw 'Failed to download Mono LICENSE.' }

curl.exe --ssl-no-revoke -L --silent --output (Join-Path $sdkRoot 'PATENTS.TXT') `
    https://raw.githubusercontent.com/mono/mono/mono-$version/PATENTS.TXT
if ($LASTEXITCODE -ne 0) { throw 'Failed to download Mono PATENTS.TXT.' }

$manifest = [PSCustomObject]@{
    Version = $version
    Source = $msiUrl
    Sha256 = $expectedHash
    SdkRoot = $sdkRoot
}
$manifest | ConvertTo-Json | Set-Content -Encoding UTF8 (Join-Path $installPath 'mono-sdk.json')

Write-Host "Mono SDK installed: $sdkRoot"
