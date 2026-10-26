#requires -Version 7.2
[CmdletBinding()]
param([Parameter(Mandatory)][string]$Project, [Parameter(Mandatory)][string]$Output)
$ErrorActionPreference = 'Stop'
$Project = [IO.Path]::GetFullPath($Project); $Output = [IO.Path]::GetFullPath($Output)
if ([IO.Path]::GetExtension($Project) -ine '.gproject' -or (Get-Item -LiteralPath $Project).Length -gt 1MB) { throw 'Choose a valid .gproject file.' }
$spec = Get-Content -LiteralPath $Project -Raw | ConvertFrom-Json -AsHashtable
if ($spec.Count -ne 2 -or $spec.schemaVersion -ne 1 -or $spec.name -isnot [string] -or -not $spec.name.Trim() -or $spec.Contains('packaging')) {
    throw 'Project files contain project metadata only; remove legacy packaging information.'
}
$projectRoot = Split-Path -Parent $Project
if ($Output.Equals($projectRoot,[StringComparison]::OrdinalIgnoreCase) -or
    $Output.StartsWith($projectRoot + [IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase) -or
    $projectRoot.StartsWith($Output.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)) {
    throw 'Choose an output folder outside the project.'
}
# Closing this process' job handle on forced cancellation also stops its compiler tree.
if (-not ('GEngineBuildJob' -as [type])) { Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public static class GEngineBuildJob {
 [StructLayout(LayoutKind.Sequential)] struct Basic { public long A,B; public uint Flags; public UIntPtr C,D; public uint E; public UIntPtr F; public uint G,H; }
 [StructLayout(LayoutKind.Sequential)] struct IO { public ulong A,B,C,D,E,F; }
 [StructLayout(LayoutKind.Sequential)] struct Extended { public Basic Basic; public IO IO; public UIntPtr A,B,C,D; }
 [DllImport("kernel32.dll", CharSet=CharSet.Unicode)] static extern IntPtr CreateJobObject(IntPtr a, string n);
 [DllImport("kernel32.dll")] static extern bool SetInformationJobObject(IntPtr j, int c, ref Extended i, int s);
 [DllImport("kernel32.dll")] static extern bool AssignProcessToJobObject(IntPtr j, IntPtr p);
 [DllImport("kernel32.dll")] static extern IntPtr GetCurrentProcess();
 [DllImport("kernel32.dll")] static extern bool CloseHandle(IntPtr h);
 public static IntPtr Begin() { var j=CreateJobObject(IntPtr.Zero,null); var e=new Extended(); e.Basic.Flags=0x2000;
   if(j==IntPtr.Zero || !SetInformationJobObject(j,9,ref e,Marshal.SizeOf<Extended>()) || !AssignProcessToJobObject(j,GetCurrentProcess())) {
     if(j!=IntPtr.Zero) CloseHandle(j); throw new Exception("Cannot establish compiler process cleanup job."); } return j; }
 public static void End(IntPtr j) { var e=new Extended(); SetInformationJobObject(j,9,ref e,Marshal.SizeOf<Extended>()); CloseHandle(j); }
}
'@
}
$job = [GEngineBuildJob]::Begin()
$mutexId = [Convert]::ToHexString([Security.Cryptography.SHA256]::HashData([Text.Encoding]::UTF8.GetBytes($Project.ToLowerInvariant())))
$mutex = [Threading.Mutex]::new($false, "Local\GEngineProject-$mutexId")
$owned = $false; $temporary = $null
try {
    try { $owned = $mutex.WaitOne(0) } catch [Threading.AbandonedMutexException] { $owned = $true }
    if (-not $owned) { throw 'This project is already being compiled/packaged.' }
    [Console]::Error.WriteLine('Compiling project and engine (Release)...')
    $nativeRoot = & (Join-Path $PSScriptRoot 'BuildProject.ps1') -ProjectRoot $projectRoot -Name $spec.name
    $temporary = Join-Path ([IO.Path]::GetTempPath()) ('gengine-package-' + [guid]::NewGuid().ToString('N'))
    [IO.Directory]::CreateDirectory($temporary) | Out-Null
    $packName = $spec.name + '.gpak'
    [Console]::Error.WriteLine('Creating resource pack...')
    & (Join-Path $PSScriptRoot 'WriteResourcePack.ps1') -ProjectRoot $projectRoot -Pack (Join-Path $temporary $packName)
    [Console]::Error.WriteLine('Publishing executable and resource pack...')
    $result = & (Join-Path $PSScriptRoot 'PublishProject.ps1') -NativeRoot $nativeRoot -Pack (Join-Path $temporary $packName) -Name $spec.name -Output $Output
    $result | ConvertTo-Json -Depth 20
} finally {
    if ($temporary -and [IO.Directory]::Exists($temporary)) {
        $full=[IO.Path]::GetFullPath($temporary)
        if ((Split-Path -Parent $full) -ne ([IO.Path]::GetTempPath()).TrimEnd('\','/') -or (Split-Path -Leaf $full) -notlike 'gengine-package-*') { throw 'Refusing staging cleanup outside the temporary directory.' }
        Remove-Item -LiteralPath $full -Recurse -Force
    }
    if ($owned) { $mutex.ReleaseMutex() }; $mutex.Dispose(); [GEngineBuildJob]::End($job)
}
