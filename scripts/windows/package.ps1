<#
.SYNOPSIS
    Produit les livrables Windows de ClassBoard à partir du dossier dist.

.DESCRIPTION
    Génère dans <OutDir> :
      - ClassBoard-Portable-x64.zip  (version portable, aucun droit requis)
      - ClassBoard-Setup-x64.exe     (Inno Setup, installation utilisateur ou machine)
      - ClassBoard-Setup-x64.msi     (WiX, déploiement silencieux Kwartz/GPO)
      - SHA256SUMS.txt               (sommes de contrôle)

    Prérequis : Inno Setup 6 (iscc) et WiX 5 (outil dotnet "wix").
#>
param(
    [Parameter(Mandatory = $true)][string]$Version,
    [string]$Dist = (Join-Path $PSScriptRoot "..\..\dist\ClassBoard"),
    [string]$OutDir = (Join-Path $PSScriptRoot "..\..\dist\out"),
    [string]$AppExe = $env:APP_EXE
)

$ErrorActionPreference = "Stop"
$Root = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$Dist = Resolve-Path $Dist
New-Item -ItemType Directory -Force $OutDir | Out-Null
$OutDir = Resolve-Path $OutDir
if (-not $AppExe) { $AppExe = "ClassBoard.exe" }

# Version numérique stricte pour MSI / Inno (ex. "0.1.0-beta.2" -> "0.1.0")
$numeric = ($Version -replace '^v', '') -replace '[-+].*$', ''
if ($numeric -notmatch '^\d+\.\d+\.\d+$') { throw "Version invalide : $Version" }
Write-Host "Version : $Version (numérique : $numeric) — exécutable : $AppExe"

# --- Portable ----------------------------------------------------------------
$zip = Join-Path $OutDir "ClassBoard-Portable-x64.zip"
if (Test-Path $zip) { Remove-Item $zip }
# Marqueur de mode portable : les réglages restent dans le dossier de l'application
$portableFlag = Join-Path $Dist "portable.txt"
Set-Content -Encoding utf8 $portableFlag "Ce fichier active le mode portable de ClassBoard : les préférences sont enregistrées dans le sous-dossier 'settings'. Supprimez-le pour utiliser le profil Windows."
Compress-Archive -Path $Dist -DestinationPath $zip -CompressionLevel Optimal
Remove-Item $portableFlag

# --- Inno Setup (.exe) -------------------------------------------------------
$iscc = (Get-Command iscc -ErrorAction SilentlyContinue).Source
if (-not $iscc) {
    foreach ($c in @("${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe", "$env:ProgramFiles\Inno Setup 6\ISCC.exe")) {
        if (Test-Path $c) { $iscc = $c; break }
    }
}
if (-not $iscc) { throw "Inno Setup (ISCC.exe) introuvable" }
& $iscc "/Qp" "/DAppVersion=$numeric" "/DAppExe=$AppExe" "/DSourceDir=$Dist" "/DOutputDir=$OutDir" (Join-Path $Root "packaging\windows\ClassBoard.iss")
if ($LASTEXITCODE -ne 0) { throw "Inno Setup a échoué ($LASTEXITCODE)" }

# --- WiX (.msi) ------------------------------------------------------------------
$msi = Join-Path $OutDir "ClassBoard-Setup-x64.msi"
Push-Location (Join-Path $Root "packaging\windows")
try {
    & wix build "ClassBoard.wxs" -arch x64 -d "Version=$numeric" -d "AppExe=$AppExe" -bindpath "dist=$Dist" -o $msi
    if ($LASTEXITCODE -ne 0) { throw "WiX a échoué ($LASTEXITCODE)" }
} finally { Pop-Location }
Get-ChildItem $OutDir -Filter "*.wixpdb" | Remove-Item -Force

# --- Sommes de contrôle ------------------------------------------------------------
$files = @("ClassBoard-Setup-x64.exe", "ClassBoard-Setup-x64.msi", "ClassBoard-Portable-x64.zip")
$lines = foreach ($f in $files) {
    $p = Join-Path $OutDir $f
    if (-not (Test-Path $p)) { throw "Livrable manquant : $f" }
    "{0}  {1}" -f (Get-FileHash -Algorithm SHA256 $p).Hash.ToLower(), $f
}
$lines | Set-Content -Encoding ascii (Join-Path $OutDir "SHA256SUMS.txt")
Get-Content (Join-Path $OutDir "SHA256SUMS.txt")
Get-ChildItem $OutDir | Format-Table Name, @{ n = "Mo"; e = { "{0:N1}" -f ($_.Length / 1MB) } }
