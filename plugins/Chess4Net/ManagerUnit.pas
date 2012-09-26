////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit ManagerUnit;

{$DEFINE GAME_LOG}

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms,
  Menus, TntMenus, ActnList, TntActnList, ExtCtrls,
{$IFDEF TRILLIAN}
  plugin,
{$ENDIF}
  // Chess4Net Units
  ChessBoardHeaderUnit, ChessRulesEngine, ChessBoardUnit,
  GameChessBoardUnit, ConnectorUnit, ConnectingUnit, GameOptionsUnit,
  ModalForm, DialogUnit, ContinueUnit, LocalizerUnit, URLVersionQueryUnit;

type
  TManager = class(TForm, ILocalizable)
    ActionList: TTntActionList;
    LookFeelOptionsAction: TTntAction;
    AboutAction: TTntAction;

    ConnectedPopupMenu: TTntPopupMenu;
    LookFeelOptionsConnected: TTntMenuItem;
    StartStandartGameConnected: TTntMenuItem;
    StartPPRandomGameConnected: TTntMenuItem;
    GameOptionsConnected: TTntMenuItem;
    ChangeColorConnected: TTntMenuItem;
    GamePopupMenu: TTntPopupMenu;
    AbortGame: TTntMenuItem;
    DrawGame: TTntMenuItem;
    ResignGame: TTntMenuItem;
    N4: TTntMenuItem;
    LookFeelOptionsGame: TTntMenuItem;
    TakebackGame: TTntMenuItem;
    GamePause: TTntMenuItem;
    N1: TTntMenuItem;
    AboutConnected: TTntMenuItem;
    N2: TTntMenuItem;
    AboutGame: TTntMenuItem;
    StartAdjournedGameConnected: TTntMenuItem;
    AdjournGame: TTntMenuItem;
    N5: TTntMenuItem;
    N6: TTntMenuItem;

    BroadcastAction: TTntAction;
    N3: TTntMenuItem;
    BroadcastConnected: TTntMenuItem;
    N7: TTntMenuItem;
    Broadcast: TTntMenuItem;
    
    ConnectorTimer: TTimer;

    procedure FormCreate(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure ActionListUpdate(Action: TBasicAction;
      var Handled: Boolean);
    procedure LookFeelOptionsActionExecute(Sender: TObject);
    procedure AbortGameClick(Sender: TObject);
    procedure DrawGameClick(Sender: TObject);
    procedure ResignGameClick(Sender: TObject);
    procedure ChangeColorConnectedClick(Sender: TObject);
    procedure GameOptionsConnectedClick(Sender: TObject);
    procedure StartStandartGameConnectedClick(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure ConnectorTimerTimer(Sender: TObject);
    procedure StartPPRandomGameConnectedClick(Sender: TObject);
    procedure TakebackGameClick(Sender: TObject);
    procedure GamePauseClick(Sender: TObject);
    procedure AboutActionExecute(Sender: TObject);
    procedure StartAdjournedGameConnectedClick(Sender: TObject);
    procedure AdjournGameClick(Sender: TObject);
    procedure GamePopupMenuPopup(Sender: TObject);
    procedure BroadcastActionExecute(Sender: TObject);

  private
    m_ConnectingForm: TConnectingForm;
    m_ContinueForm: TContinueForm;
    m_Connector: TConnector;
    m_ChessBoard: TGameChessBoard;
    m_Dialogs: TDialogs;

    m_ExtBaseList: TStringList;
    m_strExtBaseName: string;
{$IFDEF QIP}
    iProtoDllHandle: integer;
    wAccName: WideString;
    QIPConnectionError: boolean;
{$ENDIF}
{$IFDEF TRILLIAN}
    contactlistEntry: TTtkContactListEntry;
{$ENDIF}
{$IFDEF SKYPE}
    m_bDontShowCredits: boolean;
{$ENDIF}
    m_lwOpponentClientVersion: LongWord;
    // It's for ChessBoard
    you_unlimited, opponent_unlimited: boolean;
    you_time, opponent_time,
    you_inc, opponent_inc: word;
    you_takebacks, opponent_takebacks: boolean;
    can_pause_game, can_adjourn_game, move_done: boolean;

    m_strPlayerNick: string;
    m_strPlayerNickId: string;

    m_strOpponentNick: string;
    m_strOpponentId: string;
    m_strOverridedOpponentNickId: string;

    extra_exit: boolean;
    m_bConnectionOccured: boolean;

    m_bTransmittable: boolean;

    m_iDontShowLastVersion: integer;
    m_iQueriedDontShowLastVersion: integer;

{$IFDEF GAME_LOG}
    // for game log
    gameLog: string;
    procedure FInitGameLog;
    procedure FWriteToGameLog(const s: string);
    procedure FlushGameLog;
{$ENDIF}
    procedure ChessBoardHandler(e: TGameChessBoardEvent;
      d1: pointer = nil; d2: pointer = nil);
    procedure SetClock; overload;
    procedure SetClock(var sr: string); overload;

    procedure FPopulateExtBaseList;

    function FReadCommonSettings(setToOpponent: boolean): boolean;
    procedure FWritePrivateSettings;
    procedure FWriteCommonSettings;

    function ClockToStr: string;
    procedure ChangeColor;
    procedure PauseGame;
    procedure ContinueGame;
    procedure FAdjournGame;
    procedure FExitGameMode;

    procedure FBuildAdjournedStr;
    procedure FStartAdjournedGame;

    function FGetAdjournedStr: string;
    procedure FSetAdjournedStr(const strValue: string);

    function FGetPlayerColor: TFigureColor;
    procedure FSetPlayerColor(Value: TFigureColor);

    function FGetOpponentNickId: string;
{$IFDEF SKYPE}
    procedure FShowCredits;
{$ENDIF}

    procedure FSetTransmittable(bValue: boolean);

    procedure FOnURLQueryReady(Sender: TURLVersionQuery);

    property AdjournedStr: string read FGetAdjournedStr write FSetAdjournedStr;
    property _PlayerColor: TFigureColor read FGetPlayerColor write FSetPlayerColor;

  protected
    constructor RCreate;

    procedure ROnCreate; virtual; abstract;
    procedure ROnDestroy; virtual;

    procedure ConnectorHandler(e: TConnectorEvent;
      d1: pointer = nil; d2: pointer = nil); virtual;
    procedure RCreateChessBoardAndDialogs;
    procedure RCreateAndPopulateExtBaseList;
    procedure RSetChessBoardToView;

    procedure RReadPrivateSettings;
    procedure RShowConnectingForm;

    procedure ILocalizable.Localize = RLocalize;
    procedure RLocalize;

    class procedure RSplitStr(s: string; var strLeft: string; var strRight: string);
    procedure RHandleConnectorDataCommand(sl: string); virtual;
    procedure RSetOpponentClientVersion(lwVersion: LongWord); virtual;

    procedure RSendData(const cmd: string = ''); virtual; abstract;

    procedure RSetConnectionOccured; virtual;
    function RGetGameName: string; virtual;

    function RGetGameContextStr: string;
    procedure RSetGameContext(const strValue: string);

    procedure RReleaseWithConnectorGracefully;

    procedure RRetransmit(const strCmd: string); virtual;
    procedure RBroadcast; virtual;

    procedure RUpdateChessBoardCaption;

    procedure DialogFormHandler(modSender: TModalForm; msgDlgID: TModalFormID); virtual;

    property Connector: TConnector read m_Connector write m_Connector;
    property ChessBoard: TGameChessBoard read m_ChessBoard write m_ChessBoard;

    property PlayerNick: string read m_strPlayerNick write m_strPlayerNick;
    property PlayerNickId: string read m_strPlayerNickId write m_strPlayerNickId;
    property OpponentNick: string read m_strOpponentNick write m_strOpponentNick;
    property OpponentId: string read m_strOpponentId write m_strOpponentId;
    property OpponentNickId: string read FGetOpponentNickId write m_strOverridedOpponentNickId;

    property Transmittable: boolean read m_bTransmittable write FSetTransmittable;

    property pDialogs: TDialogs read m_Dialogs;

  public
{$IFDEF AND_RQ}
    class function Create: TManager; reintroduce;
{$ENDIF}
{$IFDEF QIP}
    class function Create(const accName: WideString; const protoDllHandle: integer): TManager; reintroduce;
{$ENDIF}
{$IFDEF TRILLIAN}
    class function Create(const vContactlistEntry: TTtkContactListEntry): TManager; reintroduce;
{$ENDIF}
  end;

const
  CMD_DELIMITER = '&&'; // CMD_DELIMITER has to be present in arguments

  CMD_VERSION = 'ver';
  CMD_WELCOME = 'wlcm'; // Accept of connection
  CMD_GOODBYE = 'gdb'; // Refusion of connection
  CMD_TRANSMITTING = 'trnsm';
  CMD_NICK_ID = 'nkid';
  CMD_CONTINUE_GAME = 'cont';
  CMD_GAME_CONTEXT = 'gmctxt';

implementation

{$R *.dfm}
{$J+}

uses
  // Chess4Net
  DateUtils, Math, StrUtils, Dialogs,
  //
  LookFeelOptionsUnit, GlobalsUnit, GlobalsLocalUnit, InfoUnit, ChessClockUnit,
  DontShowMessageDlgUnit, IniSettingsUnit, PosBaseChessBoardLayerUnit
{$IFDEF AND_RQ}
  , CallExec
{$ENDIF}
{$IFDEF QIP}
  , ControlUnit
{$ENDIF}
{$IFDEF SKYPE}
  , CreditsFormUnit
{$ENDIF}
  ;

const
  USR_BASE_NAME = 'Chess4Net';

  NO_CLOCK_TIME ='u u';

  HOUR_TIME_FORMAT = 'h:nn:ss';

  // Command shorthands for Connector
  CMD_ECHO = 'echo';
  CMD_START_GAME = 'strt';
  CMD_GAME_OPTIONS = 'gmopt'; // Doesn't exist from 2007.5
  CMD_CHANGE_COLOR = 'chclr';
//  CMD_NICK_ID = 'nkid';
  CMD_RESIGN = 'res';
  CMD_ABORT = 'abrt';
  CMD_ABORT_ACCEPTED = 'abrtacc';
  CMD_ABORT_DECLINED = 'abrtdec';
  CMD_DRAW = 'draw';
  CMD_DRAW_ACCEPTED = 'drawacc';
  CMD_DRAW_DECLINED = 'drawdec';
  CMD_FLAG = 'flg';
  CMD_FLAG_YES = 'flgyes';
  CMD_FLAG_NO = 'flgno';
  CMD_TAKEBACK = 'tkbk';
  CMD_TAKEBACK_YES = 'tkbkyes';
  CMD_TAKEBACK_NO = 'tkbkno';
  CMD_SWITCH_CLOCK = 'swclck';
  CMD_REPEAT_COMMAND = 'rptcmd';
  CMD_POSITION = 'pos';
//  CMD_VERSION = 'ver';
//  CMD_WELCOME = 'wlcm'; // Accept of connection
//  CMD_GOODBYE = 'gdb'; // Refusion of connection
  // существует с 2007.5
  CMD_NO_SETTINGS = 'noset'; // If global settings are absent then request from partner's client
  CMD_ALLOW_TAKEBACKS = 'alwtkb';
  CMD_SET_CLOCK = 'clck'; // Change of timing  
  CMD_SET_TRAINING = 'trnng'; // Setting training mode
  // Существует с 2007.6
  CMD_CAN_PAUSE_GAME = 'canpaus';
  CMD_PAUSE_GAME = 'paus';
  CMD_PAUSE_GAME_YES = 'pausyes';
  CMD_PAUSE_GAME_NO = 'pausno';
//  CMD_CONTINUE_GAME = 'cont';
  // Существует с 2008.1
  CMD_CAN_ADJOURN_GAME = 'canadj';
  CMD_SET_ADJOURNED = 'setadj'; // Setting of adj. position and timing
  CMD_ADJOURN_GAME = 'adj';
  CMD_ADJOURN_GAME_YES = 'adjyes';
  CMD_ADJOURN_GAME_NO = 'adjno';
  CMD_START_ADJOURNED_GAME = 'strtadj';

  // CMD_DELIMITER = '&&'; // CMD_DELIMITER has to be present in arguments
  // CMD_CLOSE = 'ext' - IS RESERVED

type
  TManagerDefault = class(TManager) // TODO: TRILLIAN, AND_RQ, QIP-> own classes
  protected
    procedure ROnCreate; override;
    procedure ROnDestroy; override;
    procedure RSendData(const cmd: string = ''); override;
  public
{$IFDEF AND_RQ}
    constructor Create; reintroduce;
{$ENDIF}
{$IFDEF QIP}
    constructor Create(const accName: WideString; const protoDllHandle: integer); reintroduce;
{$ENDIF}
{$IFDEF TRILLIAN}
    constructor Create(const vContactlistEntry: TTtkContactListEntry); reintroduce;
{$ENDIF}
  end;

////////////////////////////////////////////////////////////////////////////////
// TManager

procedure TManager.RCreateChessBoardAndDialogs;
begin
//  m_ChessBoard := TGameChessBoard.Create(self, ChessBoardHandler, Chess4NetPath + USR_BASE_NAME);
  m_ChessBoard := TGameChessBoard.Create(nil, ChessBoardHandler, Chess4NetGamesLogPath + USR_BASE_NAME);
  m_Dialogs := TDialogs.Create(ChessBoard, DialogFormHandler);
end;


procedure TManager.FormCreate(Sender: TObject);
begin
{$IFNDEF SKYPE}
  BroadcastAction.Visible := TRUE;
{$ENDIF}
  ROnCreate;
end;


procedure TManager.RShowConnectingForm;
begin
  m_ConnectingForm := (m_Dialogs.CreateDialog(TConnectingForm) as TConnectingForm);
  m_ConnectingForm.Show;
end;


procedure TManager.ChessBoardHandler(e: TGameChessBoardEvent;
                            d1: pointer = nil; d2: pointer = nil);
var
  s: string;
  wstrMsg1, wstrMsg2: WideString;
  strSwitchClockCmd: string;
begin
  case e of
    cbeKeyPressed:
      if extra_exit and (Word(d1) = VK_ESCAPE) then
        begin
{$IFDEF GAME_LOG}
          if (ChessBoard.Mode = mGame) then
          begin
            FWriteToGameLog('*');
            FlushGameLog;
          end;
{$ENDIF}
          Release;
        end;

    cbeExit:
      Close;

    cbeMenu:
      if (not m_Dialogs.Showing) then
      begin
        if ((ChessBoard.Mode = mView) or Transmittable) then
        begin
          if (Connector.connected) then
            ConnectedPopupMenu.Popup(Mouse.CursorPos.X, Mouse.CursorPos.Y);
        end
        else if (ChessBoard.Mode = mGame) then
        begin
          GamePopupMenu.Popup(Mouse.CursorPos.X, Mouse.CursorPos.Y);
        end;
      end;

    cbeMoved:
      begin
        if (not Transmittable) then
        begin
          RSendData(PString(d1)^);
          RRetransmit(PString(d1)^);
        end;
{$IFDEF GAME_LOG}
        if (ChessBoard.PositionColor = fcBlack) or (not move_done) then
        begin
          FWriteToGameLog(' ' + IntToStr(ChessBoard.NMoveDone) + '.');
          if (ChessBoard.PositionColor = fcWhite) then
            FWriteToGameLog(' ...');
        end;
        FWriteToGameLog(' ' + PString(d1)^);
{$ENDIF}
        move_done := TRUE;
        TakebackGame.Enabled := TRUE;
      end;

    cbeMate:
      with ChessBoard do
      begin
        FExitGameMode;
{$IFDEF GAME_LOG}
        FWriteToGameLog('#');
        if (PositionColor = fcWhite) then
          FWriteToGameLog(sLineBreak + '0 - 1')
        else
          FWriteToGameLog(sLineBreak + '1 - 0');
        FlushGameLog;
{$ENDIF}
        with TLocalizer.Instance do
        begin
          if (Transmittable) then
          begin
            if (PositionColor = fcWhite) then
              wstrMsg1 := GetMessage(36) // White is checkmated.
            else
              wstrMsg1 := GetMessage(37); // Black is checkmated.
            wstrMsg2 := wstrMsg1;
          end
          else // not Transmittable
          begin
            if (PositionColor = fcWhite) then
            begin
              wstrMsg1 := GetMessage(0); // White is checkmated. You win.
              wstrMsg2 := GetMessage(1); // White is checkmated. You loose.
            end
            else
            begin
              wstrMsg1 := GetMessage(2); // Black is checkmated. You win.
              wstrMsg2 := GetMessage(3); // Black is checkmated. You loose.
            end;
          end;
        end; // with

        if ((_PlayerColor <> fcWhite) and (PositionColor = fcWhite)) or
           ((_PlayerColor <> fcBlack) and (PositionColor = fcBlack)) then
        begin
          m_Dialogs.MessageDlg(wstrMsg1, mtCustom, [mbOK], mfNone);
          ChessBoard.WriteGameToBase(grWin);
        end
        else
        begin
          m_Dialogs.MessageDlg(wstrMsg2, mtCustom, [mbOK], mfNone);
          ChessBoard.WriteGameToBase(grLost);
        end;
      end;

    cbeStaleMate:
    begin
      FExitGameMode;
{$IFDEF GAME_LOG}
      FWriteToGameLog('=' + sLineBreak + '1/2 - 1/2');
      FlushGameLog;
{$ENDIF}
      if (Transmittable) then
        wstrMsg1 := TLocalizer.Instance.GetMessage(35) // Stalemate.
      else
        wstrMsg1 := TLocalizer.Instance.GetMessage(4); // It's stalemate. No one wins.
      m_Dialogs.MessageDlg(wstrMsg1, mtCustom, [mbOK], mfNone);
      ChessBoard.WriteGameToBase(grDraw);
    end;

    cbeClockSwitched:
    begin
      if (Transmittable) then
        exit;
      with ChessBoard do
      begin
        if (move_done and (ClockColor = PositionColor)) then
        begin
          if (ClockColor <> _PlayerColor) then
          begin
            Time[_PlayerColor] := IncSecond(Time[_PlayerColor], you_inc);
            s := TChessClock.ConvertToFullStr(Time[_PlayerColor]);

            if ((not Unlimited[_PlayerColor]) or (m_lwOpponentClientVersion < 200706)) then
            begin
              strSwitchClockCmd := CMD_SWITCH_CLOCK + ' ' + s;
              RSendData(strSwitchClockCmd);
              RRetransmit(strSwitchClockCmd);
            end;
          end
          else
          begin
            if (_PlayerColor = fcWhite) then
              Time[fcBlack] := IncSecond(Time[fcBlack], opponent_inc)
            else
              Time[fcWhite] := IncSecond(Time[fcWhite], opponent_inc);
          end;
        end;
      end; { with }
    end;

    cbeTimeOut:
    begin
      if (not Transmittable) then
        RSendData(CMD_FLAG);
    end;

    cbeActivate:
    begin
      m_Dialogs.BringToFront;
    end;

    cbeFormMoving:
    begin
      m_Dialogs.MoveForms(integer(d1), integer(d2));
    end;
  end;
end;


class procedure TManager.RSplitStr(s: string; var strLeft: string; var strRight: string);
var
  x: integer;
begin
  x := pos(' ', s);
  strLeft := copy(s, 1, sign(x) * (x - 1) + (1 - sign(x)) * length(s));
  strRight := copy(s, length(strLeft) + 2, length(s));
end;


procedure TManager.SetClock(var sr: string);
var
  sl: string;


  procedure NSetOpponentTime;
  begin
    RSplitStr(sr, sl, sr);
    if (sl = 'u') then
      opponent_unlimited := TRUE
    else
    begin
      opponent_unlimited:= FALSE;
      opponent_time:= StrToInt(sl);
      RSplitStr(sr, sl, sr);
      opponent_inc := StrToInt(sl);
    end;
  end;

  procedure NSetYouTime;
  begin
    RSplitStr(sr, sl, sr);
    if (sl = 'u') then
      you_unlimited:= TRUE
    else
    begin
      you_unlimited := FALSE;
      you_time := StrToInt(sl);
      RSplitStr(sr, sl, sr);
      you_inc := StrToInt(sl);
    end;
  end;

begin // TManager.SetClock
  if (Transmittable) then
  begin
    NSetYouTime;
    NSetOpponentTime;
  end
  else
  begin
    NSetOpponentTime;
    NSetYouTime;
  end;

  SetClock;
end;


procedure TManager.ConnectorHandler(e: TConnectorEvent; d1: pointer = nil; d2: pointer = nil);
var
  strCmd: string;
  strLeft: string;
begin
  case e of
    ceConnected:
    begin
      if (Assigned(m_ConnectingForm)) then
        m_ConnectingForm.Shut;
      RSendData(CMD_VERSION + ' ' + IntToStr(CHESS4NET_VERSION));
    end;

    ceDisconnected:
    begin
      if (not Connector.connected) then
        exit;

      if (Transmittable) then
      begin
        m_Dialogs.CloseNoneDialogs;
        m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(34), mtCustom,
          [mbOK], mfMsgLeave); // Broadcaster leaves. Transmition will be closed.
      end;

      case ChessBoard.Mode of
        mView:
        begin
          m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(5), mtCustom, [mbOK],
            mfMsgLeave); // 'Your opponent leaves.'
        end;
        mGame:
        begin
{$IFDEF GAME_LOG}
          FWriteToGameLog('*');
          FlushGameLog;
{$ENDIF}
          m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(6), mtWarning,
                             [mbOK], mfMsgLeave); // Your opponent leaves. The game is aborted.
        end;
      end;
    end; { ceDisconnected }

    ceError:
    begin
{$IFDEF GAME_LOG}
      if ChessBoard.Mode = mGame then
      begin
        FWriteToGameLog('*');
        FlushGameLog;
      end;
{$ENDIF}
      m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(7), mtWarning,
                         [mbOk], mfMsgLeave); // An error during connection occured.
    end;
{$IFDEF QIP}
    ceQIPError:
    begin
      QIPConnectionError := TRUE;
      // TODO: Localize
      m_Dialogs.MessageDlg('Special message channel is not responding.' + sLineBreak +
                         'This can happen due to the following reasons:' + sLineBreak +
                         '  1) Your partner is using an IM other than QIP Infium. OR' + sLineBreak +
                         '  2) Your partner is offline. OR' + sLineBreak +
                         '  3) Protocol doesn''t support multiple channels. OR' + sLineBreak +
                         '  4) Other reasons.' + sLineBreak +
                         'Chess4Net won''t start.', mtWarning, [mbOk], mfMsgLeave);
    end;
{$ENDIF}

    ceData:
    begin
      strCmd := PString(d1)^;
      repeat
        strLeft := LeftStr(strCmd, pos(CMD_DELIMITER, strCmd) - 1);
        strCmd := RightStr(strCmd, length(strCmd) - length(strLeft) - length(CMD_DELIMITER));

        RHandleConnectorDataCommand(strLeft);
      until (strCmd = '');

    end; {  ceData }

  end; { case ChessBoard.Mode }
end;


procedure TManager.RSetOpponentClientVersion(lwVersion: LongWord);
begin
  m_lwOpponentClientVersion := lwVersion;
end;


procedure TManager.RSetConnectionOccured;
begin
  m_bConnectionOccured := TRUE;
{$IFNDEF TESTING}
  with TURLVersionQuery.Create do
  begin
    OnQueryReady := FOnURLQueryReady;
  {$IFDEF SKYPE}
    Query(aidSkype, CHESS4NET_VERSION, osidWindows);
  {$ELSE}
    Free; // TODO: URL query for other clients
  {$ENDIF}
  end;
{$ENDIF}
end;


procedure TManager.FOnURLQueryReady(Sender: TURLVersionQuery);
begin
  if (not Assigned(Sender)) then
    exit;

  try
    if ((Sender.LastVersion <= m_iDontShowLastVersion)) then
      exit;

    if (Sender.Info <> '') then
    begin
      with TDontShowMessageDlg.Create(m_Dialogs, Sender.Info) do
      begin
        m_iQueriedDontShowLastVersion := Sender.LastVersion;
        Show;
      end;
    end;

  finally
    Sender.Free;
  end;

end;


procedure TManager.RUpdateChessBoardCaption;
begin
  if (m_bConnectionOccured and Assigned(ChessBoard)) then
    ChessBoard.Caption := RGetGameName;
end;


procedure TManager.RHandleConnectorDataCommand(sl: string);
var
  AMode: TMode;
  sr: string;
  strSavedCmd: string;
  wstrMsg: WideString;
begin
  strSavedCmd := sl;

  RSplitStr(sl, sl, sr);

  if (Assigned(ChessBoard)) then
    AMode := ChessBoard.Mode
  else
    AMode := mView;

  case AMode of
  mView:
    if (sl = CMD_VERSION) then
    begin
      RSplitStr(sr, sl, sr);
      RSetOpponentClientVersion(StrToIntDef(sl, CHESS4NET_VERSION));
      RSendData(CMD_WELCOME);
      if (m_lwOpponentClientVersion < CHESS4NET_VERSION) then
      begin
        m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(8), mtWarning,
          [mbOK], mfNone); // Your opponent is using an older version of Chess4Net. ...
      end;
      // 2007.4 is the first client with a backward compatibility
      // For incompatible versions:
      // else RSendData(CMD_GOODBYE);
    end
    else if (sl = CMD_WELCOME) then
    begin
      RSendData(CMD_NICK_ID + ' ' + OpponentNickId);
      if (Assigned(ChessBoard)) then
        ChessBoard.InitPosition;
      SetClock;
      RSetConnectionOccured;
    end
    else if (sl = CMD_GOODBYE) then // For the future versions
    begin
      m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(9) , mtWarning, [mbOK], mfIncompatible); // The current version of Chess4Net is incompatible ...
    end
    else if (sl = CMD_START_GAME) then
    begin
      with ChessBoard do
      begin
        if (Transmittable) then
          m_Dialogs.CloseNoneDialogs;
        // Starting from 2007.6 only white can start the game
        if ((m_lwOpponentClientVersion >= 200706) and (_PlayerColor = fcWhite) and
            (not Transmittable)) then
        begin
          ChangeColor;
        end;
        SetClock;
        ResetMoveList;
        move_done:= FALSE;
        TakebackGame.Enabled := FALSE;
        Mode := mGame;
        SwitchClock(PositionColor);
{$IFDEF GAME_LOG}
        FInitGameLog;
{$ENDIF}
      end;
      RRetransmit(strSavedCmd);
    end
    else if (sl = CMD_START_ADJOURNED_GAME) then
    begin
      FStartAdjournedGame;
      RRetransmit(CMD_GAME_CONTEXT + ' ' + RGetGameContextStr);
      RRetransmit(CMD_CONTINUE_GAME);
    end
    else if (sl = CMD_CONTINUE_GAME) then
    begin
      if (Transmittable) then
      begin
        m_Dialogs.CloseNoneDialogs;
        ChessBoard.Mode := mGame;
        ContinueGame;
      end;
    end
    else if (sl = CMD_ALLOW_TAKEBACKS) then
    begin
      RSplitStr(sr, sl, sr);
      opponent_takebacks := (sl = '1');
      TakebackGame.Visible := (opponent_takebacks or ChessBoard.pTrainingMode);
    end
    else if (sl = CMD_CAN_PAUSE_GAME) then
    begin
      RSplitStr(sr, sl, sr);
      can_pause_game := (sl = '1');
      GamePause.Visible := can_pause_game;
    end
    else if (sl = CMD_CAN_ADJOURN_GAME) then
    begin
      RSplitStr(sr, sl, sr);
      can_adjourn_game := (sl = '1');
    end
    else if (sl = CMD_SET_CLOCK) then
    begin
      SetClock(sr);
      RRetransmit(CMD_SET_CLOCK + ' ' + ClockToStr);
    end
    else if (sl = CMD_SET_TRAINING) then
    begin
      RSplitStr(sr, sl, sr);
      ChessBoard.pTrainingMode := (sl = '1');
      TakebackGame.Visible := (opponent_takebacks or ChessBoard.pTrainingMode);
    end
    else if (sl = CMD_GAME_OPTIONS) then // 2007.4
    begin
      SetClock(sr);
      RSplitStr(sr, sl, sr);
      opponent_takebacks := (sl = '1');
      RSplitStr(sr, sl, sr);
      ChessBoard.pTrainingMode := (sl = '1');
      TakebackGame.Visible := (opponent_takebacks or ChessBoard.pTrainingMode);
    end
    else if (sl = CMD_SET_ADJOURNED) then // 2008.1
    begin
      if ((AdjournedStr = '') or (CompareStr(PlayerNickId, OpponentNickId) > 0)) then
      begin
        if (pos('&w&', sr) > 0) then
          sr := StringReplace(sr, '&w&', '&b&', []) // White -> Black
        else
          sr := StringReplace(sr, '&b&', '&w&', []); // Black -> White
        AdjournedStr := sr;
      end;
    end
    else if (sl = CMD_CHANGE_COLOR) then
    begin
      ChangeColor;
      RRetransmit(strSavedCmd);
    end
    else if (sl = CMD_NICK_ID) then
    begin
      m_strPlayerNickId := sr;
      if (CompareStr(PlayerNickId, OpponentNickId) < 0) then
      begin
        StartStandartGameConnected.Enabled := TRUE;
        StartPPRandomGameConnected.Enabled := TRUE;
        _PlayerColor := fcWhite;
        if (not FReadCommonSettings(TRUE)) then
          RSendData(CMD_NO_SETTINGS);
      end
      else
      begin
        StartStandartGameConnected.Enabled := FALSE;
        StartPPRandomGameConnected.Enabled := FALSE;
        _PlayerColor := fcBlack;
        FReadCommonSettings(FALSE);
      end; // if CompareStr

      RUpdateChessBoardCaption;
    end
    else if (sl = CMD_POSITION) then
    begin
      if (Assigned(ChessBoard)) then
        ChessBoard.SetPosition(sr);
      RRetransmit(strSavedCmd);
    end
    else if (sl = CMD_NO_SETTINGS) then
    begin
      FReadCommonSettings(TRUE);
    end
    else if (sl = CMD_TRANSMITTING) then
    begin
      m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(33),
        mtCustom, [mbOK], mfMsgLeave); // Game transmition is not supported by this client!
    end;

  mGame:
    if (sl = CMD_DRAW) then
    begin
      m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(10), mtConfirmation,
        [mbYes, mbNo], mfMsgDraw) // Draw?
    end
    else if (sl = CMD_ABORT) then
    begin
      m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(11), mtConfirmation,
        [mbYes, mbNo], mfMsgAbort); // Can we abort the game?
    end
    else if (sl = CMD_RESIGN) then
    begin
      FExitGameMode;
{$IFDEF GAME_LOG}
      if (_PlayerColor = fcWhite) then
        FWriteToGameLog(sLineBreak + 'Black resigns' + sLineBreak + '1 - 0')
      else
        FWriteToGameLog(sLineBreak + 'White resigns' + sLineBreak + '0 - 1');
      FlushGameLog;
{$ENDIF}
      if (Transmittable) then
      begin
        RSplitStr(sr, sl, sr);
        if (sl = 'w') then
          wstrMsg := TLocalizer.Instance.GetMessage(31) // White resigns.
        else // (sl = 'b')
          wstrMsg := TLocalizer.Instance.GetMessage(32) // Black resigns.
      end
      else
        wstrMsg := TLocalizer.Instance.GetMessage(12); // I resign. You win this game. Congratulations!

      m_Dialogs.MessageDlg(wstrMsg, mtCustom, [mbOK], mfNone);
      ChessBoard.WriteGameToBase(grWin);

      RRetransmit(CMD_RESIGN + IfThen((_PlayerColor = fcWhite), ' b', ' w'));
    end
    else if (sl = CMD_ABORT_ACCEPTED) then
    begin
      FExitGameMode;
{$IFDEF GAME_LOG}
      FWriteToGameLog('*');
      FlushGameLog;
{$ENDIF}
      m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(13), mtCustom,
        [mbOK], mfNone); // The game is aborted.
      RRetransmit(strSavedCmd); 
    end
    else if (sl = CMD_ABORT_DECLINED) then
    begin
      m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(14),
        mtCustom, [mbOK], mfNone) // Sorry, but we have to finish this game.
    end
    else if (sl = CMD_DRAW_ACCEPTED) then
    begin
      FExitGameMode;
{$IFDEF GAME_LOG}
      FWriteToGameLog('=' + sLineBreak + '1/2 - 1/2');
      FlushGameLog;
{$ENDIF}
      m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(15), mtCustom, [mbOK], mfNone); // The game is drawn.
      ChessBoard.WriteGameToBase(grDraw);

      RRetransmit(strSavedCmd);      
    end
    else if (sl = CMD_DRAW_DECLINED) then
    begin
      m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(16), mtCustom, [mbOK], mfNone) // No draw, sorry.
    end
    else if (sl = CMD_SWITCH_CLOCK) then
    begin
      with ChessBoard do
      begin
        RSplitStr(sr, sl, sr);

        if (Transmittable) then
        begin
          if (PositionColor = fcWhite) then
            Time[fcBlack] := TChessClock.ConvertFromFullStr(sl)
          else
            Time[fcWhite] := TChessClock.ConvertFromFullStr(sl);
        end
        else
        begin
          if (_PlayerColor = fcWhite) then
            Time[fcBlack] := TChessClock.ConvertFromFullStr(sl)
          else
            Time[fcWhite] := TChessClock.ConvertFromFullStr(sl);
        end;
      end; // with
      RRetransmit(strSavedCmd);
    end
    else if (sl = CMD_FLAG) then
      with ChessBoard do
      begin
        if (Time[_PlayerColor] = 0.0) then
        begin
          RSendData(CMD_FLAG_YES);
          RRetransmit(CMD_FLAG_YES);

          FExitGameMode;
{$IFDEF GAME_LOG}
          if (_PlayerColor = fcWhite) then
            FWriteToGameLog(sLineBreak + 'White forfeits on time')
          else
            FWriteToGameLog(sLineBreak + 'Black forfeits on time');
          FlushGameLog;
{$ENDIF}
          m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(17), mtCustom, [mbOK], mfNone); // You forfeited on time.
          ChessBoard.WriteGameToBase(grLostTime);
        end
        else
          RSendData(CMD_FLAG_NO);
      end // with
    else
    if (sl = CMD_FLAG_YES) then
    begin
      FExitGameMode;
{$IFDEF GAME_LOG}
      if (_PlayerColor = fcWhite) then
        FWriteToGameLog(sLineBreak + 'Black forfeits on time')
      else
        FWriteToGameLog(sLineBreak + 'White forfeits on time');
      FlushGameLog;
{$ENDIF}
      if (Transmittable) then
      begin
        if (_PlayerColor = fcWhite) then
          wstrMsg := TLocalizer.Instance.GetMessage(29) // Black forfeits on time.
        else
          wstrMsg := TLocalizer.Instance.GetMessage(30); // White forfeits on time.
      end
      else
        wstrMsg := TLocalizer.Instance.GetMessage(18); // Your opponent forfeited on time.

      m_Dialogs.MessageDlg(wstrMsg, mtCustom, [mbOK], mfNone);
      ChessBoard.WriteGameToBase(grWinTime);

      RRetransmit(strSavedCmd);
    end
    else if (sl = CMD_FLAG_NO) then
      with ChessBoard do
      begin
        case _PlayerColor of
          fcWhite:
            if (Time[fcBlack] = 0.0) then
              RSendData(CMD_FLAG);
          fcBlack:
            if (Time[fcWhite] = 0.0) then
              RSendData(CMD_FLAG);
        end // case
      end // with
    else if (sl = CMD_PAUSE_GAME) then
    begin
      m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(19), mtConfirmation,
        [mbYes, mbNo], mfCanPause); // Can we pause the game?
    end
    else if (sl = CMD_PAUSE_GAME_YES) then
    begin
      PauseGame;
      RRetransmit(strSavedCmd);
    end
    else if (sl = CMD_PAUSE_GAME_NO) then
    begin
      m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(20), mtCustom,
        [mbOk], mfNone); // No pause, sorry.
    end
    else if (sl = CMD_CONTINUE_GAME) then
    begin
      if (Assigned(m_ContinueForm)) then
        m_ContinueForm.Shut;
      if (Transmittable) then
        m_Dialogs.CloseNoneDialogs;
      ContinueGame;
      RRetransmit(strSavedCmd); 
    end
    else if (sl = CMD_TAKEBACK) then
    begin
      if (you_takebacks or ChessBoard.pTrainingMode) then
      begin
        m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(21),
                           mtConfirmation, [mbYes, mbNo], mfMsgTakeBack); // 'May I take back last move?'
      end
      else
        RSendData(CMD_TAKEBACK_NO)
    end
    else if (sl = CMD_ADJOURN_GAME) then
    begin
      m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(22),
                             mtConfirmation, [mbYes, mbNo], mfMsgAdjourn); // Can we adjourn this game?
    end
    else if (sl = CMD_ADJOURN_GAME_YES) then
    begin
      FAdjournGame;
      RRetransmit(strSavedCmd);
    end
    else if (sl = CMD_ADJOURN_GAME_NO) then
    begin
      m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(23), mtCustom, [mbOk],
        mfNone); // No adjourns, sorry.
    end
    else
    if (sl = CMD_TAKEBACK_YES) then
    begin
      ChessBoard.TakeBack;
      FBuildAdjournedStr;
      TakebackGame.Enabled:= (ChessBoard.NMoveDone > 0);
{$IFDEF GAME_LOG}
      FWriteToGameLog(' <takeback>');
{$ENDIF}
      ChessBoard.SwitchClock(ChessBoard.PositionColor);
      RRetransmit(strSavedCmd);
    end
    else if (sl = CMD_TAKEBACK_NO) then
    begin
      m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(24), mtCustom,
        [mbOK], mfNone); // Sorry, no takebacks!
    end
    else if (sl = CMD_POSITION) then
    begin
      if (CompareStr(PlayerNickId, OpponentNickId) > 0) then
      begin
        ChessBoard.StopClock;
        ChessBoard.Mode := mView;
        ChessBoard.SetPosition(sr);
      end;
      RRetransmit(strSavedCmd);
    end
    else
    begin
      with ChessBoard do
      begin
        if ((_PlayerColor <> PositionColor) or Transmittable) then
        begin
          if (DoMove(sl)) then
          begin
{$IFDEF GAME_LOG}
            if ((PositionColor = fcBlack) or (not move_done)) then
            begin
              FWriteToGameLog(' ' + IntToStr(NMoveDone) + '.');
              if (PositionColor = fcWhite) then
                FWriteToGameLog(' ...');
            end;
            FWriteToGameLog(' ' + sl);
{$ENDIF}
            move_done := TRUE;
            TakebackGame.Enabled := TRUE;
            FBuildAdjournedStr; // AdjournedStr помечается только при входящем ходе противника
          end; // if (DoMove...
        end; // if (_Player...
      end; // with ChessBoard
      
      RRetransmit(strSavedCmd);
    end;
  end; // case ChessBoard.Mode
end;


procedure TManager.RRetransmit(const strCmd: string);
begin
end;


procedure TManager.ROnDestroy;
begin
  TLocalizer.Instance.DeleteSubscriber(self);

  if (m_bConnectionOccured) then
  begin
    FWritePrivateSettings;
    if (not Transmittable) then
      FWriteCommonSettings;
  end;

  m_ExtBaseList.Free;

  if (Assigned(ChessBoard)) then
  begin
    ChessBoard.Release;
    m_ChessBoard := nil;
  end;
  m_Dialogs.Free;

  TIniSettings.FreeInstance;
end;


procedure TManager.FormDestroy(Sender: TObject);
begin
  ROnDestroy;
end;


procedure TManager.LookFeelOptionsActionExecute(Sender: TObject);
var
  lookFeelOptionsForm: TLookFeelOptionsForm;
begin
  lookFeelOptionsForm := (m_Dialogs.CreateDialog(TLookFeelOptionsForm) as TLookFeelOptionsForm);
  with lookFeelOptionsForm, ChessBoard do
  begin
    AnimationComboBox.ItemIndex := ord(animation);
    HilightLastMoveBox.Checked := LastMoveHilighted;
    FlashIncomingMoveBox.Checked := FlashOnMove;
    CoordinatesBox.Checked := CoordinatesShown;
    StayOnTopBox.Checked := StayOnTop;
    ExtraExitBox.Checked := extra_exit;
  end;
  lookFeelOptionsForm.Show;
end;


procedure TManager.AbortGameClick(Sender: TObject);
begin
  RSendData(CMD_ABORT);
end;

procedure TManager.DrawGameClick(Sender: TObject);
begin
  RSendData(CMD_DRAW);
end;

procedure TManager.ResignGameClick(Sender: TObject);
begin
  m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(25),
                mtConfirmation, [mbYes, mbNo], mfMsgResign); // Do you really want to resign?
end;


procedure TManager.ChangeColorConnectedClick(Sender: TObject);
begin
  if (Transmittable) then
  begin
    ChangeColor;
  end
  else if (ChessBoard.Mode = mView) then
  begin
    ChangeColor;

    RSendData(CMD_CHANGE_COLOR);
    RRetransmit(CMD_CHANGE_COLOR);
  end;
end;


procedure TManager.GameOptionsConnectedClick(Sender: TObject);
var
  GameOptionsForm: TGameOptionsForm;
  i: integer;
begin
  GameOptionsForm := (m_Dialogs.CreateDialog(TGameOptionsForm) as TGameOptionsForm);
  with GameOptionsForm do
  begin
    EqualTimeCheckBox.Checked := ((you_unlimited = opponent_unlimited) and
      (you_time = opponent_time) and (you_inc = opponent_inc));
    YouUnlimitedCheckBox.Checked:= you_unlimited;
    OpponentUnlimitedCheckBox.Checked:= opponent_unlimited;
    YouMinUpDown.Position := you_time;
    YouIncUpDown.Position := you_inc;
    OpponentMinUpDown.Position := opponent_time;
    OpponentIncUpDown.Position := opponent_inc;
    AutoFlagCheckBox.Checked := ChessBoard.AutoFlag;
    TakeBackCheckBox.Checked := you_takebacks;
    TrainingEnabledCheckBox.Checked := ChessBoard.pTrainingMode;
    for i := 1 to m_ExtBaseList.Count - 1 do
    begin
      ExtBaseComboBox.Items.Append(m_ExtBaseList[i]);
      if (m_strExtBaseName = m_ExtBaseList[i]) then
        ExtBaseComboBox.ItemIndex := i;
    end;
    UsrBaseCheckBox.Checked := ChessBoard.pUseUserBase;
    GamePauseCheckBox.Checked := (can_pause_game and (m_lwOpponentClientVersion >= 200706));
    GameAdjournCheckBox.Checked := (can_adjourn_game and (m_lwOpponentClientVersion >= 200801));
    Show;
  end; // with
end;


procedure TManager.StartStandartGameConnectedClick(Sender: TObject);
var
  strPositionCmd: string;
begin
  with ChessBoard do
    begin
      SetClock;
      InitPosition;
      ResetMoveList;

      strPositionCmd := CMD_POSITION + ' ' + GetPosition;
      RSendData(strPositionCmd);
      RSendData(CMD_START_GAME);

      move_done:= FALSE;
      TakebackGame.Enabled := FALSE;
      Mode := mGame;
      SwitchClock(ChessBoard.PositionColor);

      RRetransmit(strPositionCmd);
      RRetransmit(CMD_START_GAME);
    end;
{$IFDEF GAME_LOG}
  FInitGameLog;
{$ENDIF}
end;


procedure TManager.SetClock;
begin
  if (not Assigned(ChessBoard)) then
    exit;
     
  with ChessBoard do
  begin
    Unlimited[_PlayerColor] := you_unlimited;
    Time[_PlayerColor] := EncodeTime(you_time div 60, you_time mod 60, 0,0);
    if (_PlayerColor = fcWhite) then
    begin
      Unlimited[fcBlack] := opponent_unlimited;
      Time[fcBlack] := EncodeTime(opponent_time div 60,
                               opponent_time mod 60, 0,0);
    end
    else
    begin
      Unlimited[fcWhite] := opponent_unlimited;
      Time[fcWhite] := EncodeTime(opponent_time div 60,
                               opponent_time mod 60, 0,0);
    end;
  end;
end;

procedure TManager.RSetChessBoardToView;
var
  clockTime: string;
begin
  with ChessBoard do
  begin
    clockTime := NO_CLOCK_TIME;
    SetClock(clockTime);
    Mode := mView;
    Caption := CHESS4NET_TITLE;
    ChessBoard.icon := Chess4NetIcon;
    InitPosition;

    Left:= (Screen.Width - Width) div 2;
    Top:= (Screen.Height - Height) div 2;
    Show;
  end;
end;


procedure TManager.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  if (Assigned(Connector) and Connector.connected) then
  begin
    if (Assigned(m_Dialogs)) then
    begin
      m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(26), mtConfirmation, [mbYes, mbNo], mfMsgClose); // Do you want to exit?
      Action:= caNone;
    end
    else
      Release;
  end
  else
//    Release;    
    Action := caFree;
end;


procedure TManager.RReleaseWithConnectorGracefully;
begin
  ConnectorTimer.Enabled := TRUE;
  if (Assigned(Connector)) then
    Connector.Close;
end;


procedure TManager.ConnectorTimerTimer(Sender: TObject);
begin
  ConnectorTimer.Enabled := FALSE;
  Release;
end;


procedure TManager.StartPPRandomGameConnectedClick(Sender: TObject);
var
  strPositionCmd: string;
begin
  with ChessBoard do
    begin
      SetClock;
      PPRandom;
      ResetMoveList;

      strPositionCmd := CMD_POSITION + ' ' + GetPosition;
      RSendData(strPositionCmd);
      RSendData(CMD_START_GAME);

      Mode := mGame;
      move_done := FALSE;
      TakebackGame.Enabled := FALSE;
      SwitchClock(ChessBoard.PositionColor);

      RRetransmit(strPositionCmd);
      RRetransmit(CMD_START_GAME);
    end;
{$IFDEF GAME_LOG}
  FInitGameLog;
{$ENDIF}
end;


procedure TManager.TakebackGameClick(Sender: TObject);
begin
  RSendData(CMD_TAKEBACK);
end;


constructor TManager.RCreate;
begin
//  inherited Create(Application);
  inherited Create(nil);
end;


{$IFDEF AND_RQ}
class function TManager.Create: TManager;
begin
  Result := TManagerDefault.Create;
end;
{$ENDIF}

{$IFDEF QIP}
class function TManager.Create(const accName: WideString; const protoDllHandle: integer): TManager;
begin
  Result := TManagerDefault.Create(accName, protoDllHandle);
end;
{$ENDIF}

{$IFDEF TRILLIAN}
class function TManager.Create(const vContactlistEntry: TTtkContactListEntry): TManager;
begin
  Result := TManagerDefault.Create(vContactlistEntry);
end;
{$ENDIF}

procedure TManager.DialogFormHandler(modSender: TModalForm; msgDlgID: TModalFormID);
var
  modRes: TModalResult;
  s, prevClock: string;
  strCmd: string;
begin
  modRes := modSender.ModalResult;
  case msgDlgID of
    mfNone: ;

    mfMsgClose:
    begin
      if modRes = mrYes then
      begin
{$IFDEF GAME_LOG}
        if ChessBoard.Mode = mGame then
        begin
          FWriteToGameLog('*');
          FlushGameLog;
        end;
{$ENDIF}
{$IFDEF SKYPE}
        FShowCredits;
{$ENDIF}
        Release;
      end;
    end;

    mfMsgLeave, mfIncompatible:
    begin
{$IFDEF SKYPE}
      FShowCredits;
{$ENDIF}
      if (Assigned(Connector) and Connector.Connected) then
        RReleaseWithConnectorGracefully
      else
        Close;
    end;

    mfMsgAbort:
    begin
      if ChessBoard.Mode = mGame then
      begin
        if (modRes = mrNo) or (modRes = mrNone) then
          RSendData(CMD_ABORT_DECLINED)
        else
        begin
          RSendData(CMD_ABORT_ACCEPTED);
          RRetransmit(CMD_ABORT_ACCEPTED);
          FExitGameMode;
{$IFDEF GAME_LOG}
          FWriteToGameLog('*');
          FlushGameLog;
{$ENDIF}
          m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(13), mtCustom,
            [mbOK], mfNone); // The game is aborted.
        end;
      end;
    end;

    mfMsgResign:
    begin
	    if ChessBoard.Mode = mGame then
      begin
        if modRes = mrYes then
        begin
          FExitGameMode;
          RSendData(CMD_RESIGN);
          RRetransmit(CMD_RESIGN + IfThen((_PlayerColor = fcWhite), ' w', ' b'));
          ChessBoard.WriteGameToBase(grLost);
{$IFDEF GAME_LOG}
          if (_PlayerColor = fcWhite) then
            FWriteToGameLog(sLineBreak + 'White resigns' + sLineBreak + '0 - 1')
          else
            FWriteToGameLog(sLineBreak + 'Black resigns' + sLineBreak + '1 - 0');
          FlushGameLog;
{$ENDIF}
        end;
      end;
    end;

    mfMsgDraw:
    begin
      if ChessBoard.Mode = mGame then
      begin
        if (modRes = mrNo) or (modRes = mrNone) then
          RSendData(CMD_DRAW_DECLINED)
        else
        begin
          RSendData(CMD_DRAW_ACCEPTED);
          RRetransmit(CMD_DRAW_ACCEPTED);

          FExitGameMode;
{$IFDEF GAME_LOG}
            FWriteToGameLog('=' + sLineBreak + '1/2 - 1/2');
            FlushGameLog;
{$ENDIF}
          m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(15), mtCustom, [mbOK], mfNone);
          ChessBoard.WriteGameToBase(grDraw); // The game is drawn.
        end;
      end;
    end;

    mfMsgTakeBack:
    begin
      if ChessBoard.Mode = mGame then
      begin
        if modRes = mrYes then
        begin
          RSendData(CMD_TAKEBACK_YES);
          RRetransmit(CMD_TAKEBACK_YES);

          ChessBoard.TakeBack;
          FBuildAdjournedStr;
          TakebackGame.Enabled:= (ChessBoard.NMoveDone > 0);
{$IFDEF GAME_LOG}
          FWriteToGameLog(' <takeback>');
{$ENDIF}
          ChessBoard.SwitchClock(ChessBoard.PositionColor);
        end
        else
          RSendData(CMD_TAKEBACK_NO);
      end;
    end;

    mfMsgAdjourn:
    begin
      if ChessBoard.Mode = mGame then
      begin
        if modRes = mrYes then
        begin
          RSendData(CMD_ADJOURN_GAME_YES);
          RRetransmit(CMD_ADJOURN_GAME_YES);
          FAdjournGame;
        end
        else
          RSendData(CMD_ADJOURN_GAME_NO);
      end;
    end;

    mfConnecting:
    begin
      m_ConnectingForm := nil;
      if modRes = mrAbort then
        Close; // ConnectionAbort;
    end;

    mfGameOptions:
    begin
      if (ChessBoard.Mode <> mGame) and (modRes = mrOK) then
        with (modSender as TGameOptionsForm) do
        begin
          prevClock := ClockToStr;
          you_unlimited := YouUnlimitedCheckBox.Checked;
          opponent_unlimited := OpponentUnlimitedCheckBox.Checked;
          you_time := StrToInt(YouMinEdit.Text);
          you_inc := StrToInt(YouIncEdit.Text);
          opponent_time := StrToInt(OpponentMinEdit.Text);
          opponent_inc := StrToInt(OpponentIncEdit.Text);
          ChessBoard.AutoFlag := AutoFlagCheckBox.Checked;
          // Отображение на доске
          SetClock;
          // синхронизация времени у оппонента
          s := ClockToStr;
          if (m_lwOpponentClientVersion >= 200705) then
          begin
            if (prevClock <> s) then
            begin
              strCmd := CMD_SET_CLOCK + ' ' + s;
              RSendData(strCmd);
              RRetransmit(strCmd);
            end;
            RSendData(CMD_ALLOW_TAKEBACKS + IfThen(TakeBackCheckBox.Checked, ' 1', ' 0'));
          end;
          you_takebacks := TakeBackCheckBox.Checked;
          if (m_lwOpponentClientVersion >= 200706) then
          begin
            if can_pause_game <> GamePauseCheckBox.Checked then
            begin
              can_pause_game := GamePauseCheckBox.Checked;
              RSendData(CMD_CAN_PAUSE_GAME + IfThen(can_pause_game, ' 1', ' 0'))
            end;
          end;
          if (m_lwOpponentClientVersion >= 200801) then
          begin
            if can_adjourn_game <> GameAdjournCheckBox.Checked then
            begin
              can_adjourn_game := GameAdjournCheckBox.Checked;
              RSendData(CMD_CAN_ADJOURN_GAME + IfThen(can_adjourn_game, ' 1', ' 0'))
            end;
          end;
          // Training mode
          if (m_lwOpponentClientVersion >= 200705) and (ChessBoard.pTrainingMode <> TrainingEnabledCheckBox.Checked) then
          begin
            RSendData(CMD_SET_TRAINING + IfThen(TrainingEnabledCheckBox.Checked, ' 1', ' 0'));
          end;
          ChessBoard.pTrainingMode := TrainingEnabledCheckBox.Checked;
          m_strExtBaseName := m_ExtBaseList[ExtBaseComboBox.ItemIndex];
          if (m_strExtBaseName <> '') then
            ChessBoard.SetExternalBase(Chess4NetPath + m_strExtBaseName)
          else
            ChessBoard.UnsetExternalBase;
          ChessBoard.pUseUserBase := UsrBaseCheckBox.Checked;
          GamePause.Visible := can_pause_game;
          TakebackGame.Visible := (ChessBoard.pTrainingMode or opponent_takebacks);

          if (m_lwOpponentClientVersion < 200705) then // 2007.4
          begin
            if ChessBoard.pTrainingMode then
              s := s + ' 1 1'
            else
              s := s + IfThen(you_takebacks, ' 1 0', ' 0 0');
            RSendData(CMD_GAME_OPTIONS + ' ' + s);
          end;
        end;
    end;

    mfLookFeel:
    begin
      with (modSender as TLookFeelOptionsForm), ChessBoard do
      begin
        animation := TAnimation(AnimationComboBox.ItemIndex);
        LastMoveHilighted := HilightLastMoveBox.Checked;
        FlashOnMove := FlashIncomingMoveBox.Checked;
        CoordinatesShown := CoordinatesBox.Checked;
        StayOnTop := StayOnTopBox.Checked;
        extra_exit := ExtraExitBox.Checked;
      end;
    end;

    mfContinue:
    begin
      m_ContinueForm := nil;
      if modRes = mrOk then
      begin
        RSendData(CMD_CONTINUE_GAME);
        RRetransmit(CMD_CONTINUE_GAME);
        ContinueGame;
      end;
    end;

    mfCanPause:
    begin
      if modRes = mrYes then
      begin
        RSendData(CMD_PAUSE_GAME_YES);
        RRetransmit(CMD_PAUSE_GAME_YES);
        PauseGame;
      end
      else // modRes = mrNo
        RSendData(CMD_PAUSE_GAME_NO);
    end;

    mfDontShowDlg:
    begin
      if ((modSender as TDontShowMessageDlg).DontShow) then
        m_iDontShowLastVersion := m_iQueriedDontShowLastVersion;
    end;

  end;
end;

{$IFDEF GAME_LOG}
procedure TManager.FInitGameLog;
var
  s: string;
begin
  if ((not m_bConnectionOccured) or m_bTransmittable) then
    exit; 

  gameLog := '';

  LongTimeFormat := HOUR_TIME_FORMAT;
  FWriteToGameLog('[' + DateTimeToStr(Now) + ']' + sLineBreak);

  FWriteToGameLog(RGetGameName);

  if not (you_unlimited and opponent_unlimited) then
  begin
    FWriteToGameLog(' (');
    case _PlayerColor of
      fcWhite:
      begin
        if (not you_unlimited) then
        begin
          FWriteToGameLog(IntToStr(you_time));
          if (you_inc > 0) then
            FWriteToGameLog('.' + IntToStr(you_inc));
        end
        else
          FWriteToGameLog('inf');

        FWriteToGameLog(':');

        if (not opponent_unlimited) then
        begin
          FWriteToGameLog(IntToStr(opponent_time));
          if (opponent_inc > 0) then
            FWriteToGameLog('.' + IntToStr(opponent_inc));
        end
        else
          FWriteToGameLog('inf');
      end;

      fcBlack:
      begin
        if (not opponent_unlimited) then
        begin
          FWriteToGameLog(IntToStr(opponent_time));
          if (opponent_inc > 0) then
            FWriteToGameLog('.' + IntToStr(opponent_inc));
        end
        else
          FWriteToGameLog('inf');

        FWriteToGameLog(':');

        if (not you_unlimited) then
        begin
          FWriteToGameLog(IntToStr(you_time));
          if (you_inc > 0) then
            FWriteToGameLog('.' + IntToStr(you_inc));
        end
        else
          FWriteToGameLog('inf');
      end;
    end;
    FWriteToGameLog(')');
  end;
  FWriteToGameLog(sLineBreak);

  s := ChessBoard.GetPosition;
  if (s <> INITIAL_CHESS_POSITION) then
    FWriteToGameLog(s + sLineBreak);
end;


procedure TManager.FWriteToGameLog(const s: string);
begin
  if ((not m_bConnectionOccured) or m_bTransmittable) then
    exit; 

  gameLog := gameLog + s;
end;


procedure TManager.FlushGameLog;
var
  gameLogFile: TextFile;
begin
  if ((not m_bConnectionOccured) or m_bTransmittable) then
    exit; 

  if (not move_done) then
    exit;

  AssignFile(gameLogFile, Chess4NetGamesLogPath + GAME_LOG_FILE);
{$I-}
  Append(gameLogFile);
{$I+}
  if (IOResult <> 0) then
    begin
      Rewrite(gameLogFile);
      if (IOResult = 0) then
        writeln(gameLogFile, gameLog);
    end
  else
    writeln(gameLogFile, sLineBreak + gameLog);

  CloseFile(gameLogFile);
{$IFDEF SKYPE}
  CreateLinkForGameLogFile;
{$ENDIF}
end;
{$ENDIF}

procedure TManager.FPopulateExtBaseList;
var
  sr: TSearchRec;
  extBaseName: string;
begin
  m_ExtBaseList.Append('');
  if (FindFirst(Chess4NetPath + '*.pos', faAnyFile, sr) = 0) then
  begin
    repeat
      extBaseName := LeftStr(sr.Name, length(sr.Name) - length(ExtractFileExt(sr.Name)));
      if (extBaseName <> USR_BASE_NAME) and FileExists(Chess4NetPath + extBaseName + '.mov') then
        m_ExtBaseList.Append(extBaseName);
    until FindNext(sr) <> 0;
  end; // if
  FindClose(sr);
end;


procedure TManager.RCreateAndPopulateExtBaseList;
begin
  m_ExtBaseList := TStringList.Create;
  FPopulateExtBaseList;
  m_strExtBaseName := '';
end;


procedure TManager.RReadPrivateSettings;
var
  initialClockTime: string;
begin
  // Общие настройки по умолчанию
  initialClockTime := INITIAL_CLOCK_TIME;
  SetClock(initialClockTime);

  ChessBoard.AutoFlag := TRUE;
  you_takebacks := FALSE;
  opponent_takebacks := FALSE;

  // Reading private settings
  ChessBoard.animation := TIniSettings.Instance.Animation;
  ChessBoard.LastMoveHilighted := TIniSettings.Instance.LastMoveHilighted;
  ChessBoard.FlashOnMove := TIniSettings.Instance.FlashOnMove;
  ChessBoard.CoordinatesShown := TIniSettings.Instance.CoordinatesShown;
  // TODO: read screen position and size
  ChessBoard.StayOnTop := TIniSettings.Instance.StayOnTop;
  extra_exit := TIniSettings.Instance.ExtraExit;
  TLocalizer.Instance.ActiveLanguage := TIniSettings.Instance.ActiveLanguage;
  m_iDontShowLastVersion := TIniSettings.Instance.DontShowLastVersion;
{$IFDEF SKYPE}
  m_bDontShowCredits := TIniSettings.Instance.DontShowCredits;
{$ENDIF}
end;


function TManager.FReadCommonSettings(setToOpponent: boolean): boolean;
var
  strClock: string;
  bFlag: boolean;
begin
  if (m_lwOpponentClientVersion < 200705) then // For 2007.4 common settings are not applied
  begin
    Result := TRUE;
    exit;
  end;

  Result := FALSE;

  TIniSettings.Instance.SetOpponentId(OpponentId);
  if (not TIniSettings.Instance.HasCommonSettings) then
    exit;

  if (setToOpponent) then
  begin
    if (_PlayerColor = TIniSettings.Instance.PlayerColor) then // Every time change the saved color to opposite one
    begin
      ChangeColor;
      RSendData(CMD_CHANGE_COLOR);
      RRetransmit(CMD_CHANGE_COLOR);
    end;

    strClock := TIniSettings.Instance.Clock;
    if (strClock <> ClockToStr) then
    begin
      SetClock(strClock);
      RSendData(CMD_SET_CLOCK + ' ' + ClockToStr);
    end;

    bFlag := TIniSettings.Instance.TrainingMode;
    if (ChessBoard.pTrainingMode <> bFlag) then
    begin
      ChessBoard.pTrainingMode := bFlag;
      RSendData(CMD_SET_TRAINING + IfThen(ChessBoard.pTrainingMode, ' 1', ' 0'));
    end;

    if (m_lwOpponentClientVersion >= 200706) then
    begin
      bFlag := TIniSettings.Instance.CanPauseGame;
      if (can_pause_game <> bFlag) then
      begin
        can_pause_game := bFlag;
        RSendData(CMD_CAN_PAUSE_GAME + IfThen(can_pause_game, ' 1', ' 0'));
      end;
    end; { if opponentClientVersion >= 200706}

    if (m_lwOpponentClientVersion >= 200801) then
    begin
      bFlag := TIniSettings.Instance.CanAdjournGame;
      if (can_adjourn_game <> bFlag) then
      begin
        can_adjourn_game := bFlag;
        RSendData(CMD_CAN_ADJOURN_GAME + IfThen(can_adjourn_game, ' 1', ' 0'));
      end;
    end; { opponentClientVersion >= 200801 }
  end; { if setToOpponent }

  m_strExtBaseName := TIniSettings.Instance.ExternalBaseName;
  if (m_strExtBaseName <> '') then
    ChessBoard.SetExternalBase(Chess4NetPath + m_strExtBaseName)
  else
    ChessBoard.UnsetExternalBase;

  ChessBoard.pUseUserBase := TIniSettings.Instance.UseUserBase;

  bFlag := TIniSettings.Instance.AllowTakebacks;
  if (you_takebacks <> bFlag) then
  begin
    you_takebacks := bFlag;
    RSendData(CMD_ALLOW_TAKEBACKS + IfThen(you_takebacks, ' 1', ' 0'));
  end;

  ChessBoard.AutoFlag := TIniSettings.Instance.AutoFlag;

  TakebackGame.Visible := (opponent_takebacks or ChessBoard.pTrainingMode);
  GamePause.Visible := can_pause_game;

  if (m_lwOpponentClientVersion >= 200801) then
  begin
    if (AdjournedStr <> '') then
    begin
      RSendData(CMD_SET_ADJOURNED + ' ' + AdjournedStr);
    end;
  end;

  Result := TRUE;
end;


procedure TManager.FWritePrivateSettings;
begin
  // Write private settings
  TIniSettings.Instance.Animation := ChessBoard.Animation;
  TIniSettings.Instance.LastMoveHilighted := ChessBoard.LastMoveHilighted;
  TIniSettings.Instance.FlashOnMove := ChessBoard.FlashOnMove;
  TIniSettings.Instance.CoordinatesShown := ChessBoard.CoordinatesShown;
  // TODO: write screen position
  TIniSettings.Instance.StayOnTop := ChessBoard.StayOnTop;
  TIniSettings.Instance.ExtraExit := extra_exit;
  TIniSettings.Instance.ActiveLanguage := TLocalizer.Instance.ActiveLanguage;
  if (m_iDontShowLastVersion > CHESS4NET_VERSION) then
    TIniSettings.Instance.DontShowLastVersion := m_iDontShowLastVersion;
{$IFDEF SKYPE}
  if (m_bDontShowCredits) then
    TIniSettings.Instance.DontShowCredits := m_bDontShowCredits;
{$ENDIF}
end;


procedure TManager.FWriteCommonSettings;
begin
  TIniSettings.Instance.SetOpponentId(OpponentId);

  TIniSettings.Instance.PlayerColor := _PlayerColor;
  TIniSettings.Instance.Clock := ClockToStr;
  TIniSettings.Instance.TrainingMode := ChessBoard.pTrainingMode;
  TIniSettings.Instance.ExternalBaseName := m_strExtBaseName;
  TIniSettings.Instance.UseUserBase := ChessBoard.pUseUserBase;
  TIniSettings.Instance.AllowTakebacks := you_takebacks;
  TIniSettings.Instance.CanPauseGame := can_pause_game;
  TIniSettings.Instance.CanAdjournGame := can_adjourn_game;
  TIniSettings.Instance.AutoFlag := ChessBoard.AutoFlag;
end;


function TManager.ClockToStr: string;
var
  s: string;
begin
  if (you_unlimited) then
    s := 'u'
  else
    s := IntToStr(you_time) + ' ' + IntToStr(you_inc);
  if (opponent_unlimited) then
    s := s + ' u'
  else
    s := s + ' ' + IntToStr(opponent_time) + ' ' + IntToStr(opponent_inc);

  Result := s;
end;


procedure TManager.ChangeColor;
begin
  with ChessBoard do
  begin
    if (_PlayerColor = fcWhite) then
    begin
      StartStandartGameConnected.Enabled := FALSE;
      StartPPRandomGameConnected.Enabled := FALSE;
      _PlayerColor := fcBlack;
    end
    else // fcBlack
    begin
      StartStandartGameConnected.Enabled := TRUE;
      StartPPRandomGameConnected.Enabled := TRUE;
      _PlayerColor := fcWhite;
    end;
    RUpdateChessBoardCaption;
    SetClock;
  end;
end;


procedure TManager.GamePauseClick(Sender: TObject);
begin
  RSendData(CMD_PAUSE_GAME);
end;


procedure TManager.PauseGame;
begin
  ChessBoard.StopClock;
  if (not Transmittable) then
  begin
    m_ContinueForm := (m_Dialogs.CreateDialog(TContinueForm) as TContinueForm);
    m_ContinueForm.Show;
  end;
end;


procedure TManager.ContinueGame;
begin
  ChessBoard.SwitchClock(ChessBoard.PositionColor);
end;

procedure TManager.AboutActionExecute(Sender: TObject);
begin
  ShowInfo;
end;

procedure TManager.AdjournGameClick(Sender: TObject);
begin
  RSendData(CMD_ADJOURN_GAME);
end;

procedure TManager.StartAdjournedGameConnectedClick(Sender: TObject);
begin
  if (AdjournedStr <> '') then
  begin
    RSendData(CMD_START_ADJOURNED_GAME);
    FStartAdjournedGame;
    RRetransmit(CMD_GAME_CONTEXT + ' ' + RGetGameContextStr);
    RRetransmit(CMD_CONTINUE_GAME);
  end;
end;


procedure TManager.FAdjournGame;
begin
  if (ChessBoard.Mode <> mGame) then
    exit;
  FBuildAdjournedStr;  
  ChessBoard.StopClock;
  ChessBoard.Mode := mView;
{$IFDEF GAME_LOG}
  FWriteToGameLog('*');
  FlushGameLog;
{$ENDIF}
  m_Dialogs.MessageDlg(TLocalizer.Instance.GetMessage(27), mtCustom, [mbOK], mfNone); // The game is adjourned.
end;

procedure TManager.FExitGameMode;
begin
  ChessBoard.StopClock;
  ChessBoard.Mode := mView;
  if (move_done) then
    AdjournedStr := '';
end;


function TManager.RGetGameContextStr: string;
var
  str: string;
begin
  // Result ::= <position>&<this player's color>&<time control>&<current time>
  with ChessBoard do
  begin
    // <position>
    str := ChessBoard.GetPosition + '&';
    // <this player's color>
    str := str + IfThen((_PlayerColor = fcWhite), 'w', 'b') + '&';
    // <time control>
    str := str + ClockToStr + '&';
    // <current time>
    str := str + TChessClock.ConvertToFullStr(Time[fcWhite], FALSE) + ' ' +
                 TChessClock.ConvertToFullStr(Time[fcBlack], FALSE);
  end;

  Result := str;
end;


procedure TManager.FBuildAdjournedStr;
begin
  AdjournedStr := RGetGameContextStr;
end;


procedure TManager.FStartAdjournedGame;
begin
  if (AdjournedStr = '') then
    exit;

  RSetGameContext(AdjournedStr);

  with ChessBoard do
  begin
    ResetMoveList;
    move_done := TRUE;
    TakebackGame.Enabled := FALSE;
    Mode := mGame;
    SwitchClock(PositionColor);
  end;
{$IFDEF GAME_LOG}
  FInitGameLog;
{$ENDIF}
end;


function TManager.FGetAdjournedStr: string;
begin
  Result := TIniSettings.Instance.Adjourned;
end;


procedure TManager.FSetAdjournedStr(const strValue: string);
begin
  TIniSettings.Instance.Adjourned := strValue;
end;


procedure TManager.RSetGameContext(const strValue: string);
var
  str: string;
  l: integer;
  strPosition, strPlayerColor, strTimeControl, strCurrentTime: string;
begin
  if (strValue = '') then
    exit;

  // strValue ::= <position>&<this player's color>&<time control>&<current time>

  str := strValue;

  l := pos('&', str);
  strPosition := LeftStr(str, l - 1);
  str := RightStr(str, length(str) - l);

  l := pos('&', str);
  strPlayerColor := LeftStr(str, l - 1);
  str := RightStr(str, length(str) - l);

  l := pos('&', str);
  strTimeControl := LeftStr(str, l - 1);
  strCurrentTime := RightStr(str, length(str) - l);

  SetClock(strTimeControl);

  if (((_PlayerColor = fcWhite) and (strPlayerColor <> 'w')) or
      ((_PlayerColor = fcBlack) and (strPlayerColor <> 'b'))) then
    ChangeColor;

  with ChessBoard do
  begin
    SetPosition(strPosition);

    RSplitStr(strCurrentTime, str, strCurrentTime);

    Time[fcWhite] := TChessClock.ConvertFromFullStr(str);
    Time[fcBlack] := TChessClock.ConvertFromFullStr(strCurrentTime);
  end;
end;


procedure TManager.GamePopupMenuPopup(Sender: TObject);
begin
  N6.Visible := ((not Transmittable) and
    (AdjournGame.Visible or GamePause.Visible or TakebackGame.Visible));
  ResignGame.Enabled := move_done;
  DrawGame.Enabled := (move_done and (_PlayerColor = ChessBoard.PositionColor));
end;


procedure TManager.RLocalize;
begin
  with TLocalizer.Instance do
  begin
    StartAdjournedGameConnected.Caption := GetLabel(51);
    StartStandartGameConnected.Caption := GetLabel(52);
    StartPPRandomGameConnected.Caption := GetLabel(53);
    ChangeColorConnected.Caption := GetLabel(54);
    GameOptionsConnected.Caption := GetLabel(55);
    LookFeelOptionsAction.Caption := GetLabel(56);
    AboutAction.Caption := GetLabel(57);

    AbortGame.Caption := GetLabel(58);
    DrawGame.Caption := GetLabel(59);
    ResignGame.Caption := GetLabel(60);
    AdjournGame.Caption := GetLabel(61);
    GamePause.Caption := GetLabel(62);
    TakebackGame.Caption := GetLabel(63);
    BroadcastAction.Caption := GetLabel(69);
  end;
end;


function TManager.FGetOpponentNickId: string;
begin
  if ((not m_bTransmittable) or (m_strOverridedOpponentNickId = '')) then
    Result := OpponentNick + OpponentId
  else
    Result := m_strOverridedOpponentNickId;
end;


procedure TManager.FSetTransmittable(bValue: boolean);
begin
  m_bTransmittable := bValue;
  if (bValue) then
  begin
    // connected menu
    StartAdjournedGameConnected.Visible := FALSE;
    StartStandartGameConnected.Visible := FALSE;
    StartPPRandomGameConnected.Visible := FALSE;
//    ChangeColorConnected.Visible := FALSE;
    GameOptionsConnected.Visible := FALSE;

{$IFDEF SKYPE}
    BroadcastAction.Visible := FALSE;
{$ENDIF}

    ChessBoard.ViewGaming := TRUE;
  end;
end;


function TManager.FGetPlayerColor: TFigureColor;
begin
  if (Assigned(ChessBoard)) then
    Result := ChessBoard.PlayerColor
  else
    Result := fcWhite;
end;


procedure TManager.FSetPlayerColor(Value: TFigureColor);
begin
  if (Assigned(ChessBoard)) then
    ChessBoard.PlayerColor := Value;
end;


procedure TManager.ActionListUpdate(Action: TBasicAction;
  var Handled: Boolean);
begin
  AdjournGame.Visible := (can_adjourn_game and (not Transmittable));
  AdjournGame.Enabled := ((adjournedStr <> '') and move_done);
  StartAdjournedGameConnected.Visible := ((adjournedStr <> '') and (not Transmittable));
end;

{$IFDEF SKYPE}
procedure TManager.FShowCredits;

  function NFridayThe13: boolean; // just for fun!
  begin
    Result := ((DayOfTheMonth(Today) = 13) and (DayOfWeek(Today) = 6));
  end;

begin // TManager.FShowCredits
  if (m_bConnectionOccured and (not m_bDontShowCredits) and (not NFridayThe13)) then
  begin
    with TCreditsForm.Create(nil) do
    try
      ShowModal;
      m_bDontShowCredits := DontShowAgain;
    finally
      Free;
    end;
  end;
end;
{$ENDIF}

function TManager.RGetGameName: string;
begin
  if (_PlayerColor = fcWhite) then
    Result := PlayerNick + ' - ' + OpponentNick
  else // fcBlack
    Result := OpponentNick + ' - ' + PlayerNick;
end;


procedure TManager.BroadcastActionExecute(Sender: TObject);
begin
  RBroadcast;
end;


procedure TManager.RBroadcast;
begin
end;

////////////////////////////////////////////////////////////////////////////////
// TManagerDefault

{$IFDEF AND_RQ}
constructor TManagerDefault.Create;
begin
  RCreate;
end;
{$ENDIF}

{$IFDEF QIP}
constructor TManagerDefault.Create(const accName: WideString; const protoDllHandle: integer);
begin
  iProtoDllHandle := protoDllHandle;
  wAccName := accName;

  RCreate;
end;
{$ENDIF}

{$IFDEF TRILLIAN}
constructor TManagerDefault.Create(const vContactlistEntry: TTtkContactListEntry);
begin
  contactListEntry := vContactlistEntry;
  RCreate;
end;
{$ENDIF}

procedure TManagerDefault.ROnCreate;
begin
  try
    RCreateChessBoardAndDialogs;

    TLocalizer.Instance.AddSubscriber(self);
    RLocalize;

    RSetChessBoardToView;
    RReadPrivateSettings;

{$IFDEF AND_RQ}
    Connector := TConnector.Create(RQ_GetChatUIN, ConnectorHandler);
{$ENDIF}
{$IFDEF QIP}
//    QIPConnectionError := FALSE;
    Connector := TConnector.Create(wAccName, iProtoDllHandle, ConnectorHandler);
{$ENDIF}
{$IFDEF TRILLIAN}
    Connector := TConnector.Create(@contactlistEntry, ConnectorHandler);
{$ENDIF}

    RCreateAndPopulateExtBaseList;

    // nicks initialisation
{$IFDEF AND_RQ}
    PlayerNick := RQ_GetDisplayedName(RQ_GetCurrentUser);
    OpponentNick := RQ_GetDisplayedName(RQ_GetChatUIN);
    OpponentId := IntToStr(RQ_GetChatUIN);
{$ENDIF}
{$IFDEF QIP}
    PlayerNick := GetOwnerNick(wAccName, iProtoDllHandle);
    OpponentNick := GetContactNick(wAccName, iProtoDllHandle);
    OpponentId := wAccName;
{$ENDIF}
{$IFDEF TRILLIAN}
    PlayerNick := trillianOwnerNick;
    OpponentNick := contactlistEntry.name;
    OpponentId := contactlistEntry.real_name;
{$ENDIF}
{$IFDEF QIP}
    if (not QIPConnectionError) then
    begin
{$ENDIF}
      RShowConnectingForm;
{$IFDEF QIP}
    end;
{$ENDIF}

  except
    Release;
    raise;
  end;
end;


procedure TManagerDefault.ROnDestroy;
begin
  if (Assigned(Connector)) then
  begin
    Connector.Close;
  end;

  inherited ROnDestroy;
end;


procedure TManagerDefault.RSendData(const cmd: string);
const
  last_cmd: string = '';
begin
  if (cmd = '') then
    exit;
  last_cmd := cmd + CMD_DELIMITER;
  Connector.SendData(last_cmd);
end;

end.

