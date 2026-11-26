# Journal des modifications

Format inspiré de [Keep a Changelog](https://keepachangelog.com/fr/1.1.0/).
Versions selon [SemVer](https://semver.org/lang/fr/).

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
