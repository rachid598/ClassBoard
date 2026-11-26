# Contribuer à ClassBoard

Merci ! Enseignants, développeurs, traducteurs : toutes les contributions sont bienvenues.

## Signaler un problème

Ouvrez une *issue* en précisant : version de ClassBoard (menu ClassBoard › Préférences ›
À propos), version de Windows, matériel (TBI, vidéoprojecteur, stylet), étapes pour
reproduire, et si possible une capture d'écran. **Ne joignez aucune donnée d'élève.**

## Proposer une modification

1. Créez une branche à partir de la branche principale.
2. Respectez [CODING_STYLE.md](CODING_STYLE.md) (conventions d'OpenBoard).
3. Placez le nouveau code dans `src/classboard/` ; dans le moteur OpenBoard, limitez-vous
   à de courtes retouches marquées `// ClassBoard :` (voir [ARCHITECTURE.md](ARCHITECTURE.md)).
4. Ajoutez ou mettez à jour les tests (`tests/`), puis :
   ```bash
   cmake -S . -B build -G Ninja -DQT_VERSION=6 -DCLASSBOARD_BUILD_TESTS=ON
   cmake --build build && ctest --test-dir build --output-on-failure
   ```
5. Mettez à jour [CHANGELOG.md](CHANGELOG.md) et, si besoin, la documentation.
6. Ouvrez une *pull request* : la CI compile Windows et Linux et lance les tests.

## Ajouter un outil pédagogique

Le plus simple est un widget web hors connexion dans `resources/library/applications/`.
Voir la section « Extensibilité » d'[ARCHITECTURE.md](ARCHITECTURE.md) et
[docs/technique/README.md](docs/technique/README.md).

Règles : aucun appel réseau, aucune bibliothèque téléchargée à l'exécution, interface
en français, gros boutons (≥ 44 px), licence compatible GPL-3.0 pour tout fichier repris.

## Intégrer les évolutions d'OpenBoard

```bash
git remote add upstream https://github.com/OpenBoard-org/OpenBoard.git   # une fois
git fetch upstream
git merge upstream/master
```

## Licence

En contribuant, vous acceptez que votre contribution soit distribuée sous GPL-3.0
(avec l'exception OpenSSL du projet).
