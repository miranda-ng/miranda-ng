unit general;

interface

uses
  Windows, Messages, SysUtils, IniFiles,
  m_api,
  ImportT,
  ImportTU;

var
  cp: cardinal;

var
  AppPath: array [0 .. MAX_PATH] of Char;
  TxtPatterns: array of RTxtPattern;
  Protocols: array of TDestProto;
  ProtoCount: integer;
  CheckForDuplicates: boolean;
  ShowDuplicates: boolean;

const
  BIN_PROCEDURE_COUNT = 6; // количество реализованых процедур бинарного импорта

const
{$EXTERNALSYM PBM_SETRANGE}
  PBM_SETRANGE = WM_USER + 1;
{$EXTERNALSYM PBM_SETPOS}
  PBM_SETPOS = WM_USER + 2;

const
  IMPORT_TXT_MODULE = 'ImportTXT';
  IMPORT_TXT_SERVICE = IMPORT_TXT_MODULE + '/Import';
  IMPORT_WIZ_SERVICE = IMPORT_TXT_MODULE + '/Wizard';

  // keys
  IMPORT_TXT_AS = 'AutoStart';
  IMPORT_TXT_LP = 'LastPattern';

type
  TOnAccountListChange = procedure();

var
  OnAccountListChange: TOnAccountListChange;

procedure ReadPatterns;
procedure ExtractFilePath(fName: PAnsiChar);
function GetContactProto(hContact: THandle): AnsiString;
function GetContactByUID(const proto: AnsiString; const id: AnsiString): THandle;
function GetContactByNick(const proto: AnsiString; const Nick: WideString): THandle;
procedure EnumProtocols;
function GetContactID(hContact: THandle; proto: AnsiString = ''; Contact: boolean = false): WideString;
function GetContactNick(hContact: THandle; proto: AnsiString = ''; Contact: boolean = false): WideString;
function DBReadByte(hContact: THandle; szModule: PAnsiChar; szSetting: PAnsiChar; default: byte = 0): byte;
function DBWriteByte(hContact: THandle; szModule: PAnsiChar; szSetting: PAnsiChar; val: byte): integer;
procedure SetLastPattern(lp: byte);
function GetLastPattern: byte;
function TimeStampToWStr(ts: DWORD): WideString;
function StrToTimeStamp(STime: PAnsiChar; len: integer): DWORD;

function RLWord(adr: integer): word;
function RLInteger(adr: integer): integer;

implementation

procedure ExtractFilePath(fName: PAnsiChar);
var
  p: PAnsiChar;
begin
  p := fName;
  if p <> nil then
  begin
    while p^ <> #0 do
      inc(p);
    while p^ <> '\' do
      dec(p);
    inc(p);
    p^ := #0;
  end;
end;

function ReadPattern(const FileName: String): boolean;
var
  TI: TIniFile;
  tempstr: String;
  pattern:pRTxtPattern;
begin
  TI := TIniFile.Create(FileName);
  try
    if not TI.SectionExists('General') then
      exit;

    pattern:=@TxtPatterns[High(TxtPatterns)];
    // if "General" exists
    if TI.ValueExists('General', 'Name') then
      pattern^.Name := TI.ReadString('General', 'Name', '')
    else
      exit;
    if TI.ValueExists('General', 'Type') then
      pattern^.IType := TI.ReadInteger('General', 'Type', 1)
    else
      exit;
    case pattern^.IType of
      1:
        begin
          if TI.ValueExists('General', 'Charset') then
          begin
            tempstr := TI.ReadString('General', 'Charset', 'UTF8');
            if tempstr = 'ANSI' then
              pattern^.Charset := inANSI
            else if tempstr = 'UTF8' then
              pattern^.Charset := inUTF8
            else if tempstr = 'UCS2' then
              pattern^.Charset := inUCS2
            else
              exit;
          end
          else
            exit;
          if pattern^.Charset = inANSI then
          begin
            pattern^.Codepage := TI.ReadInteger('General', 'Codepage', 0);
            if not IsValidCodePage(pattern^.Codepage) then
              pattern^.Codepage := 0;
          end;
          pattern^.UseHeader := TI.ReadInteger('General', 'UseHeader', 0);
          pattern^.UsePreMsg := TI.ReadBool('General', 'UsePreMsg', false);
          // Read message section
          if TI.SectionExists('Message') then
          begin
            if TI.ValueExists('Message', 'Pattern') then
              pattern^.Msg.Pattern := TI.ReadString('Message', 'Pattern', '')
            else
              exit;
            pattern^.Msg.Incoming := TI.ReadString('Message', 'In', '');
            pattern^.Msg.Outgoing := TI.ReadString('Message', 'Out', '');
            if TI.ValueExists('Message', 'Direction') then
              pattern^.Msg.Direction := TI.ReadInteger('Message', 'Direction', 0)
            else
              exit;
            if TI.ValueExists('Message', 'Day') then
              pattern^.Msg.Day := TI.ReadInteger('Message', 'Day', 0)
            else
              exit;
            if TI.ValueExists('Message', 'Month') then
              pattern^.Msg.Month := TI.ReadInteger('Message', 'Month', 0)
            else
              exit;
            if TI.ValueExists('Message', 'Year') then
              pattern^.Msg.Year := TI.ReadInteger('Message', 'Year', 0)
            else
              exit;
            if TI.ValueExists('Message', 'Hours') then
              pattern^.Msg.Hours := TI.ReadInteger('Message', 'Hours', 0)
            else
              exit;
            if TI.ValueExists('Message', 'Minutes') then
              pattern^.Msg.Minutes := TI.ReadInteger('Message', 'Minutes', 0)
            else
              exit;
            pattern^.Msg.Seconds := TI.ReadInteger('Message', 'Seconds', 0)
          end
          else
            exit;
          // if need read header section
          if (pattern^.UseHeader > 0) then
            if TI.SectionExists('Header') then
            begin
              if TI.ValueExists('Header', 'Pattern') then
                pattern^.Header.Pattern := TI.ReadString('Header', 'Pattern', '')
              else
                exit;
              if (not TI.ValueExists('Header', 'In')) and
                 ((pattern^.UseHeader and 1) = 1) then
                exit;
              pattern^.Header.Incoming := TI.ReadInteger('Header', 'In', 0);
              pattern^.Header.Outgoing := TI.ReadInteger('Header', 'Out', 0);
              pattern^.Header.InNick := TI.ReadInteger('Header', 'InNick', 0);
              pattern^.Header.OutNick := TI.ReadInteger('Header', 'OutNick', 0);
              pattern^.Header.InUID := TI.ReadInteger('Header', 'InUID', 0);
              pattern^.Header.OutUID := TI.ReadInteger('Header', 'OutUID', 0);
              if ((pattern^.UseHeader and 2) = 2) then
                if (pattern^.Header.InNick = 0) and
                   (pattern^.Header.InUID = 0) then
                  exit;
            end
            else
              exit;
          // if nead read PreMessage section
          if pattern^.UsePreMsg then
            if TI.SectionExists('PreMessage') then
            begin
              pattern^.PreMsg.PreRN := TI.ReadInteger('PreMessage', 'PreRN', -1);
              pattern^.PreMsg.AfterRN := TI.ReadInteger('PreMessage', 'AfterRN', -1);
              pattern^.PreMsg.PreSP := TI.ReadInteger('PreMessage', 'PreSP', 0);
              pattern^.PreMsg.AfterSP := TI.ReadInteger('PreMessage', 'AfterSP', 0);
            end
            else
              exit;
        end; // 1
      2:
        begin
          pattern^.BinProc := TI.ReadInteger('General', 'BinProcedure', 0);
          if (pattern^.BinProc > BIN_PROCEDURE_COUNT) then
            exit;
        end;
    end; // case
    pattern^.DefExtension := TI.ReadString('General', 'DefaultExtension', 'txt');
    pattern^.UseFileName := TI.ReadBool('General', 'UseFileName', false);
    // if nead read FileName section
    if pattern^.UseFileName then
      if TI.SectionExists('FileName') then
      begin
        if TI.ValueExists('FileName', 'Pattern') then
          pattern^.fName.Pattern := TI.ReadString('FileName', 'Pattern', '')
        else
          exit;
        pattern^.fName.InNick := TI.ReadInteger('FileName', 'InNick', 0);
        pattern^.fName.InUID := TI.ReadInteger('FileName', 'InUID', 0);
        if (pattern^.fName.InNick = 0) and (pattern^.fName.InUID = 0) then
          exit;
        pattern^.fName.OutNick := TI.ReadInteger('FileName', 'OutNick', 0);
        pattern^.fName.OutUID := TI.ReadInteger('FileName', 'OutUID', 0);
      end
      else
        exit;
  finally
    TI.Free;
    result := true;
  end;
  result := false;
end;

procedure ReadPatterns;
var
  SR: TSearchRec;
  FileAttrs: integer;
  i: integer;
begin
  FileAttrs := faAnyFile;
  i := 0;
  if FindFirst(AppPath + '\importtxt\*.ini', FileAttrs, SR) = 0 then
  begin
    repeat
      SetLength(TxtPatterns, i+1);
      if not ReadPattern(AppPath + '\importtxt\' + SR.Name) then
        inc(i);
    until FindNext(SR) <> 0;
    FindClose(SR);
  end;
  // cut unneded
  SetLength(TxtPatterns, i);
end;

function GetContactByUID(const proto: AnsiString; const id: AnsiString): THandle;
var
  Contact: THandle;
  otherproto: AnsiString;
  ci: TCONTACTINFO;
  idnum: integer;
  tempwstr: PWideChar;
  ws: WideString;
begin
  if not TryStrToInt(id, idnum) then
    idnum := 0;
  tempwstr := UTF8ToWide(PAnsiChar(id), tempwstr);
  ws := tempwstr;
  FreeMem(tempwstr);
  Contact := db_find_first();
  while (Contact <> 0) do
  begin
    otherproto := PAnsiChar(CallService(MS_PROTO_GETCONTACTBASEPROTO, Contact, 0));
    if otherproto = proto then
    begin
      ci.cbSize := SizeOf(ci);
      ci.dwFlag := CNF_UNIQUEID or CNF_UNICODE;
      ci.hContact := Contact;
      ci.szProto := PAnsiChar(otherproto);
      if CallService(MS_CONTACT_GETCONTACTINFO, 0, lparam(@ci)) = 0 then
      begin
        case (ci._type) of
          CNFT_BYTE:   if ci.retval.bVal = idnum then break;
          CNFT_WORD:   if ci.retval.wVal = idnum then break;
          CNFT_DWORD:  if ci.retval.dVal = DWORD(idnum) then break;
          CNFT_ASCIIZ: if ws = ci.retval.szVal.w then break;
        end; // case
      end; // if
    end; // if
    Contact := db_find_next(Contact);
  end; // while
  if Contact=0 then
    result := INVALID_HANDLE_VALUE
  else
    result := Contact;
end;

function GetContactByNick(const proto: AnsiString; const Nick: WideString): THandle;
var
  Contact: THandle;
  otherproto: AnsiString;
  ci: TCONTACTINFO;
begin
  result := INVALID_HANDLE_VALUE;
  Contact := db_find_first();
  while (Contact <> 0) do
  begin
    otherproto := PAnsiChar(CallService(MS_PROTO_GETCONTACTBASEPROTO, Contact, 0));
    if otherproto = proto then
    begin
      ci.cbSize := SizeOf(ci);
      ci.dwFlag := CNF_NICK;
      ci.dwFlag := ci.dwFlag or CNF_UNICODE;
      ci.hContact := Contact;
      ci.szProto := PAnsiChar(otherproto);
      if CallService(MS_CONTACT_GETCONTACTINFO, 0, lparam(@ci)) = 0 then
      begin
        if Nick = ci.retval.szVal.w then
        begin
          result := Contact;
          break;
        end;
      end; // if
    end; // if
    Contact := db_find_next(Contact);
  end; // while
end;

procedure EnumProtocols;
var
  i, iProtoCount: integer;
  ppAccounts: ^PPROTOACCOUNT;
  temps: WideString;
begin
  ProtoCount := 0;
  SetLength(Protocols, 30);
  CallService(MS_PROTO_ENUMACCOUNTS, int(@iProtoCount), int(@ppAccounts));
  for i := 1 to iProtoCount do
  begin
    temps := GetContactID(0, ppAccounts^^.szModuleName, false);
    if temps <> '' then
    begin
      Protocols[ProtoCount].ProtoName := ppAccounts^^.szModuleName;
      Protocols[ProtoCount].ProtoUID := temps;
      Protocols[ProtoCount].ProtoNick := GetContactNick(0, ppAccounts^^.szModuleName, false);
      inc(ProtoCount);
    end;
    inc(ppAccounts);
  end;
  SetLength(Protocols, ProtoCount);
end;

function GetContactProto(hContact: THandle): AnsiString;
begin
  result := PAnsiChar(CallService(MS_PROTO_GETCONTACTBASEPROTO, hContact, 0));
end;

function DBFreeVariant(dbv: PDBVARIANT): integer;
begin
  result := db_free(dbv);
end;

function GetContactID(hContact: THandle; proto: AnsiString = '';
  Contact: boolean = false): WideString;
var
  uid: PAnsiChar;
  dbv: TDBVARIANT;
  tempstr: PWideChar;
begin
  result := '';
  if not((hContact = 0) and Contact) then
  begin
    if proto = '' then
      proto := GetContactProto(hContact);
    uid := PAnsiChar(CallProtoService(PAnsiChar(proto), PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0));
    if (uid <> pAnsiChar(CALLSERVICE_NOTFOUND)) and (uid <> nil) then
    begin
      if db_get(hContact, PAnsiChar(proto), uid, @dbv) = 0 then
      begin
        case dbv._type of
          DBVT_BYTE:
            result := intToStr(dbv.bVal);
          DBVT_WORD:
            result := intToStr(dbv.wVal);
          DBVT_DWORD:
            result := intToStr(dbv.dVal);
          DBVT_ASCIIZ:
            begin
              tempstr := ANSIToWide(dbv.szVal.a, tempstr, cp);
              result := tempstr;
              FreeMem(tempstr);
            end;
          DBVT_UTF8:
            begin
              tempstr := UTF8ToWide(dbv.szVal.a, tempstr);
              result := tempstr;
              FreeMem(tempstr);
            end;
          DBVT_WCHAR:
            result := dbv.szVal.w;
        end;
        // free variant
        DBFreeVariant(@dbv);
      end;
    end;
  end;
end;

function GetContactNick(hContact: THandle; proto: AnsiString = '';
  Contact: boolean = false): WideString;
var
  dbv: TDBVARIANT;
  tempstr: PWideChar;
begin
  result := '';
  if not((hContact = 0) and Contact) then
  begin
    if proto = '' then
      proto := GetContactProto(hContact);
    if db_get(hContact, PAnsiChar(proto), 'Nick', @dbv) = 0 then
    begin
      case dbv._type of
        DBVT_BYTE:
          result := intToStr(dbv.bVal);
        DBVT_WORD:
          result := intToStr(dbv.wVal);
        DBVT_DWORD:
          result := intToStr(dbv.dVal);
        DBVT_ASCIIZ:
          begin
            tempstr := ANSIToWide(dbv.szVal.a, tempstr, cp);
            result := tempstr;
            FreeMem(tempstr);
          end;
        DBVT_UTF8:
          begin
            tempstr := UTF8ToWide(dbv.szVal.a, tempstr);
            result := tempstr;
            FreeMem(tempstr);
          end;
        DBVT_WCHAR:
          result := dbv.szVal.w;
      end;
      // free variant
      DBFreeVariant(@dbv);
    end;
  end;
end;

function DBReadByte(hContact: THandle; szModule: PAnsiChar;
  szSetting: PAnsiChar; default: byte = 0): byte;
var
  dbv: TDBVARIANT;
begin
  If db_get(hContact, szModule, szSetting, @dbv) <> 0 then
    result := default
  else
    result := dbv.bVal;
end;

function DBWriteByte(hContact: THandle; szModule: PAnsiChar; szSetting: PAnsiChar; val: byte): integer;
begin
  result := db_set_b(hContact, szModule, szSetting, val);
end;

procedure SetLastPattern(lp: byte);
begin
  DBWriteByte(0, IMPORT_TXT_MODULE, IMPORT_TXT_LP, lp);
end;

function GetLastPattern: byte;
begin
  result := DBReadByte(0, IMPORT_TXT_MODULE, IMPORT_TXT_LP, 0);
  if result >= Length(TxtPatterns) then
    result := 0;
end;

function TimeStampToWStr(ts: DWORD): WideString;
var
  dbtts: TDBTIMETOSTRING;
  s: WideString;
begin
  SetLength(s, 20);
  dbtts.szFormat.w := 'd s';
  dbtts.szDest.w := PWideChar(s);
  dbtts.cbDest := 20;
  CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, ts, int(@dbtts));
  result := s;
end;

function StrToTimeStamp(STime: PAnsiChar; len: integer): DWORD;
var
  hour, min, sec, Day, Month, Year: integer;
begin
  sec := 0;
  Day := (ord(STime[0]) - $30) * 10 + (ord(STime[1]) - $30);
  Month := (ord(STime[3]) - $30) * 10 + (ord(STime[4]) - $30);
  Year := (ord(STime[6]) - $30) * 1000 + (ord(STime[7]) - $30) * 100 +
    (ord(STime[8]) - $30) * 10 + (ord(STime[9]) - $30);
  hour := (ord(STime[11]) - $30) * 10 + (ord(STime[12]) - $30);
  min := (ord(STime[14]) - $30) * 10 + (ord(STime[15]) - $30);
  if len > 15 then
    sec := (ord(STime[17]) - $30) * 10 + (ord(STime[18]) - $30);
  result := Timestamp(Year, Month, Day, hour, min, sec);
end;

function RLWord(adr: integer): word;
begin
  result := PByte(adr + 1)^ + (PByte(adr)^ * $100);
end;

function RLInteger(adr: integer): integer;
begin
  result := PByte(adr + 3)^ + (PByte(adr + 2)^ * $100) +
    (PByte(adr + 1)^ * $10000) + (PByte(adr)^ * $1000000);
end;

begin
  GetModuleFileName(hInstance, @AppPath[0], MAX_PATH);
  ExtractFilePath(@AppPath);
  ReadPatterns;
end.
