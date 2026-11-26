## Téléchargements

| Fichier | Usage |
|---|---|
| `ClassBoard-Setup-x64.exe` | Installateur classique. Par défaut pour l'utilisateur courant, **sans droits administrateur**. `/ALLUSERS` pour tous les utilisateurs. |
| `ClassBoard-Setup-x64.msi` | Paquet MSI pour un déploiement silencieux (Kwartz, GPO) dans `C:\Program Files\ClassBoard`. |
| `ClassBoard-Portable-x64.zip` | Version portable : décompresser et lancer `ClassBoard.exe`. |
| `SHA256SUMS.txt` | Sommes de contrôle SHA-256. |

### Installation silencieuse

```bat
ClassBoard-Setup-x64.exe /VERYSILENT /SUPPRESSMSGBOXES /NORESTART
msiexec /i ClassBoard-Setup-x64.msi /qn /norestart
```

### Désinstallation silencieuse

```bat
msiexec /x ClassBoard-Setup-x64.msi /qn /norestart
"%LOCALAPPDATA%\Programs\ClassBoard\unins000.exe" /VERYSILENT /SUPPRESSMSGBOXES /NORESTART
```

> ⚠️ **Exécutables non signés.** Aucun certificat de signature de code n'est
> utilisé : Windows SmartScreen peut afficher « Windows a protégé votre
> ordinateur ». Cliquez sur « Informations complémentaires » puis « Exécuter
> quand même », ou vérifiez la somme SHA-256 avant installation.

Configuration requise : Windows 10 ou 11, 64 bits. Testé automatiquement par
la CI (compilation, démarrage) ; **pas encore validé sur un TBI Epson réel**.

Voir [CHANGELOG.md](https://github.com/rachid598/ClassBoard/blob/HEAD/CHANGELOG.md) pour le détail des changements.
