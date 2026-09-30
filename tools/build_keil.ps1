param(
    [string]$ProjectRoot = (Split-Path -Parent $PSScriptRoot),
    [string]$Uv4Path = ''
)

$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($Uv4Path)) {
    $candidates = @(
        $env:KEIL_UV4_PATH,
        'E:\Keil_v5\MDK\UV4\UV4.exe',
        'E:\Keil_v5\UV4\UV4.exe',
        'C:\Keil_v5\UV4\UV4.exe',
        'C:\Program Files (x86)\Keil_v5\UV4\UV4.exe'
    ) | Where-Object { -not [string]::IsNullOrWhiteSpace($_) }
    $Uv4Path = $candidates |
        Where-Object { Test-Path -LiteralPath $_ } |
        Select-Object -First 1
}

if ([string]::IsNullOrWhiteSpace($Uv4Path) -or -not (Test-Path -LiteralPath $Uv4Path)) {
    throw 'Keil UV4.exe was not found. Set KEIL_UV4_PATH or pass -Uv4Path with the installed executable.'
}

& (Join-Path $PSScriptRoot 'generate_keil_project.ps1') -ProjectRoot $ProjectRoot | Out-Null

$project = Join-Path $ProjectRoot 'legacy\nun_dx_original\MDK-ARM\stm32f103_template.uvprojx'
$buildDirectory = Join-Path $ProjectRoot 'build'
$log = Join-Path $buildDirectory 'keil_build.log'
New-Item -ItemType Directory -Path $buildDirectory -Force | Out-Null

$process = Start-Process `
    -FilePath $Uv4Path `
    -ArgumentList @('-j0', '-r', $project, '-o', $log) `
    -Wait `
    -PassThru `
    -WindowStyle Hidden
$uvExitCode = $process.ExitCode

if (Test-Path -LiteralPath $log) {
    Get-Content -LiteralPath $log
}

if (($null -ne $uvExitCode) -and ($uvExitCode -ne 0)) {
    throw "Keil rebuild failed with exit code $uvExitCode"
}

$logText = Get-Content -LiteralPath $log -Raw
if ($logText -notmatch '0 Error\(s\), 0 Warning\(s\)') {
    throw 'Keil rebuild did not report 0 errors and 0 warnings.'
}

Write-Output "Keil rebuild passed: $project"

