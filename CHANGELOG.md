# Journal des modifications

Format inspiré de [Keep a Changelog](https://keepachangelog.com/fr/1.1.0/).
Versions selon [SemVer](https://semver.org/lang/fr/).

## [0.2.1] — 2026-11-27

Nouvelle identité visuelle (logo 2026) et ajouts issus des premiers tests terrain :
Annuler en mode Bureau, zoom permanent et redimensionnement en mode Documents,
lecteur audio pédagogique hors connexion, export PDF en 1 clic, renommage rapide,
formes rapides et choix du dossier de sauvegarde Kwartz.

### Ajouté
- **Nouveau logo ClassBoard 2026** (`classboard.svg`, PNG 256/1024 px et `ClassBoard.ico`
  multi-tailles pour Windows) : monogramme « C » sur fond dégradé indigo-violet avec stylet
  doré et trait lumineux.
- **Mode Bureau — Annuler + bandeau d'état** : bouton **Annuler** dans la barre flottante
  du bureau (`Ctrl+Z` également actif) permettant d'effacer le dernier tracé d'annotation
  sans toucher au tableau, accompagné d'un bandeau discret en haut de l'écran indiquant
  que l'annotation est active (masqué dès le passage en mode « Choisir »).
- **Zoom et déplacement toujours visibles sur le tableau** : barre flottante en bas à gauche
  (`−`, `100 %`, `+`, `Ajuster` et bouton `Main` pour déplacer la page) accessible en permanence.
- **Mode Documents — boutons tactiles `−` / `+`** : ajoutés de part et d'autre du curseur
  de zoom des vignettes pour redimensionner confortablement au doigt ou au stylet.
- **Renommage du cours en 1 clic** : clic direct sur le titre du document en haut à gauche
  (et entrée « Renommer ce cours… » dans le menu principal).
- **Export PDF en 1 clic (Pronote / ENT)** : accessible depuis le menu principal et le tiroir
  « Outils pédagogiques » sans passer par le mode Documents.
- **Widget Lecteur audio hors connexion** (`LecteurAudio.wgt`, Anglais / Langues / Musique) :
  ouverture d'un fichier audio local (MP3, WAV, OGG…), `−5 s`, `+5 s`, vitesses `0,75× / 0,9× / 1× / 1,25×`,
  et **boucle A-B** pour réécouter un passage en classe.
- **Formes rapides dans le tiroir Outils** : `Flèche`, `Encadré` et `Cercle` insérés directement
  sur la page courante.
- **Dossier de sauvegarde des cours (Kwartz / clé USB)** : réglable dans « Mes matières et
  préférences › Apparence & barre » (`Storage/DocumentsDirectory`).

## [0.2.0] — 2026-11-26

Refonte complète de l'interface pour une prise en main immédiate sans surcharge,
avec fond blanc par défaut, barre verticale à droite et outils dédiés Anglais / Français.

### Ajouté
- **Barre verticale moderne à droite** (déplaçable à gauche en 1 clic pour les gauchers)
  regroupant 6 outils essentiels aux icônes bicolores (`Choisir`, `Écrire`, `Surligner`,
  `Gommer`, `Texte`, `Outils`) + `Annuler / Rétablir`, laissant tout le bas du tableau libre.
- **Bulle contextuelle flottante** à côté de l'outil actif : 5 pastilles rondes de couleurs
  vives pour le stylo, 5 surligneurs fluo pastel pour `Surligner`, 3 épaisseurs ou
  tailles de gomme + bouton `Tout effacer`.
- **Tiroir « Outils pédagogiques »** (`ClassBoardToolsDrawer`) filtré automatiquement
  selon les disciplines de l'enseignant (Anglais, Français, Classe, Tous) avec recherche rapide.
- **Sélecteur de fonds moderne** (`ClassBoardBackgroundPopover`) à cartes vectorielles
  (`Blanc`, `Quadrillé`, `Seyès`, `Ardoise`, `Quadr. noir`, `Seyès noir`) remplaçant
  les anciennes icônes cornées d'OpenBoard.
- **Bascule 1 clic Clair ↔ Ardoise sombre** dans la barre supérieure (changement en direct
  du thème et du fond de page sans redémarrer).
- Nouveaux widgets hors connexion pour l'Anglais et le Français :
  - **Vocabulaire** (`Vocabulaire.wgt`) : cartes mémoire recto-verso (flashcards), mélange,
    édition de liste `mot = traduction`, sauvegarde dans le document.
  - **Étiquettes de mots** (`Etiquettes.wgt`) : étiquettes déplaçables et colorables
    pour remettre une phrase en ordre ou analyser la grammaire.

### Modifié
- **Accueil de premier lancement** simplifié en 1 seul écran rapide avec tuiles bicolores
  par discipline (langues et lettres en tête, toutes les disciplines accessibles).
- **Mode Bureau** : barre flottante placée à droite par défaut avec icônes bicolores et
  libellés clairs (`Tableau`, `Choisir`, `Écrire`, `Surligner`, `Gommer`, `Capture`, `Effacer`).
- Onglets gris latéraux d'OpenBoard masqués par défaut sur le tableau (le panneau de
  miniatures de pages reste accessible en cliquant sur le compteur `1 / 1`).
- Palettes de stylo et surligneur par défaut modernisées dans `ClassBoard.config`.

## [0.1.0] — 2026-11-26

Première version de ClassBoard, basée sur OpenBoard 1.7.7 (branche `master`).

### Ajouté
- Assistant de premier lancement : choix des disciplines (15), du thème, de la
  taille des boutons et des favoris. Profil enregistré dans `%APPDATA%\ClassBoard`.
- Écran « Mon profil ClassBoard » (menu ClassBoard) : disciplines, apparence,
  favoris, réinitialisation.
- Barre verticale de favoris à gauche du tableau, déduite des disciplines.
- Thème clair et thème sombre (feuilles de style `ClassBoard.css` / `ClassBoard-dark.css`).
- Jeu de 41 icônes au trait (style Fluent), libellés courts sous les outils.
- Taille des boutons réglable : 100, 125 (défaut, adaptée aux TBI) ou 150 %.
- Widget **Minuteur** (compte à rebours, signal sonore, hors connexion).
- Widget **Tirage au sort** (élève sans remise, groupes, dés).
- Polices Marelle (cursive officielle) et licence de la police Écolier Court.
- Mode portable (`portable.txt` à côté de l'exécutable).
- Chaîne CI GitHub Actions : Windows x64 (exe, msi, zip, SHA256) et Linux (tests).
- Tests automatisés (Qt Test + contrôle des widgets) via CTest.
- Documentation : guide enseignant, déploiement Kwartz, documentation technique,
  architecture, feuille de route, passation.

### Modifié
- **Disposition du tableau conforme à la maquette** : page en plein écran, barres
  flottantes aux coins (document et menu, fonds/documents/web/mode bureau, zoom, pages),
  couleurs et épaisseurs au-dessus de la barre des outils, Annuler/Rétablir dans la
  barre des outils. La grande barre grise d'OpenBoard n'est plus affichée en mode tableau.
- Pages au format 16:9 par défaut (plus de bandes grises sur les écrans larges).
- Nom de l'application, icône, informations de version Windows : ClassBoard.
- Réglages et documents séparés de ceux d'OpenBoard (les deux peuvent cohabiter).
- Interface en français par défaut, quelle que soit la langue de Windows.
- Fonds Seyès proposés par défaut.
- Sauvegarde automatique toutes les 2 minutes (au lieu de 3).

### Corrigé (pendant le développement de la 0.1.0)
- Plantage au démarrage si l'assistant s'affichait avant la fenêtre principale.
- Plantage au démarrage quand les libellés d'outils étaient désactivés.
- Barre de favoris réduite à un point après modification du profil.
- Favoris figés quand on changeait de discipline sans ouvrir l'onglet Favoris.

### Supprimé / désactivé
- Recherche automatique de mises à jour sur les serveurs d'OpenBoard
  (aucune requête réseau non sollicitée). Astuces de démarrage désactivées
  par défaut (remplacées par l'assistant, toujours accessibles par le menu).
