////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit ConnectorUnit;

interface

uses
  Classes, ExtCtrls,
  m_globaldefs;

type
  TConnectorEvent = (ceConnected, ceDisconnected, ceData, ceError);

  TConnectorHandler = procedure(ce: TConnectorEvent; d1: pointer = nil;
                                d2: pointer = nil) of object;

  IConnectorable = interface
    procedure ConnectorHandler(ce: TConnectorEvent; d1: pointer = nil; d2: pointer = nil);
  end;

  TConnector = class
  private
    _sendTimer, _sendSystemTimer: TTimer;

    _connected, _opened: boolean;
    _plugin: IConnectorable;
    _hContact, _hFilterMsg: THandle;
    _lstId, _contactLstId: integer;
    // отсылаемое сообщение
    _msg_sending, _unformated_msg_sending: string;
    _cntrMsgIn: integer;  // счётчик входящих сообщений
    _cntrMsgOut: integer; // счётчик исходящих сообщений
    m_iLastCntrMsgOutInFormatting: integer;
    _msg_buf: string; // буфер сообщений
    // системное сообщение
    _systemDataList: TStringList;
    m_lwId: Longword;
    m_LastSendTime: TDateTime;

{$IFDEF DEBUG_LOG}
    _logFile: Text;

    procedure InitLog;
    procedure WriteToLog(const s: string);
    procedure CloseLog;
{$ENDIF}

    procedure FsendTimerTimer(Sender: TObject);
    procedure FsendSystemTimerTimer(Sender: TObject);

    function FGetOwnerNick: string;
    function FGetContactNick: string;
    function FFilterMsg(msg: string): boolean;
    function FSendMessage(const vMessage: string): boolean;
//    function FNotifySender(const vMessage: string): boolean;
    procedure FNotifySender;
    procedure FSendSystemData(sd: string);
    function FDeformatMsg(var msg: string; out lstId, msgCntr: integer): boolean;
    // Formatting of outgoing messages
    function FFormatMsg(const msg: string): string;
    function FGetOwnerID: integer;
    function FGetMultiSession: boolean;
    procedure FSetMultiSession(bValue: boolean);
    procedure FPluginConnectorHandler(ce: TConnectorEvent;
      d1: pointer = nil; d2: pointer = nil);

  public
    constructor Create(hContact: THandle); reintroduce;
    destructor Destroy; override;

    procedure Close;
    function Open(bMultiSession: boolean = TRUE): boolean;
    function SendData(const d: string): boolean;
    procedure SetPlugin(plugin: IConnectorable);

    property Connected: boolean read _connected;
    property Opened: boolean read _opened;
    property OwnerID: integer read FGetOwnerID;
    property OwnerNick: string read FGetOwnerNick;
    property ContactID: integer read _hContact;
    property ContactNick: string read FGetContactNick;
    property MultiSession: boolean read FGetMultiSession write FSetMultiSession;
  end;

procedure InitConnectorGlobals(const invitationStr, promtHeadStr, dataSeparator: string; maxMsgSize: integer = 256);

implementation

{$J+} {$I-}

uses
  SysUtils, DateUtils, StrUtils, Types,
  m_api,
  ControlUnit;

type
  TConnectorList = class(TList)
  private
    _iterator: integer;
    _hContact: THandle;

    function FGetLastAddedConnector: TConnector;

  public
    procedure AddConnector(Connector: TConnector);
    procedure RemoveConnector(Connector: TConnector);
    function GetFirstConnector(hContact: THandle): TConnector;
    function GetNextConnector: TConnector;
    property LastAddedConnector: TConnector read FGetLastAddedConnector;
  end;

var
  g_connectorList: TConnectorList = nil;
  g_msgBufferSize: integer;
  g_bMultiSession: boolean;

  g_hNotifySender: THandle;

  // cntrMsgIn и cntrMsgOut были введены для преодоления бага с зависающими сообщениями

const
  MSG_INVITATION: string = '!&This is a plugin invitation message&!';
  // MSG_RESPOND: string = '!&This is a plugin respond message&!';
  // <сообщение> ::= PROMPT_HEAD [PROMPT_SEPARATOR <ид клиента>] PROMPT_SEPARATOR <номер сообщения> PROMPT_TAIL <сообщение>
  PROMPT_HEAD: string = 'Plgn';
  PROMPT_SEPARATOR = ':';
  PROMPT_TAIL = '>';

  DATA_SEPARATOR: string = '&&';

  CMD_CLOSE = 'ext';
  CMD_CONTACT_LIST_ID = 'lstid';

  MAX_MSG_TRYS = 3; // максимальное количество попыток пересыла после ошибки
  MAX_RESEND_TRYS = 9; // максимальное количество попыток пересыла в таймере
  MIN_TIME_BETWEEN_MSG = 30; // время между отправкой сообщений системе IM в мс

  OWNER_ID = 0;

(*
function TConnector.FSendMessage(const vMessage: string): boolean;
const
  LAST_SEND_TIME: TDateTime = 0.0;
var
  _now: TDateTime;
begin
  _now := Now;
  if (MilliSecondsBetween(_now, LAST_SEND_TIME) < MIN_TIME_BETWEEN_MSG) then
    Result := FALSE
  else
  begin
    LAST_SEND_TIME := _now;
    CallContactService(_hContact, PSS_MESSAGE, 0, LPARAM(PChar(vMessage)));
    Result := TRUE;
  end;
end;
*)

// PP: Let's hope Miranda IM can messages to different contacts at the same time

function TConnector.FSendMessage(const vMessage: string): boolean;
// const
//  LAST_SEND_TIME: TDateTime = 0.0;
var
  _now: TDateTime;
begin
  _now := Now;
  if (MilliSecondsBetween(_now, m_LastSendTime) < MIN_TIME_BETWEEN_MSG) then
    Result := FALSE
  else
  begin
    m_LastSendTime := _now;
    CallContactService(_hContact, PSS_MESSAGE, 0, LPARAM(PChar(vMessage)));
    Result := TRUE;
  end;
end;


function NotifySender(wParam: WPARAM; lParam_: LPARAM): int; cdecl;
const
  MSG_TRYS: integer = 1;
var
  connector: TConnector;
  hContact: THandle;
begin
  Result := 0;
  hContact := PACKDATA(lParam_).hContact;

  if (PACKDATA(lParam_).type_ <> ACKTYPE_MESSAGE) then
    exit;

  case PACKDATA(lParam_)^.result_ of
    ACKRESULT_SUCCESS:
    begin
      MSG_TRYS := 1;

      connector := g_connectorList.GetFirstConnector(hContact);
      while Assigned(connector) do
      begin
        if (connector._msg_sending <> '') then
          connector.FNotifySender;
        connector := g_connectorList.GetNextConnector;
     end;
    end;

    ACKRESULT_FAILED:
    begin
      inc(MSG_TRYS);
      if (MSG_TRYS <= MAX_MSG_TRYS) then
        begin
          connector := g_connectorList.GetFirstConnector(hContact);
          while (Assigned(connector)) do
          begin
            if connector._msg_sending <> '' then
              with connector do
              begin
                _msg_buf := _unformated_msg_sending + _msg_buf;
                _sendTimer.Enabled := TRUE;
              end;
            connector := g_connectorList.GetNextConnector;
          end; // while
        end
      else
      begin
        connector := g_connectorList.GetFirstConnector(hContact);
        while (Assigned(connector)) do
        begin
          if (connector._msg_sending <> '') then
          begin
            connector.FPluginConnectorHandler(ceError);
          end;
          connector := g_connectorList.GetNextConnector;
        end;
      end; // if (MSG_TRYS <= MAX_MSG_TRYS)
    end; // ACKRESULT_FAILED
  end; // case PACKDATA
end;


procedure TConnector.FNotifySender;
begin
{$IFDEF DEBUG_LOG}
  WriteToLog('<< ' + _msg_sending);
{$ENDIF}
  if (Connected and (_msg_sending <> MSG_INVITATION)) then
  begin
    _unformated_msg_sending := '';
    inc(_cntrMsgOut);
    if (_cntrMsgOut > m_iLastCntrMsgOutInFormatting) then
      _cntrMsgOut := m_iLastCntrMsgOutInFormatting + 1;
  end;
  _msg_sending := '';
end;


// деформатирование входящих сообщений. TRUE - если декодирование удалось
function TConnector.FDeformatMsg(var msg: string; out lstId, msgCntr: integer): boolean;
var
  l: integer;
begin
  Result := FALSE;
  if LeftStr(msg, length(PROMPT_HEAD + PROMPT_SEPARATOR)) = (PROMPT_HEAD + PROMPT_SEPARATOR) then
  begin
    msg := RightStr(msg, length(msg) - length(PROMPT_HEAD + PROMPT_SEPARATOR));

    // contactListId
//      if (_contactLstId >= 0) then
    if (g_bMultiSession) then
    begin
      l := pos(PROMPT_SEPARATOR, msg);
      if (l > 0) then
      begin
        if (not TryStrToInt(LeftStr(msg, l - 1), lstId)) then
          exit;
        msg := RightStr(msg, length(msg) - l);
      end
      else
        lstId := g_connectorList.LastAddedConnector._lstId;
    end
    else
      lstId := -1; // no contactListId specified in message

    // Message counter
    l := pos(PROMPT_TAIL, msg);
    if ((l = 0) or (not TryStrToInt(LeftStr(msg, l - 1), msgCntr))) then
      exit;

    msg := RightStr(msg, length(msg) - l);
    // msg := AnsiReplaceStr(msg, '&amp;', '&');

    Result := TRUE;
  end;
end;


function TConnector.FFilterMsg(msg: string): boolean;

  procedure NProceedData(msg: string);

    function NProceedSystemCommand(msg: string): boolean;
    begin
      Result := TRUE;
      if (LeftStr(msg, length(CMD_CONTACT_LIST_ID)) = CMD_CONTACT_LIST_ID) then
      begin
        msg := RightStr(msg, length(msg) - length(CMD_CONTACT_LIST_ID) - 1);
        TryStrToInt(msg, _contactLstId);
      end
      else if (msg = CMD_CLOSE) then
      begin
        FPluginConnectorHandler(ceDisconnected);
        _connected := FALSE;
        _opened := FALSE;
      end
      else
        Result := FALSE;
    end;

  var
    n, l, i: integer;
    arrDatas: TStringDynArray;
    strCommand: string;
    bSystemCommand: boolean;
  begin { \NProceedData }
    if (RightStr(msg, length(DATA_SEPARATOR)) <> DATA_SEPARATOR) then
      msg := msg + DATA_SEPARATOR;

    n := -1;
    l := 1;
    repeat
      inc(n);
      l := PosEx(DATA_SEPARATOR, msg, l);
      inc(l, length(DATA_SEPARATOR));
    until (l = length(DATA_SEPARATOR));

    SetLength(arrDatas, n);

    bSystemCommand := TRUE;
    i := 0;
    while (i < n) do
    begin
      l := pos(DATA_SEPARATOR, msg);
      strCommand := LeftStr(msg, l - 1);

      if (bSystemCommand) then // System commands can go only in the beginning by definition
      begin
        bSystemCommand := NProceedSystemCommand(strCommand);
        if (bSystemCommand) then
        begin
          dec(n);
          SetLength(arrDatas, n);
          continue;
        end;
      end;

      arrDatas[i] := strCommand;
      msg := RightStr(msg, length(msg) - length(DATA_SEPARATOR) - l + 1);

      inc(i);
    end; { while }

    if (n > 0) then
    begin
      FPluginConnectorHandler(ceData, arrDatas);
    end;

    Finalize(arrDatas);
  end;

var
  lstId, cntrMsg: integer;
begin { TConnector.FFilterMsg }
{$IFDEF DEBUG_LOG}
  WriteToLog('>> ' + msg);
{$ENDIF}
  if (not Connected) then
  begin
 // if (msg = MSG_INVITATION) or (msg = MSG_RESPOND) then
    if (msg = MSG_INVITATION) then
    begin
   // if msg = MSG_INVITATION then
   //   FSendMessage(MSG_RESPOND);
      FSendSystemData(MSG_INVITATION);
      if (g_bMultiSession) then
        FSendSystemData(FFormatMsg(CMD_CONTACT_LIST_ID + ' ' + IntToStr(_lstId)));
      _connected := TRUE;
      FPluginConnectorHandler(ceConnected);
      Result := TRUE;
    end
    else
      Result := FALSE;
  end
  else // Connected
  begin
    if (FDeformatMsg(msg, lstId, cntrMsg) and ((not g_bMultisession) or (lstId = _lstId))) then
    begin
      Result := TRUE;

      if (cntrMsg > _cntrMsgIn) then
      begin
        inc(_cntrMsgIn);
        if (cntrMsg > _cntrMsgIn) then
        begin
          FPluginConnectorHandler(ceError); // пакет исчез
          exit;
        end;
      end
      else if (cntrMsg < _cntrMsgIn) then
        exit; // skipping packets with lower numbers

      // if (cntrMsg = _cntrMsgIn) there's no garantee that packets are synchronized, but let's hope it's so.

      NProceedData(msg);
    end
    else
      Result := FALSE;
  end;
end;


function TConnector.FFormatMsg(const msg: string): string;
var
  contactLstIdStr: string;
begin
  if (_contactLstId >= 0) then
    contactLstIdStr := PROMPT_SEPARATOR + IntToStr(_contactLstId)
  else // -1
    contactLstIdStr := '';
  Result := PROMPT_HEAD + contactLstIdStr + PROMPT_SEPARATOR + IntToStr(_cntrMsgOut) + PROMPT_TAIL + msg;
  m_iLastCntrMsgOutInFormatting := _cntrMsgOut;
end;


procedure TConnector.Close;
begin
  if (Connected) then
  begin
    FSendSystemData(FFormatMsg(CMD_CLOSE));

    _connected := FALSE;
    FPluginConnectorHandler(ceDisconnected);
  end;

  _sendTimer.Enabled := FALSE;
  _opened := FALSE;

{$IFDEF DEBUG_LOG}
  CloseLog;
{$ENDIF}
end;


function TConnector.Open(bMultiSession: boolean = TRUE): boolean;
var
  AConnector: TConnector;
begin
  Result := FALSE;

  if (not g_bMultiSession) then
    g_bMultiSession := bMultisession;

  if (Assigned(g_connectorList)) then
  begin
    AConnector := g_connectorList.GetFirstConnector(_hContact);
    while (Assigned(AConnector)) do
    begin
      if (AConnector.Opened and (AConnector._contactLstId < 0)) then
        exit;
      AConnector := g_connectorList.GetNextConnector;
    end;
  end;

  _cntrMsgIn := 0;
  _cntrMsgOut := 1;
  _msg_sending := '';
  _unformated_msg_sending := '';
  _msg_buf := '';
  _contactLstId := -1;
  _opened := TRUE;

  FSendSystemData(MSG_INVITATION);

  Result := TRUE;
end;


function TConnector.SendData(const d: string): boolean;
begin
  Result := FALSE;
  if (d = '') or
     (length(_msg_buf) + length(d) + length(DATA_SEPARATOR) > g_msgBufferSize) or
     (LeftStr(d, length(CMD_CLOSE)) = CMD_CLOSE) or
     (LeftStr(d, length(CMD_CONTACT_LIST_ID)) = CMD_CONTACT_LIST_ID) or
     (pos(DATA_SEPARATOR, d) > 0) then
  begin
    exit;
  end
  else
  begin
    _msg_buf := _msg_buf + d + DATA_SEPARATOR;
    _sendTimer.Enabled := TRUE; // Отослать сообщение с некоторой оттяжкой -> всё одним пакетом
  end;
  Result := TRUE;  
end;


function FilterMsg(wParam: WPARAM; lParam_: LPARAM): int; cdecl;
var
  msg: string;
  hContact: THandle;
  connector: TConnector;
  proceeded: boolean;
begin
  msg := string(PPROTORECVEVENT(PCCSDATA(lParam_).lParam).szMessage);
  hContact := PCCSDATA(lParam_).hContact;

  proceeded := FALSE;

  if Assigned(g_connectorList) then
  begin
    connector := g_connectorList.GetFirstConnector(hContact);
    while (Assigned(connector)) do
    begin
      if connector.Opened then
        proceeded := (connector.FFilterMsg(msg) or proceeded);
      connector := g_connectorList.GetNextConnector;
    end;
  end;

  if proceeded then
    Result := 0
  else
    Result := CallService(MS_PROTO_CHAINRECV, wParam, lParam_);
end;


constructor TConnector.Create(hContact: THandle);
const
  ID_COUNTER: Longword = 0;
var
  connector: TConnector;
begin
//  inherited Create;
  _sendTimer := TTimer.Create(nil);
  with _sendTimer do
  begin
    Enabled := FALSE;
    Interval := 100;
    OnTimer := FsendTimerTimer;
  end;

  _sendSystemTimer := TTimer.Create(nil);
  with _sendSystemTimer do
  begin
    Enabled := FALSE;
    Interval := 50;
    OnTimer := FsendSystemTimerTimer;
  end;

  _hContact := hContact;
  _systemDataList := TStringList.Create;

  if (not Assigned(g_connectorList)) then
    g_connectorList := TConnectorList.Create;

  connector := g_connectorList.GetFirstConnector(_hContact);
  if Assigned(connector) then
    _hFilterMsg := connector._hFilterMsg
  else
  begin
    _hFilterMsg := CreateProtoServiceFunction(PChar(PLUGIN_NAME), PSR_MESSAGE, FilterMsg);
    if CallService(MS_PROTO_ISPROTOONCONTACT, _hContact, LPARAM(PChar(PLUGIN_NAME))) = 0 then
      CallService(MS_PROTO_ADDTOCONTACT, _hContact, LPARAM(PChar(PLUGIN_NAME)));
  end;

  if (g_connectorList.Count = 0) then
    g_hNotifySender := HookEvent(ME_PROTO_ACK, NotifySender);

  inc(ID_COUNTER);
  m_lwId := ID_COUNTER;

  g_connectorList.AddConnector(self);

{$IFDEF DEBUG_LOG}
  InitLog;
{$ENDIF}
end;


destructor TConnector.Destroy;
begin
  if Connected then
    while (not FSendMessage(FFormatMsg(CMD_CLOSE))) do
      Sleep(1);

  _systemDataList.Free;

  g_connectorList.RemoveConnector(self);
  if (g_connectorList.Count = 0) then
    g_bMultiSession := FALSE;

  if (not Assigned(g_connectorList.GetFirstConnector(_hContact))) then
  begin
    if CallService(MS_PROTO_ISPROTOONCONTACT, _hContact, LPARAM(PChar(PLUGIN_NAME))) <> 0 then
      CallService(MS_PROTO_REMOVEFROMCONTACT, _hContact, LPARAM(PChar(PLUGIN_NAME)));
    PluginLink.DestroyServiceFunction(_hFilterMsg);
  end;

  if (g_connectorList.Count = 0) then
  begin
    if (g_hNotifySender <> 0) then
      UnhookEvent(g_hNotifySender);
    FreeAndNil(g_connectorList);
  end;

  _sendSystemTimer.Free;
  _sendTimer.Free;

{$IFDEF DEBUG_LOG}
  CloseLog;
{$ENDIF}

  inherited;
end;

{$IFDEF DEBUG_LOG}
procedure TConnector.InitLog;
begin
  AssignFile(_logFile, MirandaPluginPath + 'Chess4Net_CONNECTORLOG.txt');
  Append(_logFile);
  if IOResult <> 0 then
    begin
      Rewrite(_logFile);
      if IOResult <> 0 then
        begin
          AssignFile(_logFile, MirandaPluginPath + 'Chess4Net_CONNECTORLOG~.txt');
          Append(_logFile);
          if IOResult <> 0 then Rewrite(_logFile);
        end;
    end;

   WriteToLog('[' + DateTimeToStr(Now) + ']');
end;


procedure TConnector.WriteToLog(const s: string);
begin
  writeln(_logFile, s);
  Flush(_logFile);
end;


procedure TConnector.CloseLog;
begin
  CloseFile(_logFile);
end;
{$ENDIF}

procedure TConnector.FsendTimerTimer(Sender: TObject);
const
  RESEND_COUNT : integer = 0;
begin
  if (_systemDataList.Count > 0) then
    exit; // System data goes first

  if (_msg_sending = '') then
  begin
    _sendTimer.Enabled := FALSE;
    if (_msg_buf <> '') then
    begin
      _unformated_msg_sending := _msg_buf;
      _msg_sending := FFormatMsg(_msg_buf);
      _msg_buf := '';

      _sendTimer.Enabled := (not FSendMessage(_msg_sending));
    end;
  end
  else
  begin
{$IFDEF DEBUG_LOG}
    WriteToLog('resend: ' + _msg_sending);
{$ENDIF}
    inc(RESEND_COUNT);
    if (RESEND_COUNT = MAX_RESEND_TRYS) then
    begin
      RESEND_COUNT := 0;
      FSendMessage(_msg_sending);
    end;
  end;
end;


procedure TConnector.SetPlugin(plugin: IConnectorable);
begin
  _plugin := plugin;
end;


function TConnector.FGetOwnerNick: string;
begin
  Result := PChar(CallService(MS_CLIST_GETCONTACTDISPLAYNAME, 0, 0));
end;


function TConnector.FGetContactNick: string;
begin
  Result := PChar(CallService(MS_CLIST_GETCONTACTDISPLAYNAME, _hContact, 0));
end;


procedure InitConnectorGlobals(const invitationStr, promtHeadStr, dataSeparator: string; maxMsgSize: integer = 256);
begin
  MSG_INVITATION := invitationStr;
  PROMPT_HEAD := promtHeadStr;
  DATA_SEPARATOR := dataSeparator;
  g_msgBufferSize := maxMsgSize;
end;

{---------------------------- TConnectorList ---------------------------------}

procedure TConnectorList.AddConnector(Connector: TConnector);
var
  i: integer;
begin
  for i := 0 to Count - 1 do
  begin
    if (not Assigned(Items[i])) then
    begin
      Connector._lstId := i;
      Items[i] := Connector;
      exit;
    end;
  end; // for
  Add(Connector);
  Connector._lstId := Count - 1;
end;


procedure TConnectorList.RemoveConnector(Connector: TConnector);
begin
  Items[Connector._lstId] := nil;
  while ((Count > 0) and (not Assigned(Items[Count - 1]))) do
    Delete(Count - 1);
end;


function TConnectorList.GetFirstConnector(hContact: THandle): TConnector;
begin
  _hContact := hContact;

  _iterator := -1;
  Result := GetNextConnector;
end;


function TConnectorList.GetNextConnector: TConnector;
begin
  Result := nil;

  while (_iterator < (Count - 1)) do
  begin
    inc(_iterator);
    if (Assigned(Items[_iterator]) and
       (_hContact = TConnector(Items[_iterator])._hContact)) then
    begin
      Result := Items[_iterator];
      exit;
    end;
  end;
end;


function TConnectorList.FGetLastAddedConnector: TConnector;
var
  i: integer;
begin
  Result := nil;
  for i := 0 to Count - 1 do
  begin
    if ((not Assigned(Result)) or (TConnector(Items[i]).m_lwId > Result.m_lwId)) then
    Result := Items[i];
  end;
end;


procedure TConnector.FSendSystemData(sd: string);
begin
  if ((sd <> MSG_INVITATION) and (sd <> CMD_CLOSE)) then
    sd := sd + DATA_SEPARATOR;
  _systemDataList.Add(sd);
  _sendSystemTimer.Enabled := TRUE;
end;


procedure TConnector.FsendSystemTimerTimer(Sender: TObject);
begin
  if _systemDataList.Count = 0 then
    begin
      _sendSystemTimer.Enabled := FALSE;
      exit;
    end;

  _msg_sending := _systemDataList[0];
  if FSendMessage(_msg_sending) then
    _systemDataList.Delete(0);
  // else: try to resend
end;


function TConnector.FGetOwnerID: integer;
begin
  Result := OWNER_ID;
end;


procedure TConnector.FSetMultiSession(bValue: boolean);
begin
  if ((not g_bMultiSession) and bValue) then
  begin
    FSendSystemData(FFormatMsg(CMD_CONTACT_LIST_ID + ' ' + IntToStr(_lstId)));
    g_bMultiSession := TRUE;
  end;
end;

procedure TConnector.FPluginConnectorHandler(ce: TConnectorEvent;
  d1: pointer = nil; d2: pointer = nil);
begin
  if (Assigned(_plugin)) then
    _plugin.ConnectorHandler(ce, d1, d2); 
end;


function TConnector.FGetMultiSession: boolean;
begin
  Result := g_bMultiSession;
end;

end.
