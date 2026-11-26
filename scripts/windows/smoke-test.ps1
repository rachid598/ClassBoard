<#
.SYNOPSIS
    Test de démarrage : lance ClassBoard depuis le dossier assemblé et vérifie
    qu'il ne s'arrête pas en erreur (DLL manquante, plantage au démarrage...).

.DESCRIPTION
    Le processus doit rester vivant <Seconds> secondes, puis il est arrêté.
    Les réglages sont isolés dans un dossier temporaire (APPDATA/LOCALAPPDATA)
    pour ne pas dépendre de l'état de la machine.
#>
param(
    [string]$Dist = (Join-Path $PSScriptRoot "..\..\dist\ClassBoard"),
    [int]$Seconds = 25,
    [string]$AppExe = $env:APP_EXE
)

$ErrorActionPreference = "Stop"
$Dist = Resolve-Path $Dist
if (-not $AppExe) {
    $AppExe = (Get-ChildItem $Dist -Filter "*.exe" | Where-Object { $_.Name -notin @("QtWebEngineProcess.exe") } | Select-Object -First 1).Name
}
$exePath = Join-Path $Dist $AppExe

$sandbox = Join-Path $env:RUNNER_TEMP "classboard-smoke"
if (-not $env:RUNNER_TEMP) { $sandbox = Join-Path $env:TEMP "classboard-smoke" }
New-Item -ItemType Directory -Force $sandbox | Out-Null

$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = $exePath
$psi.WorkingDirectory = $Dist
$psi.UseShellExecute = $false
$psi.EnvironmentVariables["APPDATA"] = (Join-Path $sandbox "Roaming")
$psi.EnvironmentVariables["LOCALAPPDATA"] = (Join-Path $sandbox "Local")
$psi.EnvironmentVariables["CLASSBOARD_SKIP_FIRST_RUN"] = "1"
$psi.EnvironmentVariables["QT_LOGGING_RULES"] = "*.debug=false"

Write-Host "Lancement : $exePath"
$p = [System.Diagnostics.Process]::Start($psi)
$exited = $p.WaitForExit($Seconds * 1000)
if ($exited) {
    throw ("ClassBoard s'est arrêté au bout de moins de {0} s (code {1:X8})" -f $Seconds, $p.ExitCode)
}
Write-Host "OK : ClassBoard tourne depuis $Seconds s (PID $($p.Id), mémoire $([int]($p.WorkingSet64 / 1MB)) Mo)"
Stop-Process -Id $p.Id -Force
Get-Process -Name "QtWebEngineProcess" -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
