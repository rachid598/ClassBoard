# Déploiement en établissement (Kwartz, GPO, postes isolés)

ClassBoard est livré sous trois formes. Toutes fonctionnent **hors connexion**, sans
compte et sans prérequis à installer (le runtime Visual C++ est inclus).

| Fichier | Installation | Droits requis | Dossier |
|---|---|---|---|
| `ClassBoard-Setup-x64.msi` | Toute la machine | Administrateur (ou compte de déploiement) | `C:\Program Files\ClassBoard` |
| `ClassBoard-Setup-x64.exe` | Utilisateur courant (défaut) ou machine (`/ALLUSERS`) | Aucun en mode utilisateur | `%LOCALAPPDATA%\Programs\ClassBoard` |
| `ClassBoard-Portable-x64.zip` | Aucune | Aucun | Où vous voulez (clé USB, partage) |

> ⚠️ Les exécutables **ne sont pas signés**. Vérifiez l'intégrité avec `SHA256SUMS.txt` :
> `certutil -hashfile ClassBoard-Setup-x64.msi SHA256`

## MSI : déploiement silencieux (recommandé pour Kwartz)

```bat
:: Installation silencieuse, sans redémarrage
msiexec /i ClassBoard-Setup-x64.msi /qn /norestart

:: Avec journal d'installation
msiexec /i ClassBoard-Setup-x64.msi /qn /norestart /l*v "%TEMP%\classboard-install.log"

:: Sans raccourci sur le bureau
msiexec /i ClassBoard-Setup-x64.msi /qn /norestart DESKTOPSHORTCUT=0

:: Mise à jour : installer simplement la nouvelle version (l'ancienne est remplacée)
msiexec /i ClassBoard-Setup-x64.msi /qn /norestart

:: Désinstallation silencieuse
msiexec /x ClassBoard-Setup-x64.msi /qn /norestart
```

Le MSI associe aussi les fichiers `.ubz` (documents OpenBoard/ClassBoard) à ClassBoard.

### Avec Kwartz

Dans la console Kwartz, déclarer une installation de logiciel (paquet MSI) pointant vers
`ClassBoard-Setup-x64.msi` sur un partage accessible par les postes, avec les paramètres
`/qn /norestart`, puis l'affecter aux groupes de postes concernés (salles équipées de TBI).
Consulter la documentation de votre version de Kwartz pour la procédure exacte.

*Note : ce déploiement n'a pas encore été validé sur un serveur Kwartz réel ; vos retours sont bienvenus.*

## EXE (Inno Setup)

```bat
:: Pour l'utilisateur courant, sans droits administrateur
ClassBoard-Setup-x64.exe /VERYSILENT /SUPPRESSMSGBOXES /NORESTART

:: Pour tous les utilisateurs (droits administrateur)
ClassBoard-Setup-x64.exe /VERYSILENT /SUPPRESSMSGBOXES /NORESTART /ALLUSERS

:: Choisir le dossier
ClassBoard-Setup-x64.exe /VERYSILENT /DIR="D:\Logiciels\ClassBoard"

:: Désinstallation silencieuse
"%LOCALAPPDATA%\Programs\ClassBoard\unins000.exe" /VERYSILENT /SUPPRESSMSGBOXES /NORESTART
"C:\Program Files\ClassBoard\unins000.exe" /VERYSILENT /SUPPRESSMSGBOXES /NORESTART
```

## Version portable

Décompresser le `.zip` et lancer `ClassBoard.exe`. Le fichier `portable.txt` présent dans le
dossier active le **mode portable** : profil, réglages et documents sont enregistrés dans le
sous-dossier `settings`. Supprimer `portable.txt` pour utiliser le profil Windows.

## Où sont les données ?

| Donnée | Emplacement |
|---|---|
| Profil ClassBoard (disciplines, thème, favoris) | `%APPDATA%\ClassBoard\ClassBoardProfile.ini` — **itinérant**, suit l'enseignant sur le réseau |
| Réglages du moteur | `%LOCALAPPDATA%\ClassBoard\ClassBoardUser.config` |
| Documents | `%LOCALAPPDATA%\ClassBoard\document\` |
| Réglages par défaut (administrateur) | `<dossier d'installation>\etc\ClassBoard.config` |

### Préconfigurer les postes

Modifier `etc\ClassBoard.config` dans le dossier d'installation (après déploiement) permet de
changer les valeurs par défaut pour tous les utilisateurs, par exemple :

```ini
[App]
; Dossier des documents (ex. partage réseau personnel) ; {Documents} = dossier Documents de l'utilisateur
DataDirectory={Documents}/ClassBoard
```

Pour **sauter l'assistant de premier lancement** (postes partagés, comptes génériques),
définir la variable d'environnement `CLASSBOARD_SKIP_FIRST_RUN=1`.

## Cohabitation avec OpenBoard

ClassBoard et OpenBoard peuvent être installés sur le même poste : leurs réglages et
documents sont séparés. Les documents OpenBoard s'importent depuis le mode Documents.

## Vie privée (RGPD)

ClassBoard n'envoie **aucune donnée** : ni télémétrie, ni recherche de mise à jour,
ni compte. Les seuls accès réseau sont ceux que l'enseignant déclenche (mode Web,
widgets en ligne de la bibliothèque).
