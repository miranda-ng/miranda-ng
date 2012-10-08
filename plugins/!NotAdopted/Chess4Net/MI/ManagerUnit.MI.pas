////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit ManagerUnit.MI;

interface

uses
  SysUtils,
  //
  ControlUnit, ManagerUnit, ConnectorUnit, ModalForm, NonRefInterfacedObjectUnit;

type
  TManagerMIFactory = class(TNonRefInterfacedObject, IMirandaPlugin)
  private
    m_Connector: TConnector;
    m_Manager: IMirandaPlugin;
    m_Dialogs: TDialogs;
    m_bOwnExceptionHandler: boolean;
    FErrorDuringPluginStart: TProcedure;
    procedure FDialogsHandler(modSender: TModalForm; msgDlgID: TModalFormID);
    function FGetDialogs: TDialogs;
    procedure FStartGaming;
    procedure FStartTransmitting; overload;
    procedure FStartTransmitting(ManagerForTransmition: TManager); overload;
    function FCanStartTransmitting: boolean;
    procedure FHandleStartException;
    property Dialogs: TDialogs read FGetDialogs;
  protected
    procedure Start;
    procedure Stop;
    procedure ConnectorHandler(ce: TConnectorEvent; d1: pointer = nil; d2: pointer = nil);
  public
    constructor Create(Connector: TConnector; AErrorDuringPluginStart: TProcedure); reintroduce;
    destructor Destroy; override;
  end;

procedure StopAllPlugins;

implementation

uses
  Types, StrUtils, Classes, Dialogs, Controls,
  //
  LocalizerUnit, TransmitGameSelectionUnit, GlobalsLocalUnit, ChessBoardUnit,
  GameChessBoardUnit;

type
  TManagerMI = class(TManager, IMirandaPlugin) // abstract
  protected
    procedure Start;
    procedure Stop;
    procedure RSendData(const cmd: string); override;
    procedure ROnDestroy; override;

    procedure ConnectorHandler(e: TConnectorEvent; d1: pointer = nil; d2: pointer = nil); override;
    procedure RSetOpponentClientVersion(lwVersion: LongWord); override;

  public
    constructor Create(Connector: TConnector); reintroduce;
    destructor Destroy; override;
  end;

  TTransmittingManagerMI = class;

  EGamingManagerMI = class(Exception);
  TGamingManagerMI = class(TManagerMI, IMirandaPlugin)
  private
    m_lstTransmittingManagers: TList;

    procedure FAddTransmitter(ATransmitter: TTransmittingManagerMI);
    function FRemoveTransmitter(ATransmitter: TTransmittingManagerMI): boolean;
    procedure FSetGameContextToTransmitter(ATransmitter: TTransmittingManagerMI);
    function FContainsContactIDInTransmitters(iContactID: integer): boolean;
    procedure FUpdateChessBoardCaptions;

  protected
    procedure Start;
    procedure ROnCreate; override;
    procedure ROnDestroy; override;
    procedure ConnectorHandler(e: TConnectorEvent; d1: pointer = nil; d2: pointer = nil); override;
    procedure RSetConnectionOccured; override;
    procedure RHandleConnectorDataCommand(sl: string); override;
    procedure RRetransmit(const strCmd: string); override;
    function RGetGameName: string; override;
  end;

  ETransmittingManagerMI = class(Exception);
  TTransmittingManagerMI = class(TManagerMI, IMirandaPlugin) // TODO: move to separate hierarchy
  private
    m_GamingManager: TGamingManagerMI;
    m_bReady: boolean; // ready for transmition
    property Ready: boolean read m_bReady;
  protected
    procedure Start;
    procedure ROnCreate; override;
    procedure ROnDestroy; override;

    procedure ConnectorHandler(e: TConnectorEvent; d1: pointer = nil; d2: pointer = nil); override;
    procedure RHandleConnectorDataCommand(sl: string); override;
//    procedure RWriteSettings; override; // no need because m_bConnectionOccured is always FALSE

  public
    constructor Create(Connector: TConnector; GamingManager: TGamingManagerMI); reintroduce;
  end;

var
  g_lstGamingManagers: TList = nil;
  g_Plugins: TInterfaceList = nil;

procedure AddToPlugins(Plugin: IMirandaPlugin);
begin
  if (not Assigned(g_Plugins)) then
    g_Plugins := TInterfaceList.Create;
  g_Plugins.Add(Plugin);
end;

procedure RemoveFromPlugins(Plugin: IMirandaPlugin);
begin
  if (not Assigned(g_Plugins)) then
    exit;
  g_Plugins.Remove(Plugin);
  if (g_Plugins.Count = 0) then
    FreeAndNil(g_Plugins);
end;


procedure StopAllPlugins;
var
  i: integer;
  Plugin: IMirandaPlugin;
begin
  if (not Assigned(g_Plugins)) then
    exit;
  i := g_Plugins.Count - 1;
  while ((i >= 0) and Assigned(g_Plugins)) do
  begin
    Plugin := g_Plugins[i] as IMirandaPlugin;
    Plugin.Stop;
    Pointer(Plugin) := nil;
    dec(i);
  end;
  FreeAndNil(g_Plugins);
end;

////////////////////////////////////////////////////////////////////////////////
// TGamingManagerMI

procedure TGamingManagerMI.FAddTransmitter(ATransmitter: TTransmittingManagerMI);
begin
  if (not Assigned(m_lstTransmittingManagers)) then
    m_lstTransmittingManagers := TList.Create;
  m_lstTransmittingManagers.Add(ATransmitter);
end;


function TGamingManagerMI.FRemoveTransmitter(ATransmitter: TTransmittingManagerMI): boolean;
var
  i: integer;
begin
  Result := FALSE;

  if (not Assigned(m_lstTransmittingManagers)) then
    exit;

  for i := 0 to m_lstTransmittingManagers.Count - 1 do
  begin
    if (m_lstTransmittingManagers[i] = ATransmitter) then
    begin
      m_lstTransmittingManagers.Delete(i);
      if (m_lstTransmittingManagers.Count = 0) then
        FreeAndNil(m_lstTransmittingManagers);

      Result := TRUE;
      exit;
    end;
  end; // for
end;


procedure TGamingManagerMI.FSetGameContextToTransmitter(ATransmitter: TTransmittingManagerMI);
begin
  if (not (Assigned(ATransmitter) and ATransmitter.Ready)) then
    exit;

  ATransmitter.RSendData(CMD_NICK_ID + ' ' + PlayerNickId + ' ' + OpponentNickId + ' ' + OpponentNick);
  ATransmitter.RSendData(CMD_GAME_CONTEXT + ' ' + RGetGameContextStr);

  if (ChessBoard.Mode = mGame) then
    ATransmitter.RSendData(CMD_CONTINUE_GAME);
end;


function TGamingManagerMI.FContainsContactIDInTransmitters(iContactID: integer): boolean;
var
  i: integer;
  ATransmitter: TTransmittingManagerMI;
begin
  Result := FALSE;
  if (not Assigned(m_lstTransmittingManagers)) then
    exit;
  for i := 0 to m_lstTransmittingManagers.Count - 1 do
  begin
    ATransmitter := m_lstTransmittingManagers[i];
    if (Assigned(ATransmitter) and (ATransmitter.Connector.ContactID = iContactID)) then
    begin
      Result := TRUE;
      exit;
    end;
  end;
end;


procedure TGamingManagerMI.Start;
begin
  if (Assigned(ChessBoard)) then
  begin
    Show;
    exit;
  end;

  if (not Connector.Opened) then
  begin
    if (not Connector.Open(FALSE)) then
      raise EGamingManagerMI.Create('ERROR: Cannot open connector!');
  end;

  RCreateChessBoardAndDialogs;
  RSetChessBoardToView;

  RReadPrivateSettings;

  RShowConnectingForm;
end;


procedure TGamingManagerMI.ROnCreate;
begin
  RCreateAndPopulateExtBaseList;

  // Nicks initialization
  PlayerNick := Connector.OwnerNick;
  OpponentNick := Connector.ContactNick;
  OpponentId := IntToStr(Connector.ContactID);

  TLocalizer.Instance.AddSubscriber(self); // TODO: -> TManager.ROnCreate
  RLocalize;
end;


procedure TGamingManagerMI.ROnDestroy;

  procedure NRemoveFromGamings;
  var
    lstTmp: TList;
    i: integer;
    ATransmitter: TTransmittingManagerMI;
  begin
    if (Assigned(g_lstGamingManagers)) then
      g_lstGamingManagers.Remove(self);

    if (not Assigned(m_lstTransmittingManagers)) then
      exit;

    lstTmp := m_lstTransmittingManagers;
    m_lstTransmittingManagers := nil;

    for i := 0 to lstTmp.Count - 1 do
    begin
      ATransmitter := lstTmp[i];
      if (Assigned(ATransmitter)) then
      begin
        lstTmp[i] := nil;
        ATransmitter.Stop;
      end;
    end;

    lstTmp.Free;
  end;

begin // TGamingManagerMI.ROnDestroy
  NRemoveFromGamings;
  FUpdateChessBoardCaptions;
  inherited ROnDestroy;
end;


procedure TGamingManagerMI.ConnectorHandler(e: TConnectorEvent;
  d1: pointer = nil; d2: pointer = nil);
begin
  case e of
    ceError:
    begin
      Connector.Close;
    end;
  end; // case

  inherited ConnectorHandler(e, d1, d2);
end;


procedure TGamingManagerMI.RSetConnectionOccured;
var
  iIndex: integer;
begin
  inherited RSetConnectionOccured;

  if (not Assigned(g_lstGamingManagers)) then
    g_lstGamingManagers := TList.Create;

  iIndex := g_lstGamingManagers.IndexOf(self);
  if (iIndex < 0) then
    g_lstGamingManagers.Add(self);

  FUpdateChessBoardCaptions;
end;


procedure TGamingManagerMI.FUpdateChessBoardCaptions;
var
  i: integer;
  GM: TGamingManagerMI;
begin
  if (not Assigned(g_lstGamingManagers)) then
    exit;
  for i := 0 to g_lstGamingManagers.Count - 1 do
  begin
    GM := g_lstGamingManagers[i];
    if (Assigned(GM)) then
      GM.RUpdateChessBoardCaption;
  end;
end;


procedure TGamingManagerMI.RRetransmit(const strCmd: string);
var
  i: integer;
  ATransmitter: TTransmittingManagerMI;
begin
  if (Transmittable or (not Assigned(m_lstTransmittingManagers))) then
    exit;

  for i := 0 to m_lstTransmittingManagers.Count - 1 do
  begin
    ATransmitter := m_lstTransmittingManagers[i];
    if (Assigned(ATransmitter) and (ATransmitter.Ready)) then
      ATransmitter.RSendData(strCmd);
  end;
end;


function TGamingManagerMI.RGetGameName: string;
var
  i: integer;
  iIndex: integer;
  iWithSameConnectorCount: integer;
  GM: TGamingManagerMI;
begin
  Result := inherited RGetGameName;

  if (not Assigned(g_lstGamingManagers)) then
    exit;

  iWithSameConnectorCount := 0;
  iIndex := 0;

  for i := 0 to g_lstGamingManagers.Count - 1 do
  begin
    GM := g_lstGamingManagers[i];
    if (Assigned(GM) and (GM.Connector.ContactID = self.Connector.ContactID)) then
    begin
      inc(iWithSameConnectorCount);
      if (GM = self) then
        iIndex := iWithSameConnectorCount;
    end; // if
  end; // for

  if (iWithSameConnectorCount > 1) then
    Result := Result + ' (' + IntToStr(iIndex) + ')';
end;


procedure TGamingManagerMI.RHandleConnectorDataCommand(sl: string);
var
  strCmdSaved, sr: string;
begin
  strCmdSaved := sl;

  RSplitStr(sl, sl, sr);

  if (sl = CMD_TRANSMITTING) then
  begin
    Transmittable := TRUE;
    exit;
  end;

  if (not Transmittable) then
  begin
    inherited RHandleConnectorDataCommand(strCmdSaved);
    exit;
  end;

  if (sl = CMD_WELCOME) then
  begin
    if (Assigned(ChessBoard)) then
      ChessBoard.InitPosition;
    RSetConnectionOccured;
  end
  else if (sl = CMD_NICK_ID) then
  begin
    // sr ::= <PlayerNickId><OpponentNickId><OpponentNick>
    PlayerNick := OpponentNick; // change for transmittion

    RSplitStr(sr, sl, sr);
    PlayerNickId := sl;

    RSplitStr(sr, sl, sr);
    OpponentNickId := sl;

    RSplitStr(sr, sl, sr);
    OpponentNick := sl;

    ChessBoard.Caption := RGetGameName;
  end
  else if (sl = CMD_GAME_CONTEXT) then
  begin
    RSetGameContext(sr);
  end
  else
    inherited RHandleConnectorDataCommand(strCmdSaved);
end;

////////////////////////////////////////////////////////////////////////////////
// TManagerMIFactory

constructor TManagerMIFactory.Create(Connector: TConnector; AErrorDuringPluginStart: TProcedure);
begin
  inherited Create;
  m_Connector := Connector;
  FErrorDuringPluginStart := AErrorDuringPluginStart;
  AddToPlugins(self);
end;


destructor TManagerMIFactory.Destroy;
begin
  RemoveFromPlugins(self);
  m_Dialogs.Free;
  inherited;
end;


procedure TManagerMIFactory.Start;
begin
  if (FCanStartTransmitting) then
  begin
    Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(28), mtCustom,
      [mbYes, mbNo], mfTransmitting); // You are currently playing some games. Do you want to start broadcasting?
    m_bOwnExceptionHandler := TRUE;
    exit;
  end;

  FStartGaming;
end;


function TManagerMIFactory.FCanStartTransmitting: boolean;
var
  i: integer;
  GM: TGamingManagerMI;
begin
  Result := FALSE;

  if (not (Assigned(g_lstGamingManagers) and Assigned(m_Connector))) then
    exit;

  for i := 0 to g_lstGamingManagers.Count - 1 do
  begin
    GM := g_lstGamingManagers[i];
    Result := (Assigned(GM) and (not GM.Transmittable) and
               (m_Connector.ContactID <> GM.Connector.ContactID) and
               (not GM.FContainsContactIDInTransmitters(m_Connector.ContactID)));
    if (Result) then
      exit;
  end; // for
end;


procedure TManagerMIFactory.FStartGaming;
begin
  m_Manager := TGamingManagerMI.Create(m_Connector);
  m_Connector.SetPlugin(m_Manager);
  m_Connector := nil;

  try
    m_Manager.Start;
    Pointer(m_Manager) := nil;
  except
    if (m_bOwnExceptionHandler) then
      FHandleStartException
    else
      raise;
  end;

  Stop;
end;


procedure TManagerMIFactory.FHandleStartException;
begin
  if (Assigned(FErrorDuringPluginStart)) then
    FErrorDuringPluginStart;
end;


procedure TManagerMIFactory.FStartTransmitting(ManagerForTransmition: TManager);
begin
  m_Manager := TTransmittingManagerMI.Create(m_Connector, ManagerForTransmition as TGamingManagerMI);
  m_Connector.SetPlugin(m_Manager);
  m_Connector := nil;

  try
    m_Manager.Start;
    Pointer(m_Manager) := nil;
  except
    if (m_bOwnExceptionHandler) then
      FHandleStartException
    else
      raise;
  end;

  Stop;
end;


procedure TManagerMIFactory.FStartTransmitting;
var
  strlGames: TStringList;
  i: integer;
  GM: TGamingManagerMI;
  ATransmitGameSelectionForm: TTransmitGameSelectionForm;
begin
  if (not Assigned(g_lstGamingManagers)) then
  begin
    Stop; // Don't do anything
    exit;
  end;

  strlGames := TStringList.Create;
  try
    for i := 0 to g_lstGamingManagers.Count - 1 do
    begin
      GM := g_lstGamingManagers[i];
      if (Assigned(GM) and (not GM.Transmittable) and
          (m_Connector.ContactID <> GM.Connector.ContactID) and
          (not GM.FContainsContactIDInTransmitters(m_Connector.ContactID))) then
        strlGames.AddObject(GM.RGetGameName, GM);
    end;

    if (strlGames.Count > 1) then
    begin
      ATransmitGameSelectionForm :=
        Dialogs.CreateDialog(TTransmitGameSelectionForm) as TTransmitGameSelectionForm;
      ATransmitGameSelectionForm.SetGames(strlGames);
      ATransmitGameSelectionForm.Show;
    end
    else if (strlGames.Count = 1) then
    begin
      GM := strlGames.Objects[0] as TGamingManagerMI;
      FStartTransmitting(GM);
    end
    else // = 0
      Stop;

  finally
    strlGames.Free;
  end;
end;


procedure TManagerMIFactory.Stop;
begin
  if (Assigned(m_Connector)) then
  begin
    m_Connector.SetPlugin(nil); 
    m_Connector.Free;
  end;
  if (Assigned(m_Manager)) then
  begin
    m_Manager.Stop;
    Pointer(m_Manager) := nil;
  end;
  Free;
end;


procedure TManagerMIFactory.ConnectorHandler(ce: TConnectorEvent;
  d1: pointer = nil; d2: pointer = nil);
begin
end;


function TManagerMIFactory.FGetDialogs: TDialogs;
begin
  if (not Assigned(m_Dialogs)) then
    m_Dialogs := TDialogs.Create(nil, FDialogsHandler);
  Result := m_Dialogs;
end;


procedure TManagerMIFactory.FDialogsHandler(modSender: TModalForm; msgDlgID: TModalFormID);
var
  modRes: TModalResult;
  GM: TGamingManagerMI;
begin
  modRes := modSender.ModalResult;
  case msgDlgID of
    mfNone:
      ;
    mfTransmitting:
    begin
      if (modRes = mrYes) then
        FStartTransmitting
      else // mrNo
        FStartGaming;
    end;

    mfTransmitGame:
    begin
      if (modRes = mrOk) then
      begin
        with modSender as TTransmitGameSelectionForm do
        begin
          GM := GetSelected as TGamingManagerMI;
          FStartTransmitting(GM); 
        end;
      end
      else
        Stop;
    end;
  end; // case
end;

////////////////////////////////////////////////////////////////////////////////
// TTransmittingManagerMI

constructor TTransmittingManagerMI.Create(Connector: TConnector; GamingManager: TGamingManagerMI);
begin
  inherited Create(Connector);
  m_GamingManager := GamingManager;
end;

procedure TTransmittingManagerMI.Start;
begin
  if (not Connector.Opened) then
  begin
    if (not Connector.Open(FALSE)) then
      raise ETransmittingManagerMI.Create('ERROR: Cannot open connector!');
  end;
  m_GamingManager.FAddTransmitter(self);
end;


procedure TTransmittingManagerMI.ROnDestroy;

  procedure NRemoveFromTransmittings;
  var
    i: integer;
    GM: TGamingManagerMI;
  begin
    if (not Assigned(g_lstGamingManagers)) then
      exit;

    for i := 0 to g_lstGamingManagers.Count - 1 do
    begin
      GM := g_lstGamingManagers[i];
      if (Assigned(GM) and GM.FRemoveTransmitter(self)) then
        break;
    end;
  end;

begin // TTransmittingManagerMI.ROnDestroy
  NRemoveFromTransmittings;
  inherited ROnDestroy;
end;


procedure TTransmittingManagerMI.ROnCreate;
begin
//  PlayerNick := m_GamingManager.PlayerNick;
//  OpponentNick := m_GamingManager.OpponentNick;
//  OpponentId := m_GamingManager.OpponentId;
end;


procedure TTransmittingManagerMI.ConnectorHandler(e: TConnectorEvent; d1: pointer = nil; d2: pointer = nil);
begin
  case e of
    ceConnected:
    begin
      RSendData(CMD_VERSION + ' ' + IntToStr(CHESS4NET_VERSION));
      RSendData(CMD_TRANSMITTING);
    end;

    ceError, ceDisconnected:
    begin
      Connector.Close;
      Stop;
    end;

  else
    inherited ConnectorHandler(e, d1, d2);
  end; // case
end;


procedure TTransmittingManagerMI.RHandleConnectorDataCommand(sl: string);
var
  sr: string;
  lwOpponentClientVersion: Longword;
begin
  RSplitStr(sl, sl, sr);
  if (sl = CMD_VERSION) then
  begin
    RSplitStr(sr, sl, sr);
    lwOpponentClientVersion := StrToIntDef(sl, CHESS4NET_VERSION);

    if (lwOpponentClientVersion < 201000) then
    begin
      RSendData(CMD_GOODBYE);
      RReleaseWithConnectorGracefully;
    end;

    RSetOpponentClientVersion(lwOpponentClientVersion);
  end
  else if (sl = CMD_TRANSMITTING) then
  begin
    RSendData(CMD_GOODBYE); // TODO: some message or output to log
    RReleaseWithConnectorGracefully;
  end
  else if (sl = CMD_GOODBYE) then
  begin
    Stop;
  end
  else if (sl = CMD_WELCOME) then
  begin
    RSendData(CMD_WELCOME);
    m_bReady := TRUE;
    m_GamingManager.FSetGameContextToTransmitter(self);
  end;
end;


////////////////////////////////////////////////////////////////////////////////
// TManagerMI

constructor TManagerMI.Create(Connector: TConnector);
begin
  self.Connector := Connector;
  RCreate;
  AddToPlugins(self);
end;


destructor TManagerMI.Destroy;
begin
  RemoveFromPlugins(self);
  inherited;
end;


procedure TManagerMI.Start;
begin
  Assert(FALSE);
end;


procedure TManagerMI.Stop;
begin
  Release;
end;


procedure TManagerMI.RSendData(const cmd: string);
begin
  Connector.SendData(cmd);
end;


procedure TManagerMI.ROnDestroy;
begin
  if (Assigned(Connector)) then
  begin
    Connector.Free;
    Connector := nil;
  end;

  inherited ROnDestroy;
end;


procedure TManagerMI.ConnectorHandler(e: TConnectorEvent; d1: pointer = nil; d2: pointer = nil);
var
  iData: integer;
  strCmd: string;
  strLeft: string;
begin
  case e of
    ceData:
    begin
      Assert(High(TStringDynArray(d1)) >= 0);
      iData := 0;
      repeat
        strLeft := TStringDynArray(d1)[iData];
        inc(iData);
        strCmd := IfThen((iData <= High(TStringDynArray(d1))), '*');

        RHandleConnectorDataCommand(strLeft);
      until (strCmd = '');
    end; // ceData

  else
    inherited ConnectorHandler(e, d1, d2);
  end; // case
end;


procedure TManagerMI.RSetOpponentClientVersion(lwVersion: LongWord);
begin
  inherited RSetOpponentClientVersion(lwVersion);

  if (lwVersion >= 200901) then
    Connector.MultiSession := TRUE;
end;


initialization

finalization
  FreeAndNil(g_lstGamingManagers);

end.
