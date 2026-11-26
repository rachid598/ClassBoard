# HANDOVER — reprendre le développement de ClassBoard

Document de passation destiné à un développeur ou une IA qui reprend le projet.
**À tenir à jour à chaque étape importante.** Dernière mise à jour : 2026-11-26.

## 1. Le projet en une minute

- **Objectif** : remplacer Interwrite Workspace dans un collège (TBI/vidéoprojecteurs
  interactifs Epson, Windows 10/11, réseau Kwartz), pour toutes les disciplines.
- **Base** : fork d'OpenBoard 1.7.7 (branche `master` amont, historique conservé,
  remote `upstream` = https://github.com/OpenBoard-org/OpenBoard).
- **Dépôt** : https://github.com/rachid598/ClassBoard (privé). Branche de travail :
  `claude/gifted-turing-1gc7gi` (c'est aussi, à ce jour, la seule branche du dépôt).
- **Langage** : C++17 / Qt 6 (8.3 sous Windows, 6.4 sous Ubuntu 24.04). Pas de Rust (voir ARCHITECTURE.md).
- **Contraintes** : hors connexion, sans compte, sans télémétrie, RGPD, sans droits
  administrateur, GPL-3.0, interface française par défaut.

## 2. Ce qui est fait (v0.1.0)

| Domaine | État | Où |
|---|---|---|
| Import OpenBoard + historique | ✅ | — |
| Renommage ClassBoard (exe, icône, réglages séparés, RC Windows, À propos) | ✅ | `UBApplication.cpp`, `resources/win/`, `UBPreferencesController.cpp` |
| Français par défaut | ✅ | `UBApplication::setupTranslators` |
| Pas de requête réseau de mise à jour | ✅ | `ClassBoard.config`, `UBApplicationController::checkUpdate` |
| Profil enseignant + 15 disciplines | ✅ testé | `src/classboard/ClassBoardProfile*`, `ClassBoardDisciplines*` |
| Assistant de premier lancement + « Mon profil » | ✅ vérifié visuellement (Linux) | `ClassBoardSetupDialog*` |
| Barre de favoris | ✅ vérifié (Linux) | `ClassBoardFavoritesPalette*`, `UBBoardPaletteManager` |
| Thème clair/sombre, icônes, libellés, taille des boutons | ✅ vérifié (Linux) | `ClassBoardTheme*`, `ClassBoardIcons*`, `resources/etc/ClassBoard*.css` |
| Minuteur, Tirage au sort | ✅ vérifié (Linux, tirage testé) | `resources/library/applications/*.wgt` |
| Seyès, polices Marelle/Écolier | ✅ | `ClassBoard.config`, `resources/customizations/fonts` |
| Mode portable | ✅ (logique testée, pas testé sous Windows réel) | `UBSettings::userDataDirectory`, `ClassBoardProfile::isPortable` |
| CI Linux (compilation + tests) | voir §4 | `.github/workflows/build.yml` |
| CI Windows (exe/msi/zip/SHA256) | voir §4 | `scripts/windows/*`, `packaging/windows/*` |
| Documentation | ✅ | `README.md`, `docs/`, `ARCHITECTURE.md`, `ROADMAP.md`… |

Captures et maquettes : maquette de design publiée en Artifact (claude.ai), captures
d'écran Linux prises sous Xvfb pendant le développement (non versionnées).

## 3. Ce qui n'a PAS été vérifié

- **Aucun test sur un TBI Epson réel**, ni sur écran tactile/stylet réel.
- Déploiement MSI sur un serveur **Kwartz** réel.
- Le rendu Windows de l'interface n'a été vu que via le test de démarrage CI (pas de capture Windows).
- Import des fichiers **Workspace `.gwb`** : non étudié en pratique (aucun fichier d'exemple).

## 4. État de la CI

Workflow `Build` (`.github/workflows/build.yml`) : https://github.com/rachid598/ClassBoard/actions

Historique des corrections déjà faites :
1. `tools: tools_jom` d'install-qt-action ne fonctionne plus (aqt ne trouve pas l'outil) →
   `jom` installé par Chocolatey.
2. Le test `classboard_widgets` échouait tant que les widgets n'étaient pas poussés (normal).

→ Consulter la dernière exécution pour l'état courant ; mettre à jour cette section.

## 5. Pièges connus

- `UBApplication::eventFilter` suppose que `mainWindow`/`boardController` existent :
  une garde a été ajoutée car l'assistant s'affiche avant leur création (sinon plantage).
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

1. Faire passer la CI Windows au vert si ce n'est pas le cas, télécharger l'artefact,
   **tester sur un vrai poste Windows** puis sur un TBI.
2. Publier `v0.1.0` (tag) une fois validé → Release automatique.
3. Choix du dossier des documents (lecteur réseau Kwartz) dans « Mon profil ».
4. Barre de favoris en mode bureau.
5. Fichier `resources/i18n/ClassBoard_en.ts` pour l'anglais.
6. Modules disciplinaires web (voir ROADMAP.md §0.3).

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
