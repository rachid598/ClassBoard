# HANDOVER — reprendre le développement de ClassBoard

Document de passation destiné à un développeur ou une IA qui reprend le projet.
**À tenir à jour à chaque étape importante.** Dernière mise à jour : 2026-11-27.

## 1. Le projet en une minute

- **Objectif** : remplacer Interwrite Workspace dans un collège (TBI/vidéoprojecteurs
  interactifs Epson, Windows 10/11, réseau Kwartz), pour toutes les disciplines.
- **Base** : fork d'OpenBoard 1.7.7 (branche `master` amont, historique conservé,
  remote `upstream` = https://github.com/OpenBoard-org/OpenBoard).
- **Dépôt** : https://github.com/rachid598/ClassBoard (privé). Branche de travail :
  `claude/gifted-turing-1gc7gi` (c'est aussi, à ce jour, la seule branche du dépôt).
- **Langage** : C++17 / Qt 6 (6.8.3 sous Windows, 6.4 sous Ubuntu 24.04). Pas de Rust (voir ARCHITECTURE.md).
- **Contraintes** : hors connexion, sans compte, sans télémétrie, RGPD, sans droits
  administrateur, GPL-3.0, interface française par défaut.

## 2. Ce qui est fait (v0.2.1)

| Domaine | État | Où |
|---|---|---|
| Import OpenBoard + historique | ✅ | — |
| Renommage ClassBoard + nouveau logo 2026 (SVG/PNG/ICO multi-tailles) | ✅ | `resources/classboard/classboard.svg`, `resources/win/ClassBoard.ico`, `UBApplication.cpp` |
| Français par défaut | ✅ | `UBApplication::setupTranslators` |
| Pas de requête réseau de mise à jour | ✅ | `ClassBoard.config`, `UBApplicationController::checkUpdate` |
| Profil enseignant + 15 disciplines + côté de la barre + dossier documents Kwartz | ✅ testé | `src/classboard/ClassBoardProfile*`, `ClassBoardDisciplines*`, `UBSettings.cpp` |
| Accueil 1 écran (tuiles bicolores) + « Mes matières et préférences » (live) | ✅ vérifié visuellement (Linux/Xvfb) | `ClassBoardSetupDialog*` |
| Barre verticale moderne à droite + zoom/Main permanents + renommage & export PDF 1-clic | ✅ vérifié visuellement (Linux/Xvfb, clair et ardoise) | `ClassBoardBoardChrome*` |
| Mode Bureau : barre à droite + Annuler (`Ctrl+Z`) + bandeau orange d'annotation | ✅ vérifié (Linux build + code) | `UBDesktopPalette*`, `UBDesktopAnnotationController*` |
| Mode Documents : boutons tactiles `−` / `+` autour du zoom des vignettes | ✅ vérifié visuellement (Linux/Xvfb) | `src/document/UBDocumentController.cpp` |
| Tiroir « Outils pédagogiques » (disciplines + formes rapides + export PDF) + Sélecteur de fonds | ✅ vérifié visuellement (Linux/Xvfb) | `ClassBoardToolsDrawer*`, `ClassBoardBackgroundPopover*` |
| Thème clair (blanc défaut) / ardoise sombre 1-clic, icônes bicolores | ✅ vérifié (Linux) | `ClassBoardTheme*`, `ClassBoardIcons*`, `resources/etc/ClassBoard*.css` |
| 5 widgets hors connexion : Minuteur, Tirage au sort, Vocabulaire, Étiquettes, **Lecteur audio** (vitesse + boucle A-B) | ✅ vérifié (Linux, CTest widgets hors connexion OK) | `resources/library/applications/*.wgt` |
| Seyès, polices Marelle/Écolier | ✅ | `ClassBoard.config`, `resources/customizations/fonts` |
| Mode portable | ✅ (logique testée, pas testé sous Windows réel) | `UBSettings::userDataDirectory`, `ClassBoardProfile::isPortable` |
| CI Linux + Windows x64 + GitHub Release auto sur tag `v*` | ✅ (`v0.2.0` publiée, `v0.2.1` en cours) | `.github/workflows/build.yml`, `scripts/windows/*`, `packaging/windows/*` |
| Documentation | ✅ | `README.md`, `docs/`, `ARCHITECTURE.md`, `ROADMAP.md`… |

Captures réelles mises à jour : `docs/images/` (prises sous Linux/Xvfb 1600×900).

## 3. Ce qui n'a PAS été vérifié

- **Aucun test sur un TBI Epson réel**, ni sur écran tactile/stylet réel.
- Déploiement MSI et sauvegarde sur un partage réseau **Kwartz** réel.
- Import des fichiers **Workspace `.gwb`** : non étudié en pratique (aucun fichier d'exemple).

## 4. État de la CI et des Releases

Workflow `Build` (`.github/workflows/build.yml`) : https://github.com/rachid598/ClassBoard/actions
Releases téléchargeables : https://github.com/rachid598/ClassBoard/releases

- **Release `v0.2.0`** (26/11/2026) : publiée et vérifiée verte (Linux ✓, Windows x64 ✓,
  GitHub Release ✓) avec les 4 paquets : `ClassBoard-Setup-x64.msi`, `ClassBoard-Setup-x64.exe`,
  `ClassBoard-Portable-x64.zip`, `SHA256SUMS.txt`.
- **Release `v0.2.1`** (27/11/2026) : publiée et vérifiée verte (Linux ✓, Windows x64 ✓,
  GitHub Release ✓).
- **Release `v0.2.2`** (27/11/2026) : suppression de l'ancien tiroir gris `UBRightPalette`
  en mode Bureau et modernisation des sous-palettes couleurs/épaisseurs du mode Bureau.
- **Publication de Release** : pousser un tag `vX.Y.Z` (`git tag vX.Y.Z && git push origin vX.Y.Z`)
  déclenche automatiquement le job `release` (`softprops/action-gh-release@v2`) qui joint les
  4 fichiers Windows x64 à la GitHub Release.
- Les exécutions s'enchaînent sans annulation (`cancel-in-progress: false`).
- Les modifications de `*.md` et `docs/**` seules ne déclenchent pas la CI.

Corrections déjà faites :
1. `tools: tools_jom` d'install-qt-action ne fonctionne plus (aqt ne trouve pas l'outil) →
   `jom` installé par Chocolatey.
2. Le test `classboard_widgets` échouait tant que les widgets n'étaient pas poussés (normal).

## 5. Pièges connus

- La barre d'outils d'OpenBoard (`boardToolBar`) est **masquée mais toujours créée** :
  les groupes de couleurs, le menu ClassBoard et les raccourcis en dépendent. Ne pas la supprimer.
- Les outils « Zoom + / Zoom − » sont masqués (`setVisible(false)`) mais restent dans le
  groupe de la barre des outils : l'ordre des actions y correspond à l'énumération
  `UBStylusTool`, ne pas les retirer de la liste.
- Profils INI de test : la section générale s'écrit `[%General]` (convention QSettings),
  pas `[General]`.

- `UBApplication::eventFilter` suppose que `mainWindow`/`boardController` existent :
  une garde a été ajoutée car l'assistant s'affiche avant leur création (sinon plantage).
- Même piège pour `UBApplication::toolBarDisplayTextChanged` : le profil applique le
  réglage « libellés » avant la création de la fenêtre ; garde ajoutée (plantage au
  démarrage si l'enseignant désactivait les libellés). **Tout slot relié à un `UBSetting`
  modifié avant `mainWindow` doit tolérer `mainWindow == nullptr`.**
- Ubuntu : la compilation exige `libpoppler-private-dev` (en-têtes Splash).
- Les traductions françaises (`resources/i18n/OpenBoard_fr.ts`) ont été adaptées
  (« ClassBoard ») ; les nouvelles chaînes ClassBoard sont écrites directement en
  français dans `tr()` (pas encore de fichier `.ts` dédié).
- `OpenBoard.pro` et les fichiers `OpenBoard_*.ts` gardent leur nom pour faciliter les
  fusions amont ; la cible produite est `ClassBoard.exe` (`TARGET` dans `.pro`), et
  `classboard` sous Linux (`OUTPUT_NAME` CMake ; projet CMake toujours nommé `openboard`).
- Les polices *Écriture A/B* et *GETypoLibre* héritées d'OpenBoard ont une licence à vérifier.
- Le profil est dans `%APPDATA%` (itinérant), les documents dans `%LOCALAPPDATA%` (hérité).

## 6. Prochaines étapes conseillées (par priorité)

1. Télécharger `v0.2.1` depuis GitHub Releases et **tester sur un vrai TBI Epson** + poste Kwartz.
2. Envisager de renommer la branche de travail en `main` (branche par défaut).
3. Fichier `resources/i18n/ClassBoard_en.ts` pour l'anglais (si besoin d'interface bilingue).
4. Modules disciplinaires web complémentaires (voir `ROADMAP.md` §0.3).

## 7. Commandes utiles

```bash
# Compiler et tester sous Linux
cmake -S . -B build -G Ninja -DQT_VERSION=6 -DCLASSBOARD_BUILD_TESTS=ON -DCMAKE_INSTALL_PREFIX=/opt
cmake --build build && ctest --test-dir build --output-on-failure

# Lancer sans écran et capturer
Xvfb :99 & export DISPLAY=:99
HOME=/tmp/cbhome QTWEBENGINE_CHROMIUM_FLAGS=--no-sandbox classboard &
import -window root capture.png

# Régénérer les icônes
python3 tools/icons/generate_icons.py

# Intégrer OpenBoard amont
git fetch upstream && git merge upstream/master
```
