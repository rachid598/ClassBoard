<#
.SYNOPSIS
    Test de démarrage : lance ClassBoard depuis le dossier assemblé et vérifie
    qu'il ne s'arrête pas en erreur (DLL manquante, plantage au démarrage...).

.DESCRIPTION
    Le processus doit rester vivant <Seconds> secondes, puis il est arrêté.
    Prévu pour la CI (machine éphémère) : avec -ProfileFile, le profil fourni est
    copié dans %APPDATA%\ClassBoard (Qt lit l'emplacement réel du dossier, pas la
    variable d'environnement), puis supprimé après le test.
#>
param(
    [string]$Dist = (Join-Path $PSScriptRoot "..\..\dist\ClassBoard"),
    [int]$Seconds = 25,
    [string]$AppExe = $env:APP_EXE,
    # Profil ClassBoard à injecter (fichier INI) pour tester une configuration particulière
    [string]$ProfileFile = ""
)

$ErrorActionPreference = "Stop"
$Dist = Resolve-Path $Dist
if (-not $AppExe) {
    $AppExe = (Get-ChildItem $Dist -Filter "*.exe" | Where-Object { $_.Name -notin @("QtWebEngineProcess.exe") } | Select-Object -First 1).Name
}
$exePath = Join-Path $Dist $AppExe

$profileDir = Join-Path ([Environment]::GetFolderPath("ApplicationData")) "ClassBoard"
$profileTarget = Join-Path $profileDir "ClassBoardProfile.ini"
if (Test-Path $profileTarget) { Remove-Item -Force $profileTarget }

if ($ProfileFile) {
    New-Item -ItemType Directory -Force $profileDir | Out-Null
    Copy-Item $ProfileFile $profileTarget
    Write-Host "Profil injecté : $ProfileFile -> $profileTarget"
}

$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = $exePath
$psi.WorkingDirectory = $Dist
$psi.UseShellExecute = $false
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
# ClassBoard n'accepte qu'une instance : attendre l'arrêt complet avant un autre test
$baseName = [IO.Path]::GetFileNameWithoutExtension($AppExe)
for ($i = 0; $i -lt 20 -and (Get-Process -Name $baseName, "QtWebEngineProcess" -ErrorAction SilentlyContinue); $i++) {
    Start-Sleep -Milliseconds 500
}
if (Test-Path $profileTarget) { Remove-Item -Force $profileTarget -ErrorAction SilentlyContinue }
