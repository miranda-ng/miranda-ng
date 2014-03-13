unit ImportThrd;

interface

uses
  Classes,
  Windows,
  SysUtils,
  StrUtils,
  PerlRegEx,
  m_api,
  general,
  ImportT,
  ImportTU,
  KOLEdb {ADODB if want to use ADO};

const
  ITXT_THREAD_BASE = $8000 + $2000; // WM_APP + $2000
  ITXT_THREAD_START = ITXT_THREAD_BASE + 1; // Поток запустился (0,0)
  ITXT_THREAD_MAXPROGRESS = ITXT_THREAD_BASE + 2;
  // Мах прогресс (0, MaxProgress)
  ITXT_THREAD_PROGRESS = ITXT_THREAD_BASE + 3; // Прогресс  (Current, 0)
  ITXT_THREAD_ERROR = ITXT_THREAD_BASE + 4;
  // Возникла ошибка (PWideChar(ErrorString),0)
  ITXT_THREAD_FINISH = ITXT_THREAD_BASE + 5;
  // Завершение файла, подведение результатов (Added, Duplicates)
  ITXT_THREAD_START_FILE = ITXT_THREAD_BASE + 6;
  // Начали работать с файлом(PWideChar(FileName),0);
  ITXT_THREAD_DEST_CONTACT = ITXT_THREAD_BASE + 7;
  // Определили контакт (hContact,0)
  ITXT_THREAD_ALLSTARTED = ITXT_THREAD_BASE + 8; // Началось
  ITXT_THREAD_ALLFINISHED = ITXT_THREAD_BASE + 9; // Всё закончено :)

type
  TSendMethod = (smSend, smPost);

type
  TImportThrd = class(TThread)
  private
    { Private declarations }
    RegExpr: TPerlRegEx;
    hMapedFile: THandle;
    hFile: THandle;
    pFileText: Pointer;
    FolderName: WideString;
    FileName: WideString;
    FileLen: Cardinal;
    fContact: TDestContact; // Contact recognised by filename
    AddedMessages: integer;
    Duplicates: integer;
    function DoMessage(Message: Longword; wParam: WPARAM; lParam: LPARAM;
      Method: TSendMethod = smSend): Boolean;
    function DoMapFile: Boolean;
    procedure DoUnMapFile;
    Procedure PreMessageSP(var src: AnsiString; CSP: integer);
    procedure AddMsgToDB(hContact: THandle; Direction: integer;
      MsgTimeStamp: Longword; const Text: AnsiString; var AddMsg, Dupy: integer);
    procedure TextImportProcedure;
    procedure BinImportProcedure;
  protected
    procedure Execute; override;
  public
    FileNames: WideString; // File Names
    OffsetFileName: integer; // offset name of file in FileNames
    WorkPattern: RTxtPattern; // Pattern for work
    DContact: TDestContact; // Recognised or defined contact
    Destination: TDestProto; // destination protocol
    ParentHWND: Longword; // HWND of parent window
  end;

function IsDuplicateEvent(hContact: THandle; dbei: TDBEVENTINFO): Boolean;

function PassMessage(Handle: THandle; Message: Longword; wParam: wParam;
  lParam: lParam; Method: TSendMethod = smSend): Boolean;

implementation

// Returns TRUE if event already is in base
function IsDuplicateEvent(hContact: THandle; dbei: TDBEVENTINFO): Boolean;
var
  hExistingDbEvent: THandle;
  dbeiExisting: TDBEVENTINFO;
  dwFirstEventTimeStamp: Longword;
  dwLastEventTimeStamp: Longword;
  dwPreviousTimeStamp: Longword;
begin
  result := FALSE;
  if not CheckForDuplicates then
    exit;
  hExistingDbEvent := db_event_first(hContact);
  if hExistingDbEvent = 0 then
  begin
    result := FALSE;
    exit;
  end;

  FillChar(dbeiExisting, SizeOf(dbeiExisting), Byte(0));
  dbeiExisting.cbSize := SizeOf(dbeiExisting);
  dbeiExisting.cbBlob := 0;
  db_event_get(hExistingDbEvent, @dbeiExisting);
  dwFirstEventTimeStamp := dbeiExisting.timestamp;

  hExistingDbEvent := db_event_last(hContact);
  if hExistingDbEvent = 0 then
  begin
    result := FALSE;
    exit;
  end;

  FillChar(dbeiExisting, SizeOf(dbeiExisting), Byte(0));
  dbeiExisting.cbSize := SizeOf(dbeiExisting);
  dbeiExisting.cbBlob := 0;
  db_event_get(hExistingDbEvent, @dbeiExisting);
  dwLastEventTimeStamp := dbeiExisting.timestamp;

  // If before the first
  if (dbei.timestamp < dwFirstEventTimeStamp) then
  begin
    result := FALSE;
    exit;
  end;

  // If after the last
  if (dbei.timestamp > dwLastEventTimeStamp) then
  begin
    result := FALSE;
    exit;
  end;

  dwPreviousTimeStamp := dwLastEventTimeStamp;

  if (dbei.timestamp <= dwPreviousTimeStamp) then // search from the end
  begin
    while (hExistingDbEvent <> 0) do
    begin
      FillChar(dbeiExisting, SizeOf(dbeiExisting), Byte(0));
      dbeiExisting.cbSize := SizeOf(dbeiExisting);
      dbeiExisting.cbBlob := 0;
      db_event_get(hExistingDbEvent, @dbeiExisting);
      // compare event
      if (dbei.timestamp = dbeiExisting.timestamp) and
        ((dbei.flags) = (dbeiExisting.flags)) and
      // fix for first event
        (dbei.eventType = dbeiExisting.eventType) and
        (dbei.cbBlob = dbeiExisting.cbBlob) then
      begin
        result := true;
        exit;
      end;

      if (dbei.timestamp > dbeiExisting.timestamp) then
      begin
        result := FALSE;
        exit;
      end;
      // get the previous
      hExistingDbEvent := db_event_prev(hContact,hExistingDbEvent);
    end;
  end;
end;

Procedure TImportThrd.PreMessageSP(var src: AnsiString; CSP: integer);
var
  i: integer;
  ls: integer;
  PSP, ASP: integer;
begin
  ls := -1;
  repeat
    i := ls + 2;
    PSP := 0;
    while (src[i + PSP] = ' ') do
      inc(PSP);
    if PSP > 0 then
      case WorkPattern.PreMsg.PreSP of
        0: PSP := 0;
        -1: ;
        -2:  if PSP > CSP then
            PSP := CSP;
      else
        if PSP > WorkPattern.PreMsg.PreSP then
          PSP := WorkPattern.PreMsg.PreSP;
      end; // case
    Delete(src, i, PSP);
    ls := PosEx(#$0D#$0A, src, i);
    ASP := 0;
    while (ls > 1) and (src[ls - ASP - 1] = ' ') do
      inc(ASP);
    if ASP > 0 then
      case WorkPattern.PreMsg.AfterSP of
        0:  ASP := 0;
        -1: ;
        -2: if ASP > CSP then
            ASP := CSP;
      else
        if ASP > WorkPattern.PreMsg.AfterSP then
          ASP := WorkPattern.PreMsg.AfterSP;
      end; // case
    Delete(src, ls - ASP - 1, ASP);
  Until ls <= 0
end;

Procedure TImportThrd.AddMsgToDB(hContact: THandle; Direction: integer;
  MsgTimeStamp: Longword; const Text: AnsiString; var AddMsg, Dupy: integer);
var
  dbei: TDBEVENTINFO;
  proto: AnsiString;
  s: WideString;
begin
  FillChar(dbei, SizeOf(dbei), Byte(0));
  dbei.cbSize := SizeOf(dbei);
  dbei.eventType := EVENTTYPE_MESSAGE;
  dbei.flags := Direction;
  proto := GetContactProto(hContact);
  dbei.szModule := PAnsiChar(proto);
  dbei.timestamp := MsgTimeStamp;
  dbei.cbBlob := Length(Text) + 1;
  dbei.pBlob := PByte(AllocMem(dbei.cbBlob));
  try
    Move(Text[1], dbei.pBlob^, dbei.cbBlob);
    if not IsDuplicateEvent(hContact, dbei) then
      if db_event_add(hContact, @dbei) <> 0 then
        inc(AddMsg)
      else
      begin
        s := 'Error adding message to database';
        DoMessage(ITXT_THREAD_ERROR, wparam(PWideChar(s)), 0);
      end
    else
    begin
      if ShowDuplicates then
      begin
        if (dbei.flags and DBEF_SENT) > 0 then
          s := '>'
        else
          s := '<';
        s := TranslateWideString('Duplicate:') + ' ' + s + ' ' +
          TimeStampToWStr(dbei.timestamp);
        DoMessage(ITXT_THREAD_ERROR, wparam(PWideChar(s)), 0, smSend);
      end;
      inc(Dupy);
    end;
  finally
    FreeMem(dbei.pBlob);
  end;
end;

function PassMessage(Handle: THandle; Message: Longword; wParam: wParam;
  lParam: lParam; Method: TSendMethod = smSend): Boolean;
var
  Tries: integer;
begin
  result := true;
  case Method of
    smSend:
      SendMessage(Handle, Message, wParam, lParam);
    smPost:  begin
      Tries := 5;
      while (Tries > 0) and not PostMessage(Handle, Message, wParam,
        lParam) do
      begin
        Dec(Tries);
        Sleep(5);
      end;
      result := (Tries > 0);
    end;
  end;
end;

function TImportThrd.DoMessage(Message: Longword; wParam: wParam;
  lParam: lParam; Method: TSendMethod = smSend): Boolean;
begin
  result := PassMessage(ParentHWND, Message, wParam, lParam, Method);
end;

function TImportThrd.DoMapFile: Boolean;
var
  s: pWideChar;
begin
  result := true;
  hFile := CreateFileW(PWideChar(FileName), GENERIC_READ, 0, nil,
    OPEN_EXISTING, 0, 0);
  if hFile = INVALID_HANDLE_VALUE then
  begin
    result := FALSE;
    s := 'Error opening file';
    DoMessage(ITXT_THREAD_ERROR, wparam(s), 0);
    exit;
  end;
  FileLen := GetFileSize(hFile, nil);
  hMapedFile := CreateFileMapping(hFile, nil, PAGE_READONLY, 0, 0, 'ImportTXTmapfile');
  if hMapedFile = 0 then
  begin
    result := FALSE;
    s := 'Error mapping file';
    DoMessage(ITXT_THREAD_ERROR, wparam(s), 0);
    exit;
  end;
  pFileText := MapViewOfFile(hMapedFile, FILE_MAP_READ, 0, 0, 0);
  if pFileText = nil then
  begin
    result := FALSE;
    s := 'Error mapping';
    DoMessage(ITXT_THREAD_ERROR, wparam(s), 0);
    exit;
  end;
end;

procedure TImportThrd.DoUnMapFile;
begin
  UnmapViewOfFile(pFileText);
  pFileText := nil;
  CloseHandle(hMapedFile);
  CloseHandle(hFile);
end;

procedure TryDetermContact(var DContact: TDestContact);
begin
  if DContact.ProtoName <> '' then
  begin
    if DContact.ContactUID <> '' then
    begin
      DContact.hContact := GetContactByUID(DContact.ProtoName, DContact.ContactUID)
    end
    else if DContact.ContactNick <> '' then
    begin
      DContact.hContact := GetContactByNick(DContact.ProtoName, DContact.ContactNick);
    end
    else
      DContact.hContact := INVALID_HANDLE_VALUE;
  end
  else
    DContact.hContact := INVALID_HANDLE_VALUE;
end;

procedure TImportThrd.TextImportProcedure;
var
  PosCur, LenCur, PosNext: integer;
  TextLength, h1, h2: integer;
  PRN, ARN, j: DWORD;
  msg_flag: integer;
  DT: Longword;
  TxtMsg: AnsiString;
  s: WideString;
  tempstr: PAnsiChar;
  tempwstr: PWideChar;
begin
  AddedMessages := 0;
  Duplicates := 0;
  Case WorkPattern.Charset of
    inANSI:
      begin
        if WorkPattern.Codepage <> 0 then
          tempstr := ANSIToUTF8(PAnsiChar(pFileText), tempstr, WorkPattern.Codepage)
        else
          tempstr := ANSIToUTF8(PAnsiChar(pFileText), tempstr, cp);
        RegExpr.Subject := tempstr;
        FreeMem(tempstr);
      end;
    inUTF8:
      RegExpr.Subject := PAnsiChar(pFileText) + 3;
    inUCS2:
      begin
        GetMem(tempwstr, FileLen + 2);
        lstrcpynW(tempwstr, PWideChar(pFileText), FileLen);
        tempwstr[FileLen div SizeOf(WideChar)] := #$0000;
        // file is not ended dy #0000
        tempstr := WidetoUTF8(ChangeUnicode(tempwstr), tempstr);
        RegExpr.Subject := tempstr;
        FreeMem(tempstr);
        FreeMem(tempwstr);
      end;
  end; // case
  if (WorkPattern.UseHeader and 1) = 0 then
  // If the information on a direction is not present that we will transform a line
  begin
    tempstr := ANSIToUTF8(PAnsiChar(WorkPattern.Msg.Incoming), tempstr, cp);
    WorkPattern.Msg.Incoming := tempstr;
    FreeMem(tempstr);
    tempstr := ANSIToUTF8(PAnsiChar(WorkPattern.Msg.Outgoing), tempstr, cp);
    WorkPattern.Msg.Outgoing := tempstr;
    FreeMem(tempstr);
  end;
  if (WorkPattern.UseHeader > 0) then
  begin
    tempstr := ANSIToUTF8(PAnsiChar(WorkPattern.Header.Pattern), tempstr, cp);
    RegExpr.RegEx := tempstr;
    RegExpr.Options := [preMultiLine, preUTF8];
    FreeMem(tempstr);
    if not RegExpr.Match then
    begin
      s := TranslateWideString('Header not found');
      DoMessage(ITXT_THREAD_ERROR, wparam(PWideChar(s)), 0);
      exit;
    end
    else
    begin
      if (WorkPattern.UseHeader and 1) = 1 then
      begin
        WorkPattern.Msg.Incoming := RegExpr.SubExpressions[WorkPattern.Header.Incoming];
        WorkPattern.Msg.Outgoing := RegExpr.SubExpressions[WorkPattern.Header.Outgoing];
      end;
      if (WorkPattern.UseHeader and 2) = 2 then
        if (DContact.hContact = 0) or
          (DContact.hContact = INVALID_HANDLE_VALUE) then
        begin
          if WorkPattern.Header.InUID <> 0 then
            DContact.ContactUID := RegExpr.SubExpressions[WorkPattern.Header.InUID]
          else
            DContact.ContactUID := '';
          if WorkPattern.Header.InNick <> 0 then
            DContact.ContactNick := RegExpr.SubExpressions[WorkPattern.Header.InNick]
          else
            DContact.ContactNick := '';
          TryDetermContact(DContact);
        end;
    end;
  end;
  // Whether if it has not turned out to define in header then we look it was defined in a file
  if (DContact.hContact = 0) or (DContact.hContact = INVALID_HANDLE_VALUE) then
    if (fContact.hContact <> 0) and (fContact.hContact <> INVALID_HANDLE_VALUE) then
      DContact := fContact;
  if (DContact.hContact <> 0) and (DContact.hContact <> INVALID_HANDLE_VALUE) then
  begin
    DoMessage(ITXT_THREAD_DEST_CONTACT, DContact.hContact, 0);
    DoMessage(ITXT_THREAD_START, 0, 0);
    tempstr := ANSIToUTF8(PAnsiChar(WorkPattern.Msg.Pattern), tempstr, cp);
    RegExpr.RegEx := tempstr;
    RegExpr.Options := [preMultiLine, preUTF8];
    FreeMem(tempstr);

    TextLength := Length(RegExpr.Subject) - 1; // Position of last symbol
    DoMessage(ITXT_THREAD_MAXPROGRESS, 0, TextLength);
    RegExpr.State := [preNotEmpty];
    // search for regular expression
    if not RegExpr.Match then
    begin
      s := TranslateWideString('No messages in this file');
      DoMessage(ITXT_THREAD_ERROR, wparam(PWideChar(s)), 0);
    end
    else
    begin
      PosCur := RegExpr.MatchedExpressionOffset;
      // get the position of RegExpression
      repeat
        LenCur := RegExpr.MatchedExpressionLength;
        // get the length of RegExpression
        // Further we define a message direction (incoming or outgoing)
        if RegExpr.SubExpressions[WorkPattern.Msg.Direction] = WorkPattern.Msg.Incoming then
          msg_flag := DBEF_READ
        else
          msg_flag := DBEF_READ or DBEF_SENT;
        msg_flag := msg_flag or DBEF_UTF;
        // make timestamp
        if WorkPattern.Msg.Seconds <> 0 then
          DT := timestamp
           (StrToInt(RegExpr.SubExpressions[WorkPattern.Msg.Year]),
            StrToInt(RegExpr.SubExpressions[WorkPattern.Msg.Month]),
            StrToInt(RegExpr.SubExpressions[WorkPattern.Msg.Day]),
            StrToInt(RegExpr.SubExpressions[WorkPattern.Msg.Hours]),
            StrToInt(RegExpr.SubExpressions[WorkPattern.Msg.Minutes]),
            StrToInt(RegExpr.SubExpressions[WorkPattern.Msg.Seconds]))
        else
          DT := timestamp
            (StrToInt(RegExpr.SubExpressions[WorkPattern.Msg.Year]),
            StrToInt(RegExpr.SubExpressions[WorkPattern.Msg.Month]),
            StrToInt(RegExpr.SubExpressions[WorkPattern.Msg.Day]),
            StrToInt(RegExpr.SubExpressions[WorkPattern.Msg.Hours]),
            StrToInt(RegExpr.SubExpressions[WorkPattern.Msg.Minutes]), 0);

        if RegExpr.MatchAgain then
          PosNext := RegExpr.MatchedExpressionOffset
          // search for next regexpr
        else
          PosNext := TextLength; // if not then end of file
        h1 := PosCur + LenCur; // The message text beginning presumably
        h2 := PosNext - PosCur - LenCur - 2; // its presumably message length
        // working with message text
        if WorkPattern.UsePreMsg then
          PRN := DWORD(WorkPattern.PreMsg.PreRN)
        else
          PRN := DWORD(-1);
        if PRN <> 0 then
        begin
          j := 1;
          while ((RegExpr.Subject[h1] = Char($0D)) and
            (RegExpr.Subject[h1 + 1] = Char($0A))) and (j <= PRN) do
          begin
            inc(h1, 2);
            Dec(h2, 2);
            inc(j);
          end; // remove carriage return in the beginning
        end;
        if WorkPattern.UsePreMsg then
          ARN := DWORD(WorkPattern.PreMsg.AfterRN)
        else
          ARN := DWORD(-1);
        if ARN <> 0 then
        begin
          j := 1;
          while ((RegExpr.Subject[h1 + h2] = Char($0D)) and
            (RegExpr.Subject[h1 + h2 + 1] = Char($0A))) and (j <= ARN) do
          begin
            Dec(h2, 2);
            inc(j)
          end; // remove carriage return in the end
        end;
        // get the message text
        TxtMsg := Copy(RegExpr.Subject, h1, h2 + 2);
        // remove spaces if needs
        if WorkPattern.UsePreMsg and
          ((WorkPattern.PreMsg.PreSP <> 0) or
          (WorkPattern.PreMsg.AfterSP <> 0)) then
          PreMessageSP(TxtMsg, UTF8Len(PAnsiChar(RegExpr.MatchedExpression)));
        AddMsgToDB(DContact.hContact, msg_flag, DT, TxtMsg, AddedMessages,
          Duplicates); // adding in base
        PosCur := PosNext;
        DoMessage(ITXT_THREAD_PROGRESS, PosCur, 0);
      until (PosNext = TextLength) or Terminated;
    end; // RegExpr.Exec
  end
  else
  begin
    s := TranslateWideString('Can''t determine destination contact');
    DoMessage(ITXT_THREAD_ERROR, wparam(PWideChar(s)), 0);
  end;
end;

procedure TImportThrd.BinImportProcedure;
var
  i: integer;
  s: WideString;
  tempstr: PAnsiChar;
var
  dbei: TDBEVENTINFO;
  proto: AnsiString;
  pt: int_ptr;
  fsz: cardinal;
{$DEFINE BIN_IMPORT_}
{$I BmContactIP.inc}
{$I BqhfIP.inc}
{$I BICQ6IP.inc}
{$I BICQ5IP.inc}
{$I BRMSIP.inc}
{$I BbayanIP.inc}
{$UNDEF BIN_IMPORT_}
begin
  AddedMessages := 0;
  Duplicates := 0;
  case WorkPattern.BinProc of
    1: // mContactImport
  {$I BmContactIP.inc}
    2: // QHF
  {$I BqhfIP.inc}
    3: // ICQ6
  {$I BICQ6IP.inc}
    4: // ICQ5
  {$I BICQ5IP.inc}
    5: // Nokia midp-rms
  {$I BRMSIP.inc}
    6: // BayanICQ
  {$I BbayanIP.inc}
  end;
end;

procedure TImportThrd.Execute;
var
  i: integer;
  s1: WideString;
  tempstr: PAnsiChar;
begin
  DoMessage(ITXT_THREAD_ALLSTARTED, 0, 0);
  FolderName := Copy(FileNames, 1, OffsetFileName - 1);
  i := OffsetFileName;
  while (FileNames[i + 1] <> #0) and not Terminated do
  begin // начало цикла по файлам
    s1 := '';
    inc(i);
    while FileNames[i] <> #0 do
    begin
      s1 := s1 + FileNames[i];
      inc(i);
    end;
    if (s1 <> '') and (s1 <> #0) then
    begin // Начинаем работать с файлом
      FileName := FolderName + '\' + s1;
      DoMessage(ITXT_THREAD_START_FILE, wparam(PWideChar(FileName)),0, smSend);
      pFileText := nil;
      hFile := INVALID_HANDLE_VALUE;
      DContact.ProtoName := Destination.ProtoName;
      fContact.hContact := INVALID_HANDLE_VALUE;
      RegExpr := TPerlRegEx.Create;
      // Создаём объект для работы с рег. выражениями
      try
        // Поработаем с именем файла
        if WorkPattern.UseFileName then
          if (DContact.hContact = 0) or
            (DContact.hContact = INVALID_HANDLE_VALUE) then
          begin
            tempstr := WidetoUTF8(PWideChar(FileName), tempstr);
            RegExpr.Subject := tempstr;
            FreeMem(tempstr);
            tempstr := ANSIToUTF8(PAnsiChar(WorkPattern.FName.Pattern),tempstr, cp);
            RegExpr.RegEx := tempstr;
            FreeMem(tempstr);
            RegExpr.Options := [preUTF8];

            if RegExpr.Match then
            begin
              fContact.ProtoName := Destination.ProtoName;
              if WorkPattern.FName.InUID <> 0 then
                fContact.ContactUID := RegExpr.SubExpressions[WorkPattern.FName.InUID]
              else
                fContact.ContactUID := '';
              if WorkPattern.FName.InNick <> 0 then
                fContact.ContactNick := RegExpr.SubExpressions[WorkPattern.FName.InNick]
              else
                fContact.ContactNick := '';
              TryDetermContact(fContact);
            end;
          end;

        // Загружаем сам файл
        // [preMultiLine] модификатор для восприятия многострочного текста
        if DoMapFile then // Загружаем файл
        begin
          CallService(MS_DB_SETSAFETYMODE, wParam(FALSE), 0);
          case WorkPattern.IType of
            1: TextImportProcedure;
            2: BinImportProcedure;
          end; // case
        end; // DoMapFile
      finally
        CallService(MS_DB_SETSAFETYMODE, wParam(true), 0);
        DoMessage(ITXT_THREAD_FINISH, AddedMessages, Duplicates);
        DoUnMapFile;
        RegExpr.Free;
      end;
      DContact.hContact := INVALID_HANDLE_VALUE;
      Sleep(10); // чтобы все сообщения дошли до окна
    end; // закончили работать с файлом
  end; // цикла по файлам
  DoMessage(ITXT_THREAD_ALLFINISHED, 0, 0);
end;

end.
