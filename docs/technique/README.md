# Documentation technique

Voir aussi [ARCHITECTURE.md](../../ARCHITECTURE.md) (choix et structure) et
[HANDOVER.md](../../HANDOVER.md) (état courant).

## 1. Chaîne CI (GitHub Actions)

Fichier : `.github/workflows/build.yml`. Déclencheurs : push sur toute branche, pull
request, lancement manuel, tags `v*`.

| Job | Runner | Rôle |
|---|---|---|
| `linux` | ubuntu-24.04 | Compilation CMake/Ninja (ccache), tests CTest. Contrôle rapide. |
| `windows` | windows-2022 | Compilation Release x64, test de démarrage, installateurs, artefacts. |
| `release` | ubuntu-24.04 | Sur tag `v*` uniquement : crée la GitHub Release avec les livrables. |

Étapes du job Windows :

1. `actions/checkout` de ClassBoard et d'`OpenBoard-org/OpenBoard-ThirdParty`
   (commit figé `THIRDPARTY_REF`) : Poppler, OpenSSL 3, zlib précompilés (MSVC), sources QuaZip.
2. `jurplel/install-qt-action` : Qt `6.8.3` `win64_msvc2022_64` + modules
   `qtwebengine qtwebchannel qtpositioning qtmultimedia qt5compat qtimageformats qtactiveqt`
   (mis en cache).
3. `ilammy/msvc-dev-cmd` (vcvars64), `jom` via Chocolatey.
4. `scripts/windows/build.ps1` : QuaZip statique → `lrelease` → `qmake OpenBoard.pro` → `jom release-install`.
5. `scripts/windows/deploy.ps1` : dossier autonome `dist/ClassBoard` (windeployqt, DLL
   ThirdParty, runtime VC++ local, traductions Qt, polices, bibliothèque).
6. `scripts/windows/smoke-test.ps1` : lance `ClassBoard.exe` (profil isolé, assistant
   désactivé) et vérifie qu'il tourne 25 s sans s'arrêter (DLL manquante, plantage…).
7. `scripts/windows/package.ps1` : zip portable (+ `portable.txt`), Inno Setup
   (`packaging/windows/ClassBoard.iss`), WiX 5 (`packaging/windows/ClassBoard.wxs`), `SHA256SUMS.txt`.
8. Artefact `ClassBoard-<version>-windows-x64` (conservé 30 jours).

### Publier une version

```bash
# 1. mettre à jour version.txt et CHANGELOG.md, committer
git tag v0.1.0
git push origin v0.1.0
```

Le job `release` publie `ClassBoard-Setup-x64.exe`, `ClassBoard-Setup-x64.msi`,
`ClassBoard-Portable-x64.zip` et `SHA256SUMS.txt`, avec les notes de
`packaging/release-notes.md`. Un tag contenant `-` (ex. `v0.2.0-beta.1`) crée une préversion.

### Signature de code

Non configurée : aucun certificat disponible. Les exécutables ne sont **pas signés**
(avertissement SmartScreen). Pour l'ajouter : stocker le certificat dans les secrets du
dépôt et signer `ClassBoard.exe`, puis l'installateur et le MSI avec `signtool` dans
`package.ps1`.

## 2. Compiler sous Windows (poste de développement)

Prérequis : Visual Studio 2022 (C++), Qt 6.8 MSVC 2022 64 bits avec les modules
ci-dessus, Inno Setup 6, `dotnet tool install --global wix --version 5.0.2`.

```powershell
git clone https://github.com/OpenBoard-org/OpenBoard-ThirdParty.git ..\OpenBoard-ThirdParty
# Dans une "x64 Native Tools Command Prompt" avec Qt dans le PATH :
pwsh scripts\windows\build.ps1
pwsh scripts\windows\deploy.ps1
pwsh scripts\windows\package.ps1 -Version 0.1.0
```

## 3. Compiler sous Linux (développement, tests)

```bash
sudo apt install cmake ninja-build qt6-base-dev qt6-webengine-dev qt6-multimedia-dev \
  qt6-svg-dev qt6-tools-dev qt6-l10n-tools qt6-5compat-dev libqt6svgwidgets6 \
  libpoppler-dev libpoppler-cpp-dev libpoppler-private-dev libquazip1-qt6-dev libssl-dev \
  libavcodec-dev libavformat-dev libswresample-dev libswscale-dev libx11-dev libpaper-dev
cmake -S . -B build -G Ninja -DQT_VERSION=6 -DCLASSBOARD_BUILD_TESTS=ON -DCMAKE_INSTALL_PREFIX=/opt
cmake --build build
ctest --test-dir build --output-on-failure
sudo cmake --install build && classboard
```

Captures d'écran sans écran : `Xvfb :99 &` puis `DISPLAY=:99 classboard` et `import -window root capture.png`.

## 4. Tests

| Test | Contenu |
|---|---|
| `classboard_profile` (`tests/tst_classboardprofile.cpp`) | Catalogue des disciplines, fusion des favoris, persistance du profil, thème, bornes, réinitialisation, mode portable |
| `classboard_widgets` (`tests/check_widgets.cmake`) | Présence et validité de `config.xml`/`index.html`/`icon.png`, absence de ressource réseau |
| Test de démarrage Windows (`smoke-test.ps1`) | L'exécutable empaqueté démarre et reste actif 25 s, avec le profil par défaut puis avec `tests/profiles/sombre-sans-libelles.ini` |
| Test de démarrage Linux (Xvfb, job `linux`) | Mêmes deux profils, sous Xvfb |

## 5. Le module `src/classboard`

| Classe | Rôle | Dépendances |
|---|---|---|
| `ClassBoardProfile` | Lecture/écriture du profil INI (disciplines, thème, échelle, favoris, langue) | QtCore |
| `ClassBoardDisciplines` | Catalogue statique des 15 disciplines ; fusion « à tour de rôle » des favoris | QtCore |
| `ClassBoardTheme` | Couleurs clair/sombre, palette Qt, chargement de `ClassBoard(-dark).css`, icônes recolorées | QtWidgets, QtSvg |
| `ClassBoardIcons` | Rendu des icônes SVG (états normal/actif/désactivé) et application aux actions | QtSvg, UBMainWindow |
| `ClassBoardSetupDialog` | Assistant (3 étapes) et écran « Mon profil » | QtWidgets |
| `ClassBoardFavoritesPalette` | Barre de favoris ; un clic appelle `UBBoardController::downloadURL` | moteur OpenBoard |

### Identifiants de favoris

- `openboardtool://ruler|triangle|protractor|compass|axes|cache|magnifier`,
  `openboardtool://openboard/mask` : instruments natifs (`UBToolsManager`).
- `widget:<Nom>` : `library/applications/<Nom>.wgt`.

### Icônes

Les tracés sont définis dans `tools/icons/generate_icons.py` (source unique) :

```bash
python3 tools/icons/generate_icons.py   # régénère resources/classboard/{disciplines,tools,icons}/*.svg
```

Ajouter une icône : l'ajouter au dictionnaire `TOOLS`, régénérer, l'ajouter dans
`resources/classboard/classboard.qrc`. Les icônes PNG des widgets se produisent avec
`tools/icons/svg2png.cpp`.

## 6. Widgets web

Format W3C Widgets d'OpenBoard : dossier `<Nom>.wgt/` avec `config.xml`, `index.html`,
`icon.png`. API disponible dans la page : `window.sankore.setPreference(clé, valeur)`,
`window.sankore.async.preference(clé, défaut)` (Promise) pour mémoriser l'état dans le document.
Contraintes ClassBoard : aucune URL `http(s)://` dans `index.html` (vérifié par les tests),
HTML/CSS/JS autonomes, boutons ≥ 44 px.
