{ ############################################################################ }
{ #                                                                          # }
{ #  MirandaNG HistoryToDB Plugin v2.5                                       # }
{ #                                                                          # }
{ #  License: GPLv3                                                          # }
{ #                                                                          # }
{ #  Author: Grigorev Michael (icq: 161867489, email: sleuthhound@gmail.com) # }
{ #                                                                          # }
{ ############################################################################ }

unit Global;

interface

uses
  Windows, SysUtils, IniFiles, Messages, XMLIntf, XMLDoc,
  FSMonitor, DCPcrypt2, DCPblockciphers, DCPsha1, DCPdes, DCPmd5, ActiveX, MapStream;

type
  TCopyDataType = (cdtString = 0, cdtImage = 1, cdtRecord = 2);
  TCopyDataStruct = packed record
    dwData: DWORD;
    cbData: DWORD;
    lpData: Pointer;
  end;
  //TByteArr = Array of Byte;
  TArrayOfString = Array of String;

const
  htdPluginShortName = 'MirandaNGHistoryToDB';
  htdDescription_RU  = 'Хранение истории сообщений в базе данных.';
  htdDescription_EN  = 'Storing the history in the database.';
  htdAuthor_EN       = 'Michael Grigorev';
  htdAuthor_RU       = 'Michael Grigorev';
  htdAuthorEmail     = 'sleuthhound@gmail.com';
  htdCopyright_EN    = '(c) 2011-2013 Michael Grigorev';
  htdCopyright_RU    = '(c) 2011-2013 Michael Grigorev';
  htdHomePageURL  = 'http://www.im-history.ru/';
  htdVerMajor     = {MAJOR_VER}2{/MAJOR_VER};
  htdVerMinor     = {MINOR_VER}5{/MINOR_VER};
  htdVerRelease   = {SUB_VER}0{/SUB_VER};
  htdVerBuild     = {BUILD}0{/BUILD};
  htdVersion      = htdVerMajor shl 24 + htdVerMinor shl 16 + htdVerRelease shl 8 + htdVerBuild;
  {$IFDEF WIN32}
  htdPlatform     = 'x86';
  {$ELSE}
  htdPlatform     = 'x64';
  {$ENDIF}
  htdDBName       = 'MirandaNGHistoryToDB';
  htdIMClientName = 'MirandaNG';
  {htdFLUpdateURL    = 'http://addons.miranda-im.org/feed.php?dlfile=0';
  htdFLVersionURL   = 'http://addons.miranda-im.org/details.php?action=viewfile&id=0';
  htdFLVersionPrefix= '<span class="fileNameHeader">'+htdPluginShortName+' ';
  htdUpdateURL      = 'http://www.im-history.ru/get.php?file=MirandaNGHistoryToDB';
  htdVersionURL     = 'http://www.im-history.ru/get.php?file=MirandaNGHistoryToDB-Version';
  htdVersionPrefix  = htdPluginShortName+' version ';
  htdChangelogURL = 'http://www.im-history.ru/changelog/miranda.html';}

  // Generate your own unique id for your plugin.
  // Do not use this UUID!
  // Use Shift+Ctrl+G or uuidgen.exe to generate the uuuid
  MIID_HISTORYTODBDLL:TGUID = '{1F83C057-C59F-483B-B82E-1AE5CA6138EB}';

  MS_MHTD_SHOWHISTORY: PAnsiChar = 'MirandaNGHistoryToDB/ShowHistory';
  MS_MHTD_GETVERSION: PAnsiChar = 'MirandaNGHistoryToDB/GetVersion';
  MS_MHTD_SHOWCONTACTHISTORY: PAnsiChar = 'MirandaNGHistoryToDB/ShowContactHistory';

  DefaultDBAddres = 'db01.im-history.ru';
  DefaultDBName = 'imhistory';
  ININame = 'HistoryToDB.ini';
  DefININame = 'DefaultUser.ini';
  MesLogName = 'HistoryToDBMes.sql';
  ErrLogName = 'HistoryToDBErr.log';
  ImportLogName = 'HistoryToDBImport.sql';
  ContactListName = 'ContactList.csv';
  ProtoListName = 'ProtoList.csv';
  DebugLogName = 'HistoryToDBDebug.log';
  MSG_LOG : WideString = 'insert into uin_%s values (null, %s, ''%s'', ''%s'', ''%s'', ''%s'', %s, ''%s'', ''%s'', ''%s'', null);';
  MSG_LOG_ORACLE : WideString = 'insert into uin_%s values (null, %s, ''%s'', ''%s'', ''%s'', ''%s'', %s, %s, ''%s'', ''%s'', null)';
  CHAT_MSG_LOG : WideString = 'insert into uin_chat_%s values (null, %s, ''%s'', ''%s'', ''%s'', ''%s'', %s, %s, %s, ''%s'', ''%s'', null);';
  CHAT_MSG_LOG_ORACLE : WideString = 'insert into uin_chat_%s values (null, %s, %s, ''%s'', ''%s'', ''%s'', %s, %s, %s, ''%s'', ''%s'', null)';
  // Начальная дата (01/01/1970) Unix Timestamp для функций конвертации
  UnixStartDate: TDateTime = 25569.0;
  // Ключ для шифрования посылок программам HistoryToDBSync и HistoryToDBViewer
  EncryptKey = 'jsU6s2msoxghsKsn7';
  // Для мультиязыковой поддержки
  WM_LANGUAGECHANGED = WM_USER + 1;
  dirLangs = 'langs\';
  defaultLangFile = 'English.xml';
  ThankYouText_Rus = 'Анна Никифорова за активное тестирование плагина.' + #13#10 +
                    'Кирилл Уксусов (UksusoFF) за активное тестирование плагина и новые идеи.' + #13#10 +
                    'Игорь Гурьянов за активное тестирование плагина.' + #13#10 +
                    'Вячеслав С. (HDHMETRO) за активное тестирование плагина.' + #13#10 +
                    'Providence за активное тестирование плагина и новые идеи.' + #13#10 +
                    'Cy6 за помощь в реализации импорта истории RnQ.';
  ThankYouText_Eng = 'Anna Nikiforova for active testing of plug-in.' + #13#10 +
                    'Kirill Uksusov (UksusoFF) for active testing of plug-in and new ideas.' + #13#10 +
                    'Igor Guryanov for active testing of plug-in.' + #13#10 +
                    'Vyacheslav S. (HDHMETRO) for active testing of plug-in.' + #13#10 +
                    'Providence for active testing of plug-in and new ideas.' + #13#10 +
                    'Cy6 for help in implementing the import history RnQ.';

var
  hppCodepage: Cardinal;
  hppVersionStr: AnsiString;
  MetaContactsEnabled: Boolean;
  MetaContactsProto: AnsiString;
  WriteErrLog, AniEvents, EnableHistoryEncryption, ShowPluginButton, AddSpecialContact, BlockSpamMsg: Boolean;
  EnableDebug, EnableCallBackDebug, ExPrivateChatName, GetContactList: Boolean;
  SyncMethod, SyncInterval, SyncMessageCount, MaxErrLogSize: Integer;
  DBType, DBName, DBUserName, DefaultLanguage: String;
  //Global_AccountUIN: WideString;
  //Global_AccountName: WideString;
  //Global_CurrentAccountUIN: WideString;
  //Global_CurrentAccountName: WideString;
  Global_CurrentAccountProtoID: Integer;
  Global_CurrentAccountProtoName, Global_CurrentAccountProtoAccount: WideString;
  Glogal_History_Type: Integer;
  //Global_ChatName: WideString;
  Global_AboutForm_Showing: Boolean;
  DllPath, DllName, ProfilePath, MyAccount: String;
  MessageCount: Integer;
  // Для мультиязыковой поддержки
  CoreLanguage: String;
  AboutFormHandle: HWND;
  ExportFormHandle: HWND;
  LangDoc: IXMLDocument;
  PluginPath: String = '';
  // Шифрование
  Cipher: TDCP_3des;
  Digest: Array[0..19] of Byte;
  Hash: TDCP_sha1;
  // Лог-файлы
  TFMsgLog: TextFile;
  MsgLogOpened: Boolean;
  TFErrLog: TextFile;
  ErrLogOpened: Boolean;
  TFDebugLog: TextFile;
  DebugLogOpened: Boolean;
  TFContactListLog: TextFile;
  ContactListLogOpened: Boolean;
  TFProtoListLog: TextFile;
  ProtoListLogOpened: Boolean;
  TFImportLog: TextFile;
  ImportLogOpened: Boolean;
  ExportFormDestroy: Boolean;
  // MMF
  FMap: TMapStream;

function BoolToIntStr(Bool: Boolean): String;
function UnixToDateTime(USec: Longint): TDateTime;
function PrepareString(const Source : PWideChar) : WideString;
function MatchStrings(Source, Pattern: String): Boolean;
function ReadCustomINI(INIPath, CustomParams, DefaultParamsStr: String): String;
function EncryptMD5(Str: String): String;
function EncryptStr(const Str: String): String;
function SearchMainWindow(MainWindowName: pWideChar): Boolean;
function OpenLogFile(LogPath: String; LogType: Integer): Boolean;
function GetMyFileSize(const Path: String): Integer;
function ExtractFileNameEx(FileName: String; ShowExtension: Boolean): String;
function WideStringToString(const ws: WideString; codePage: Word): AnsiString;
function AnsiToWideString(const S: AnsiString; CodePage: Cardinal; InLength: Integer = -1): WideString;
function WideToAnsiString(const WS: WideString; CodePage: Cardinal; InLength: Integer = -1): AnsiString;
function Utf8ToWideChar(Dest: PWideChar; MaxDestChars: Integer; Source: PAnsiChar; SourceBytes: Integer; CodePage: Cardinal = CP_ACP): Integer;
function StrContactProtoToInt(Proto: AnsiString): Integer;
function UnixToLocalTime(tUnix :Longint): TDateTime;
function GetUserTempPath: WideString;
procedure IMDelay(Value: Cardinal);
procedure EncryptInit;
procedure EncryptFree;
procedure WriteInLog(LogPath: String; TextString: String; LogType: Integer);
procedure CloseLogFile(LogType: Integer);
procedure LoadINI(INIPath: String);
procedure OnSendMessageToAllComponent(Msg: String);
procedure OnSendMessageToOneComponent(WinName, Msg: String);
procedure WriteCustomINI(INIPath, CustomParams, ParamsStr: String);
procedure ProfileDirChangeCallBack(pInfo: TInfoCallBack);
// Для мультиязыковой поддержки
procedure CoreLanguageChanged;
procedure MsgDie(Caption, Msg: WideString);
procedure MsgInf(Caption, Msg: WideString);
function GetLangStr(StrID: String): WideString;

implementation

uses Menu;

function BoolToIntStr(Bool: Boolean): String;
begin
  if Bool then
    Result := '1'
  else
    Result := '0'
end;

// Функция конвертации Unix Timestamp в DateTime
function UnixToDateTime(USec: Longint): TDateTime;
begin
  Result := (Usec / 86400) + UnixStartDate;
end;

// Функция для экранирования спецсимволов в строке
function PrepareString(const Source : PWideChar) : WideString;
var
  SLen,i : Cardinal;
  WSTmp : WideString;
  WChar : WideChar;
begin
 Result := '';
 SLen := Length(WideString(Source));
 if (SLen>0) then
  begin
   for i:=1 to SLen do
    begin
     WChar:=WideString(Source)[i];
     case WChar of
      #$09 :{tab}  WSTmp:=WSTmp+'\t';
      #$0A :{line feed}  WSTmp:=WSTmp+'\n';
      #$0D :{carriage return}  WSTmp:=WSTmp+'\r';
      #$27 :{single quote mark aka apostrophe?} WSTmp:=WSTmp+WChar+WChar;
      #$22, {double quote mark aka inch sign?}
      #$5C, {backslash itself}
      #$60 :{another single quote mark} WSTmp:=WSTmp+'\'+WChar;
      else WSTmp := WSTmp + WChar;
     end;
    end;
   Result := WSTmp;
  end;
end;

// LogType = 0 - сообщения добавляются в файл MesLogName
// LogType = 1 - ошибки добавляются в файл ErrLogName
// LogType = 2 - сообщения добавляются в файл DebugLogName
// LogType = 3 - сообщения добавляются в файл ContactListName
// LogType = 4 - сообщения добавляются в файл ProtoListName
// LogType = 5 - сообщения добавляются в файл ImportLogName
function OpenLogFile(LogPath: String; LogType: Integer): Boolean;
var
  Path: WideString;
begin
  if LogType = 0 then
    Path := LogPath + MesLogName
  else if LogType = 1 then
  begin
    Path := LogPath + ErrLogName;
    if (LogType > 0) and (GetMyFileSize(Path) > MaxErrLogSize*1024) then
      DeleteFile(Path);
  end
  else if LogType = 2 then
    Path := LogPath + DebugLogName
  else if LogType = 3 then
  begin
    Path := LogPath + ContactListName;
    if FileExists(Path) then
    begin
      try
        DeleteFile(Path);
      except
      end;
    end;
  end
  else if LogType = 4 then
  begin
    Path := LogPath + ProtoListName;
    if FileExists(Path) then
    begin
      try
        DeleteFile(Path);
      except
      end;
    end;
  end
  else
    Path := LogPath + ImportLogName;
  {$I-}
  try
    if LogType = 0 then
      Assign(TFMsgLog, Path)
    else if LogType = 1 then
      Assign(TFErrLog, Path)
    else if LogType = 2 then
      Assign(TFDebugLog, Path)
    else if LogType = 3 then
      Assign(TFContactListLog, Path)
    else if LogType = 4 then
      Assign(TFProtoListLog, Path)
    else
      Assign(TFImportLog, Path);
    if FileExists(Path) then
    begin
      if LogType = 0 then
        Append(TFMsgLog)
      else if LogType = 1 then
        Append(TFErrLog)
      else if LogType = 2 then
        Append(TFDebugLog)
      else if LogType = 3 then
        Append(TFContactListLog)
      else if LogType = 4 then
        Append(TFProtoListLog)
      else
        Append(TFImportLog);
    end
    else
    begin
      if LogType = 0 then
        Rewrite(TFMsgLog)
      else if LogType = 1 then
        Rewrite(TFErrLog)
      else if LogType = 2 then
        Rewrite(TFDebugLog)
      else if LogType = 3 then
        Rewrite(TFContactListLog)
      else if LogType = 4 then
        Rewrite(TFProtoListLog)
      else
        Rewrite(TFImportLog);
    end;
    Result := True;
  except
    on e :
      Exception do
      begin
        CloseLogFile(LogType);
        Result := False;
        Exit;
      end;
  end;
  {$I+}
end;

// LogType = 0 - сообщения добавляются в файл MesLogName
// LogType = 1 - ошибки добавляются в файл ErrLogName
// LogType = 2 - сообщения добавляются в файл DebugLogName
// LogType = 3 - сообщения добавляются в файл ContactListName
// LogType = 4 - сообщения добавляются в файл ProtoListName
procedure WriteInLog(LogPath: String; TextString: String; LogType: Integer);
var
  Path: WideString;
begin
  if LogType = 0 then
  begin
    if not MsgLogOpened then
      MsgLogOpened := OpenLogFile(LogPath, 0);
    Path := LogPath + MesLogName
  end
  else if LogType = 1 then
  begin
    if not ErrLogOpened then
      ErrLogOpened := OpenLogFile(LogPath, 1);
    Path := LogPath + ErrLogName;
    if (LogType > 0) and (GetMyFileSize(Path) > MaxErrLogSize*1024) then
    begin
      CloseLogFile(LogType);
      DeleteFile(Path);
      if not OpenLogFile(LogPath, LogType) then
        Exit;
    end;
  end
  else if LogType = 2 then
  begin
    if not DebugLogOpened then
      DebugLogOpened := OpenLogFile(LogPath, 2);
    Path := LogPath + DebugLogName;
  end
  else if LogType = 3 then
  begin
    if not ContactListLogOpened then
      ContactListLogOpened := OpenLogFile(LogPath, 3);
    Path := LogPath + ContactListName;
  end
  else if LogType = 4 then
  begin
    if not ProtoListLogOpened then
      ProtoListLogOpened := OpenLogFile(LogPath, 4);
    Path := LogPath + ProtoListName;
  end
  else
  begin
    if not ImportLogOpened then
      ImportLogOpened := OpenLogFile(LogPath, 5);
    Path := LogPath + ImportLogName;
  end;
  {$I-}
  try
    if LogType = 0 then
      WriteLn(TFMsgLog, TextString)
    else if LogType = 1 then
      WriteLn(TFErrLog, TextString)
    else if LogType = 2 then
      WriteLn(TFDebugLog, TextString)
    else if LogType = 3 then
      WriteLn(TFContactListLog, TextString)
    else if LogType = 4 then
      WriteLn(TFProtoListLog, TextString)
    else
      WriteLn(TFImportLog, TextString);
  except
    on e :
      Exception do
      begin
        CloseLogFile(LogType);
        Exit;
      end;
  end;
  if MsgLogOpened then
    CloseLogFile(0);
  {$I+}
end;

procedure CloseLogFile(LogType: Integer);
begin
  {$I-}
  if LogType = 0 then
  begin
    CloseFile(TFMsgLog);
    MsgLogOpened := False;
  end
  else if LogType = 1 then
  begin
    CloseFile(TFErrLog);
    ErrLogOpened := False;
  end
  else if LogType = 2 then
  begin
    CloseFile(TFDebugLog);
    DebugLogOpened := False;
  end
  else if LogType = 3 then
  begin
    CloseFile(TFContactListLog);
    ContactListLogOpened := False;
  end
  else if LogType = 4 then
  begin
    CloseFile(TFProtoListLog);
    ProtoListLogOpened := False;
  end
  else
  begin
    CloseFile(TFImportLog);
    ImportLogOpened := False;
  end;
  {$I+}
end;

// Если файл не существует, то вместо размера файла функция вернёт -1
function GetMyFileSize(const Path: String): Integer;
var
  FD: TWin32FindData;
  FH: THandle;
begin
  FH := FindFirstFile(PChar(Path), FD);
  Result := 0;
  if FH = INVALID_HANDLE_VALUE then
    Exit;
  Result := FD.nFileSizeLow;
  if ((FD.nFileSizeLow and $80000000) <> 0) or
     (FD.nFileSizeHigh <> 0) then
    Result := -1;
  //FindClose(FH);
end;

// Загружаем настройки
procedure LoadINI(INIPath: String);
var
  Path: WideString;
  Temp: String;
  INI: TIniFile;
begin
  // Проверяем наличие каталога
  if not DirectoryExists(INIPath) then
    CreateDir(INIPath);
  Path := INIPath + ININame;
  if FileExists(Path) then
  begin
   Ini := TIniFile.Create(Path);
   DBType := INI.ReadString('Main', 'DBType', 'mysql');  // mysql или postgresql
   DBUserName := INI.ReadString('Main', 'DBUserName', 'username');
   SyncMethod := INI.ReadInteger('Main', 'SyncMethod', 1);
   SyncInterval := INI.ReadInteger('Main', 'SyncInterval', 0);

   Temp := INI.ReadString('Main', 'WriteErrLog', '1');
   if Temp = '1' then WriteErrLog := True
   else WriteErrLog := False;

   Temp := INI.ReadString('Main', 'ShowAnimation', '1');
   if Temp = '1' then AniEvents := True
   else AniEvents := False;

   Temp := INI.ReadString('Main', 'EnableHistoryEncryption', '0');
   if Temp = '1' then EnableHistoryEncryption := True
   else EnableHistoryEncryption := False;

   Temp := INI.ReadString('Main', 'AddSpecialContact', '1');
   if Temp = '1' then AddSpecialContact := True
   else AddSpecialContact := False;

   DefaultLanguage := INI.ReadString('Main', 'DefaultLanguage', 'Russian');
   SyncMessageCount := INI.ReadInteger('Main', 'SyncMessageCount', 50);

   Temp := INI.ReadString('Main', 'ShowPluginButton', '1');
   if Temp = '1' then ShowPluginButton := True
   else ShowPluginButton := False;

   Temp := INI.ReadString('Main', 'BlockSpamMsg', '0');
   if Temp = '1' then BlockSpamMsg := True
   else BlockSpamMsg := False;

   Temp := INI.ReadString('Main', 'EnableExPrivateChatName', '0');
   if Temp = '1' then ExPrivateChatName := True
   else ExPrivateChatName := False;

   Temp := INI.ReadString('Main', 'EnableDebug', '0');
   if Temp = '1' then EnableDebug := True
   else EnableDebug := False;

   Temp := INI.ReadString('Main', 'EnableCallBackDebug', '0');
   if Temp = '1' then EnableCallBackDebug := True
   else EnableCallBackDebug := False;

   MaxErrLogSize := INI.ReadInteger('Main', 'MaxErrLogSize', 20);
  end
 else
  begin
    INI := TIniFile.Create(path);
    // Значения по-умолчанию
    DBType := 'mysql';
    DBName := DefaultDBName;
    DBUserName := 'username';
    SyncMethod := 1;
    SyncInterval := 0;
    SyncMessageCount := 50;
    WriteErrLog := True;
    AniEvents := True;
    ShowPluginButton := True;
    EnableHistoryEncryption := False;
    AddSpecialContact := True;
    BlockSpamMsg := False;
    EnableDebug := False;
    EnableCallBackDebug := False;
    MaxErrLogSize := 20;
    // Сохраняем настройки
    INI.WriteString('Main', 'DBType', DBType);
    INI.WriteString('Main', 'DBAddress', DefaultDBAddres);
    INI.WriteString('Main', 'DBSchema', 'username');
    INI.WriteString('Main', 'DBPort', '3306');
    INI.WriteString('Main', 'DBName', DefaultDBName);
    INI.WriteString('Main', 'DBUserName', DBUserName);
    INI.WriteString('Main', 'DBPasswd', 'skGvQNyWUHcHohJS2+2r4A==');
    INI.WriteInteger('Main', 'SyncMethod', SyncMethod);
    INI.WriteInteger('Main', 'SyncInterval', SyncInterval);
    INI.WriteInteger('Main', 'SyncTimeCount', 40);
    INI.WriteInteger('Main', 'SyncMessageCount', SyncMessageCount);
    INI.WriteInteger('Main', 'NumLastHistoryMsg', 6);
    INI.WriteString('Main', 'WriteErrLog', BoolToIntStr(WriteErrLog));
    INI.WriteString('Main', 'ShowAnimation', BoolToIntStr(AniEvents));
    INI.WriteString('Main', 'EnableHistoryEncryption', BoolToIntStr(EnableHistoryEncryption));
    INI.WriteString('Main', 'DefaultLanguage', CoreLanguage);
    INI.WriteString('Main', 'HideHistorySyncIcon', '0');
    INI.WriteString('Main', 'ShowPluginButton', BoolToIntStr(ShowPluginButton));
    INI.WriteString('Main', 'AddSpecialContact', BoolToIntStr(AddSpecialContact));
    INI.WriteString('Main', 'BlockSpamMsg', BoolToIntStr(BlockSpamMsg));
    INI.WriteInteger('Main', 'MaxErrLogSize', MaxErrLogSize);
    INI.WriteString('Main', 'AlphaBlend', '0');
    INI.WriteString('Main', 'AlphaBlendValue', '255');
    INI.WriteString('Main', 'EnableDebug', '0');
    INI.WriteString('Main', 'EnableCallBackDebug', '0');
    INI.WriteString('Fonts', 'FontInTitle', '183|-11|Verdana|0|96|8|Y|N|N|N|');
    INI.WriteString('Fonts', 'FontOutTitle', '8404992|-11|Verdana|0|96|8|Y|N|N|N|');
    INI.WriteString('Fonts', 'FontInBody', '-16777208|-11|Verdana|0|96|8|N|N|N|N|');
    INI.WriteString('Fonts', 'FontOutBody', '-16777208|-11|Verdana|0|96|8|N|N|N|N|');
    INI.WriteString('Fonts', 'FontService', '16711680|-11|Verdana|0|96|8|Y|N|N|N|');
    INI.WriteString('Fonts', 'TitleParagraph', '4|4|');
    INI.WriteString('Fonts', 'MessagesParagraph', '2|2|');
    INI.WriteString('HotKey', 'GlobalHotKey', '0');
    INI.WriteString('HotKey', 'SyncHotKey', 'Ctrl+Alt+F12');
    INI.WriteString('HotKey', 'ExSearchHotKey', 'Ctrl+F3');
    INI.WriteString('HotKey', 'ExSearchNextHotKey', 'F3');
  end;
  INI.Free;
end;

{ Процедура записи значения параметра в файл настроек }
procedure WriteCustomINI(INIPath, CustomParams, ParamsStr: String);
var
  Path: String;
  INI: TIniFile;
begin
  Path := INIPath + ININame;
  if FileExists(Path) then
  begin
    INI := TIniFile.Create(Path);
    try
      INI.WriteString('Main', CustomParams, ParamsStr);
    finally
      INI.Free;
    end;
  end
  else
  begin
    if EnableDebug then WriteInLog(ProfilePath, FormatDateTime('dd.mm.yy hh:mm:ss', Now) + ' - Процедура WriteCustomINI: ' + GetLangStr('SettingsErrSave'), 2);
    MsgDie(htdPluginShortName, GetLangStr('SettingsErrSave'));
  end;
end;

{ Функция чтения значения параметра из файла настроек }
function ReadCustomINI(INIPath, CustomParams, DefaultParamsStr: String): String;
var
  Path: String;
  INI: TIniFile;
begin
  Path := INIPath + ININame;
  if FileExists(Path) then
  begin
    INI := TIniFile.Create(Path);
    try
      Result := INI.ReadString('Main', CustomParams, DefaultParamsStr);
    finally
      INI.Free;
    end;
  end
  else
  begin
    if EnableDebug then WriteInLog(ProfilePath, FormatDateTime('dd.mm.yy hh:mm:ss', Now) + ' - Процедура ReadCustomINI: ' + GetLangStr('SettingsErrRead'), 2);
    MsgDie(htdPluginShortName, GetLangStr('SettingsErrRead'));
  end;
end;

{ Процедура для отправки сообщений программе }
{ Стандартные команды:
  001  - Перечитать настройки из файла HistoryToDB.ini
  002  - Синхронизация истории
  003  - Закрыть все компоненты плагина
  0040 - Показать все окна плагина (Режим AntiBoss)
  0041 - Скрыть все окна плагина (Режим AntiBoss)
  005  - Показать окно настроек
  0050 - Запустить перерасчет MD5-хешей
  0051 - Запустить перерасчет MD5-хешей и удаления дубликатов
  0060 - Запущен импорт истории
  0061 - Импорт истории завершен
  007  - Обновить контакт-лист в БД
  008  - Показать историю контакта/чата
         Формат команды:
           для истории контакта:
             008|0|UserID|UserName|ProtocolType
           для истории чата:
             008|2|ChatName
  009 - Экстренно закрыть все компоненты плагина.
  010 - Строка SQL-insert передана в память
}
procedure OnSendMessageToAllComponent(Msg: String);
var
  HToDB: HWND;
  copyDataStruct : TCopyDataStruct;
  EncryptMsg, WinName: String;
begin
  if EnableDebug then WriteInLog(ProfilePath, FormatDateTime('dd.mm.yy hh:mm:ss', Now) + ' - Функция OnSendMessageToAllComponent: Отправка запроса "' + Msg + '" всем компонентам плагина.', 2);
  EncryptMsg := EncryptStr(Msg);
  // Ищем окно HistoryToDBViewer и посылаем ему команду
  WinName := 'HistoryToDBViewer for ' + htdIMClientName + ' ('+MyAccount+')';
  HToDB := FindWindow(nil, pWideChar(WinName));
  if HToDB <> 0 then
  begin
    copyDataStruct.dwData := {$IFDEF WIN32}Integer{$ELSE}LongInt{$ENDIF}(cdtString);
    copyDataStruct.cbData := Length(EncryptMsg) * SizeOf(Char);
    copyDataStruct.lpData := PChar(EncryptMsg);
    SendMessage(HToDB, WM_COPYDATA, 0, {$IFDEF WIN32}Integer{$ELSE}LongInt{$ENDIF}(@copyDataStruct));
    if EnableDebug then WriteInLog(ProfilePath, FormatDateTime('dd.mm.yy hh:mm:ss', Now) + ' - Функция OnSendMessageToAllComponent: Отправка запроса "' + Msg + '" окну ' + WinName, 2);
  end;
  // Ищем окно HistoryToDBSync и посылаем ему команду
  WinName := 'HistoryToDBSync for ' + htdIMClientName + ' ('+MyAccount+')';
  HToDB := FindWindow(nil, pWideChar(WinName));
  if HToDB <> 0 then
  begin
    copyDataStruct.dwData := {$IFDEF WIN32}Integer{$ELSE}LongInt{$ENDIF}(cdtString);
    copyDataStruct.cbData := Length(EncryptMsg) * SizeOf(Char);
    copyDataStruct.lpData := PChar(EncryptMsg);
    SendMessage(HToDB, WM_COPYDATA, 0, {$IFDEF WIN32}Integer{$ELSE}LongInt{$ENDIF}(@copyDataStruct));
    if EnableDebug then WriteInLog(ProfilePath, FormatDateTime('dd.mm.yy hh:mm:ss', Now) + ' - Функция OnSendMessageToAllComponent: Отправка запроса "' + Msg + '" окну ' + WinName, 2);
  end;
  // Ищем окно HistoryToDBImport и посылаем ему команду
  WinName := 'HistoryToDBImport for ' + htdIMClientName + ' ('+MyAccount+')';
  HToDB := FindWindow(nil, pWideChar(WinName));
  if HToDB <> 0 then
  begin
    copyDataStruct.dwData := {$IFDEF WIN32}Integer{$ELSE}LongInt{$ENDIF}(cdtString);
    copyDataStruct.cbData := Length(EncryptMsg) * SizeOf(Char);
    copyDataStruct.lpData := PChar(EncryptMsg);
    SendMessage(HToDB, WM_COPYDATA, 0, {$IFDEF WIN32}Integer{$ELSE}LongInt{$ENDIF}(@copyDataStruct));
    if EnableDebug then WriteInLog(ProfilePath, FormatDateTime('dd.mm.yy hh:mm:ss', Now) + ' - Функция OnSendMessageToAllComponent: Отправка запроса "' + Msg + '" окну ' + WinName, 2);
  end;
  // Ищем окно HistoryToDBUpdater и посылаем ему команду
  WinName := 'HistoryToDBUpdater for ' + htdIMClientName + ' ('+MyAccount+')';
  HToDB := FindWindow(nil, pWideChar(WinName));
  if HToDB <> 0 then
  begin
    copyDataStruct.dwData := {$IFDEF WIN32}Integer{$ELSE}LongInt{$ENDIF}(cdtString);
    copyDataStruct.cbData := Length(EncryptMsg) * SizeOf(Char);
    copyDataStruct.lpData := PChar(EncryptMsg);
    SendMessage(HToDB, WM_COPYDATA, 0, {$IFDEF WIN32}Integer{$ELSE}LongInt{$ENDIF}(@copyDataStruct));
    if EnableDebug then WriteInLog(ProfilePath, FormatDateTime('dd.mm.yy hh:mm:ss', Now) + ' - Функция OnSendMessageToAllComponent: Отправка запроса "' + Msg + '" окну ' + WinName, 2);
  end;
end;

procedure OnSendMessageToOneComponent(WinName, Msg: String);
var
  HToDB: HWND;
  copyDataStruct : TCopyDataStruct;
  AppNameStr, EncryptMsg: String;
begin
  // Ищем окно WinName и посылаем ему команду
  HToDB := FindWindow(nil, pWideChar(WinName));
  if HToDB <> 0 then
  begin
    EncryptMsg := EncryptStr(Msg);
    copyDataStruct.dwData := {$IFDEF WIN32}Integer{$ELSE}LongInt{$ENDIF}(cdtString);
    copyDataStruct.cbData := Length(EncryptMsg) * SizeOf(Char);
    copyDataStruct.lpData := PChar(EncryptMsg);
    SendMessage(HToDB, WM_COPYDATA, 0, {$IFDEF WIN32}Integer{$ELSE}LongInt{$ENDIF}(@copyDataStruct));
    if EnableDebug then WriteInLog(ProfilePath, FormatDateTime('dd.mm.yy hh:mm:ss', Now) + ' - Функция OnSendMessageToOneComponent: Отправка запроса "' + Msg + '" окну ' + WinName, 2);
  end;
end;

{ Поиск окна программы }
function SearchMainWindow(MainWindowName: pWideChar): Boolean;
var
  HToDB: HWND;
begin
  // Ищем окно
  HToDB := FindWindow(nil, MainWindowName);
  if HToDB <> 0 then
    Result := True
  else
    Result := False
end;

{Функция осуществляет сравнение двух строк. Первая строка
может быть любой, но она не должна содержать символов соответствия (* и ?).
Строка поиска (искомый образ) может содержать абсолютно любые символы.
Для примера: MatchStrings('David Stidolph','*St*') возвратит True.
Автор оригинального C-кода Sean Stanley
Автор портации на Delphi David Stidolph}
function MatchStrings(Source, Pattern: String): Boolean;
var
  pSource: array[0..255] of Char;
  pPattern: array[0..255] of Char;

  function MatchPattern(element, pattern: PChar): Boolean;

  function IsPatternWild(pattern: PChar): Boolean;
  begin
    Result := StrScan(pattern, '*') <> nil;
    if not Result then
      Result := StrScan(pattern, '?') <> nil;
  end;

  begin
    if 0 = StrComp(pattern, '*') then
      Result := True
    else if (element^ = Chr(0)) and (pattern^ <> Chr(0)) then
      Result := False
    else if element^ = Chr(0) then
      Result := True
    else
    begin
      case pattern^ of
        '*': if MatchPattern(element, @pattern[1]) then
            Result := True
          else
            Result := MatchPattern(@element[1], pattern);
        '?': Result := MatchPattern(@element[1], @pattern[1]);
      else
        if element^ = pattern^ then
          Result := MatchPattern(@element[1], @pattern[1])
        else
          Result := False;
      end;
    end;
  end;
begin
  StrPCopy(pSource, source);
  StrPCopy(pPattern, pattern);
  Result := MatchPattern(pSource, pPattern);
end;

// Для мультиязыковой поддержки
procedure MsgDie(Caption, Msg: WideString);
begin
  //{$IFDEF UNICODE}
  //MessageBoxW(GetForegroundWindow, PWideChar(Msg), PWideChar(Caption), MB_ICONERROR);
  //{$ELSE}
  //MessageBoxA(GetForegroundWindow, PAnsiChar(Msg), PAnsiChar(Caption), MB_ICONERROR);
  //{$ENDIF}
  MessageBox(GetForegroundWindow, PWideChar(Msg), PWideChar(Caption), MB_ICONERROR);
end;

// Для мультиязыковой поддержки
procedure MsgInf(Caption, Msg: WideString);
begin
  //{$IFDEF UNICODE}
  //MessageBoxW(GetForegroundWindow, PWideChar(Msg), PWideChar(Caption), MB_ICONINFORMATION);
  //{$ELSE}
  //MessageBoxA(GetForegroundWindow, PAnsiChar(Msg), PAnsiChar(Caption), MB_ICONINFORMATION);
  //{$ENDIF}
  MessageBox(GetForegroundWindow, PWideChar(Msg), PWideChar(Caption), MB_ICONINFORMATION);
end;

// Для мультиязыковой поддержки
function GetLangStr(StrID: String): WideString;
begin
  if (not Assigned(LangDoc)) or (not LangDoc.Active) then
  begin
    Result := '';
    Exit;
  end;
  if LangDoc.ChildNodes['strings'].ChildNodes.FindNode(StrID) <> nil then
    Result := LangDoc.ChildNodes['strings'].ChildNodes[StrID].Text
  else
    Result := 'String not found.';
end;

{ Обработчик изменений файлов в каталоге профиля }
procedure ProfileDirChangeCallBack(pInfo: TInfoCallBack);
var
  SettingsFormRequest: String;
begin
  SettingsFormRequest := ReadCustomINI(ProfilePath, 'SettingsFormRequestSend', '0');
  if EnableCallBackDebug then WriteInLog(ProfilePath, FormatDateTime('dd.mm.yy hh:mm:ss', Now) + ' - Процедура ProfileDirChangeCallBack: Параметр SettingsFormRequestSend = ' + SettingsFormRequest + ' | FAction = ' + IntToStr(pInfo.FAction) + ' | FOldFileName = ' + pInfo.FOldFileName + ' | FNewFileName = ' + Trim(pInfo.FNewFileName), 2);
  if (pInfo.FAction = 3) and (Trim(pInfo.FNewFileName) = 'HistoryToDB.ini') and (SettingsFormRequest = '0') then
  begin
    IMDelay(500);
    LoadINI(ProfilePath);
    CoreLanguage := DefaultLanguage;
    if EnableCallBackDebug then WriteInLog(ProfilePath, FormatDateTime('dd.mm.yy hh:mm:ss', Now) + ' - Процедура ProfileDirChangeCallBack: Настройки HistoryToDB.ini перечитаны. | Новый язык программы = ' + CoreLanguage, 2);
    // Инициализация COM с поддержкой многопоточности
    // ХЗ зачем это нужно, но если этого не делать, то при вызове CoreLanguageChanged
    // вылазит ошибка "Не был произведен вызов CoInitialize"
    // CoInitialize объявлен в модуле ActiveX
    CoInitializeEx(nil, COINIT_MULTITHREADED);
    // Перезагружяем языковой файл
    CoreLanguageChanged;
    // Освобождение COM (дважды)
    CoUninitialize();
    CoUninitialize();
    // Перестраиваем меню
    RebuildMainMenu;
    // MMF
    if SyncMethod = 0 then
    begin
      if not Assigned(FMap) then
      begin
        if EnableCallBackDebug then WriteInLog(ProfilePath, FormatDateTime('dd.mm.yy hh:mm:ss', Now) + ' - Процедура ProfileDirChangeCallBack: Создаем TMapStream', 2);
        FMap := TMapStream.CreateEx('HistoryToDB for QIP ('+MyAccount+')',MAXDWORD,2000);
      end;
    end
    else
    begin
      if Assigned(FMap) then
      begin
        FMap.Free;
        FMap := nil;
      end;
    end;
  end;
end;

// Подсчет MD5 строки
function EncryptMD5(Str: String): String;
var
  Hash: TDCP_md5;
  Digest: Array[0..15] of Byte;
  I: Integer;
  P: String;
begin
  if Str <> '' then
  begin
    Hash:= TDCP_md5.Create(nil);
    try
      Hash.HashSize := 128;
      Hash.Init;
      Hash.UpdateStr(Str);
      Hash.Final(Digest);
      P := '';
      for I:= 0 to 15 do
        P:= P + IntToHex(Digest[I], 2);
    finally
      Hash.Free;
    end;
    Result := P;
  end
  else
    Result := 'MD5';
end;

// Инициируем криптование
procedure EncryptInit;
begin
  Hash:= TDCP_sha1.Create(nil);
  try
    Hash.Init;
    Hash.UpdateStr(EncryptKey);
    Hash.Final(Digest);
  finally
    Hash.Free;
  end;
  Cipher := TDCP_3des.Create(nil);
  try
    Cipher.Init(Digest,Sizeof(Digest)*8,nil);
  except
    on E: Exception do
      if EnableDebug then WriteInLog(ProfilePath, FormatDateTime('dd.mm.yy hh:mm:ss', Now) + ' - Процедура EncryptInit: ' + E.Message, 2);
  end;
end;

// Освобождаем ресурсы
procedure EncryptFree;
begin
  if Assigned(Cipher) then
  begin
    Cipher.Burn;
    Cipher.Free;
  end;
end;

// Зашифровываем строку
function EncryptStr(const Str: String): String;
begin
  Result := '';
  if Str <> '' then
  begin
    Cipher.Reset;
    Result := Cipher.EncryptString(Str);
  end;
end;

{ Функция для получения имени файла из пути без или с его расширением.
  Возвращает имя файла, без или с его расширением.
  Входные параметры:
  FileName - имя файла, которое надо обработать
  ShowExtension - если TRUE, то функция возвратит короткое имя файла
  (без полного пути доступа к нему), с расширением этого файла, иначе, возвратит
  короткое имя файла, без расширения этого файла. }
function ExtractFileNameEx(FileName: String; ShowExtension: Boolean): String;
var
  I: Integer;
  S, S1: string;
begin
  I := Length(FileName);
  if I <> 0 then
  begin
    while (FileName[i] <> '\') and (i > 0) do
      i := i - 1;
    S := Copy(FileName, i + 1, Length(FileName) - i);
    i := Length(S);
    if i = 0 then
    begin
      Result := '';
      Exit;
    end;
    while (S[i] <> '.') and (i > 0) do
      i := i - 1;
    S1 := Copy(S, 1, i - 1);
    if s1 = '' then
      s1 := s;
    if ShowExtension = True then
      Result := s
    else
      Result := s1;
  end
  else
    Result := '';
end;

function WideStringToString(const ws: WideString; codePage: Word): AnsiString;
var
  l: integer;
begin
  if ws = '' then
    Result := ''
  else
  begin
    l := WideCharToMultiByte(codePage,
      WC_COMPOSITECHECK or WC_DISCARDNS or WC_SEPCHARS or WC_DEFAULTCHAR,
      @ws[1], -1, nil, 0, nil, nil);
    SetLength(Result, l - 1);
    if l > 1 then
      WideCharToMultiByte(codePage,
        WC_COMPOSITECHECK or WC_DISCARDNS or WC_SEPCHARS or WC_DEFAULTCHAR,
        @ws[1], -1, @Result[1], l - 1, nil, nil);
  end;
end;

function AnsiToWideString(const S: AnsiString; CodePage: Cardinal; InLength: Integer = -1): WideString;
var
  InputLength,
  OutputLength: Integer;
begin
  Result := '';
  if S = '' then
    exit;
  if Codepage = CP_UTF8 then
  begin
    Result := UTF8ToWideString(S); // CP_UTF8 not supported on Windows 95
  end
  else
  begin
    if InLength < 0 then
      InputLength := Length(S)
    else
      InputLength := InLength;
    OutputLength := MultiByteToWideChar(Codepage, 0, PAnsiChar(S), InputLength, nil, 0);
    SetLength(Result, OutputLength);
    MultiByteToWideChar(Codepage, MB_PRECOMPOSED, PAnsiChar(S), InputLength, PWideChar(Result),
      OutputLength);
  end;
end;

function WideToAnsiString(const WS: WideString; CodePage: Cardinal; InLength: Integer = -1): AnsiString;
var
  InputLength,
  OutputLength: Integer;
begin
  Result := '';
  if WS = '' then
    exit;
  if Codepage = CP_UTF8 then
    Result := UTF8Encode(WS) // CP_UTF8 not supported on Windows 95
  else
  begin
    if InLength < 0 then
      InputLength := Length(WS)
    else
      InputLength := InLength;
    OutputLength := WideCharToMultiByte(Codepage, 0, PWideChar(WS), InputLength, nil, 0,
      nil, nil);
    SetLength(Result, OutputLength);
    WideCharToMultiByte(Codepage, 0, PWideChar(WS), InputLength, PAnsiChar(Result),
      OutputLength, nil, nil);
  end;
end;

function StrContactProtoToInt(Proto: AnsiString): Integer;
var
  ProtoType: Integer;
begin
  { Протоколы
    0 - ICQ
    1 - Google Talk
    2 - MRA
    3 - Jabber
    4 - QIP.Ru
    5 - Facebook
    6 - VKontacte
    7 - Twitter
    8 - Social (LiveJournal)
    9 - AIM
    10 - IRC
    11 - MSN
    12 - YAHOO
    13 - GADU
    14 - SKYPE
    15 - MetaContacts
    16 - Unknown
  }
  if MatchStrings(LowerCase(Proto), 'icq*') then
    ProtoType := 0
  else if MatchStrings(LowerCase(Proto), 'google talk*') then
    ProtoType := 1
  else if MatchStrings(LowerCase(Proto), 'mrad*') then
    ProtoType := 15
  else if MatchStrings(LowerCase(Proto), 'mra*') then
    ProtoType := 2
  else if MatchStrings(LowerCase(Proto), 'jabber*') then
    ProtoType := 3
  else if (LowerCase(Proto) = 'qip.ru') then
    ProtoType := 4
  else if MatchStrings(LowerCase(Proto), 'facebook*') then
    ProtoType := 5
  else if MatchStrings(LowerCase(Proto), 'vkontakte*') then
    ProtoType := 6
  else if MatchStrings(LowerCase(Proto), 'вконтакте*') then
    ProtoType := 6
  else if MatchStrings(LowerCase(Proto), 'twitter*') then
    ProtoType := 7
  else if MatchStrings(LowerCase(Proto), 'livejournal*') then
    ProtoType := 8
  else if MatchStrings(LowerCase(Proto), 'aim*') then
    ProtoType := 9
  else if MatchStrings(LowerCase(Proto), 'irc*') then
    ProtoType := 10
  else if MatchStrings(LowerCase(Proto), 'msn*') then
    ProtoType := 11
  else if MatchStrings(LowerCase(Proto), 'yahoo*') then
    ProtoType := 12
  else if MatchStrings(LowerCase(Proto), 'gadu*') then
    ProtoType := 13
  else if MatchStrings(LowerCase(Proto), 'skype*') then
    ProtoType := 14
  else if MatchStrings(LowerCase(Proto), 'metacontacts*') then
    ProtoType := 15
  else
    ProtoType := 16;
  Result := ProtoType;
end;

{ Задержка не грузящая процессор }
procedure IMDelay(Value: Cardinal);
var
  F, N: Cardinal;
begin
  N := 0;
  while N <= (Value div 10) do
  begin
    SleepEx(1, True);
    //Application.ProcessMessages;
    Inc(N);
  end;
  F := GetTickCount;
  repeat
    //Application.ProcessMessages;
    N := GetTickCount;
  until (N - F >= (Value mod 10)) or (N < F);
end;

function Utf8ToWideChar(Dest: PWideChar; MaxDestChars: Integer; Source: PAnsiChar; SourceBytes: Integer; CodePage: Cardinal = CP_ACP): Integer;
const
  MB_ERR_INVALID_CHARS = 8;
var
  Src,SrcEnd: PAnsiChar;
  Dst,DstEnd: PWideChar;
begin
  if (Source = nil) or (SourceBytes <= 0) then
  begin
    Result := 0;
  end
  else if (Dest = nil) or (MaxDestChars <= 0) then
  begin
    Result := -1;
  end
  else
  begin
    Src := Source;
    SrcEnd := Source + SourceBytes;
    Dst := Dest;
    DstEnd := Dst + MaxDestChars;
    while (PAnsiChar(Src) < PAnsiChar(SrcEnd)) and (Dst < DstEnd) do
    begin
      if (Byte(Src[0]) and $80) = 0 then
      begin
        Dst[0] := WideChar(Src[0]);
        Inc(Src);
      end
      else if (Byte(Src[0]) and $E0) = $E0 then
      begin
        if Src + 2 >= SrcEnd then
          break;
        if (Src[1] = #0) or ((Byte(Src[1]) and $C0) <> $80) then
          break;
        if (Src[2] = #0) or ((Byte(Src[2]) and $C0) <> $80) then
          break;
        Dst[0] := WideChar(((Byte(Src[0]) and $0F) shl 12) + ((Byte(Src[1]) and $3F) shl 6) +
          ((Byte(Src[2]) and $3F)));
        Inc(Src, 3);
      end
      else if (Byte(Src[0]) and $E0) = $C0 then
      begin
        if Src + 1 >= SrcEnd then
          break;
        if (Src[1] = #0) or ((Byte(Src[1]) and $C0) <> $80) then
          break;
        Dst[0] := WideChar(((Byte(Src[0]) and $1F) shl 6) + ((Byte(Src[1]) and $3F)));
        Inc(Src, 2);
      end
      else
      begin
        if MultiByteToWideChar(CodePage, MB_ERR_INVALID_CHARS, Src, 1, Dst, 1) = 0 then
          Dst[0] := '?';
        Inc(Src);
      end;
      Inc(Dst);
    end;
    Dst[0] := #0;
    Inc(Dst);
    Result := Dst - Dest;
  end;
end;

// Конвертация Unix Timestamp в Локальное время с учетом пояса и Перехода на летнее время
function UnixToLocalTime(tUnix :Longint): TDateTime;
var
  TimeZone :TTimeZoneInformation;
  Bias     :Integer;
begin
  if (GetTimeZoneInformation(TimeZone) = TIME_ZONE_ID_DAYLIGHT) then
    Bias := TimeZone.Bias + TimeZone.DaylightBias
  else
    Bias := TimeZone.Bias + TimeZone.StandardBias;
  Result := EncodeDate(1970,1,1) - Bias / 1440 + tUnix / 86400;
end;

{ Функция для мультиязыковой поддержки }
procedure CoreLanguageChanged;
var
  LangFile: String;
begin
  if CoreLanguage = '' then
    Exit;
  try
    LangFile := PluginPath + dirLangs + CoreLanguage + '.xml';
    if FileExists(LangFile) then
      LangDoc.LoadFromFile(LangFile)
    else
    begin
      if FileExists(PluginPath + dirLangs + defaultLangFile) then
        LangDoc.LoadFromFile(PluginPath + dirLangs + defaultLangFile)
      else
      begin
        MsgDie(htdPluginShortName, 'Not found any language file!');
        Exit;
      end;
    end;
    SendMessage(AboutFormHandle, WM_LANGUAGECHANGED, 0, 0);
    SendMessage(ExportFormHandle, WM_LANGUAGECHANGED, 0, 0);
  except
    on E: Exception do
    begin
      if EnableDebug then WriteInLog(ProfilePath, FormatDateTime('dd.mm.yy hh:mm:ss', Now) + ' - Error on CoreLanguageChanged: ' + Trim(E.Message) + ' | CoreLanguage: ' + CoreLanguage, 2);
      MsgDie(htdPluginShortName, 'Error on CoreLanguageChanged: ' + E.Message + sLineBreak +
        'CoreLanguage: ' + CoreLanguage);
    end;
  end;
end;

{ Функция возвращает путь до пользовательской временной папки }
function GetUserTempPath: WideString;
var
  UserPath: WideString;
begin
  Result := '';
  SetLength(UserPath, MAX_PATH);
  GetTempPath(MAX_PATH, PChar(UserPath));
  GetLongPathName(PChar(UserPath), PChar(UserPath), MAX_PATH);
  SetLength(UserPath, StrLen(PChar(UserPath)));
  Result := UserPath;
end;

begin
  hppVersionStr := AnsiString(Format('%d.%d.%d.%d',[htdVerMajor,htdVerMinor,htdVerRelease,htdVerBuild]));
end.
