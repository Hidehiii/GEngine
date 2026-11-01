param([ValidateSet('Debug', 'Release', 'Both')][string]$Configuration = 'Both')
$ErrorActionPreference = 'Stop'
$repo = [IO.Path]::GetFullPath("$PSScriptRoot/..")
$buildDirectory = Join-Path $repo 'build/shader-container'
$cmake = (Get-Command cmake).Source
New-Item -ItemType Directory -Force -Path $buildDirectory | Out-Null

function Invoke-Checked([string[]]$Arguments, [string]$LogName) {
    $info = New-Object Diagnostics.ProcessStartInfo
    $info.FileName = $cmake
    $info.Arguments = ($Arguments | ForEach-Object { '"' + $_ + '"' }) -join ' '
    $info.WorkingDirectory = $repo
    $info.UseShellExecute = $false
    $info.CreateNoWindow = $true
    $info.RedirectStandardOutput = $true
    $info.RedirectStandardError = $true
    # Keep the host's duplicate PATH/Path keys out of MSBuild's child environment.
    $info.EnvironmentVariables.Clear()
    foreach ($entry in [Environment]::GetEnvironmentVariables().GetEnumerator()) {
        if ($entry.Key -ine 'Path') { $info.EnvironmentVariables[$entry.Key] = $entry.Value }
    }
    $info.EnvironmentVariables['Path'] = $env:Path
    $process = New-Object Diagnostics.Process
    $process.StartInfo = $info
    try {
        [void]$process.Start()
        $stdout = $process.StandardOutput.ReadToEndAsync()
        $stderr = $process.StandardError.ReadToEndAsync()
        $process.WaitForExit()
        $output = $stdout.Result + $stderr.Result
        [IO.File]::WriteAllText((Join-Path $buildDirectory "$LogName.log"), $output)
        Write-Host "$LogName exit=$($process.ExitCode)"
        if ($process.ExitCode -ne 0) { throw $output }
    } finally { $process.Dispose() }
}

Invoke-Checked @('-S', "$repo/Tests/ShaderContainer", '-B', $buildDirectory,
    '-G', 'Visual Studio 17 2022', '-A', 'x64') 'configure'
$configurations = if ($Configuration -eq 'Both') { @('Debug', 'Release') } else { @($Configuration) }
foreach ($buildConfiguration in $configurations) {
    Invoke-Checked @('--build', $buildDirectory, '--config', $buildConfiguration,
        '--parallel', '4', '--', '/verbosity:minimal', '/nodeReuse:false') "build-$buildConfiguration"
    $executable = Join-Path $buildDirectory "$buildConfiguration/ShaderContainerChecks.exe"
    & $executable
    if ($LASTEXITCODE -ne 0) { throw "Parser regression failed: $buildConfiguration" }
}
