; ---------------------------------------------------------------------------
; Installateur ClassBoard (Inno Setup 6)
;
; Compilation (faite par la CI) :
;   iscc /DAppVersion=0.1.0 /DSourceDir=<dist\ClassBoard> /DAppExe=ClassBoard.exe
;        /DOutputDir=<sortie> packaging\windows\ClassBoard.iss
;
; Installation :
;   - par défaut pour l'utilisateur courant, SANS droits administrateur ;
;   - pour tous les utilisateurs : ClassBoard-Setup-x64.exe /ALLUSERS
;
; Installation silencieuse :
;   ClassBoard-Setup-x64.exe /VERYSILENT /SUPPRESSMSGBOXES /NORESTART [/ALLUSERS]
; Désinstallation silencieuse :
;   "%LOCALAPPDATA%\Programs\ClassBoard\unins000.exe" /VERYSILENT /SUPPRESSMSGBOXES /NORESTART
;   (ou "C:\Program Files\ClassBoard\unins000.exe" pour une installation /ALLUSERS)
; ---------------------------------------------------------------------------

#define AppName "ClassBoard"
#ifndef AppVersion
  #define AppVersion "0.0.0"
#endif
#ifndef AppExe
  #define AppExe "ClassBoard.exe"
#endif
#ifndef SourceDir
  #define SourceDir "..\..\dist\ClassBoard"
#endif
#ifndef OutputDir
  #define OutputDir "..\..\dist\out"
#endif

[Setup]
AppId={{9AD6B99F-7455-433A-91BA-BC6BC28B6D7B}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher=Contributeurs ClassBoard
AppPublisherURL=https://github.com/rachid598/ClassBoard
AppSupportURL=https://github.com/rachid598/ClassBoard/issues
AppCopyright=GPL-3.0 — basé sur OpenBoard
VersionInfoVersion={#AppVersion}
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=commandline dialog
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0
OutputDir={#OutputDir}
OutputBaseFilename=ClassBoard-Setup-x64
SetupIconFile=..\..\resources\win\ClassBoard.ico
UninstallDisplayIcon={app}\{#AppExe}
UninstallDisplayName={#AppName}
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
CloseApplications=yes
ChangesAssociations=yes
LicenseFile=..\..\LICENSE

[Languages]
Name: "fr"; MessagesFile: "compiler:Languages\French.isl"
Name: "en"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "assocubz"; Description: "Ouvrir les documents OpenBoard (.ubz) avec ClassBoard"; GroupDescription: "Associations de fichiers :"

[InstallDelete]
; Nettoie les anciennes DLL et plugins avant une mise à jour
Type: files; Name: "{app}\*.dll"
Type: filesandordirs; Name: "{app}\library"
Type: filesandordirs; Name: "{app}\platforms"
Type: filesandordirs; Name: "{app}\imageformats"
Type: filesandordirs; Name: "{app}\multimedia"
Type: filesandordirs; Name: "{app}\resources"
Type: filesandordirs; Name: "{app}\translations"

[Files]
Source: "{#SourceDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\{#AppExe}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExe}"; Tasks: desktopicon

[Registry]
; HKA = HKCU pour une installation utilisateur, HKLM pour /ALLUSERS
Root: HKA; Subkey: "Software\Classes\.ubz"; ValueType: string; ValueName: ""; ValueData: "ClassBoard.Document"; Flags: uninsdeletevalue; Tasks: assocubz
Root: HKA; Subkey: "Software\Classes\ClassBoard.Document"; ValueType: string; ValueName: ""; ValueData: "Document ClassBoard / OpenBoard"; Flags: uninsdeletekey; Tasks: assocubz
Root: HKA; Subkey: "Software\Classes\ClassBoard.Document\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#AppExe},1"; Tasks: assocubz
Root: HKA; Subkey: "Software\Classes\ClassBoard.Document\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#AppExe}"" ""%1"""; Tasks: assocubz

[Run]
Filename: "{app}\{#AppExe}"; Description: "{cm:LaunchProgram,{#AppName}}"; Flags: nowait postinstall skipifsilent
