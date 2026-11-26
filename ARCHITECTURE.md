# Architecture de ClassBoard

## 1. Décision : partir d'OpenBoard

| Projet audité | Langage / état | Licence | Décision |
|---|---|---|---|
| [OpenBoard](https://github.com/OpenBoard-org/OpenBoard) | C++17 / Qt 6, très actif (1.7.7 stable, 1.8 en développement), installateurs Windows officiels | GPL-3.0 + exception OpenSSL | **Socle retenu** (branche `master`, v1.7.7) |
| [Open-Sankoré Community Edition](https://github.com/davidguyomarch/Sankore) | C++ / Qt 6, interface QML, relancé en 2026 | GPL-3.0 + exception OpenSSL | Source de **ressources** (polices scolaires). Code non fusionné : base plus ancienne et divergente d'OpenBoard. |
| [ppInk](https://github.com/pubpub-zz/ppInk) | C# / .NET Framework, WinForms, Windows uniquement | MIT | **Inspiration UX** pour l'annotation du bureau (préréglages de stylos, raccourcis). Code non réutilisable dans une application Qt. |

Pourquoi OpenBoard :

- moteur d'écriture éprouvé (pression, lissage, reconnaissance de formes), instruments
  de géométrie, documents `.ubz`, import PDF via Poppler, annotation du bureau Windows ;
- base la plus maintenue, déjà en Qt 6, avec une procédure de compilation Windows
  connue (dépôt `OpenBoard-ThirdParty`) ;
- compatibilité directe avec les documents existants des enseignants (OpenBoard et
  Open-Sankoré utilisent le même format `.ubz`).

Le dépôt ClassBoard **conserve tout l'historique** d'OpenBoard. Le remote `upstream`
pointe vers OpenBoard pour intégrer ses évolutions (`git fetch upstream && git merge upstream/master`).

### Rust ?

Écarté : il faudrait réécrire tout le moteur (écriture, PDF, annotation du bureau,
instruments, format de fichier). Contraire à l'objectif « ne pas repartir de zéro ».

## 2. Principe : un module ClassBoard autour du moteur, des retouches minimales dedans

```
ClassBoard/
├── src/                      Moteur OpenBoard (modifications minimales, commentées « ClassBoard : »)
│   └── classboard/           ★ Module ClassBoard (nouveau code)
│       ├── ClassBoardProfile        profil enseignant (INI, QtCore seul, testé)
│       ├── ClassBoardDisciplines    catalogue des 15 disciplines et de leurs favoris
│       ├── ClassBoardTheme          thème clair/sombre, couleurs, feuille de style
│       ├── ClassBoardIcons          jeu d'icônes SVG appliqué aux actions à l'exécution
│       ├── ClassBoardSetupDialog    assistant de premier lancement + « Mon profil »
│       └── ClassBoardFavoritesPalette  barre de favoris à gauche du tableau
├── resources/
│   ├── classboard/           ★ icônes SVG (disciplines, outils), logo, classboard.qrc
│   ├── etc/ClassBoard.css    ★ thème clair (ClassBoard-dark.css : thème sombre)
│   ├── etc/ClassBoard.config réglages par défaut (anciennement OpenBoard.config)
│   ├── customizations/fonts  polices (dont Marelle, Écolier)
│   └── library/applications  widgets web ; ★ Minuteur.wgt, ★ TirageAuSort.wgt
├── tests/                    ★ tests Qt Test + contrôle des widgets (CTest)
├── scripts/windows/          ★ build.ps1, deploy.ps1, smoke-test.ps1, package.ps1
├── packaging/windows/        ★ ClassBoard.iss (Inno Setup), ClassBoard.wxs (WiX 5)
├── tools/icons/              ★ générateur d'icônes (source unique des tracés)
└── .github/workflows/        ★ build.yml : Linux + Windows + Release sur tag
```

### Points d'accroche dans le moteur OpenBoard

Toutes les retouches sont courtes et marquées `// ClassBoard :` pour faciliter les fusions.

| Fichier | Retouche |
|---|---|
| `core/UBApplication.cpp` | nom de l'application, thème, langue française par défaut, assistant au démarrage, icônes, entrée de menu « Mon profil », garde dans `eventFilter` |
| `core/UBSettings.cpp` | mode portable (`portable.txt`) |
| `core/UBApplicationController.cpp` | aucune requête si l'URL de mise à jour est vide |
| `core/UBPreferencesController.cpp` | mentions légales ClassBoard, masquage des mises à jour |
| `board/UBBoardPaletteManager.*` | création, placement et visibilité de la barre de favoris |
| `gui/UBStylusPalette.cpp` | taille des boutons et libellés selon le profil |
| `gui/UBActionPalette.cpp`, `gui/UBFloatingPalette.cpp` | couleurs du thème |
| `desktop/UBDesktopPalette.cpp` | icônes ClassBoard en mode bureau |

## 3. Données et vie privée

| Donnée | Emplacement (Windows) | Remarque |
|---|---|---|
| Profil ClassBoard (disciplines, thème, favoris) | `%APPDATA%\ClassBoard\ClassBoardProfile.ini` | Profil **itinérant** : suit l'enseignant sur un réseau Kwartz |
| Réglages du moteur | `%LOCALAPPDATA%\ClassBoard\ClassBoardUser.config` | Hérité d'OpenBoard |
| Documents | `%LOCALAPPDATA%\ClassBoard\document\` | Export/enregistrement possible vers un lecteur réseau |
| Mode portable | `<dossier de ClassBoard>\settings\` | Activé par `portable.txt` |

Aucune donnée n'est transmise. Les seules connexions réseau possibles sont celles que
l'enseignant déclenche (mode Web, widgets en ligne de la bibliothèque OpenBoard).

## 4. Extensibilité : ajouter un outil pédagogique

1. **Outil web (recommandé)** : créer `resources/library/applications/MonOutil.wgt/`
   (`config.xml`, `index.html`, `icon.png`). HTML/JS autonome, sans ressource réseau.
   Les préférences se sauvent dans le document via `window.sankore.setPreference`.
2. **Le proposer à une discipline** : ajouter `widget:MonOutil` à la liste de la
   discipline dans `ClassBoardDisciplines.cpp` et, si besoin, une icône dans
   `tools/icons/generate_icons.py` + `ClassBoardIcons::iconNameForFavorite`.
3. **Outil natif C++** : s'inspirer des instruments de `src/tools/` (règle, compas…)
   enregistrés dans `UBToolsManager` (URI `openboardtool://…`).

Les modules sont indépendants du moteur : un widget web ne demande aucune recompilation.

## 5. Compilation et livraison

- **Windows** : Qt 6.8.3 (MSVC 2022 x64) installé par `install-qt-action`, bibliothèques
  précompilées d'`OpenBoard-ThirdParty` (Poppler, OpenSSL, zlib, QuaZip), compilation
  `qmake` + `jom`, déploiement `windeployqt`, runtime VC++ copié localement (aucun
  prérequis à installer), empaquetage Inno Setup et WiX 5.
- **Linux** : CMake + Ninja, utilisé par la CI pour un contrôle rapide et les tests.
- Le fichier projet `OpenBoard.pro` garde son nom d'origine pour simplifier les fusions
  avec OpenBoard ; la cible produite s'appelle `ClassBoard.exe`.
