<#
.SYNOPSIS
    Compile ClassBoard (Release x64) avec Qt 6 / MSVC.

.DESCRIPTION
    Reproduit le processus de compilation Windows d'OpenBoard :
      1. compile QuaZip (bibliothèque statique) depuis OpenBoard-ThirdParty ;
      2. génère les traductions (.qm) ;
      3. lance qmake + nmake release-install.

    Prérequis (installés automatiquement par la CI) :
      - Visual Studio 2022 (environnement MSVC chargé : vcvars64) ;
      - Qt 6 msvc2022_64 avec qtwebengine, qtmultimedia, qt5compat... dans le PATH ;
      - le dépôt OpenBoard-ThirdParty cloné à côté du dépôt ClassBoard.

.PARAMETER ThirdPartyDir
    Chemin vers OpenBoard-ThirdParty (défaut : ..\OpenBoard-ThirdParty).
#>
param(
    [string]$ThirdPartyDir = (Join-Path $PSScriptRoot "..\..\..\OpenBoard-ThirdParty")
)

$ErrorActionPreference = "Stop"
$Root = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$ThirdPartyDir = Resolve-Path $ThirdPartyDir

function Invoke-Checked {
    param([string]$Exe, [string[]]$Arguments)
    Write-Host ">> $Exe $($Arguments -join ' ')"
    & $Exe @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Exe a échoué (code $LASTEXITCODE)" }
}

# Nombre de cœurs : nmake ne parallélise pas, jom oui s'il est disponible.
$Make = "nmake"
if (Get-Command jom -ErrorAction SilentlyContinue) { $Make = "jom" }

# --- 1. QuaZip ---------------------------------------------------------------
Write-Host "=== QuaZip ==="
Push-Location (Join-Path $ThirdPartyDir "quazip")
try {
    Invoke-Checked "qmake" @("quazip.pro", "-spec", "win32-msvc")
    Invoke-Checked $Make @("release")
} finally { Pop-Location }

# --- 2. Traductions ------------------------------------------------------------
Write-Host "=== Traductions ==="
$tsFiles = Get-ChildItem (Join-Path $Root "resources\i18n") -Filter "*.ts" | ForEach-Object { $_.FullName }
Invoke-Checked "lrelease" (@("-silent") + $tsFiles)

# --- 3. Application ---------------------------------------------------------
Write-Host "=== ClassBoard ==="
Push-Location $Root
try {
    $buildDir = Join-Path $Root "build\win32\release"
    if (Test-Path $buildDir) { Remove-Item -Recurse -Force $buildDir }
    Invoke-Checked "qmake" @("OpenBoard.pro", "-spec", "win32-msvc")
    Invoke-Checked $Make @("release-install")
} finally { Pop-Location }

Write-Host "Compilation terminée : $(Join-Path $Root 'build\win32\release\product')"
