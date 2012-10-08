////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit GameChessBoardUnit;

interface

uses
  ExtCtrls, TntStdCtrls, Buttons, Controls, StdCtrls, Classes, Forms, TntForms,
  Graphics, Messages,
  // Chess4net
  ChessBoardHeaderUnit, ChessBoardUnit, ChessRulesEngine,
  LocalizerUnit, PosBaseChessBoardLayerUnit;

type
  TGameChessBoardEvent =
    (cbeMoved, cbeMate, cbeStaleMate, cbeInsuffMaterial, cbeKeyPressed,
     cbeClockSwitched, cbeTimeOut, cbeExit, cbeMenu, cbeActivate, cbeFormMoving,
     cbeRefreshAll);
     // It's possible to add new events. cbeRefreshAll signals that global options had been changed

  TGameChessBoardHandler = procedure(e: TGameChessBoardEvent;
                                 d1: pointer = nil; d2: pointer = nil) of object;

{$IFDEF THREADED_CHESSCLOCK}
  TGameChessBoard = class;

  TTimeLabelThread = class(TThread)
  private
    ChessBoard: TGameChessBoard;
    player_time: array[TFigureColor] of TDateTime;
  protected
    procedure Execute; override;
  public
    WhiteTime, BlackTime: string;
    constructor Create(ChessBoard: TGameChessBoard);
  end;
{$ENDIF}

  TGameChessBoard = class(TTntForm, ILocalizable)
    TimePanel: TPanel;
    WhiteLabel: TTntLabel;
    WhiteTimeLabel: TLabel;
    BlackLabel: TTntLabel;
    BlackTimeLabel: TLabel;
    GameTimer: TTimer;
    WhiteFlagButton: TSpeedButton;
    BlackFlagButton: TSpeedButton;
    WhitePanel: TPanel;
    BlackPanel: TPanel;
    ChessBoardPanel: TPanel;
    procedure FormCreate(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure GameTimerTimer(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure FormKeyDown(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure FormActivate(Sender: TObject);
    procedure FlagButtonClick(Sender: TObject);
    procedure FormCanResize(Sender: TObject; var NewWidth, NewHeight: Integer;
      var Resize: Boolean);
    procedure TimePanelResize(Sender: TObject);

  private
    m_ChessBoard: TChessBoard;
    m_PosBaseChessBoardLayer: TPosBaseChessBoardLayer;

    FHandler: TGameChessBoardHandler;

    auto_flag: boolean; // индикатор автофлага
    player_time: array[TFigureColor] of TDateTime; // время белых и чёрных
    past_time: TDateTime; // время начала обдумывания хода
    unlimited_var: array[TFigureColor] of boolean; // партия без временного контроля
    clock_color: TFigureColor; // цвет анимируемой фигуры

    shuted: boolean; // индикатор внешнего закрытия окна

    // Resizing
    m_ResizingType: (rtNo, rtHoriz, rtVert);
//    m_iDeltaWidthHeight: integer;

    m_iTimePanelInitialWidth: integer;
    m_iWhitePanelInitialLeft, m_iBlackPanelInitialLeft: integer;
    m_iWhitePanelInitialWidth, m_iBlackPanelInitialWidth: integer;
    m_TimeFont: TFont;

    m_bFlashOnMove: boolean; // flag for flashing window on icoming move

{$IFDEF THREADED_CHESSCLOCK}
    TimeLabelThread: TTimeLabelThread; // нить используется для борьбы с лагом в Миранде
{$ENDIF}

    function FGetMode: TMode;
    procedure FSetMode(const Value: TMode);
    procedure ShowTime(const c: TFigureColor);
    function FGetPlayerColor: TFigureColor;
    procedure FSetPlayerColor(const Value: TFigureColor);
    function FGetFlipped: boolean;
    procedure FSetFlipped(bValue: boolean);
    function FGetTime(color: TFigureColor): TDateTime;
    procedure FSetTime(color: TFigureColor; const tm: TDateTime);
    procedure FSetUnlimited(color: TFigureColor; const unl: boolean);
    function FGetUnlimited(color: TFigureColor): boolean;
    function FGetLastMoveHilighted: boolean;
    procedure FSetLastMoveHilighted(bValue: boolean);
    function FGetCoordinatesShown: boolean;
    procedure FSetCoordinatesShown(bValue: boolean);
    function GetStayOnTop: boolean;
    procedure FSetStayOnTop(onTop: boolean);
    procedure FSetAutoFlag(auto_flag: boolean);
    procedure FFlashWindow;
    function FGetAnimation: TAnimation;
    procedure FSetAnimation(const Value: TAnimation);
    function FGetViewGaming: boolean;
    procedure FSetViewGaming(bValue: boolean);

    // Localization
    procedure ILocalizable.Localize = FLocalize;
    procedure FLocalize;

    procedure WMMoving(var Msg: TWMMoving); message WM_MOVING;
    procedure WMSizing(var Msg: TMessage); message WM_SIZING;

    function FGetPositionColor: TFigureColor;

    function FGetTrainingMode: boolean;
    procedure FSetTrainingMode(bValue: boolean);

    function FGetUseUserBase: boolean;
    procedure FSetUseUserBase(bValue: boolean);

    procedure FChessBoardHandler(e: TChessBoardEvent; d1: pointer = nil;
      d2: pointer = nil);
    procedure FDoHandler(e: TGameChessBoardEvent; d1: pointer = nil;
      d2: pointer = nil);

  public
    constructor Create(Owner: TComponent; AHandler: TGameChessBoardHandler = nil;
      const strPosBaseName: string = ''); reintroduce;

    procedure FCreateChessBoard(const strPosBaseName: string);
    procedure FDestroyChessBoard;

    procedure TakeBack; // взятие хода обратно
    procedure SwitchClock(clock_color: TFigureColor);
    procedure InitPosition;
    procedure PPRandom;
    procedure StopClock;

    procedure ResetMoveList;
    function SetPosition(const strPosition: string): boolean;
    function GetPosition: string;
    function NMoveDone: integer; // количество сделанных ходов
    function DoMove(const strMove: string): boolean;
    procedure Shut;

    procedure WriteGameToBase(vGameResult: TGameResult);
    procedure SetExternalBase(const strExtPosBaseName: string);
    procedure UnsetExternalBase;

    property Unlimited[color: TFigureColor]: boolean read FGetUnlimited write FSetUnlimited;
    property Time[color: TFigureColor]: TDateTime read FGetTime write FSetTime;
    property PlayerColor: TFigureColor read FGetPlayerColor write FSetPlayerColor;
    property PositionColor: TFigureColor read FGetPositionColor; // Whos move it is in the current position
    property ClockColor: TFigureColor read clock_color;
    property Mode: TMode read FGetMode write FSetMode;
    property CoordinatesShown: boolean read FGetCoordinatesShown write FSetCoordinatesShown;

    property Flipped: boolean read FGetFlipped write FSetFlipped;
    property LastMoveHilighted: boolean read FGetLastMoveHilighted write FSetLastMoveHilighted;
    property FlashOnMove: boolean read m_bFlashOnMove write m_bFlashOnMove;
    property StayOnTop: boolean read GetStayOnTop write FSetStayOnTop;
    property AutoFlag: boolean read auto_flag write FSetAutoFlag;
    property Animation: TAnimation read FGetAnimation write FSetAnimation;
    property ViewGaming: boolean read FGetViewGaming write FSetViewGaming;

    property pTrainingMode: boolean read FGetTrainingMode write FSetTrainingMode;
    property pUseUserBase: boolean read FGetUseUserBase write FSetUseUserBase;
  end;

implementation

{$J+}

{$R *.dfm}

uses
  SysUtils, Types, Windows,
  //
  ChessClockUnit;

const
  TIME_COLOR = clBlack;

  ZEITNOT_COLOR = clMaroon;
//  CHEAT_TIME_CONST = 1.5; // > 1
  WHITE_LONG_LABEL: WideString   =   'White   ';
  WHITE_MEDIUM_LABEL: WideString = 'White ';
  WHITE_SHORT_LABEL: WideString  =  'W ';
  BLACK_LONG_LABEL: WideString   =   'Black   ';
  BLACK_MEDIUM_LABEL: WideString = 'Black ';
  BLACK_SHORT_LABEL: WideString  =  'B ';

////////////////////////////////////////////////////////////////////////////////
// TTimeLabelThread

{$IFDEF THREADED_CHESSCLOCK}
procedure TTimeLabelThread.Execute;
begin
  while ChessBoard.GameTimer.Enabled do
    begin
      if self.player_time[fcWhite] <> ChessBoard.player_time[fcWhite] then
        ChessBoard.ShowTime(fcWhite);
      if self.player_time[fcBlack] <> ChessBoard.player_time[fcBlack] then
        ChessBoard.ShowTime(fcBlack);
      Sleep(ChessBoard.GameTimer.Interval div 2);
    end;
  ChessBoard.TimeLabelThread := nil;  
end;


constructor TTimeLabelThread.Create(ChessBoard: TGameChessBoard);
begin
  self.ChessBoard := ChessBoard;
  self.player_time[fcWhite] := ChessBoard.player_time[fcWhite];
  self.player_time[fcBlack] := ChessBoard.player_time[fcBlack];

  inherited Create(TRUE);
//Priority := tpNormal;
  FreeOnTerminate := TRUE;
  Resume;
end;
{$ENDIF}

////////////////////////////////////////////////////////////////////////////////
// TGameChessBoard

function TGameChessBoard.DoMove(const strMove: string): boolean;
begin
  Result := m_ChessBoard.DoMove(strMove);
  if (Result) then
  begin
    if (m_bFlashOnMove and (Mode = mGame)) then
      FFlashWindow;
  end;
end;


procedure TGameChessBoard.ShowTime(const c: TFigureColor);
var
  time_label: TLabel;
begin
  if c = fcWhite then time_label:= WhiteTimeLabel
    else time_label:= BlackTimeLabel;

  if unlimited_var[c] then
    begin
      time_label.Caption:= '';
      exit;
    end;

  if (TChessClock.IsZeitnot(player_time[c])) then
    time_label.Font.Color := ZEITNOT_COLOR
  else
    time_label.Font.Color := TIME_COLOR;

  time_label.Caption := TChessClock.ConvertToStr(player_time[c]);
end;


procedure TGameChessBoard.ResetMoveList;
begin
  m_ChessBoard.ResetMoveList;
end;


function TGameChessBoard.SetPosition(const strPosition: string): boolean;
begin
  Result := m_ChessBoard.SetPosition(strPosition);
  if (Result) then
    clock_color := PositionColor;
end;


function TGameChessBoard.GetPosition: string;
begin
  Result := m_ChessBoard.GetPosition;
end;


procedure TGameChessBoard.FormCreate(Sender: TObject);
begin
  m_iTimePanelInitialWidth := TimePanel.Width;
  m_iWhitePanelInitialLeft := WhitePanel.Left;
  m_iWhitePanelInitialWidth := WhitePanel.Width;
  m_iBlackPanelInitialLeft := BlackPanel.Left;
  m_iBlackPanelInitialWidth := BlackPanel.Width;

  m_TimeFont := TFont.Create;
  m_TimeFont.Assign(WhiteTimeLabel.Font);

  BlackFlagButton.Glyph := WhiteFlagButton.Glyph; // For size minimization

  TLocalizer.Instance.AddSubscriber(self);
  FLocalize;

  // Clock initialization
  FSetUnlimited(fcWhite, TRUE);
  FSetUnlimited(fcBlack, TRUE);
end;


constructor TGameChessBoard.Create(Owner: TComponent;
  AHandler: TGameChessBoardHandler = nil; const strPosBaseName: string = '');
begin
  FHandler := AHandler;
  inherited Create(Owner);
  FCreateChessBoard(strPosBaseName);
end;


procedure TGameChessBoard.FCreateChessBoard(const strPosBaseName: string);
begin
  m_ChessBoard := TChessBoard.Create(self, FChessBoardHandler);
  m_PosBaseChessBoardLayer := TPosBaseChessBoardLayer.Create(strPosBaseName);

  m_ChessBoard.AddLayer(m_PosBaseChessBoardLayer);

  with ChessBoardPanel do
    SetBounds(Left, Top, m_ChessBoard.ClientWidth, m_ChessBoard.ClientHeight);

  m_ChessBoard.BorderStyle := bsNone;
  m_ChessBoard.Align := alClient;
  m_ChessBoard.Parent := ChessBoardPanel;
  m_ChessBoard.Visible := TRUE;

  InitPosition;
end;


procedure TGameChessBoard.FDestroyChessBoard;
begin
  m_ChessBoard.RemoveLayer(m_PosBaseChessBoardLayer); // m_ChessBoard is destroyed by its parent
  FreeAndNil(m_PosBaseChessBoardLayer);
end;


procedure TGameChessBoard.FormDestroy(Sender: TObject);
begin
  TLocalizer.Instance.DeleteSubscriber(self);

  m_TimeFont.Free;

  FDestroyChessBoard;
end;


procedure TGameChessBoard.InitPosition;
begin
  m_ChessBoard.InitPosition;
end;


procedure TGameChessBoard.FSetMode(const Value: TMode);
begin
  m_ChessBoard.Mode := Value;
  if (Value <> mGame) then
  begin
    WhiteFlagButton.Visible := FALSE;
    BlackFlagButton.Visible := FALSE;
  end;
end;


function TGameChessBoard.FGetMode: TMode;
begin
  Result := m_ChessBoard.Mode;
end;


procedure TGameChessBoard.FSetTime(color: TFigureColor; const tm: TDateTime);
begin
  if (not Unlimited[color]) then
  begin
    if ((not auto_flag) and (not ViewGaming)) then
    begin
      case color of
        fcWhite:
          WhiteFlagButton.Visible := ((PlayerColor = fcBlack) and (tm = 0.0));
        fcBlack:
          BlackFlagButton.Visible := ((PlayerColor = fcWhite) and (tm = 0.0));
      end;
    end;
    player_time[color] := tm;
    ShowTime(color);
  end;
end;


function TGameChessBoard.FGetTime(color: TFigureColor): TDateTime;
begin
  Result:= player_time[color];
end;


procedure TGameChessBoard.GameTimerTimer(Sender: TObject);
begin
  if unlimited_var[clock_color] then
    begin
      GameTimer.Enabled := FALSE;
      exit;
    end;
  // TODO: cheating check
  player_time[clock_color] := player_time[clock_color] - (Now - past_time);
  if player_time[clock_color] <= 0.0 then
    begin
      player_time[clock_color] := 0.0;
      ShowTime(clock_color);
      if ((not auto_flag) and (PlayerColor <> clock_color) and (not ViewGaming)) then
      begin
        case clock_color of
          fcWhite:
            WhiteFlagButton.Visible := TRUE;
          fcBlack:
            BlackFlagButton.Visible := TRUE;
        end;
      end;
      if ((PlayerColor <> clock_color) and (Mode = mGame) and (auto_flag)) then
        FDoHandler(cbeTimeOut, self);
      GameTimer.Enabled := FALSE;
    end;
{$IFNDEF THREADED_CHESSCLOCK}
  ShowTime(clock_color);
{$ENDIF}

  past_time:= Now;
end;


procedure TGameChessBoard.FSetUnlimited(color: TFigureColor; const unl: boolean);
begin
  unlimited_var[color]:= unl;
  ShowTime(color);
end;


function TGameChessBoard.FGetUnlimited(color: TFigureColor): boolean;
begin
  Result := unlimited_var[color];
end;


procedure TGameChessBoard.SwitchClock(clock_color: TFigureColor);
begin
  self.clock_color := clock_color;
  if (not GameTimer.Enabled) then
  begin
    past_time := Now;
    GameTimer.Enabled := TRUE;
  end;
  if (Mode = mGame) then
    FDoHandler(cbeClockSwitched, self);
  ShowTime(clock_color);

{$IFDEF THREADED_CHESSCLOCK}
  if (not Assigned(TimeLabelThread)) then
    TimeLabelThread := TTimeLabelThread.Create(self);
{$ENDIF}
end;


procedure TGameChessBoard.FSetPlayerColor(const Value: TFigureColor);
begin
  m_ChessBoard.PlayerColor := Value;
end;


function TGameChessBoard.FGetPlayerColor: TFigureColor;
begin
  Result := m_ChessBoard.PlayerColor;
end;


procedure TGameChessBoard.StopClock;
begin
  GameTimer.Enabled := FALSE;
  WhiteFlagButton.Visible := FALSE;
  BlackFlagButton.Visible := FALSE;
end;


procedure TGameChessBoard.FormCanResize(Sender: TObject; var NewWidth,
  NewHeight: Integer; var Resize: Boolean);
var
  iNewChessBoardWidth, iNewChessBoardHeight: integer;
begin
  Resize := (m_ResizingType <> rtNo);
  if (not Resize) then
    exit;

  iNewChessBoardWidth := m_ChessBoard.Width + (NewWidth - self.Width);
  iNewChessBoardHeight := m_ChessBoard.Height + (NewHeight - self.Height);

  m_ChessBoard.FormCanResize(self, iNewChessBoardWidth, iNewChessBoardHeight, Resize);
  if (Resize) then
  begin
    NewWidth := self.Width + (iNewChessBoardWidth - m_ChessBoard.Width);
    NewHeight := self.Height + (iNewChessBoardHeight - m_ChessBoard.Height);
  end;
end;

procedure TGameChessBoard.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  if (not shuted) then
  begin
    FDoHandler(cbeExit, self);
    Action:= caNone;
  end
  else
    shuted := FALSE;
end;


procedure TGameChessBoard.Shut;
begin
  shuted:= TRUE;
  Close;
end;


procedure TGameChessBoard.PPRandom;
begin
  m_ChessBoard.PPRandom;
end;


procedure TGameChessBoard.TakeBack;
begin
  m_ChessBoard.TakeBack;
end;


function TGameChessBoard.FGetLastMoveHilighted: boolean;
begin
  Result := m_ChessBoard.LastMoveHilighted;
end;


procedure TGameChessBoard.FSetLastMoveHilighted(bValue: boolean);
begin
  m_ChessBoard.LastMoveHilighted := bValue;
end;


procedure TGameChessBoard.FSetCoordinatesShown(bValue: boolean);
begin
  m_ChessBoard.CoordinatesShown := bValue;
end;


function TGameChessBoard.FGetCoordinatesShown: boolean;
begin
  Result := m_ChessBoard.CoordinatesShown;
end;


function TGameChessBoard.NMoveDone: integer;
begin
  Result := m_ChessBoard.NMoveDone;
end;


procedure TGameChessBoard.FormKeyDown(Sender: TObject; var Key: Word;
  Shift: TShiftState);
begin
  FDoHandler(cbeKeyPressed, Pointer(Key), self);
end;


function TGameChessBoard.GetStayOnTop: boolean;
begin
  Result := (self.FormStyle = fsStayOnTop);
end;


procedure TGameChessBoard.FSetStayOnTop(onTop: boolean);
begin
  if (onTop) then
    self.FormStyle := fsStayOnTop
  else
    self.FormStyle := fsNormal;
end;


procedure TGameChessBoard.FormActivate(Sender: TObject);
begin
  FDoHandler(cbeActivate, self);
end;


procedure TGameChessBoard.WMMoving(var Msg: TWMMoving);
begin
  // TODO: it's possible to handle if form is outside of the screen
  FDoHandler(cbeFormMoving, Pointer(Msg.DragRect.Left - Left), Pointer(Msg.DragRect.Top - Top));
  inherited;
end;


procedure TGameChessBoard.WMSizing(var Msg: TMessage);
begin
  m_ChessBoard.Perform(Msg.Msg, Msg.WParam, Msg.LParam);

  case Msg.WParam of
    WMSZ_RIGHT, WMSZ_LEFT, WMSZ_BOTTOMRIGHT, WMSZ_TOPLEFT:
      m_ResizingType := rtHoriz;
    WMSZ_BOTTOM, WMSZ_TOP:
      m_ResizingType := rtVert;
  else
    begin
      m_ResizingType := rtNo;
      PRect(Msg.LParam).Left := Left;
      PRect(Msg.LParam).Top := Top;
    end;
  end; // case
end;


function TGameChessBoard.FGetPositionColor: TFigureColor;
begin
  Result := m_ChessBoard.PositionColor;
end;


procedure TGameChessBoard.FlagButtonClick(Sender: TObject);
begin
  if (Mode = mGame) then
    FDoHandler(cbeTimeOut, self);
end;


procedure TGameChessBoard.FSetAutoFlag(auto_flag: boolean);
begin
  self.auto_flag := auto_flag;
  if (auto_flag) then
  begin
    WhiteFlagButton.Visible := FALSE;
    BlackFlagButton.Visible := FALSE;
  end;
end;


procedure TGameChessBoard.TimePanelResize(Sender: TObject);
var
  rRatio: real;
begin
  // Adjust panels on TimePanel
  rRatio := TimePanel.Width / m_iTimePanelInitialWidth;
  WhitePanel.Left := Round(rRatio * m_iWhitePanelInitialLeft);
  WhitePanel.Width := m_iWhitePanelInitialWidth;
  BlackPanel.Left := Round(rRatio * m_iBlackPanelInitialLeft);
  BlackPanel.Width := m_iBlackPanelInitialWidth;


  WhiteTimeLabel.Font.Assign(m_TimeFont);
  BlackTimeLabel.Font.Assign(m_TimeFont);
  
  if (WhitePanel.Left + WhitePanel.Width < BlackPanel.Left) then
  begin
    WhiteLabel.Caption := WHITE_LONG_LABEL;
    BlackLabel.Caption := BLACK_LONG_LABEL;
  end
  else
  begin
    WhitePanel.Left := 4;
    WhitePanel.Width := TimePanel.Width div 2;
    BlackPanel.Left := TimePanel.Width div 2;
    BlackPanel.Width := TimePanel.Width div 2 - 4;

    WhiteLabel.Caption := WHITE_MEDIUM_LABEL;
    BlackLabel.Caption := BLACK_MEDIUM_LABEL;
  end;

  // Adjust color labels
  if ((WhiteTimeLabel.Left + WhiteTimeLabel.Width > WhitePanel.Width) or
      (BlackTimeLabel.Left + BlackTimeLabel.Width > BlackPanel.Width)) then
  begin
    WhiteLabel.Caption := WHITE_MEDIUM_LABEL;
    BlackLabel.Caption := BLACK_MEDIUM_LABEL;
    if ((WhiteTimeLabel.Left + WhiteTimeLabel.Width <= WhitePanel.Width) and
      (BlackTimeLabel.Left + BlackTimeLabel.Width <= BlackPanel.Width)) then
      exit; // TODO: a KLUDGE - make it nice!
  
    WhiteTimeLabel.Font.Size := WhiteTimeLabel.Font.Size - 4;
    BlackTimeLabel.Font.Size := BlackTimeLabel.Font.Size - 4;
    WhiteLabel.Caption := WHITE_SHORT_LABEL;
    BlackLabel.Caption := BLACK_SHORT_LABEL;
  end;
end;


procedure TGameChessBoard.FFlashWindow;
var
  flushWindowInfo: TFlashWInfo;
begin
 // Flash with taskbar
 flushWindowInfo.cbSize := SizeOf(flushWindowInfo);
 flushWindowInfo.hwnd := Application.Handle;
 flushWindowInfo.dwflags := FLASHW_TRAY; // FLASHW_ALL; //FLASHW_TRAY;
 flushWindowInfo.ucount := 3; // Flash times
 flushWindowInfo.dwtimeout := 0; // speed in msec, 0 - frequency of cursor flashing
 FlashWindowEx(flushWindowInfo);

 if (self.Focused) then
   exit;
 // Flash window
 flushWindowInfo.hwnd := self.Handle; // handle of the flashing window
 flushWindowInfo.dwflags := FLASHW_CAPTION; // FLASHW_TRAY; // FLASHW_ALL; //FLASHW_TRAY;
 FlashWindowEx(flushWindowInfo);
end;


procedure TGameChessBoard.FLocalize;
begin
  with TLocalizer.Instance do
  begin
    WHITE_LONG_LABEL := GetLabel(13);
    WHITE_MEDIUM_LABEL := GetLabel(14);
    WHITE_SHORT_LABEL := GetLabel(15);
    BLACK_LONG_LABEL := GetLabel(16);
    BLACK_MEDIUM_LABEL := GetLabel(17);
    BLACK_SHORT_LABEL := GetLabel(18);
  end;

  TimePanelResize(nil);
end;


procedure TGameChessBoard.WriteGameToBase(vGameResult: TGameResult);
begin
  m_PosBaseChessBoardLayer.WriteGameToBase(vGameResult);
end;


procedure TGameChessBoard.SetExternalBase(const strExtPosBaseName: string);
begin
  m_PosBaseChessBoardLayer.SetExternalBase(strExtPosBaseName);
end;


procedure TGameChessBoard.UnsetExternalBase;
begin
  m_PosBaseChessBoardLayer.UnsetExternalBase;
end;


function TGameChessBoard.FGetTrainingMode: boolean;
begin
  Result := m_PosBaseChessBoardLayer.TrainingMode;
end;


procedure TGameChessBoard.FSetTrainingMode(bValue: boolean);
begin
  m_PosBaseChessBoardLayer.TrainingMode := bValue;
end;


function TGameChessBoard.FGetUseUserBase: boolean;
begin
  Result := m_PosBaseChessBoardLayer.UseUserBase;
end;


procedure TGameChessBoard.FSetUseUserBase(bValue: boolean);
begin
  m_PosBaseChessBoardLayer.UseUserBase := bValue;
end;


function TGameChessBoard.FGetFlipped: boolean;
begin
  Result := m_ChessBoard.Flipped;
end;


procedure TGameChessBoard.FSetFlipped(bValue: boolean);
begin
  m_ChessBoard.Flipped := bValue;
end;


function TGameChessBoard.FGetAnimation: TAnimation;
begin
  Result := m_ChessBoard.Animation;
end;


procedure TGameChessBoard.FSetAnimation(const Value: TAnimation);
begin
  m_ChessBoard.Animation := Value;
end;


function TGameChessBoard.FGetViewGaming: boolean;
begin
  Result := m_ChessBoard.ViewGaming;
end;


procedure TGameChessBoard.FSetViewGaming(bValue: boolean);
begin
  m_ChessBoard.ViewGaming := bValue;
end;


procedure TGameChessBoard.FChessBoardHandler(e: TChessBoardEvent; d1: pointer = nil;
  d2: pointer = nil);
begin
  case e of
    ChessBoardUnit.cbeMate:
      FDoHandler(cbeMate, self);

    ChessBoardUnit.cbeStaleMate:
      FDoHandler(cbeStaleMate, self);

    ChessBoardUnit.cbeMoved:
    begin
      if ((Mode = mGame) and (PositionColor <> PlayerColor)) then
        FDoHandler(cbeMoved, d1, self);
      SwitchClock(PositionColor);      
    end;

    ChessBoardUnit.cbeMenu:
      FDoHandler(cbeMenu, self);
  end;
end;


procedure TGameChessBoard.FDoHandler(e: TGameChessBoardEvent; d1: pointer = nil;
  d2: pointer = nil);
begin
  if (Assigned(FHandler)) then
    FHandler(e, d1, d2);
end;

end.
