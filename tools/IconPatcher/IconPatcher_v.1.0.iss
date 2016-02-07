; Patcher's features:
; Х Icons' replacement in libraries and executables.
; Х Sets of icons should be packed to the 7z archive.
; Х 7z-archives icons should be in the same folder next to IconPatcher.exe.
; Х It has English and Russian interface (automatically detected)
; Х Intuitive graphical user interface.
; Х Command line support. Command line parameters:
;    Х /PID = "PID of Miranda procees for wait closing it".
;    Х /BIN = "path to Miranda32/64.exe". The exe-file path, related to which all archived files' paths would be calculated with respect to 'files from the archive to the resources' from the 'files Miranda'.
;    Х /ARC = archive name without extension, for example, /ARC=Nova.
;    Х /RES = list of archive folders to process, they will be marked in the patcher's UI. If you want to mark all folders, you can simply omit /RES parameter, or use /RES=* either.
;    Х /SILENT = simplified setup mode. only the progress indicator will be displayed.
;    Х /VERYSILENT = automated setup mode. Nothing is displayed at all.
;    Х /LOG="log.txt" = creates a log-file" log.txt" near IconPatcher.exe. The parameter is used for tracking errors.
; Х The relative folders structure in archives (for example, Icons\Proto_ICQ\*.ico) means that the folder specified via /BIN must have the subfolder Icons with the file Proto_ICQ.dll in it (possible extensions .exe, .dll, .mir).
; Х The 'root' folder is for beautifulness only and doesn't repeat the original program's structure, it contains the .exe resources only.

; ¬озможности патчера:
; Х «амена значков в библиотеках и исполн€емых файлах.
; Х Ќаборы значков должны быть запакованы в архив 7z.
; Х 7z-архивы значков должны лежать в одной папке р€дом с IconPatcher.exe.
; Х »меет английский и русский интерфейс (определ€етс€ автоматически)
; Х »нтуитивно пон€тный графический интерфейс пользовател€.
; Х ѕоддержка командной строки. ѕараметры командной строки:
;    Х /BIN = "путь к Miranda32/64.exe". ѕуть к exe-файлу, относительно которого будут сопоставл€тьс€ 'файлы из архива с ресурсами' с 'файлами Miranda'.
;    Х /ARC = им€ архива без расширени€, например, /ARC=Nova.
;    Х /RES = список папок из архива дл€ обработки, например, /RES=icons,plugins,root. Ёто то, что будет отмечено в интерфейсе патчера. ≈сли нужно отметить все, то параметр /RES можно либо опустить, либо /RES=*.
;    Х /SILENT = ”становка в ускоренном режиме. ќтображаетс€ диалог прогресса.
;    Х /VERYSILENT = ”становка в ускоренном режиме. Ќичего не отображаетс€.
;    Х /LOG="log.txt" = —оздаЄт log-файл "log.txt" р€дом с IconPatcher.exe. ѕараметр примен€етс€ при отслеживании ошибок.
; Х —труктура папок в архивах, например, Icons\Proto_ICQ\*.ico, означает, что по пути, указанному в /BIN должна быть папка Icons, в ней файл Proto_ICQ (возможных расширений .exe, .dll, .mir).
; Х ѕапка root дл€ красоты и не повтор€ет оригинальную структуру программы, в ней только ресурсы дл€ exe.

#define AppName "IconPatcher"
#define AppFullName "Miranda icon patcher"
#define AppVerName "1.0"
#define AppPublisher "El Sanchez"

[Setup]
AppName={#AppName}
AppVerName={#AppVerName}
DefaultDirName={tmp}
CreateAppDir=no
Uninstallable=no
CreateUninstallRegKey=no
DisableWelcomePage=yes
DisableReadyPage=yes
DisableFinishedPage=yes
ShowLanguageDialog=no
OutputBaseFilename={#AppName}
SetupIconFile={res}\icon.ico
VersionInfoVersion={#AppVerName}
VersionInfoDescription={#AppFullName}
AppPublisher={#AppPublisher}
PrivilegesRequired=lowest
IconResource=plugin:{res}\plugin.ico|done:{res}\done.ico|skip:{res}\skip.ico|error:{res}\error.ico

[Languages]
Name: en; MessagesFile: compiler:default.isl,{res}\iconpatcher_en.isl
Name: ru; MessagesFile: compiler:Languages\russian.isl,{res}\iconpatcher_ru.isl
Name: de; MessagesFile: compiler:Languages\german.isl,{res}\iconpatcher_de.isl

[Files]
Source: {res}\7-zip32.dll; Flags: dontcopy

[Code]
#define A = (Defined UNICODE) ? "W" : "A"
const
  SZ_ERROR = 1;
  SZ_DLLERROR = 3;
  LOAD_LIBRARY_AS_DATAFILE = $2;
  RT_ICON = 3;
  RT_GROUP_ICON = (RT_ICON + 11);
  GENERIC_READ = $80000000;
  FILE_SHARE_READ = $0001;
  OPEN_EXISTING = $0003;
  INVALID_HANDLE_VALUE = (-1);
  PAGE_READONLY = $0002;
  FILE_MAP_READ = $0004;
  GWL_STYLE = (-16);
  GWL_EXSTYLE = (-20);
  TVS_CHECKBOXES = $0100;
  WS_EX_COMPOSITED = $02000000;
  TV_FIRST = $1100;
  TVM_GETIMAGELIST = (TV_FIRST + 8);
  TVSIL_STATE = 2;
  ILC_COLOR32 = $0020;
  SHGFI_ICON = $0100;
  SHGFI_SMALLICON = $0001;
  SHGFI_OPENICON = $0002;
  DI_NORMAL = $3;
  IMAGE_ICON = 1;
  BI_RGB = $0;
  DIB_RGB_COLORS = 0;

type
  INDIVIDUALINFO = record
    dwOriginalSize: DWORD;
    dwCompressedSize: DWORD;
    dwCRC: DWORD;
    uFlag: UINT;
    uOSType: UINT;
    wRatio: WORD;
    wDate: WORD;
    wTime: WORD;
  #ifdef UNICODE
    szFileName: array [0..512] of AnsiChar;
  #else
    szFileName: array [0..512] of Char;
  #endif
    dummy1: array [0..2] of Byte;
  #ifdef UNICODE
    szAttribute: array [0..7] of AnsiChar;
    szMode: array [0..7] of AnsiChar;
  #else
    szAttribute: array [0..7] of Char;
    szMode: array [0..7] of Char;
  #endif
  end;

  SHFILEINFO = record
    hIcon: HICON;
    iIcon: Integer;
    dwAttributes: DWORD;
    szDisplayName: array [0..259] of Char;
    szTypeName: array [0..79] of Char;
  end;

  BITMAPINFOHEADER = record
    biSize: DWORD;
    biWidth: Longint;
    biHeight: Longint;
    biPlanes: WORD;
    biBitCount: WORD;
    biCompression: DWORD;
    biSizeImage: DWORD;
    biXPelsPerMeter: Longint;
    biYPelsPerMeter: Longint;
    biClrUsed: DWORD;
    biClrImportant: DWORD;
  end;

  RGBQUAD = record
    rgbBlue: Byte;
    rgbGreen: Byte;
    rgbRed: Byte;
    rgbReserved: Byte;
  end;

  BITMAPINFO = record
    bmiHeader: BITMAPINFOHEADER;
    bmiColors: array of RGBQUAD;
  end;

  ICONDIRENTRY = record
    bWidth: Byte;
    bHeight: Byte;
    bColorCount: Byte;
    bReserved: Byte;
    wPlanes: WORD;
    wBitCount: WORD;
    dwBytesInRes: DWORD;
    dwImageOffset: DWORD;
  end;

  ICONDIR = record
    idReserved: WORD;
    idType: WORD;
    idCount: WORD;
    idEntries: array [0..0] of ICONDIRENTRY;
  end;

  GRPICONDIRENTRY = record
    bWidth: Byte;
    bHeight: Byte;
    bColorCount: Byte;
    bReserved: Byte;
    wPlanes: WORD;
    wBitCount: WORD;
    dwBytesInRes: DWORD;
    nID: WORD;
  end;

  GRPICONDIR = record
    idReserved: WORD;
    idType: WORD;
    idCount: WORD;
    idEntries: array [0..255] of GRPICONDIRENTRY;
  end;

  TUpdateResItem = record
    szName: string;
    wLanguage: WORD;
    lpIconDir: Longint;
  end;

  TUpdateResHelper = record
    Items: array of TUpdateResItem;
    IconList: TStringList;
  end;

function WaitForSingleObject(hHandle : THandle; dwMilliseconds : DWORD) : DWORD; external 'WaitForSingleObject@kernel32.dll stdcall';
function OpenProcess(dwDesiredAccess: DWORD; bInheritHandle: BOOL; dwProcessId: Longint): THandle; external 'OpenProcess@kernel32.dll stdcall';
function TerminateProcess(hProcess:THandle; exitCode: DWORD): BOOL; external 'TerminateProcess@kernel32.dll stdcall';

function SevenZipOpenArchive(const hwnd: HWND; szFileName: AnsiString; const dwMode: DWORD): THandle; external 'SevenZipOpenArchive@files:7-zip32.dll stdcall';
function SevenZipCloseArchive(harc: THandle): Integer; external 'SevenZipCloseArchive@files:7-zip32.dll stdcall';
function SevenZipFindFirst(harc: THandle; szWildName: AnsiString; var lpSubInfo: INDIVIDUALINFO): Integer; external 'SevenZipFindFirst@files:7-zip32.dll stdcall';
function SevenZipFindNext(harc: THandle; var lpSubInfo: INDIVIDUALINFO): Integer; external 'SevenZipFindNext@files:7-zip32.dll stdcall';
function SevenZipCheckArchive(szFileName: AnsiString; const iMode: Integer): BOOL; external 'SevenZipCheckArchive@files:7-zip32.dll stdcall';
function SevenZip(const hwnd: HWND; szCmdLine, szOutput: AnsiString; const dwSize: DWORD): Integer; external 'SevenZip@files:7-zip32.dll stdcall';
function SevenZipSetOwnerWindowEx(hwnd: HWND; lpArcProc: Longint): BOOL; external 'SevenZipSetOwnerWindowEx@files:7-zip32.dll stdcall';
function SevenZipKillOwnerWindowEx(hwnd: HWND): BOOL; external 'SevenZipKillOwnerWindowEx@files:7-zip32.dll stdcall';

function RtlMoveMemoryWORD(var Destination: WORD; const Source: Longint; len: Integer): Integer; external 'RtlMoveMemory@kernel32.dll stdcall';
function RtlMoveMemoryGRPIDE(var Destination: GRPICONDIRENTRY; const Source: Longint; len: Integer): Integer; external 'RtlMoveMemory@kernel32.dll stdcall';
function RtlMoveMemoryIDE(var Destination: ICONDIRENTRY; const Source: Longint; len: Integer): Integer; external 'RtlMoveMemory@kernel32.dll stdcall';
function RtlMoveMemoryBI(var Destination: BITMAPINFOHEADER; const Source: Longint; len: Integer): Integer; external 'RtlMoveMemory@kernel32.dll stdcall';

function LoadLibraryEx(lpFileName: string; hFile: THandle; dwFlags: DWORD): THandle; external 'LoadLibraryEx{#A}@kernel32.dll stdcall';
function FreeLibrary(hModule: THandle): BOOL; external 'FreeLibrary@kernel32.dll stdcall';

function BeginUpdateResource(pFileName: string; bDeleteExistingResources: BOOL): THandle; external 'BeginUpdateResource{#A}@kernel32.dll stdcall';
function UpdateResource(hUpdate: THandle; lpType, lpName: Longint; wLanguage: WORD; lpData: Longint; cbData: DWORD): BOOL; external 'UpdateResource{#A}@kernel32.dll stdcall';
function UpdateResourcePtr(hUpdate: THandle; lpType, lpName: Longint; wLanguage: WORD; var lpData: GRPICONDIR; cbData: DWORD): BOOL; external 'UpdateResource{#A}@kernel32.dll stdcall';
function EndUpdateResource(hUpdate: THandle; fDiscard: BOOL): BOOL; external 'EndUpdateResource{#A}@kernel32.dll stdcall';

function EnumResourceNames(hModule: THandle; lpszType, lpEnumFunc, lParam: Longint): BOOL; external 'EnumResourceNames{#A}@kernel32.dll stdcall';
function EnumResourceLanguages(hModule: THandle; lpType, lpName, lpEnumFunc, lParam: Longint): BOOL; external 'EnumResourceLanguages{#A}@kernel32.dll stdcall';

function LoadImage(hInst: THandle; lpszName: string; uType: UINT; cxDesired, cyDesired: Integer; fuLoad: UINT): THandle; external 'LoadImage{#A}@user32.dll stdcall';
function LoadResource(hModule, hResInfo: THandle): THandle; external 'LoadResource@kernel32.dll stdcall';
function LockResource(hResData: THandle): Longint; external 'LockResource@kernel32.dll stdcall';
function FindResource(hModule: THandle; lpName, lpType: Longint): THandle; external 'FindResource{#A}@kernel32.dll stdcall';

function CreateFile(lpFileName: string; dwDesiredAccess, dwShareMode: DWORD; lpSecurityAttributes: Longint; dwCreationDisposition, dwFlagsAndAttributes: DWORD; hTemplateFile: THandle): THandle; external 'CreateFile{#A}@kernel32.dll stdcall';
function CreateFileMapping(hFile: THandle; lpAttributes: Longint; flProtect, dwMaximumSizeHigh, dwMaximumSizeLow: DWORD; lpName: String): THandle; external 'CreateFileMapping{#A}@kernel32.dll stdcall';
function MapViewOfFile(hFileMappingObject: THandle; dwDesiredAccess, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap: DWORD): Longint; external 'MapViewOfFile@kernel32.dll stdcall';
function UnmapViewOfFile(lpBaseAddress: Longint): BOOL; external 'UnmapViewOfFile@kernel32.dll stdcall';
function CloseHandle(hObject: THandle): BOOL; external 'CloseHandle@kernel32.dll stdcall';

function GetWindowLong(hWnd: HWND; nIndex: Integer): Longint; external 'GetWindowLong{#A}@user32.dll stdcall';
function SetWindowLong(hWnd: HWND; nIndex: Integer; dwNewLong: Longint): Longint; external 'SetWindowLong{#A}@user32.dll stdcall';

function ImageList_Create(cx, cy: Integer; flags: UINT; cInitial, cGrow: Integer): HIMAGELIST; external 'ImageList_Create@comctl32.dll stdcall';
function ImageList_Add(himl: HIMAGELIST; hbmImage, hbmMask: HBITMAP): Integer; external 'ImageList_Add@comctl32.dll stdcall';
function ImageList_GetIconSize(himl: HIMAGELIST; out cx, cy: Integer): BOOL; external 'ImageList_GetIconSize@comctl32.dll stdcall';

function SHGetFileInfo(pszPath: string; dwFileAttributes: DWORD; var psfi: SHFILEINFO; cbFileInfo, uFlags: UINT): DWORD; external 'SHGetFileInfo{#A}@shell32.dll stdcall';
function DestroyIcon(hIcon: HICON): BOOL; external 'DestroyIcon@user32.dll stdcall';
function DrawIconEx(hdc: Longint; xLeft, yTop: Integer; hIcon: HICON; cxWidth, cyWidth: Integer; istepIfAniCur: UINT; hbrFlickerFreeDraw: Longint; diFlags: UINT): BOOL; external 'DrawIconEx@user32.dll stdcall';

function CreateDIBSection(hdc: THandle; const pbmi: BITMAPINFO; iUsage: UINT; out ppvBits: Longint; hSection: THandle; dwOffset: DWORD): HBITMAP; external 'CreateDIBSection@gdi32.dll stdcall';

function ShowWindow(hWnd: HWND; nCmdShow: Integer): BOOL; external 'ShowWindow@user32.dll stdcall';

var
  UpdateForm: TForm;
  StatusBar: TStatusBar;
  ProgressBar: TNewProgressBar;
  BrowseButton: TNewButton;
  BrowseEdit: TEdit;
  SelectArchiveComboBox: TComboBox;
  SelectResButton, PatchButton: TNewButton;
  SelectResTreeView: TTreeView;
  ResourcesList: TStringList;
  ResHelper: TUpdateResHelper;

///////////////////////////////////////////////////////////////////
function CharArrayToString(const AChar: array of AnsiChar): string;
var
  i: Integer;
begin
  Result := '';
  while AChar[i] <> #0 do
  begin
    Insert(AChar[i], Result, Length(Result) + 1);
    i := i + 1;
  end;
end;

///////////////////////////////////////////////////////
function IsParameter(const Parameter: string): Boolean;
var
  i: Integer;
begin
  for i := 2 to ParamCount do
    if CompareText(Parameter, ParamStr(i)) = 0 then
    begin
      Result := True;
      Break;
    end;
end;

//////////////////////////////
function DisplayHelp: Boolean;
var
  i: Integer;
begin
  for i := 2 to ParamCount do
    if CompareText('/?', ParamStr(i)) = 0 then
    begin
      MsgBoxEx(0, FmtMessage(CustomMessage('cmHelpMsg'), [ExtractFileName(ExpandConstant('{srcexe}'))]), FmtMessage(CustomMessage('cmHelpMsgCaption'), ['{#AppName}']), MB_OK, Application.Icon.Handle, 0);
      Result := True;
      Break;
    end;
end;

//////////////////////////////
function CheckSource: Boolean;
var
  FR: TFindRec;
  FileName: AnsiString;
  hArc: THandle;
  lpSubInfo: INDIVIDUALINFO;
begin
  Log(CustomMessage('cmLogFindResource7ZipFile1a'));
  ResourcesList := TStringList.Create;
  ResourcesList.Sorted := True;
  if FindFirst(ExpandConstant('{src}\*.7z'), FR) then
  try
    repeat
      FileName := ExpandConstant(Format('{src}\%s', [FR.Name]));
      if SevenZipCheckArchive(FileName, 0) then
      begin
        Log(FmtMessage(CustomMessage('cmLogResource7ZipFile'), [FR.Name]));
        ResourcesList.AddObject(FileName, TStringList.Create);
        with TStringList(ResourcesList.Objects[ResourcesList.Count - 1] as TStringList) do
        begin
          Sorted := True;
          hArc := SevenZipOpenArchive(0, ResourcesList[ResourcesList.Count - 1], 0);
          if hArc <> 0 then
          begin
            if SevenZipFindFirst(hArc, '*.ico', lpSubInfo) = 0 then
            begin
              Add(CharArrayToString(lpSubInfo.szFileName));
              while SevenZipFindNext(hArc, lpSubInfo) = 0 do
                Add(CharArrayToString(lpSubInfo.szFileName));
            end;
            SevenZipCloseArchive(hArc);
          end;
          Log(FmtMessage(CustomMessage('cmLogResource7ZipFileContent'), [CommaText]));
        end;
      end;
    until not FindNext(FR);
  finally
    FindClose(FR);
  end;
  Result := (ResourcesList.Count > 0);
  if not Result then
    MsgBox(CustomMessage('cmFileNotFound'), mbError, MB_OK);
  Log(CustomMessage('cmLogFindResource7ZipFile1b'));
end;

//////////////////////////////////////////////////////////////////////////////////
function ExtractArchive(const FileName, OutputDir, AddParam: AnsiString): Integer;
var
  Params: AnsiString;
begin
  Result := SZ_ERROR;
  Params := Format('e "%s" -o"%s" "%s" -y -hide', [FileName, OutputDir, AddParam]);
  CharToOemBuff(Params);
  try
    SevenZipSetOwnerWindowEx(UpdateForm.Handle, 0);
    Result := SevenZip(UpdateForm.Handle, Params, '', 0);
  finally
    SevenZipKillOwnerWindowEx(UpdateForm.Handle);
  except
    Result := SZ_DLLERROR;
  end;
end;

///////////////////////////////////
function GetFreeIconIndex: Integer;
var
  i: Integer;
begin
  Result := 1;
  while ResHelper.IconList.Find(Padz(IntToStr(Result), 5), i) do
    Inc(Result);
  ResHelper.IconList.Add(Padz(IntToStr(Result), 5));
end;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
function EnumResLangProc(hModule: THandle; lpszType, lpszName: Longint; wIDLanguage: WORD; lParam: Longint): BOOL;
begin
  ResHelper.Items[lParam].wLanguage := wIDLanguage;
  Result := False;
end;

//////////////////////////////////////////////////////////////////////////////////////
function EnumResNameProc(hModule: THandle; lpszType, lpszName, lParam: Longint): BOOL;
var
  ResName: string;
  bUpdate: Boolean;
  R, Res, hFile, hMapObject: THandle;
  lpGroupIconDir: Longint;
  idCount: WORD;
  grpide: GRPICONDIRENTRY;
  i, n: Integer;
begin
  Result := True;

  { resource name }
  if lpszName shr $10 > 0 then
    ResName := CastIntegerToString(lpszName)
  else
    ResName := IntToStr(lpszName);
  bUpdate := FileExists(ExpandConstant(Format('{tmp}\resources\%s.ico', [ResName])));

  { load group icon resource }
  R := FindResource(hModule, lpszName, lpszType);
  if R = 0 then Exit;
  Res := LoadResource(hModule, R);
  if Res = 0 then Exit;
  lpGroupIconDir := LockResource(Res);
  if lpGroupIconDir = 0 then Exit;
  RtlMoveMemoryWORD(idCount, lpGroupIconDir + 4{offset idCount}, SizeOf(idCount));
  if idCount = 0 then Exit;

  if bUpdate then
  begin
    { update resource helper }
    n := GetArrayLength(ResHelper.Items);
    SetArrayLength(ResHelper.Items, n + 1);
    if lpszName shr $10 > 0 then
      ResHelper.Items[n].szName := CastIntegerToString(lpszName)
    else
      ResHelper.Items[n].szName := Format('%d', [lpszName]);
    EnumResourceLanguages(hModule, lpszType, lpszName, CallBackAddr('EnumResLangProc'), n);

    { delete 'RT_GROUP_ICON' }
    if not UpdateResource(lParam, lpszType, lpszName, ResHelper.Items[n].wLanguage, 0, 0) then Exit;

    { delete the RT_ICON icon resources that belonged to 'RT_GROUP_ICON' }
    for i := 0 to idCount - 1 do
    begin
      RtlMoveMemoryGRPIDE(grpide, lpGroupIconDir + 6{offset idEntries} + i * SizeOf(grpide), SizeOf(grpide));
      if not UpdateResource(lParam, RT_ICON, grpide.nID, ResHelper.Items[n].wLanguage, 0, 0) then Exit;
    end;

    { open icon resource }
    hFile := CreateFile(ExpandConstant(Format('{tmp}\resources\%s.ico', [ResName])), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if hFile = INVALID_HANDLE_VALUE then Exit;
    try
      hMapObject := CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, '');
      if hMapObject = 0 then Exit;
      ResHelper.Items[n].lpIconDir := MapViewOfFile(hMapObject, FILE_MAP_READ, 0, 0, 0);
      if ResHelper.Items[n].lpIconDir = 0 then Exit;
    finally
      CloseHandle(hMapObject);
      CloseHandle(hFile);
    end;
  end
  else
  begin
    { add nonupdatable icon ordinals }
    for i := 0 to idCount - 1 do
    begin
      RtlMoveMemoryGRPIDE(grpide, lpGroupIconDir + 6{offset idEntries} + i * SizeOf(grpide), SizeOf(grpide));
      ResHelper.IconList.Add(Padz(IntToStr(grpide.nID), 5));
    end;
  end;
end;

//////////////////////////////////////////////////////////////
function UpdateFileResources(const FileName: string): Boolean;
var
  H, M: THandle;
  i, j: Integer;
  idCount: WORD;
  lpGrpData: GRPICONDIR;
  cbGrpData: DWORD;
  grpide: GRPICONDIRENTRY;
  ide: ICONDIRENTRY;
  bih: BITMAPINFOHEADER;
begin
  M := LoadLibraryEx(FileName, 0, LOAD_LIBRARY_AS_DATAFILE);
  if M = 0 then Exit;
  H := BeginUpdateResource(FileName, False);
  if H = 0 then Exit;
  try
    { enum resources }
    EnumResourceNames(M, RT_GROUP_ICON, CallbackAddr('EnumResNameProc'), H);

    { update the resources }
    for i := 0 to GetArrayLength(ResHelper.Items) - 1 do
    begin
      { check IconDir }
      Result := (ResHelper.Items[i].lpIconDir <> 0);
      if not Result then Exit;

      { build the new group icon resource }
      RtlMoveMemoryWORD(idCount, ResHelper.Items[i].lpIconDir + 4{offset idCount}, SizeOf(idCount));
      cbGrpData := 6{offset idEntries} + idCount * SizeOf(grpide);
      lpGrpData.idReserved := 0;
      lpGrpData.idType := 1;
      lpGrpData.idCount := idCount;
      for j := 0 to idCount - 1 do
      begin
        RtlMoveMemoryIDE(ide, ResHelper.Items[i].lpIconDir + 6{offset idEntries} + j * SizeOf(ide), SizeOf(ide));
        RtlMoveMemoryBI(bih, ResHelper.Items[i].lpIconDir + ide.dwImageOffset, SizeOf(bih));
        lpGrpData.idEntries[j].bWidth := ide.bWidth;
        lpGrpData.idEntries[j].bHeight := ide.bHeight;
        if (bih.biClrUsed = 0) and (bih.biBitCount < 9) then
          lpGrpData.idEntries[j].bColorCount := 1 shl bih.biBitCount
        else
          lpGrpData.idEntries[j].bColorCount := bih.biClrUsed;
        lpGrpData.idEntries[j].bReserved := ide.bReserved;
        lpGrpData.idEntries[j].wPlanes := bih.biPlanes;
        lpGrpData.idEntries[j].wBitCount := bih.biBitCount;
        lpGrpData.idEntries[j].dwBytesInRes := ide.dwBytesInRes;
        lpGrpData.idEntries[j].nID := GetFreeIconIndex;

        { update 'MAINICON' }
        with ResHelper.Items[i] do
          if not UpdateResource(H, RT_ICON, lpGrpData.idEntries[j].nID, wLanguage, lpIconDir + ide.dwImageOffset, ide.dwBytesInRes) then Continue;
      end;

      { update group }
      with ResHelper.Items[i] do
        if not UpdateResourcePtr(H, RT_GROUP_ICON, StrToIntDef(szName, CastStringToInteger(szName)), wLanguage, lpGrpData, cbGrpData) then Continue;

      { free memory }
      UnmapViewOfFile(ResHelper.Items[i].lpIconDir);
    end;
  finally
    FreeLibrary(M);
    if Result then
      Result := EndUpdateResource(H, not Result);
    SetArrayLength(ResHelper.Items, 0);
    ResHelper.IconList.Clear;
  end;
end;

//////////////////////////////
procedure UpdateResourcesProc;
var
  RootPath, RootDir, DirName, FileName, OutputDir, AddParam: AnsiString;
  ItemText: AnsiString;
  i, ResultCode: Integer;
  handle: THandle;
  PID: Longint;
begin

  PID := StrToIntDef(ExpandConstant('{param:PID}'), 0);
  if PID <> 0 then
  begin
    handle := OpenProcess(1048576 or 1, false, PID);
    if handle <> 0 then
    begin
      StatusBar.Panels[0].Text := CustomMessage('cmWaiting');
      StatusBar.Refresh;
      if WaitForSingleObject(handle, 15000) = 258 then
        TerminateProcess(handle, 9);
      CloseHandle(handle);
    end
  end;

  { ResHelper }
  Log(CustomMessage('cmLogUpdateResources1a'));
  ResHelper.IconList := TStringList.Create;
  ResHelper.IconList.Sorted := True;
  ResHelper.IconList.Duplicates := dupIgnore;

  { update UI }
  UpdateForm.Enabled := False;
  ProgressBar.Max := 0;
  SelectResTreeView.Selected := SelectResTreeView.TopItem;
  for i := 0 to SelectResTreeView.Items.Count - 1 do
    if (SelectResTreeView.Items[i].Level = 1) and (SelectResTreeView.Items[i].StateIndex = 2) then
      ProgressBar.Max := ProgressBar.Max + 1;

  { prepare }
  RootPath := ExtractFileDir(BrowseEdit.Text);
  OutputDir := ExpandConstant('{tmp}\resources');
  for i := 0 to SelectResTreeView.Items.Count - 1 do
  begin
    ItemText := SelectResTreeView.Items[i].Text;
    SelectResTreeView.Items[i].Selected := True;
    if (SelectResTreeView.Items[i].StateIndex = 1) and (SelectResTreeView.Items[i].Level = 1) then
      SelectResTreeView.Items[i].StateIndex := 4
    else if SelectResTreeView.Items[i].StateIndex = 2 then
    begin
      if SelectResTreeView.Items[i].Level = 0 then
        RootDir := ItemText
      else if SelectResTreeView.Items[i].Level = 1 then
      begin
        { update UI }
        ProgressBar.Position := ProgressBar.Position + 1;

        { extract files }
        FileName := ResourcesList[SelectArchiveComboBox.ItemIndex];
        AddParam := Format('%s\%s\*.ico', [RootDir, ItemText]);
        Log(FmtMessage(CustomMessage('cmLogExtractResource7ZipFile'), [ExtractFileName(ItemText), ExtractFileName(FileName)]));
        ResultCode := ExtractArchive(FileName, OutputDir, AddParam);
        Log(FmtMessage(CustomMessage('cmLogResultCode'), [IntToStr(ResultCode)]));
        if ResultCode = 0 then
        begin
          { get file }
          case Lowercase(RootDir) of
            'root':
              DirName := RootPath;
          else
            DirName := Format('%s\%s', [RootPath, RootDir]);
          end;
            FileName := FileSearch(ItemText + '.exe', DirName);
          if FileName = '' then
            FileName := FileSearch(ItemText + '.dll', DirName);
          if FileName = '' then
            FileName := FileSearch(ItemText + '.mir', DirName);

          { update UI }
          StatusBar.Panels[0].Text := FmtMessage(CustomMessage('cmStatusLabelCaption1a'), [ExtractFileName(FileName)]);
          StatusBar.Refresh;

          { update resources in file }
          if FileExists(FileName) then
          begin
            if UpdateFileResources(FileName) then
            begin
              SelectResTreeView.Items[i].StateIndex := 3; // ok
              Log(FmtMessage(CustomMessage('cmLogUpdateFileResourcesStatus1a'), [ExtractFileName(FileName)]));
            end
            else
            begin
              SelectResTreeView.Items[i].StateIndex := 5; // error
              Log(FmtMessage(CustomMessage('cmLogUpdateFileResourcesStatus1b'), [ExtractFileName(FileName)]));
            end;
          end
          else
          begin
            SelectResTreeView.Items[i].StateIndex := 4;   // skip
            Log(FmtMessage(CustomMessage('cmLogUpdateFileResourcesStatus1c'), [Format('%0:s.exe, %0:s.dll, %0:s.mir', [ItemText])]));
          end;

          { cleanup }
          DelTree(OutputDir + '\*.ico', False, True, False);
        end;
      end;
    end;
  end;

  { update UI }
  SelectResButton.Caption := CustomMessage('cmSelectResButtonCaption1c');
  PatchButton.Enabled := False;
  StatusBar.Panels[0].Text := CustomMessage('cmStatusLabelCaption1b');
  ProgressBar.Position := 0;
  UpdateForm.Enabled := True;

  { cleanup }
  ResHelper.IconList.Free;
  Log(CustomMessage('cmLogUpdateResources1b'));
end;

/////////////////////////////////////////
procedure ButtonOnClick(Sender: TObject);
var
  i: Integer;
  FileName: string;
begin
  with TNewButton(Sender as TNewButton) do
  case Sender of
    SelectResButton:
      begin
        case Caption of
          CustomMessage('cmSelectResButtonCaption1a'):
            begin
              Caption := CustomMessage('cmSelectResButtonCaption1b');
              for i := 0 to SelectResTreeView.Items.Count - 1 do
                SelectResTreeView.Items[i].StateIndex := 2;
            end;
          CustomMessage('cmSelectResButtonCaption1b'):
            begin
              Caption := CustomMessage('cmSelectResButtonCaption1a');
              for i := 0 to SelectResTreeView.Items.Count - 1 do
                SelectResTreeView.Items[i].StateIndex := 1;
            end;
          CustomMessage('cmSelectResButtonCaption1c'):
            begin
              PatchButton.Enabled := FileExists(BrowseEdit.Text) and (ResourcesList.Count > 0);
              Caption := CustomMessage('cmSelectResButtonCaption1b');
              for i := 0 to SelectResTreeView.Items.Count - 1 do
                SelectResTreeView.Items[i].StateIndex := 2;
            end;
        end;
      end;
    BrowseButton:
      if GetOpenFileName(CustomMessage('cmBinariesPath'), FileName, '', CustomMessage('cmFilter'), 'Miranda??.exe') then
      begin
        BrowseEdit.Text := FileName;
        PatchButton.Enabled := FileExists(BrowseEdit.Text) and (ResourcesList.Count > 0);
        case SelectResButton.Caption of
          CustomMessage('cmSelectResButtonCaption1c'):
            begin
              SelectResButton.Caption := CustomMessage('cmSelectResButtonCaption1b');
              for i := 0 to SelectResTreeView.Items.Count - 1 do
                SelectResTreeView.Items[i].StateIndex := 2;
            end;
        end;
      end;
    PatchButton:
      begin
        Log(FmtMessage(CustomMessage('cmLogSelectedBinaryFile'), [BrowseEdit.Text]));
        Log(FmtMessage(CustomMessage('cmLogSelectedResource7ZipFile'), [SelectArchiveComboBox.Text]));
        UpdateResourcesProc;
      end;
  end;
end;

//////////////////////////////////////////////
procedure BrowseEditOnChange(Sender: TObject);
begin
  with TEdit(Sender as TEdit) do
  begin
    if FileExists(Text) then
      Font.Color := clDefault
    else
      Font.Color := clRed;
  end;
  PatchButton.Enabled := FileExists(BrowseEdit.Text) and (ResourcesList.Count > 0);
end;

/////////////////////////////////////////////////////////
procedure SelectArchiveComboBoxOnChange(Sender: TObject);
var
  FileList, TempList: TStringList;
  FileName, DirName, PrevFileName, PrevDirName: string;
  i, Index: Integer;
  bCheck: Boolean;
begin
  with SelectResTreeView do
  begin
    if SelectArchiveComboBox.ItemIndex = -1 then Exit;
    Items.Clear;
    TempList := TStringList.Create;
    TempList.Sorted := True;
    TempList.CommaText := Lowercase(ExpandConstant('{param:RES|*}'));
    FileList := TStringList(ResourcesList.Objects[SelectArchiveComboBox.ItemIndex] as TStringList);
    for i := 0 to FileList.Count - 1 do
    begin
      DirName := ExtractFileDir(ExtractFileDir(FileList[i]));
      FileName := ExtractFileName(ExtractFileDir(FileList[i]));
      if DirName <> '' then
      begin
        if DirName <> PrevDirName then
        begin
          PrevDirName := DirName;
          bCheck := (Trim(TempList.Text) = '*') or TempList.Find(Lowercase(DirName), Index);
          Items.Add(Selected, DirName).Selected := True;
          if bCheck then
            Selected.StateIndex := 2
          else
            Selected.StateIndex := 1;
        end;
        if FileName <> PrevFileName then
        begin
          PrevFileName := FileName;
          Items.AddChild(Selected, FileName).StateIndex := Selected.StateIndex;
        end;
      end;
    end;
    Selected := Items[0];
    TempList.Free;

    { update UI }
    PatchButton.Enabled := FileExists(BrowseEdit.Text) and (ResourcesList.Count > 0);
    case SelectResButton.Caption of
      CustomMessage('cmSelectResButtonCaption1c'):
        begin
          SelectResButton.Caption := CustomMessage('cmSelectResButtonCaption1b');
          for i := 0 to Items.Count - 1 do
            Items[i].StateIndex := 2;
        end;
    end;
  end;
end;

//////////////////////////////////////////////////////////////////////////////////////////////
procedure StatusBarOnDrawPanel(StatusBar: TStatusBar; Panel: TStatusPanel; const Rect: TRect);
begin
  ProgressBar.SetBounds(Rect.Left, Rect.Top, Rect.Right - Rect.Left, Rect.Bottom - Rect.Top);
  StatusBar.OnDrawPanel := nil;
end;

//////////////////////////////////////////////////////////////////////
procedure TreeViewOnGetImageIndex(Sender: TObject; Node: TTreeNode);
begin
  with Node do
  begin
    if Level = 0 then
    begin
      if HasChildren and Expanded then
        ImageIndex := 1
      else
        ImageIndex := 0;
    end
    else if Level = 1 then
      ImageIndex := 2;
  end;
end;

///////////////////////////////////////////////////////////////////////
procedure TreeViewOnGetSelectedIndex(Sender: TObject; Node: TTreeNode);
begin
  with Node do
  begin
    if Level = 0 then
    begin
      if HasChildren and Expanded then
        SelectedIndex := 1
      else
        SelectedIndex := 0;
    end
    else if Level = 1 then
      SelectedIndex := 2;
  end;
end;

////////////////////////////////////////////////////////////////////////////////////////////////////////
procedure TreeViewOnMouseDown(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
var
  TreeView: TTreeView;
  CurNode, FindNode, ParentNode: TTreeNode;
begin
  if Button <> mbLeft then Exit;
  TreeView := TTreeView(Sender as TTreeView);
  CurNode := TreeView.GetNodeAt(X, Y);
  if htOnStateIcon in TreeView.GetHitTestInfoAt(X, Y) then
  with CurNode do
  begin
    if StateIndex = 2 then
      StateIndex := 1
    else
      StateIndex := 2;
    if (Level = 0) and HasChildren then
    begin
      FindNode := GetFirstChild;
      while Assigned(FindNode) do
      begin
        FindNode.StateIndex := StateIndex;
        FindNode := GetNextChild(FindNode);
      end;
    end
    else if (Level = 1) then
    begin
      FindNode := CurNode;
      while Assigned(FindNode.GetPrevSibling) do
        FindNode := FindNode.GetPrevSibling;
      if StateIndex = 1 then
      begin
        ParentNode := FindNode.GetPrev;
        while Assigned(FindNode) do
        begin
          if FindNode.StateIndex = 2 then Exit;
          FindNode := GetNextChild(FindNode);
        end;
        ParentNode.StateIndex := StateIndex;
      end
      else
        FindNode.GetPrev.StateIndex := 2;
    end;
  end;
end;

///////////////////////////////////////////////////////////////////////////////////////////////////
procedure AddIconToImageList(const himl: HIMAGELIST; const hIcon: HICON; const bFreeIcon: Boolean);
var
  bi: BITMAPINFO;
  lpvBits: Longint;
  cx, cy: Integer;
begin
  with TBitmap.Create do
  try
    if not ImageList_GetIconSize(himl, cx, cy) then Exit;
    Width := ScaleX(cx);
    Height := ScaleY(cy);
    bi.bmiHeader.biSize := SizeOf(bi.bmiHeader);
    bi.bmiHeader.biWidth := Width;
    bi.bmiHeader.biHeight := Height;
    bi.bmiHeader.biPlanes := 1;
    bi.bmiHeader.biBitCount := 32;
    bi.bmiHeader.biCompression := BI_RGB;
    bi.bmiHeader.biSizeImage := Width * Height * 4;
    SetArrayLength(bi.bmiColors, Width * Height);
    Handle := CreateDIBSection(Canvas.Handle, bi, DIB_RGB_COLORS, lpvBits, 0, 0);
    DrawIconEx(Canvas.Handle, Width - Height, 0, hIcon, Height, Height, 0, 0, DI_NORMAL);
    ImageList_Add(himl, Handle, 0);
  finally
    Free;
    if bFreeIcon then
      DestroyIcon(hIcon);
  end;
end;

procedure UpdateFormOnActivate(Sender: TObject);
begin
  ShowWindow(Application.Handle, SW_HIDE);
  SetWindowLong(Application.Handle, GWL_EXSTYLE, GetWindowLong(Application.Handle, GWL_EXSTYLE) or $80) ;
  ShowWindow(Application.Handle, SW_SHOW);
end;

///////////////////////////
procedure CreateUpdateForm;
var
  MainPanel, BrowsePanel, ButtonPanel: TPanel;
  i: Integer;
  shfi: SHFILEINFO;
  hStateImgList: THandle;
  hIcon: HICON;
begin
  Application.Title := '{#AppName} {#AppVerName}';

  { UpdateForm }
  UpdateForm := TForm.Create(Application);
  with UpdateForm do
  begin
    Width := ScaleX(370);
    Height := ScaleY(400);
    BorderStyle := bsSingle;
    Caption := '{#AppName} {#AppVerName}';
    Position := poDesktopCenter;
  end;

  { StatusBar }
  StatusBar := TStatusBar.Create(UpdateForm);
  with StatusBar do
  begin
    Parent := UpdateForm;
    Align := alBottom;
    ThemeBackground := False;
    Panels.Add;
    Panels[0].Width := 2 * UpdateForm.ClientWidth div 3;
    Panels[0].Bevel := pbNone;
    Panels.Add;
    Panels[1].Style := psOwnerDraw;
    Panels[1].Bevel := pbNone;
    OnDrawPanel := @StatusBarOnDrawPanel;
  end;

  { ProgressBar }
  ProgressBar := TNewProgressBar.Create(UpdateForm);
  with ProgressBar do
  begin
    Parent := StatusBar;
  end;

  { MainPanel }
  MainPanel := TPanel.Create(UpdateForm);
  with MainPanel do
  begin
    Parent := UpdateForm;
    Align := alClient;
    BevelOuter := bvNone;
  end;

  { Bevel }
  with TBevel.Create(UpdateForm) do
  begin
    Parent := UpdateForm;
    Align := alBottom;
    Shape := bsTopLine;
    Style := bsLowered;
    Height := 1;
  end;

  { BinariesPath }
  with TStatusBar.Create(UpdateForm) do
  begin
    Parent := MainPanel;
    Height := ScaleY(21);
    Align := alTop;
    ThemeBackground := False;
    Panels.Add;
    Panels[0].Bevel := pbNone;
    Panels[0].Text := CustomMessage('cmBinariesPath');
  end;

  { BrowsePanel }
  BrowsePanel := TPanel.Create(UpdateForm);
  with BrowsePanel do
  begin
    Parent := MainPanel;
    Top := Parent.ClientHeight;
    Align := alTop;
    BevelOuter := bvNone;
  end;

  { BrowseButton }
  BrowseButton := TNewButton.Create(UpdateForm);
  with BrowseButton do
  begin
    Parent := BrowsePanel;
    Align := alRight;
    Height := ScaleY(21);
    Width := Height;
    BrowsePanel.Height := Height;
    Caption := '...';
    OnClick := @ButtonOnClick;
  end;

  { BrowseEdit }
  BrowseEdit := TEdit.Create(UpdateForm);
  with BrowseEdit do
  begin
    Parent := BrowsePanel;
    Top := (BrowseButton.Height - Height) div 2;
    Width := Parent.ClientWidth - Parent.ClientHeight;
    ReadOnly := True;
    if FileExists(ExpandConstant('{param:BIN}')) then
      Text := ExpandConstant('{param:BIN}');
    OnChange := @BrowseEditOnChange;
  end;

  { SelectArchive }
  with TStatusBar.Create(UpdateForm) do
  begin
    Parent := MainPanel;
    Top := Parent.ClientHeight;
    Height := ScaleY(24);
    Align := alTop;
    ThemeBackground := False;
    Panels.Add;
    Panels[0].Bevel := pbNone;
    Panels[0].Text := CustomMessage('cmSelectArchive');
  end;

  { SelectArchiveComboBox }
  SelectArchiveComboBox := TComboBox.Create(UpdateForm);
  with SelectArchiveComboBox do
  begin
    Parent := MainPanel;
    Top := Parent.ClientHeight;
    Align := alTop;
    Style := csDropDownList;
    for i := 0 to ResourcesList.Count - 1 do
      Items.Add(ExtractFileName(ResourcesList[i]));
    ResourcesList.Find(ExpandFileName(ExpandConstant('{param:ARC}.7z')), i);
    ItemIndex := i;
    OnChange := @SelectArchiveComboBoxOnChange;
  end;

  { SelectResources }
  with TStatusBar.Create(UpdateForm) do
  begin
    Parent := MainPanel;
    Top := Parent.ClientHeight;
    Height := ScaleY(24);
    Align := alTop;
    ThemeBackground := False;
    Panels.Add;
    Panels[0].Bevel := pbNone;
    Panels[0].Text := CustomMessage('cmSelectResources');
  end;

  { ButtonPanel }
  ButtonPanel := TPanel.Create(UpdateForm);
  with ButtonPanel do
  begin
    Parent := MainPanel;
    Align := alBottom;
    BevelOuter := bvNone;
  end;

  { SelectResButton }
  SelectResButton := TNewButton.Create(UpdateForm);
  with SelectResButton do
  begin
    Parent := ButtonPanel;
    Align := alLeft;
    ButtonPanel.Height := Height;
    Caption := CustomMessage('cmSelectResButtonCaption1b');
    OnClick := @ButtonOnClick;
  end;

  { PatchButton }
  PatchButton := TNewButton.Create(UpdateForm);
  with PatchButton do
  begin
    Parent := ButtonPanel;
    Align := alRight;
    Caption := CustomMessage('cmPatchButtonCaption');
    OnClick := @ButtonOnClick;
  end;

  { SelectResTreeView }
  SelectResTreeView := TTreeView.Create(UpdateForm);
  with SelectResTreeView do
  begin
    Parent := MainPanel;
    Align := alClient;
    ReadOnly := True;
    HideSelection := False;
    SetWindowLong(Handle, GWL_STYLE, GetWindowLong(Handle, GWL_STYLE) or TVS_CHECKBOXES);
    SetWindowLong(Handle, GWL_EXSTYLE, GetWindowLong(Handle, GWL_EXSTYLE) or WS_EX_COMPOSITED);

    { create imagelist }
    Images := TImageList.Create(UpdateForm);
    Images.Handle := ImageList_Create(ScaleX(18), ScaleY(16), ILC_COLOR32, 0, 0);
    if SHGetFileInfo(ExpandConstant('{tmp}'), 0, shfi, SizeOf(shfi), SHGFI_ICON or SHGFI_SMALLICON) <> 0 then
      AddIconToImageList(Images.Handle, shfi.hIcon, False);
    if SHGetFileInfo(ExpandConstant('{tmp}'), 0, shfi, SizeOf(shfi), SHGFI_ICON or SHGFI_SMALLICON or SHGFI_OPENICON) <> 0 then
      AddIconToImageList(Images.Handle, shfi.hIcon, False);
    hIcon := LoadImage(HInstance, '_IS_PLUGIN', IMAGE_ICON, 0, 0, 0);
    if hIcon <> 0 then
      AddIconToImageList(Images.Handle, hIcon, True);

    { state imagelist }
    hStateImgList := SendMessage(Handle, TVM_GETIMAGELIST, TVSIL_STATE, 0);
    if hStateImgList <> 0 then
    begin
      hIcon := LoadImage(HInstance, '_IS_DONE', IMAGE_ICON, 0, 0, 0);
      if hIcon <> 0 then
        AddIconToImageList(hStateImgList, hIcon, False);
      hIcon := LoadImage(HInstance, '_IS_SKIP', IMAGE_ICON, 0, 0, 0);
      if hIcon <> 0 then
        AddIconToImageList(hStateImgList, hIcon, False);
      hIcon := LoadImage(HInstance, '_IS_ERROR', IMAGE_ICON, 0, 0, 0);
      if hIcon <> 0 then
        AddIconToImageList(hStateImgList, hIcon, False);
      StateImages := TImageList.Create(UpdateForm);
      StateImages.Handle := hStateImgList;
    end;

    OnGetImageIndex := @TreeViewOnGetImageIndex;
    OnGetSelectedIndex := @TreeViewOnGetSelectedIndex;
    OnMouseDown := @TreeViewOnMouseDown;
    SelectArchiveComboBoxOnChange(SelectArchiveComboBox);
  end;

  { do work }
  try
    if IsParameter('/SILENT') then
    begin
      MainPanel.Hide;
      UpdateForm.ClientHeight := StatusBar.Height;
      UpdateForm.Show;
      if PatchButton.Enabled then
        PatchButton.Click;
      Exit;
    end
    else if IsParameter('/VERYSILENT') then
    begin
      ShowWindow(Application.Handle, SW_HIDE);
      if PatchButton.Enabled then
        PatchButton.Click;
      Exit;
    end
    else
      UpdateForm.ShowModal;
  finally
    { cleanup }
    for i := 0 to ResourcesList.Count - 1 do
      TStringList(ResourcesList.Objects[i] as TStringList).Free;
    ResourcesList.Free;
    UpdateForm.Free;
  end;
end;

//////////////////////////////////
function InitializeSetup: Boolean;
begin
  Result := False;
  if not DisplayHelp then
    if CheckSource then
      CreateUpdateForm;
end;
