<#
.SYNOPSIS
    Assemble le dossier autonome de ClassBoard (exécutable + DLL + ressources).

.DESCRIPTION
    Copie le produit compilé dans un dossier "dist", puis y ajoute :
      - les DLL et plugins Qt (windeployqt, y compris Qt WebEngine) ;
      - les DLL Poppler, OpenSSL et zlib fournies par OpenBoard-ThirdParty ;
      - le runtime Visual C++ (déploiement local, sans installation séparée) ;
      - les ressources (polices, personnalisations, astuces de démarrage).

    Le dossier obtenu fonctionne tel quel : il sert de base à la version
    portable (.zip) et aux installateurs (.exe et .msi).
#>
param(
    [string]$ThirdPartyDir = (Join-Path $PSScriptRoot "..\..\..\OpenBoard-ThirdParty"),
    [string]$Dist = (Join-Path $PSScriptRoot "..\..\dist\ClassBoard")
)

$ErrorActionPreference = "Stop"
$Root = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$ThirdPartyDir = Resolve-Path $ThirdPartyDir
$Product = Join-Path $Root "build\win32\release\product"

if (-not (Test-Path $Product)) { throw "Produit introuvable : $Product (lancez build.ps1)" }

$exe = Get-ChildItem $Product -Filter "*.exe" | Where-Object { $_.Name -ne "QtWebEngineProcess.exe" } | Select-Object -First 1
if (-not $exe) { throw "Aucun exécutable dans $Product" }
Write-Host "Exécutable : $($exe.Name)"

if (Test-Path $Dist) { Remove-Item -Recurse -Force $Dist }
New-Item -ItemType Directory -Force $Dist | Out-Null
$Dist = Resolve-Path $Dist

# 1. Produit compilé (sans fichiers de débogage / d'édition de liens)
Copy-Item -Recurse -Force (Join-Path $Product "*") $Dist
Get-ChildItem $Dist -Recurse -Include *.pdb, *.ilk, *.exp, *.lib, *.obj | Remove-Item -Force

# 2. Ressources non installées par qmake
foreach ($dir in @("customizations", "startupHints", "fonts")) {
    $src = Join-Path $Root "resources\$dir"
    if (Test-Path $src) { Copy-Item -Recurse -Force $src (Join-Path $Dist $dir) }
}
Copy-Item -Force (Join-Path $Root "resources\windows\xpdfrc") $Dist

# 3. Qt (DLL, plugins, Qt WebEngine)
$deployArgs = @("--release", "--no-compiler-runtime", "--no-system-d3d-compiler", "--no-opengl-sw",
                "--dir", "$Dist", (Join-Path $Dist $exe.Name))
Write-Host ">> windeployqt $($deployArgs -join ' ')"
& windeployqt @deployArgs
if ($LASTEXITCODE -ne 0) { throw "windeployqt a échoué ($LASTEXITCODE)" }

# 4. Bibliothèques tierces
# poppler-cpp.dll n'est pas utilisé sous Windows (seul poppler.dll est lié)
Get-ChildItem (Join-Path $ThirdPartyDir "poppler\bin") -Filter "*.dll" |
    Where-Object { $_.Name -ne "poppler-cpp.dll" } | Copy-Item -Destination $Dist -Force
Copy-Item -Force (Join-Path $ThirdPartyDir "openssl\openssl-3.0.15-win64\bin\libssl-3-x64.dll") $Dist
Copy-Item -Force (Join-Path $ThirdPartyDir "openssl\openssl-3.0.15-win64\bin\libcrypto-3-x64.dll") $Dist
Copy-Item -Force (Join-Path $ThirdPartyDir "zlib\1.2.11\bin\zlib.dll") $Dist

# 5. Runtime Visual C++ (déploiement local : aucun droit administrateur requis)
$redist = $env:VCToolsRedistDir
if (-not $redist) { throw "VCToolsRedistDir non défini : chargez l'environnement MSVC (vcvars64)" }
$crt = Get-ChildItem (Join-Path $redist "x64") -Directory -Filter "Microsoft.VC*.CRT" | Select-Object -First 1
if (-not $crt) { throw "Runtime VC++ introuvable dans $redist" }
Copy-Item -Force (Join-Path $crt.FullName "*.dll") $Dist

# 6. Contrôles
$required = @($exe.Name, "Qt6Core.dll", "Qt6WebEngineCore.dll", "QtWebEngineProcess.exe",
              "poppler.dll", "vcruntime140.dll", "platforms\qwindows.dll", "library", "etc")
foreach ($item in $required) {
    if (-not (Test-Path (Join-Path $Dist $item))) { throw "Élément manquant dans le paquet : $item" }
}

$size = (Get-ChildItem $Dist -Recurse | Measure-Object -Property Length -Sum).Sum / 1MB
Write-Host ("Dossier prêt : {0} ({1:N0} Mo)" -f $Dist, $size)
if ($env:GITHUB_ENV) { "APP_EXE=$($exe.Name)" | Out-File -Encoding ascii -Append $env:GITHUB_ENV }
