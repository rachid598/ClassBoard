# Feuille de route

Priorité : une version **stable et installable** avant d'ajouter des fonctionnalités.

## ✅ 0.1 — Socle (version actuelle)

- Socle OpenBoard 1.7.7, renommé ClassBoard, interface française par défaut
- Assistant de disciplines, profil enseignant, barre de favoris
- Thème Fluent clair/sombre, nouvelles icônes, gros boutons
- Minuteur, Tirage au sort, fonds Seyès, polices Marelle/Écolier
- Installateurs exe / msi / zip, CI GitHub Actions, tests

## 0.2 — Validation en classe

- [ ] Tests sur TBI et vidéoprojecteurs interactifs **Epson réels** (stylet, tactile, multi-écran)
- [ ] Vérification du déploiement MSI silencieux sur un réseau **Kwartz**
- [ ] Enregistrement direct sur lecteur réseau : choix du dossier des documents dans « Mon profil »
- [ ] Barre de favoris en mode bureau (annotation sur Windows)
- [ ] Bascule souris/annotation plus visible en mode bureau (cadre coloré, raccourci clavier documenté)
- [ ] Récupération après plantage : dialogue explicite au redémarrage
- [ ] Vérifier la licence des polices *Écriture A/B* et *GETypoLibre* héritées d'OpenBoard
      (retirées par Open-Sankoré CE pour raison de licence)

## 0.3 — Modules disciplinaires (web, hors connexion)

| Discipline | Modules prévus |
|---|---|
| Mathématiques | Fractions visuelles, éditeur LaTeX hors ligne, repère paramétrable |
| Français | Étiquettes grammaticales, tableau de conjugaison |
| Histoire-géographie | Frise chronologique, cartes muettes (fonds libres de droits) |
| Sciences | Tableau périodique, circuits électriques, verrerie |
| SVT | Schémas à légender |
| Langues | Flashcards, lecture audio |
| Technologie | Organigrammes, symboles normalisés |
| Arts plastiques | Palettes, calques |
| Musique | Portées, notes, métronome |
| EPS | Terrains, tactiques, constitution d'équipes |
| ULIS / SEGPA | Pictogrammes, mode accessibilité renforcée (police et contrastes) |
| Transversal | Cartes mentales |

## 0.4 et au-delà

- [ ] Signature de code des exécutables (si un certificat est disponible)
- [ ] Traductions de l'interface ClassBoard (anglais, langues de l'établissement)
- [ ] Reconnaissance d'écriture manuscrite (Windows Ink), à étudier
- [ ] Intégration des évolutions d'OpenBoard 1.8 (fusion `upstream/dev`)

## À étudier, sans promesse

- **Import des fichiers Interwrite Workspace (`.gwb`)** : format propriétaire non documenté.
  Faisabilité à démontrer sur de vrais fichiers fournis par des enseignants avant toute annonce.
- Réutilisation d'autres ressources d'Open-Sankoré Community Edition (icônes Phosphor, QML),
  licence par licence.
