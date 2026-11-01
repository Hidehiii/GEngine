param([switch]$NoPopup)

$ErrorActionPreference = 'Stop'

function Test-BlockedD3D12Version([Version]$Version) {
    return $Version -eq [Version]'10.0.26100.4768'
}

function Get-D3D12RuntimeBuildError {
    # The engine targets x64; Sysnative avoids WOW64 redirection in 32-bit hosts.
    $systemDirectory = if ([Environment]::Is64BitOperatingSystem -and -not [Environment]::Is64BitProcess) {
        Join-Path $env:SystemRoot 'Sysnative'
    } else {
        Join-Path $env:SystemRoot 'System32'
    }
    $layer = Join-Path $systemDirectory 'd3d12SDKLayers.dll'
    if (-not [IO.File]::Exists($layer)) {
        Write-Host 'D3D12 runtime check: optional system validation layer is not installed.'
        return
    }
    $info = [Diagnostics.FileVersionInfo]::GetVersionInfo($layer)
    $version = [Version]::new($info.FileMajorPart, $info.FileMinorPart, $info.FileBuildPart, $info.FilePrivatePart)
    if ($version -eq [Version]'0.0.0.0') {
        throw "Cannot read D3D12 validation runtime version: $layer"
    }
    Write-Host "D3D12 runtime check: $layer ($version)"
    if (Test-BlockedD3D12Version $version) {
        return @"
Build rejected: D3D12 validation runtime $version leaks Event handles during pending cross-queue waits.
File: $layer

How to change versions:
1. Install available Windows Updates and restart if requested. In Windows Settings, search Optional features > Graphics Tools to manage the OS debug-layer component.
2. Recheck: powershell -NoProfile -File BuildScripts/CheckD3D12Runtime.ps1 -NoPopup
   Rebuild only after the reported version changes. Reinstalling Graphics Tools on the same OS may keep the same version.
3. Changing Visual Studio's Windows SDK Version changes build headers/libraries, not this system DLL; it will not clear this rejection.
4. App-local Agility SDK selection requires separate engine integration; installing its NuGet package alone does not switch this project's runtime or bypass this build check.
Details: Documentation/Guides/BuildAndRun.md (Changing SDK/runtime versions).
"@
    }
}

# Dot sourcing exposes the exact-version policy to focused regression checks.
if ($MyInvocation.InvocationName -eq '.') { return }

try {
    $failure = Get-D3D12RuntimeBuildError
} catch {
    $failure = "D3D12 runtime build check failed: $($_.Exception.Message)"
}
if ($failure) {
    [Console]::Error.WriteLine($failure)
    if (-not $NoPopup -and $env:GENGINE_BUILD_NO_POPUP -ne '1' -and -not $env:CI) {
        $popup = $null
        try {
            $popup = New-Object -ComObject WScript.Shell
            # Bounded warning: unattended builds still terminate with failure.
            [void]$popup.Popup($failure, 10, 'GEngine - incompatible D3D12 runtime', 48)
        } catch {
            [Console]::Error.WriteLine("Warning popup unavailable: $($_.Exception.Message)")
        } finally {
            if ($null -ne $popup) { [void][Runtime.InteropServices.Marshal]::FinalReleaseComObject($popup) }
        }
    }
    exit 1
}
exit 0
