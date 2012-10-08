////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit ChessBoardUnit;

interface

uses
  Forms, ExtCtrls, Classes, Controls, Graphics, Types, Messages,
  //
  ChessRulesEngine, BitmapResUnit, PromotionUnit;

type
  TMode = (mView, mGame, mAnalyse, mEdit); // Board mode

  TAnimation = (aNo, aSlow, aQuick);

  TChessBoardEvent = (cbeMate, cbeStaleMate, cbeMoved, cbePosSet, cbeMenu);
  TChessBoardHandler = procedure(e: TChessBoardEvent;
                                 d1: pointer = nil; d2: pointer = nil) of object;

  TChessBoardLayerBase = class;

  TChessBoard = class(TForm, IChessRulesEngineable)
    PBoxBoard: TPaintBox;
    AnimateTimer: TTimer;

    procedure FormCreate(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure FormCanResize(Sender: TObject; var NewWidth,
      NewHeight: Integer; var Resize: Boolean);
    procedure FormResize(Sender: TObject);
    procedure AnimateTimerTimer(Sender: TObject);
    procedure PBoxBoardPaint(Sender: TObject);
    procedure PBoxBoardDragDrop(Sender, Source: TObject; X, Y: Integer);
    procedure PBoxBoardDragOver(Sender, Source: TObject; X, Y: Integer;
      State: TDragState; var Accept: Boolean);
    procedure PBoxBoardEndDrag(Sender, Target: TObject; X, Y: Integer);
    procedure PBoxBoardMouseDown(Sender: TObject;
      Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
    procedure PBoxBoardMouseMove(Sender: TObject; Shift: TShiftState; X, Y: Integer);
    procedure PBoxBoardMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure PBoxBoardStartDrag(Sender: TObject; var DragObject: TDragObject);

  private
    m_ChessRulesEngine: TChessRulesEngine;
    m_BitmapRes: TBitmapRes; // Manager for bitmaps

    FHandler: TChessBoardHandler;

    dx, dy: integer;  // Расстояние от курсора до верхнего левого угла
    x0, y0: integer; // Предыдущие координаты курсора
    _flipped: boolean; // Доска перевёрнута или нет
    m_bHilighted: boolean; // Hilight the move that is being done

    m_i0, m_j0: integer;
    m_fig: TFigure;

    m_Mode: TMode;
    m_bViewGaming: boolean;

    m_bmHiddenBoard: TBitmap;
    m_bmChessBoard: TBitmap;
    m_bmFigure: array[TFigure] of TBitmap;
    m_bmBuf: TBitmap;

    m_iSquareSize: integer; // Size of one chess board field

    m_animation: TAnimation; // Animation speed
    m_iAnimStep, m_iPrevAnimStep, m_iAnimStepsCount: integer;
    anim_dx, anim_dy: real; // Variables for animation of a dragged piece

    m_PlayerColor: TFigureColor; // Color of player client
    m_bDraggedMoved: boolean; // Flag for switching of dragging
    last_hilight: boolean; // Flag for hilighting of the last move done
    coord_show: boolean; // Flag for showing coordinates

    // Resizing
    m_ResizingType: (rtNo, rtHoriz, rtVert);
    m_iDeltaWidthHeight: integer;
    m_bDeltaWidthHeightFlag: boolean;

    m_PromotionForm: TPromotionForm;

    m_EditPiece: TFigure;

    m_iUpdateCounter: integer;

    m_lstLayers: TList;

    procedure HilightLastMove;
    procedure Evaluate;

    function FGetLastMove: PMoveAbs;
    property lastMove: PMoveAbs read FGetLastMove;

    function FGetPosition: PChessPosition;
    property Position: PChessPosition read FGetPosition;

    function AskPromotionFigure(FigureColor: TFigureColor): TFigureName;

    procedure FSetMode(const Value: TMode);

    function FDoMove(i, j: integer; prom_fig: TFigureName = K): boolean;
    procedure FOnAfterMoveDone;
    procedure FOnAfterSetPosition;

    procedure FAnimate(const i, j: integer); // Animates a disposition of a piece from (i0,j0) to (i,j)
    procedure FDoAnimationStep;
    procedure FEndAnimation;

    procedure FWhatSquare(const P: TPoint; var i: Integer; var j: Integer);

    procedure FSetPlayerColor(const Value: TFigureColor);
    procedure FCancelAnimationDragging; // Caneling of animation and dragging for trace removal after draw
    procedure FSetFlipped(Value: boolean); // Flips chess position
    procedure FSetCoordinatesShown(Value: boolean);
    procedure FSetLastMoveHilighted(Value: boolean);
    function FGetPositionsList: TList;
    function FGetPositionColor: TFigureColor;
    function FGetMoveNotationFormat: TMoveNotationFormat;
    procedure FSetMoveNotationFormat(Value: TMoveNotationFormat);
    function FGetFENFormat: boolean;
    procedure FSetFENFormat(bValue: boolean);

    procedure FDrawHiddenBoard;
    function FGetHiddenBoardCanvas: TCanvas;

    procedure FDrawBoard;    
    procedure FOnDrawLayerUpdate(const ADrawLayer: TChessBoardLayerBase);

    function FGetMovesOffset: integer;
    function FGetColorStarts: TFigureColor;    

    procedure WMSizing(var Msg: TMessage); message WM_SIZING;

    procedure FDoHandler(e: TChessBoardEvent; d1: pointer = nil; d2: pointer = nil);

    property SquareSize: integer read m_iSquareSize;
    property PositionsList: TList read FGetPositionsList;    

  public
    constructor Create(Owner: TComponent; AHandler: TChessBoardHandler = nil); reintroduce;

    function DoMove(const strMove: string): boolean;
    procedure ResetMoveList;
    function SetPosition(const strPosition: string): boolean;
    function GetPosition: string;
    procedure InitPosition;
    procedure PPRandom;
    procedure TakeBack;
    function NMoveDone: integer;
    function NPlysDone: integer;

    function IsMoveAnimating: boolean;

    procedure BeginUpdate;
    procedure EndUpdate;

    procedure AddLayer(const ALayer: TChessBoardLayerBase);
    procedure RemoveLayer(const ALayer: TChessBoardLayerBase);

    property PlayerColor: TFigureColor read m_PlayerColor write FSetPlayerColor;
    property Mode: TMode read m_Mode write FSetMode;
    property CoordinatesShown: boolean read coord_show write FSetCoordinatesShown;
    property Flipped: boolean read _flipped write FSetFlipped;
    property LastMoveHilighted: boolean read last_hilight write FSetLastMoveHilighted;
    property Animation: TAnimation read m_animation write m_animation;
    property ViewGaming: boolean read m_bViewGaming write m_bViewGaming;
    property PositionColor: TFigureColor read FGetPositionColor; // Whos move it is in the current position
    property MoveNotationFormat: TMoveNotationFormat
      read FGetMoveNotationFormat write FSetMoveNotationFormat;
    property MovesOffset: integer read FGetMovesOffset;
    property FENFormat: boolean read FGetFENFormat write FSetFENFormat;
    property EditPiece: TFigure read m_EditPiece write m_EditPiece;
  end;


  TChessBoardLayerBase = class
  private
    m_ChessBoard: TChessBoard;
    function FGetSquareSize: integer;
    function FGetCanvas: TCanvas;
    function FGetPosition: PChessPosition;
    function FGetPositionsList: TList;
  protected
    procedure RDraw; virtual; abstract;
    function RGetColorStarts: TFigureColor;

    procedure RDoUpdate;

    procedure ROnAfterMoveDone; virtual;
    procedure ROnAfterSetPosition; virtual;
    procedure ROnAfterModeSet(const OldValue, NewValue: TMode); virtual;
    procedure ROnResetMoveList; virtual;

    property ChessBoard: TChessBoard read m_ChessBoard write m_ChessBoard;
    property SquareSize: integer read FGetSquareSize;
    property Canvas: TCanvas read FGetCanvas;
    property Position: PChessPosition read FGetPosition;
    property PositionsList: TList read FGetPositionsList;
  end;

implementation

{$R *.dfm}

uses
  Math, SysUtils, Windows,
  //
  ChessBoardHeaderUnit;

const
  HILIGHT_WIDTH = 1;
  HILIGHT_COLOR: TColor = clRed;
  HILIGHT_LAST_MOVE_WIDTH = 1;
  HILIGHT_LAST_MOVE_COLOR: TColor = clBlue;
  ANIMATION_SLOW = 30; // Time of animation in frames >= 1
  ANIMATION_QUICK = 9;
  CHB_WIDTH = 4;

////////////////////////////////////////////////////////////////////////////////
// TChessBoard

constructor TChessBoard.Create(Owner: TComponent; AHandler: TChessBoardHandler = nil);
begin
  FHandler := AHandler;
  inherited Create(Owner);
end;


procedure TChessBoard.AnimateTimerTimer(Sender: TObject);
begin
  FDoAnimationStep;
  if (m_iAnimStep >= m_iAnimStepsCount) then
    FEndAnimation;
end;


procedure TChessBoard.FDoAnimationStep;
var
  iX, iY: integer;
  rect: TRect;
begin
  if (m_iAnimStep < m_iAnimStepsCount) then
  begin
    inc(m_iAnimStep);

    iX := round(x0 + anim_dx * m_iAnimStep);
    iY := round(y0 + anim_dy * m_iAnimStep);
    dx := iX - x0 - Round(anim_dx * m_iPrevAnimStep);
    dy := iY - y0 - Round(anim_dy * m_iPrevAnimStep);

    // Восстановить фрагмент на m_bmHiddenBoard
    m_bmHiddenBoard.Canvas.Draw(iX - dx, iY - dy, m_bmBuf);
    // Копировать новый фрагмент в буфер
    m_bmBuf.Canvas.CopyRect(Bounds(0, 0, m_iSquareSize, m_iSquareSize),
      m_bmHiddenBoard.Canvas, Bounds(iX, iY, m_iSquareSize, m_iSquareSize));
    // Нарисовать перетаскиваемую фигуру в новой позиции
    m_bmHiddenBoard.Canvas.Draw(iX, iY, m_bmFigure[m_fig]);
    // Перенести новый фрагмент на экран
    rect := Bounds(Min(iX - dx, iX), Min(iY - dy, iY),
      abs(dx) + m_iSquareSize, abs(dy) + m_iSquareSize);
    PBoxBoard.Canvas.CopyRect(rect, m_bmHiddenBoard.Canvas, rect);
  end;

  m_iPrevAnimStep := m_iAnimStep;
  
end;


procedure TChessBoard.FEndAnimation;
begin
  AnimateTimer.Enabled := FALSE;

  m_iAnimStep := m_iAnimStepsCount;

  FDrawBoard;
  HilightLastMove;
  Evaluate;
end;


procedure TChessBoard.FDrawBoard;
var
  i: integer;
begin
  if (csDestroying in ComponentState) then
    exit;

  if (m_iUpdateCounter > 0) then
    exit;

  FDrawHiddenBoard;

  for i := 0 to m_lstLayers.Count - 1 do
    TChessBoardLayerBase(m_lstLayers[i]).RDraw;

  PBoxBoardPaint(nil);
end;


procedure TChessBoard.HilightLastMove;
var
  i, j, l,
  _i0, _j0, x, y: integer;
begin
  if (not (m_Mode in [mGame, mAnalyse])) then
    exit;
    
  // Output the last move done
  if (last_hilight  and (lastMove.i0 <> 0)) then
  begin
    if (_flipped) then
    begin
      _i0 := 9 - lastMove.i0;
      _j0 := lastMove.j0;
      i := 9 - lastMove.i;
      j := lastMove.j;
    end
    else
    begin
      _i0 := lastMove.i0;
      _j0 := 9 - lastMove.j0;
      i := lastMove.i;
      j := 9 - lastMove.j;
    end;

    x := m_iSquareSize * (_i0 - 1) + CHB_X;
    y := m_iSquareSize * (_j0 - 1) + CHB_Y;
    m_bmHiddenBoard.Canvas.Pen.Color := HILIGHT_LAST_MOVE_COLOR;
    m_bmHiddenBoard.Canvas.Pen.Width := HILIGHT_LAST_MOVE_WIDTH;

    for l := 1 to 2 do
      with m_bmHiddenBoard.Canvas do
      begin
        MoveTo(x, y);
        LineTo(x + m_iSquareSize - 1, y);
        LineTo(x + m_iSquareSize - 1, y + m_iSquareSize - 1);
        LineTo(x, y + m_iSquareSize - 1);
        LineTo(x, y);

        x := m_iSquareSize * (i - 1) + CHB_X;
        y := m_iSquareSize * (j - 1) + CHB_Y;
      end;
    PBoxBoardPaint(nil);
  end;
end;


procedure TChessBoard.FDrawHiddenBoard;
var
  i, j: integer;
  x, y: integer;
begin
  if (not Assigned(m_bmHiddenBoard)) then
    exit;

  // Copy empty board to the hidden one
  with m_bmHiddenBoard do
  begin
    Canvas.CopyRect(Bounds(0,0, Width,Height), m_bmChessBoard.Canvas, Bounds(0,0, Width,Height));
  end;

  // Draw coordinates
  if (coord_show) then
    with m_bmHiddenBoard, m_bmHiddenBoard.Canvas do
    begin
      x:= CHB_X + m_iSquareSize div 2;
      y:= (m_bmHiddenBoard.Height + CHB_Y + 8 * m_iSquareSize + CHB_WIDTH) div 2;
      if _flipped then j := ord('h')
        else j:= ord('a');
      for i:= 1 to 8 do // буквы
        begin
          TextOut(x - TextWidth(chr(j)) div 2,
                  y + 1 - TextHeight(chr(j)) div 2 , chr(j));
          x := x + m_iSquareSize;
          if _flipped then dec(j)
            else inc(j);
        end;
      x:= (CHB_X - CHB_WIDTH) div 2;
      y:= CHB_Y + m_iSquareSize div 2;
      if _flipped then j:= ord('1')
        else j := ord('8');
      for i := 1 to 8 do // цифры
        begin
          TextOut(x - TextWidth(chr(j)) div 2,
                  y - TextHeight(chr(j)) div 2, chr(j));
          y:= y + m_iSquareSize;
          if _flipped then inc(j)
            else dec(j);
        end;
  end;

  // Draw pieces
  for i := 1 to 8 do
    for j := 1 to 8 do
      begin
        if ((Position.board[i,j] = ES)) then
          continue; // There's nothing to draw
        if not _flipped then // Загрузить нужную фигуру из ресурса и нарисовать
          m_bmHiddenBoard.Canvas.Draw(CHB_X + m_iSquareSize * (i-1),
                                    CHB_Y + m_iSquareSize * (8-j),
                                    m_bmFigure[Position.board[i,j]])
        else // Black is below
          m_bmHiddenBoard.Canvas.Draw(CHB_X + m_iSquareSize * (8-i),
                                    CHB_Y + m_iSquareSize * (j-1),
                                    m_bmFigure[Position.board[i,j]]);
      end;
end;


function TChessBoard.FGetHiddenBoardCanvas: TCanvas;
begin
  if (Assigned(m_bmHiddenBoard)) then
    Result := m_bmHiddenBoard.Canvas
  else
    Result := nil;
end;


procedure TChessBoard.Evaluate;
begin
  case m_ChessRulesEngine.GetEvaluation of
    evMate:
      FDoHandler(cbeMate, self);
    evStaleMate:
      FDoHandler(cbeStaleMate, self);
  end;
end;


procedure TChessBoard.PBoxBoardPaint(Sender: TObject);
begin
  PBoxBoard.Canvas.Draw(0, 0, m_bmHiddenBoard); // Draw hidden board on the form
//  PBoxBoard.Canvas.StretchDraw(Bounds(0, 0, PBoxBoard.Width, PBoxBoard.Height), m_bmHiddenBoard);
end;


function TChessBoard.FGetLastMove: PMoveAbs;
begin
  Result := m_ChessRulesEngine.lastMove;
end;


function TChessBoard.FGetPosition: PChessPosition;
begin
  Result := m_ChessRulesEngine.Position;
end;


function TChessBoard.AskPromotionFigure(FigureColor: TFigureColor): TFigureName;
var
  frmOwner: TForm;
begin
  if (Owner is TForm) then
    frmOwner := TForm(Owner)
  else
    frmOwner := self;

  if (Showing) then
  begin
    m_PromotionForm := TPromotionForm.Create(frmOwner, m_BitmapRes);
    try
      Result := m_PromotionForm.ShowPromotion(FigureColor);
    finally
      FreeAndNil(m_PromotionForm);
    end;
  end
  else
    Result := Q;
end;


procedure TChessBoard.FSetPlayerColor(const Value: TFigureColor);
begin
  FCancelAnimationDragging;
  m_PlayerColor := Value;
  if (m_PlayerColor = fcWhite) then
    FSetFlipped(FALSE)
  else // fcBlack
    FSetFlipped(TRUE);
end;


procedure TChessBoard.FCancelAnimationDragging;
begin
  // Cancel animation and dragging
  if (AnimateTimer.Enabled) then
  begin
    AnimateTimer.Enabled := FALSE;
    // iAnimStep := iAnimStepsCount;
    // AnimateTimerTimer(nil);
  end;
  
  if (PBoxBoard.Dragging) then
  begin
    m_bDraggedMoved := FALSE;
    PBoxBoard.EndDrag(FALSE);
  end;
end;


procedure TChessBoard.FSetFlipped(Value: boolean);
begin
  // TODO: ???
  _flipped := Value;
  FDrawBoard;
end;


procedure TChessBoard.FSetMode(const Value: TMode);
var
  OldMode: TMode;
  i: integer;
begin
  if (m_Mode = Value) then
    exit;

  OldMode := m_Mode;
  m_Mode := Value;

  if ((m_Mode in [mView, mEdit]) and (Assigned(m_PromotionForm))) then
    m_PromotionForm.Close;

  for i := 0 to m_lstLayers.Count - 1 do
    TChessBoardLayerBase(m_lstLayers[i]).ROnAfterModeSet(OldMode, m_Mode);

  FDrawBoard;
  HilightLastMove;
end;


procedure TChessBoard.FSetCoordinatesShown(Value: boolean);
begin
  coord_show := Value;
  FDrawBoard;
  HilightLastMove;
end;


procedure TChessBoard.FSetLastMoveHilighted(Value: boolean);
begin
  last_hilight := Value;
  FDrawBoard;
  HilightLastMove;
end;


function TChessBoard.DoMove(const strMove: string): boolean;
begin
  Result := FALSE;

  if (m_Mode = mEdit) then
    exit;

  // Animation canceling
  if (AnimateTimer.Enabled) then
    FEndAnimation;

  Result := m_ChessRulesEngine.DoMove(strMove);

  if (Result) then
  begin
    FOnAfterMoveDone;
    FAnimate(lastMove.i, lastMove.j);
  end;
end;


procedure TChessBoard.FOnAfterMoveDone;
var
  _fig: TFigure;
  strLastMove: string;
  i: integer;
begin
  m_i0 := lastMove.i0;
  m_j0 := lastMove.j0;

  _fig := Position.board[lastMove.i, lastMove.j];
  if (lastMove.prom_fig in [Q, R, B, N]) then
  begin
    if (_fig < ES) then
      m_fig := WP
    else
      m_fig := BP;
  end
  else
    m_fig := _fig;

  strLastMove := m_ChessRulesEngine.LastMoveStr;
  FDoHandler(cbeMoved, @strLastMove, self);

  if (m_Mode = mAnalyse) then
    m_PlayerColor := PositionColor;

  for i := 0 to m_lstLayers.Count - 1 do
    TChessBoardLayerBase(m_lstLayers[i]).ROnAfterMoveDone;
end;


procedure TChessBoard.FAnimate(const i, j: integer);
var
  x, y: integer;
begin
  if (not Showing) then
    exit;

  if ((m_i0 = 0) or (m_j0 = 0)) then
    exit;

  if (AnimateTimer.Enabled) then
  begin
    m_iAnimStep := m_iAnimStepsCount;
    exit;
  end;

  case animation of
    aNo:
      m_iAnimStepsCount := 1;
    aSlow:
      m_iAnimStepsCount := ANIMATION_SLOW;
    aQuick:
      m_iAnimStepsCount := ANIMATION_QUICK;
  end;

  if (_flipped) then
  begin
    x0 := (8 - m_i0) * m_iSquareSize + CHB_X;
    y0 := (m_j0 - 1) * m_iSquareSize + CHB_Y;
    x := (8 - i) * m_iSquareSize + CHB_X;
    y := (j - 1) * m_iSquareSize + CHB_Y;
  end
  else
  begin
    x0 := (m_i0 - 1) * m_iSquareSize + CHB_X;
    y0 := (8 - m_j0) * m_iSquareSize + CHB_Y;
    x := (i - 1) * m_iSquareSize + CHB_X;
    y := (8 - j) * m_iSquareSize + CHB_Y;
  end;

  anim_dx := (x - x0) / m_iAnimStepsCount;
  anim_dy := (y - y0) / m_iAnimStepsCount;

  m_iAnimStep := 0;
  m_iPrevAnimStep := m_iAnimStep;  

  // Copy image of the empty square to m_bmBuf
  m_bmBuf.Width := m_iSquareSize;
  m_bmBuf.Height := m_iSquareSize;
  if (((m_i0 + m_j0) and 1) <> 0) then
    m_bmBuf.Canvas.CopyRect(Bounds(0, 0, m_iSquareSize, m_iSquareSize),
      m_bmFigure[ES].Canvas, Bounds(0, 0, m_iSquareSize, m_iSquareSize))
  else
    m_bmBuf.Canvas.CopyRect(Bounds(0, 0, m_iSquareSize, m_iSquareSize),
      m_bmFigure[ES].Canvas, Bounds(m_iSquareSize, 0, m_iSquareSize, m_iSquareSize));

  AnimateTimer.Enabled := TRUE;
end;


procedure TChessBoard.ResetMoveList;
var
  i: integer;
begin
  m_ChessRulesEngine.ResetMoveList;

  for i := 0 to m_lstLayers.Count - 1 do
    TChessBoardLayerBase(m_lstLayers[i]).ROnResetMoveList;
end;


function TChessBoard.SetPosition(const strPosition: string): boolean;
begin
  Result := m_ChessRulesEngine.SetPosition(strPosition);
  if (Result) then
  begin
    FCancelAnimationDragging;
    FOnAfterSetPosition;
    FDrawBoard;
  end;
end;


function TChessBoard.GetPosition: string;
begin
  Result := m_ChessRulesEngine.GetPosition;
end;


procedure TChessBoard.FOnAfterSetPosition;
var
  strPosition: string;
  i: integer;
begin
  case m_Mode of
    mAnalyse:
      m_PlayerColor := PositionColor;

    mEdit:
      ResetMoveList;
  end;

  m_i0 := 0;
  m_j0 := 0;

  strPosition := GetPosition;
  FDoHandler(cbePosSet, @strPosition, self);

  for i := 0 to m_lstLayers.Count - 1 do
    TChessBoardLayerBase(m_lstLayers[i]).ROnAfterSetPosition;
end;


procedure TChessBoard.FormCreate(Sender: TObject);
begin
  // m_iDeltaWidthHeight := Width - Height;

  m_BitmapRes := TBitmapRes.Create(Size(PBoxBoard.Width, PBoxBoard.Height));

  coord_show:= TRUE;
  last_hilight:= FALSE;
  m_animation := aQuick;

  m_ChessRulesEngine := TChessRulesEngine.Create(self);
  m_lstLayers := TList.Create;
end;


procedure TChessBoard.FormDestroy(Sender: TObject);
var
  _fig: TFigure;
  i: integer;
begin
  for i := m_lstLayers.Count - 1 downto 0 do
    RemoveLayer(m_lstLayers[i]);
  m_lstLayers.Free;

  m_ChessRulesEngine.Free;

  m_bmHiddenBoard.Free;
  m_bmBuf.Free;

  for _fig := Low(TFigure) to High(TFigure) do
    m_bmFigure[_fig].Free;
  m_bmChessBoard.Free;

  m_BitmapRes.Free;
end;


procedure TChessBoard.PBoxBoardDragDrop(Sender, Source: TObject; X,
  Y: Integer);
var
  i, j: Integer;
begin
  FWhatSquare(Point(X, Y), i, j);
  case m_Mode of
    mGame, mAnalyse:
    begin
      if (FDoMove(i, j)) then
        m_bDraggedMoved := TRUE;
    end;

    mEdit:
      m_bDraggedMoved := TRUE;
  end;
end;


procedure TChessBoard.FWhatSquare(const P: TPoint;
  var i: Integer; var j: Integer);
begin
  with P do
  begin
    i := (X - CHB_X + m_iSquareSize) div m_iSquareSize;
    j := 8 - (Y - CHB_Y) div m_iSquareSize;
    if (_flipped) then
    begin
      i := 9 - i;
      j := 9 - j;
    end;
  end;
end;


function TChessBoard.FDoMove(i, j: integer; prom_fig: TFigureName = K): boolean;
begin
  Result := m_ChessRulesEngine.DoMove(m_i0, m_j0, i, j, prom_fig);
  if (Result) then
    FOnAfterMoveDone;
end;


procedure TChessBoard.PBoxBoardDragOver(Sender, Source: TObject; X,
  Y: Integer; State: TDragState; var Accept: Boolean);
var
  rect: TRect;
  i, j: integer;
begin
  case State of
    dsDragEnter:
      m_bHilighted := FALSE;

    dsDragMove:
      begin
        // Repaint a fragment on m_bmHiddenBoard
        m_bmHiddenBoard.Canvas.Draw(x0 - dx, y0 - dy, m_bmBuf);
        // Copy new fragment to the buffer
        m_bmBuf.Canvas.CopyRect(Bounds(0, 0, m_iSquareSize, m_iSquareSize),
          m_bmHiddenBoard.Canvas, Bounds(X - dx, Y - dy, m_iSquareSize, m_iSquareSize));
        // Draw the dragging piece in a new position
        m_bmHiddenBoard.Canvas.Draw(X - dx, Y - dy, m_bmFigure[m_fig]);
        // Copy the new fragment to the screen
        rect:= Bounds(Min(x0,X) - dx, Min(y0, Y) - dy,
          abs(X - x0) + m_iSquareSize, abs(Y - y0) + m_iSquareSize);
        PBoxBoard.Canvas.CopyRect(rect, m_bmHiddenBoard.Canvas, rect);

        x0 := X;
        y0 := Y;

        FWhatSquare(Point(X,Y), i, j);

        Accept := ((i in [1..8]) and (j in [1..8]));
      end;
  end;
end;


procedure TChessBoard.PBoxBoardEndDrag(Sender, Target: TObject; X, Y: Integer);
var
  i, j: integer;
  bRes: boolean;
begin
  case m_Mode of
    mGame, mAnalyse:
    begin
      if (m_bHilighted) then
      begin
        with m_bmHiddenBoard.Canvas do
        begin
          Pen.Color:= HILIGHT_COLOR;
          Pen.Width := HILIGHT_WIDTH;
          x0:= x0 - dx;
          y0:= y0 - dy;
          MoveTo(x0,y0);
          LineTo(x0 + m_iSquareSize - 1, y0);
          LineTo(x0 + m_iSquareSize - 1, y0 + m_iSquareSize - 1);
          LineTo(x0, y0 + m_iSquareSize - 1);
          LineTo(x0, y0);

          PBoxBoardPaint(nil);
        end;
      end
      else
      begin
        if (AnimateTimer.Enabled) then
          AnimateTimer.Enabled := FALSE;
        FDrawBoard;
        if (m_bDraggedMoved) then
        begin
          HilightLastMove;
          Evaluate;
          m_bDraggedMoved := FALSE;
        end;
      end;
    end;

    mEdit:
    begin
      if (m_bDraggedMoved) then
      begin
        FWhatSquare(Point(X, Y), i, j);
        bRes := (((i <> m_i0) or (j <> m_j0)) and  Position.SetPiece(i, j, m_fig));
      end
      else
        bRes := TRUE;

      if (bRes) then
      begin
        Position.SetPiece(m_i0, m_j0, ES);
        FOnAfterSetPosition;
      end;

      FDrawBoard;      
    end;
  end; // case
end;


procedure TChessBoard.PBoxBoardMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
var
  i, j: Integer;
  f: TFigure;
begin
  if (Button <> mbLeft) then
    exit;

  FWhatSquare(Point(X, Y), i, j);
  if (not ((i in [1..8]) and (j in [1..8]))) then
    exit;

  m_bDraggedMoved := FALSE;

  f := Position.board[i,j];

  case m_Mode of
    mGame, mAnalyse:
    begin
      if (m_bViewGaming) then
        exit;
      if ((Position.color <> m_PlayerColor) or
          (((Position.color <> fcWhite) or (f >= ES)) and
           ((Position.color <> fcBlack) or (f <= ES)))) then
        exit;

      if ((i = m_i0) and (j = m_j0)) then
        m_bHilighted := (m_bHilighted xor TRUE)
      else
        m_bHilighted := TRUE;
    end;

    mEdit:
    begin
      if (f = ES) then
        exit;    
    end;

  else
    exit;
  end;

  if (m_iAnimStep < m_iAnimStepsCount) then
    FEndAnimation;

  m_fig := f;
  m_i0 := i;
  m_j0 := j;

  dx := (X - CHB_X) mod m_iSquareSize;
  dy := (Y - CHB_Y) mod m_iSquareSize;
  x0 := X;
  y0 := Y;

  m_bDraggedMoved := TRUE;
  PBoxBoard.BeginDrag(FALSE);
end;


procedure TChessBoard.PBoxBoardMouseMove(Sender: TObject; Shift: TShiftState;
  X, Y: Integer);
var
  f: TFigure;
  i,j: Integer;
begin
  FWhatSquare(Point(X,Y), i,j);
  if (not ((i in [1..8]) and (j in [1..8]))) then
  begin
    PBoxBoard.Cursor:= crDefault;
    exit;
  end;

  f := Position.board[i,j];

  case m_Mode of
    mGame, mAnalyse:
    begin
      if (m_bViewGaming) then
        exit;

      if (m_PlayerColor = Position.color) and
         (((Position.color = fcWhite) and (f < ES)) or
          ((Position.color = fcBlack) and (f > ES))) then
        PBoxBoard.Cursor:= crHandPoint
      else
        PBoxBoard.Cursor:= crDefault;
    end;

    mEdit:
    begin
      if (f <> ES) then
        PBoxBoard.Cursor:= crHandPoint
      else
        PBoxBoard.Cursor:= crDefault;
    end;

  else
    PBoxBoard.Cursor := crDefault;
  end;
end;


function TChessBoard.FGetPositionsList: TList;
begin
  Result := m_ChessRulesEngine.PositionsList;
end;


function TChessBoard.FGetColorStarts: TFigureColor;
begin
  Result := m_ChessRulesEngine.GetColorStarts;
end;


procedure TChessBoard.PBoxBoardMouseUp(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
var
  i, j: integer;
begin
  case Button of
    mbLeft:
    begin
      case m_Mode of
        mGame, mAnalyse:
        begin
          if (not m_bHilighted) then
            exit;
          FWhatSquare(Point(X, Y), i, j);
          if (m_bDraggedMoved) then
            FDrawBoard
          else
          begin
            m_bHilighted := FALSE;
            if (FDoMove(i, j)) then
              FAnimate(i, j)
            else
              FDrawBoard;
          end;
        end;

        mEdit:
        begin
          if (m_bDraggedMoved) then
            exit;
          // Assert(empty field)
          FWhatSquare(Point(X, Y), i, j);
          if (Position.SetPiece(i, j, m_EditPiece)) then
          begin
            FOnAfterSetPosition;
            FDrawBoard;
          end;
        end;

      end; // case
    end;

    mbRight:
    begin
      FDoHandler(cbeMenu, self);
    end;

  end;
end;


procedure TChessBoard.PBoxBoardStartDrag(Sender: TObject;
  var DragObject: TDragObject);
begin
  // Copy image of an empty square to m_bmBuf
  m_bmBuf.Width := m_iSquareSize;
  m_bmBuf.Height:= m_iSquareSize;
  if (((m_i0 + m_j0) and 1) <> 0) then
      m_bmBuf.Canvas.CopyRect(Bounds(0,0, m_iSquareSize, m_iSquareSize),
        m_bmFigure[ES].Canvas, Bounds(0,0, m_iSquareSize, m_iSquareSize))
  else
    m_bmBuf.Canvas.CopyRect(Bounds(0,0, m_iSquareSize, m_iSquareSize),
      m_bmFigure[ES].Canvas, Bounds(m_iSquareSize,0, m_iSquareSize, m_iSquareSize));

  m_bDraggedMoved := FALSE;
end;


procedure TChessBoard.InitPosition;
begin
  m_ChessRulesEngine.InitNewGame;

  FCancelAnimationDragging;
  FOnAfterSetPosition;

  FDrawBoard;
end;


procedure TChessBoard.PPRandom;
begin
  m_ChessRulesEngine.InitNewPPRandomGame;

  FCancelAnimationDragging;
  FOnAfterSetPosition;
  
  FDrawBoard;
end;


procedure TChessBoard.TakeBack;
begin
  if (m_Mode = mEdit) then
    exit;

  if (not m_ChessRulesEngine.TakeBack) then
    exit;

  FOnAfterSetPosition;
  // TODO: animation
  FDrawBoard;
end;


function TChessBoard.NMoveDone: integer;
begin
  Result := m_ChessRulesEngine.NMovesDone;
end;


function TChessBoard.NPlysDone: integer;
begin
  Result := m_ChessRulesEngine.NPlysDone;
end;


function TChessBoard.FGetMovesOffset: integer;
begin
  Result := m_ChessRulesEngine.MovesOffset;
end;


function TChessBoard.FGetPositionColor: TFigureColor;
begin
  Result := Position.color;
end;


procedure TChessBoard.FormCanResize(Sender: TObject; var NewWidth,
  NewHeight: Integer; var Resize: Boolean);
var
  NewBoardSize: TSize;
begin
  if (not m_bDeltaWidthHeightFlag) then
  begin
    m_iDeltaWidthHeight := Width - Height;
    m_bDeltaWidthHeightFlag := TRUE;
  end;

  Resize := (m_ResizingType <> rtNo);
  if (not Resize) then
    exit;

  if (m_ResizingType = rtVert) then
    NewWidth := NewHeight + m_iDeltaWidthHeight
  else // rtHoriz
    NewHeight := NewWidth - m_iDeltaWidthHeight;

  NewBoardSize := m_BitmapRes.GetOptimalBoardSize(
    Size(PBoxBoard.Width + (NewWidth - Width), PBoxBoard.Height + (NewHeight - Height)));

  Resize := (NewBoardSize.cx > 0) and (NewBoardSize.cy > 0) and
    ((NewBoardSize.cx <> PBoxBoard.Width) or (NewBoardSize.cy <> PBoxBoard.Height));
  if (Resize) then
  begin
    NewWidth := Width + (NewBoardSize.cx - PBoxBoard.Width);
    NewHeight := Height + (NewBoardSize.cy - PBoxBoard.Height);
  end;
end;


procedure TChessBoard.FormResize(Sender: TObject);
var
  _fig: TFigure;
begin
  FreeAndNil(m_bmChessBoard);
  m_BitmapRes.CreateBoardBitmap(Size(PBoxBoard.Width, PBoxBoard.Height), self.Color,
    m_bmChessBoard);
  m_iSquareSize := m_BitmapRes.SquareSize;

  for _fig := Low(TFigure) to High(TFigure) do
  begin
    FreeAndNil(m_bmFigure[_fig]);
    m_BitmapRes.CreateFigureBitmap(_fig, m_bmFigure[_fig]);
  end;

  // Graphics initialization
  if (not Assigned(m_bmHiddenBoard)) then
  begin
    m_bmHiddenBoard := Graphics.TBitmap.Create;
    m_bmHiddenBoard.Palette := m_bmChessBoard.Palette;
    m_bmHiddenBoard.Canvas.Font := PBoxBoard.Font; // Характеристики шрифта координат задаются в инспекторе
    m_bmHiddenBoard.Canvas.Brush.Style := bsClear;
  end;
  m_bmHiddenBoard.Width := m_bmChessBoard.Width;
  m_bmHiddenBoard.Height := m_bmChessBoard.Height;

  if (not Assigned(m_bmBuf)) then
  begin
    m_bmBuf := Graphics.TBitmap.Create;
    m_bmBuf.Palette:= m_bmChessBoard.Palette;
  end;

  FDrawBoard;
end;


procedure TChessBoard.WMSizing(var Msg: TMessage);
begin
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


procedure TChessBoard.FDoHandler(e: TChessBoardEvent; d1: pointer = nil; d2: pointer = nil);
begin
  if (Assigned(FHandler)) then
    FHandler(e, d1, d2);
end;


function TChessBoard.FGetMoveNotationFormat: TMoveNotationFormat;
begin
  Result := m_ChessRulesEngine.MoveNotationFormat;
end;


procedure TChessBoard.FSetMoveNotationFormat(Value: TMoveNotationFormat);
begin
  m_ChessRulesEngine.MoveNotationFormat := Value;
end;


function TChessBoard.FGetFENFormat: boolean;
begin
  Result := m_ChessRulesEngine.FENFormat;
end;


procedure TChessBoard.FSetFENFormat(bValue: boolean);
begin
  m_ChessRulesEngine.FENFormat := bValue;
end;


procedure TChessBoard.BeginUpdate;
begin
  inc(m_iUpdateCounter);
end;


procedure TChessBoard.EndUpdate;
begin
  if (m_iUpdateCounter > 0) then
  begin
    dec(m_iUpdateCounter);
    if (m_iUpdateCounter = 0) then
      FDrawBoard;
  end;
end;


procedure TChessBoard.FOnDrawLayerUpdate(const ADrawLayer: TChessBoardLayerBase);
begin
  if (not AnimateTimer.Enabled) then
    FDrawBoard;
end;


procedure TChessBoard.AddLayer(const ALayer: TChessBoardLayerBase);
begin
  if (m_lstLayers.IndexOf(ALayer) >= 0) then
    exit;

  ALayer.ChessBoard := self;
  m_lstLayers.Add(ALayer);

  FOnDrawLayerUpdate(ALayer);
end;


procedure TChessBoard.RemoveLayer(const ALayer: TChessBoardLayerBase);
begin
  if (m_lstLayers.Remove(ALayer) >= 0) then
  begin
    ALayer.ChessBoard := nil;

    FOnDrawLayerUpdate(ALayer);
  end;
end;


function TChessBoard.IsMoveAnimating: boolean;
begin
  Result := AnimateTimer.Enabled; 
end;

////////////////////////////////////////////////////////////////////////////////
// TChessBoardDrawBase

procedure TChessBoardLayerBase.RDoUpdate;
begin
  if (Assigned(m_ChessBoard)) then
    m_ChessBoard.FOnDrawLayerUpdate(self);
end;


function TChessBoardLayerBase.FGetSquareSize: integer;
begin
  if (Assigned(m_ChessBoard)) then
    Result := m_ChessBoard.SquareSize
  else
    Result := 0;  
end;


function TChessBoardLayerBase.FGetCanvas: TCanvas;
begin
  if (Assigned(m_ChessBoard)) then
    Result := m_ChessBoard.FGetHiddenBoardCanvas
  else
    Result := nil;
end;


function TChessBoardLayerBase.FGetPosition: PChessPosition;
begin
  if (Assigned(m_ChessBoard)) then
    Result := m_ChessBoard.Position
  else
    Result := nil;
end;


function TChessBoardLayerBase.RGetColorStarts: TFigureColor;
begin
  if (Assigned(m_ChessBoard)) then
    Result := m_ChessBoard.FGetColorStarts
  else
    Result := fcWhite;
end;


function TChessBoardLayerBase.FGetPositionsList: TList;
begin
  if (Assigned(m_ChessBoard)) then
    Result := m_ChessBoard.PositionsList
  else
    Result := nil;
end;


procedure TChessBoardLayerBase.ROnAfterMoveDone;
begin
end;


procedure TChessBoardLayerBase.ROnAfterSetPosition;
begin
end;


procedure TChessBoardLayerBase.ROnAfterModeSet(const OldValue, NewValue: TMode);
begin
end;


procedure TChessBoardLayerBase.ROnResetMoveList;
begin
end;

end.
