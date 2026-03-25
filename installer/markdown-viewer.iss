; Inno Setup Script for Markdown Viewer
; This script creates a Windows installer with proper desktop shortcut support

#define AppName "Markdown Viewer"
#define AppVersion "1.0.0"
#define AppPublisher "George Halifax"
#define AppURL "https://github.com/promptengineer1768/markdown-viewer"
#define AppExeName "markdown_viewer.exe"
#define BuildDir "..\build\windows-msvc-release"

[Setup]
AppId={{2f10b8cf-862d-45db-abe9-1b5ae412d274}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
AllowNoIcons=yes
LicenseFile=..\LICENSE
OutputDir=..\dist\windows
OutputBaseFilename=markdown-viewer-{#AppVersion}-win64
SetupIconFile=..\resources\icon.ico
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
; Do not silently reuse old task selections from previous installs.
UsePreviousTasks=no
UninstallDisplayIcon={app}\bin\{#AppExeName}
UninstallDisplayName={#AppName}
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "portuguese"; MessagesFile: "compiler:Languages\Portuguese.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: checkedonce

[Files]
Source: "{#BuildDir}\bin\{#AppExeName}"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#BuildDir}\bin\*.dll"; DestDir: "{app}\bin"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\bin\locales\*"; DestDir: "{app}\bin\locales"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\resources\icon.ico"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\resources\README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\resources\MD_Viewer_*.jpg"; DestDir: "{app}\resources"; Flags: ignoreversion
Source: "..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\bin\{#AppExeName}"; IconFilename: "{app}\bin\icon.ico"
Name: "{group}\{cm:ProgramOnTheWeb,{#AppName}}"; Filename: "{#AppURL}"
Name: "{group}\{cm:UninstallProgram,{#AppName}}"; Filename: "{uninstallexe}"
Name: "{userdesktop}\{#AppName}"; Filename: "{app}\bin\{#AppExeName}"; IconFilename: "{app}\bin\icon.ico"; Tasks: desktopicon

[Run]
Filename: "{app}\bin\{#AppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(AppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Registry]
Root: HKCU; Subkey: "Software\{#AppName}"; ValueType: string; ValueName: "InstallPath"; ValueData: "{app}"
