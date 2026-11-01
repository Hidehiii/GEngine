$ErrorActionPreference = 'Stop'
. "$PSScriptRoot/../BuildScripts/CheckD3D12Runtime.ps1"

if (-not (Test-BlockedD3D12Version ([Version]'10.0.26100.4768'))) {
    throw 'The reproduced failing validation runtime must be excluded.'
}
foreach ($allowed in @('10.0.26100.4767', '10.0.26100.4769', '10.0.26101.4768', '10.1.26100.4768', '11.0.26100.4768', '1.619.5.0')) {
    if (Test-BlockedD3D12Version ([Version]$allowed)) {
        throw "Exact-version exclusion incorrectly rejected $allowed"
    }
}
Write-Host 'PASS: reproduced version excluded; neighboring and alternative versions allowed.'
