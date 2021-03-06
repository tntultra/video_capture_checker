; Script generated by the Inno Script Studio Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Video Checker"
#define MyAppVersion "0.1"
#define MyAppPublisher "Ivan Alexandrov"
#define MyAppURL "https://github.com/tntultra/video_capture_checker"
#define MyAppExeName "VideoChecker.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{C01B9DAA-A88C-47BE-AE89-040F668C8098}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DisableDirPage=yes
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputDir=E:\Video
OutputBaseFilename=Video Checker Setup
Compression=lzma
SolidCompression=yes
RestartIfNeededByRun=False
AllowCancelDuringInstall=False

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"

[Files]
Source: "E:\Video\VideoChecker.exe"; DestDir: "{app}"; Flags: ignoreversion;
; NOTE: Don't use "Flags: ignoreversion" on any shared system files
Source: "E:\Video\config.ini"; DestDir: "{app}"; Flags: ignoreversion;

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"

[Run]
Filename: "{sys}\sc.exe"; Parameters: "stop VideoCheckerService"; Flags: runhidden;
Filename: "{sys}/sc.exe"; Parameters: "delete VideoCheckerService"; Flags: runhidden;
Filename: "{sys}/sc.exe"; Parameters: "create VideoCheckerService binPath= ""\""{app}\VideoChecker.exe\"" --service \""{app}\config.ini\"""; Flags: runhidden;

[UninstallRun]
Filename: "{sys}\sc.exe"; Parameters: "stop VideoCheckerService"; Flags: runhidden;
Filename: "{sys}\sc.exe"; Parameters: "delete VideoCheckerService"; Flags: runhidden;