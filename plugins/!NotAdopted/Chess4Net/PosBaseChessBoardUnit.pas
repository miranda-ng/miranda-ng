unit PosBaseChessBoardUnit;

interface

uses
  Classes, PosBaseUnit, ChessBoardHeaderUnit, ChessRulesEngine, ChessBoardUnit;

type
  TGameResult = (grWin, grWinTime, grDraw, grLost, grLostTime);

  // Расширение TChessBoard базой данных позиций
  TPosBaseChessBoard = class(TChessBoard)
  private
    _bUseUserBase: boolean;
    _lstMovePrior: TList;
    _oPosBase, _oExtPosBase: TPosBase;
    _bTrainingMode: boolean;
    _sPosBaseName, _sExtPosBaseName: string;
    procedure FSetTrainingMode(vbTrainingMode: boolean);
    procedure FUseUserBase(vbUseUserBase: boolean);
    procedure FReadFromBase;
    procedure FWriteGameToBase;

  protected
    procedure ROnAfterMoveDone; override;
    procedure ROnAfterSetPosition; override;
    procedure RDrawHiddenBoard; override;

  public
    procedure WriteGameToBase(vGameResult: TGameResult);
    procedure SetExternalBase(const vsExtPosBaseName: string);
    procedure UnsetExternalBase;
    constructor Create(voOwner: TComponent; vfHandler: TChessBoardHandler; const vsPosBaseName: string);
    destructor Destroy; override;
    property pTrainingMode: boolean read _bTrainingMode write FSetTrainingMode;
    property pUseUserBase: boolean read _bUseUserBase write FUseUserBase;
    procedure PPRandom; reintroduce;
  end;

implementation

uses
  SysUtils, Graphics;

type
  TPrior = (mpNo, mpHigh, mpMid, mpLow);

  PMovePrior = ^TMovePrior;
  TMovePrior = record
    move: TMoveAbs;
    prior: TPrior;
  end;

  TPosBaseOperator = class(TThread)
  private
    _enuOperation: (opRead, opWrite);
    _oChessBoard: TPosBaseChessBoard;
    _bHidden: boolean;
  protected
    procedure Execute; override;
  public
    constructor CreateRead(voChessBoard: TPosBaseChessBoard; vbHidden: boolean; vbFreeOnTerminate: boolean = TRUE);
    constructor CreateWrite(voChessBoard: TPosBaseChessBoard);
  end;

var
  gameResult: TGameResult; // не нитебезопасно
  gameID: word; // используется при записи уникальных позиций (не нитебезопасно)

const
  NUM_PRIORITIES = 3; // максимальное количество приоритетов
{$IFDEF RESTRICT_TRAINING_DB}
  MAX_PLY_TO_BASE = 60;
{$ELSE}
  MAX_PLY_TO_BASE = -1; // в базу сохраняется вся игра полностью
{$ENDIF}

{------------- TPosBaseChessBoard --------------}

constructor TPosBaseChessBoard.Create(voOwner: TComponent; vfHandler: TChessBoardHandler; const vsPosBaseName: string);
begin
  inherited Create(voOwner, vfHandler);

  _bUseUserBase := TRUE;
  _sPosBaseName := vsPosBaseName;
  _lstMovePrior := TList.Create;
end;


destructor TPosBaseChessBoard.Destroy;
var
  i: integer;
begin
  for i := 0 to _lstMovePrior.Count - 1 do
    dispose(_lstMovePrior[i]);
  _lstMovePrior.Free;

  pTrainingMode := FALSE;

  inherited;
end;


procedure Reestimate(lstMoveEsts: TList; viRec: integer);
var
  est: SmallInt;
  id: word;
begin
  id := LongWord(lstMoveEsts[viRec]) shr 16;
  if id = gameID then
    exit; // позиция дублируется в рамках одной партии

  est := SmallInt(lstMoveEsts[viRec]);
  case gameResult of
    grWin:     inc(est, 2);
    grWinTime: inc(est);
    grDraw:       ;
    grLost:     dec(est, 2);
    grLostTime: dec(est);
  end;
  lstMoveEsts[viRec] := Pointer((gameID shl 16) or Word(est));
end;


procedure TPosBaseChessBoard.FSetTrainingMode(vbTrainingMode: boolean);
begin
  if _bTrainingMode = vbTrainingMode then
    exit;

  _bTrainingMode := vbTrainingMode;
  try
    if _bTrainingMode then
      begin
        _oPosBase := TPosBase.Create(_sPosBaseName, Reestimate);
        if _sExtPosBaseName <> '' then
          _oExtPosBase := TPosBase.Create(_sExtPosBaseName);
        TPosBaseOperator.CreateRead(self, FALSE);
      end
    else
      begin
        FreeAndNil(_oPosBase);
        FreeAndNil(_oExtPosBase);
      end;
  except
    on Exception do
      begin
        FreeAndNil(_oPosBase);
        FreeAndNil(_oExtPosBase);
        _bTrainingMode := FALSE;
      end;
  end;
end;


procedure TPosBaseChessBoard.FUseUserBase(vbUseUserBase: boolean);
begin
  if _bUseUserBase = vbUseUserBase then
    exit;
  _bUseUserBase := vbUseUserBase;
  TPosBaseOperator.CreateRead(self, FALSE);
end;


procedure TPosBaseChessBoard.RDrawHiddenBoard;
const
  ARROW_END_LENGTH = 10; // в пикселях
  ARROW_END_ANGLE = 15 * (Pi / 180); // угол концов стрелки
  ARROW_INDENT = 7;

  HIGH_ARROW_COLOR = clRed;
  HIGH_ARROW_WIDTH = 2;
  MID_ARROW_COLOR =  clTeal;
  MID_ARROW_WIDTH = 2;
  LOW_ARROW_COLOR = clSkyBlue;
  LOW_ARROW_WIDTH = 1;

var
  i, x0, y0, x, y: integer;
  xa, ya, ca, sa: double;
  move: TMoveAbs;
begin
  if (not Assigned(bmHiddenBoard)) then
    exit;

  inherited;

  if not _bTrainingMode or (Mode <> mGame) or (PlayerColor <> PositionColor)  then
    exit;

  bmHiddenBoard.Canvas.Pen.Style := psSolid;

  for i := 0 to _lstMovePrior.Count - 1 do
    begin
      case PMovePrior(_lstMovePrior[i]).prior of
        mpNo: continue;
        mpHigh:
          begin
            bmHiddenBoard.Canvas.Pen.Color := HIGH_ARROW_COLOR;
            bmHiddenBoard.Canvas.Pen.Width := HIGH_ARROW_WIDTH;
          end;
        mpMid:
          begin
            bmHiddenBoard.Canvas.Pen.Color := MID_ARROW_COLOR;
            bmHiddenBoard.Canvas.Pen.Width := MID_ARROW_WIDTH;
          end;
        mpLow:
          begin
            bmHiddenBoard.Canvas.Pen.Color := LOW_ARROW_COLOR;
            bmHiddenBoard.Canvas.Pen.Width := LOW_ARROW_WIDTH;
          end;
      end;
       move := PMovePrior(_lstMovePrior[i]).move;
      if not flipped then
        begin
          x0 := CHB_X + iSquareSize * (move.i0 - 1) + (iSquareSize div 2);
          y0 := CHB_Y + iSquareSize * (8 - move.j0) + (iSquareSize div 2);
          x := CHB_X + iSquareSize * (move.i - 1) + (iSquareSize div 2);
          y := CHB_Y + iSquareSize * (8 - move.j) + (iSquareSize div 2);
        end
      else
        begin
          x0 := CHB_X + iSquareSize * (8 - move.i0) + (iSquareSize div 2);
          y0 := CHB_Y + iSquareSize * (move.j0 - 1) + (iSquareSize div 2);
          x := CHB_X + iSquareSize * (8 - move.i) + (iSquareSize div 2);
          y := CHB_Y + iSquareSize * (move.j - 1) + (iSquareSize div 2);
        end;

      // Рисование стрелки
      ca := (x - x0) / sqrt(sqr(x - x0) + sqr(y - y0));
      sa := (y - y0) / sqrt(sqr(x - x0) + sqr(y - y0));
      x0 := x0 + Round(ARROW_INDENT * ca);
      y0 := y0 + Round(ARROW_INDENT * sa);
      x := x - Round(ARROW_INDENT * ca);
      y := y - Round(ARROW_INDENT * sa);

      bmHiddenBoard.Canvas.MoveTo(x0, y0);
      bmHiddenBoard.Canvas.LineTo(x, y);

      xa := x + (-ARROW_END_LENGTH * cos(ARROW_END_ANGLE)) * ca -
                (ARROW_END_LENGTH * sin(ARROW_END_ANGLE)) * sa;
      ya := y + (-ARROW_END_LENGTH * cos(ARROW_END_ANGLE)) * sa +
                (ARROW_END_LENGTH * sin(ARROW_END_ANGLE)) * ca;

      bmHiddenBoard.Canvas.LineTo(Round(xa), Round(ya));

      xa := x + (-ARROW_END_LENGTH * cos(ARROW_END_ANGLE)) * ca -
                (-ARROW_END_LENGTH * sin(ARROW_END_ANGLE)) * sa;
      ya := y + (-ARROW_END_LENGTH * cos(ARROW_END_ANGLE)) * sa +
                (-ARROW_END_LENGTH * sin(ARROW_END_ANGLE)) * ca;

      bmHiddenBoard.Canvas.MoveTo(x, y);
      bmHiddenBoard.Canvas.LineTo(Round(xa), Round(ya));
    end;

end;


procedure TPosBaseChessBoard.ROnAfterMoveDone;
begin
  inherited;
  if (_bTrainingMode) then
  begin
    if (PlayerColor = PositionColor) then
      TPosBaseOperator.CreateRead(self, TRUE) // чтение из базы и вывод на скрытую доску
  end;
end;


procedure TPosBaseChessBoard.ROnAfterSetPosition;
var
  PosBaseOperator: TPosBaseOperator;
begin
  if (_bTrainingMode) then
  begin
    PosBaseOperator := TPosBaseOperator.CreateRead(self, FALSE, FALSE); // чтение из базы и вывод на скрытую доску
    PosBaseOperator.WaitFor;
    PosBaseOperator.Free;
  end;
end;


procedure TPosBaseChessBoard.SetExternalBase(const vsExtPosBaseName: string);
begin
  if _bTrainingMode then
    begin
      if _sExtPosBaseName = vsExtPosBaseName then
        exit;
      FreeAndNil(_oExtPosBase);
      _oExtPosBase := TPosBase.Create(vsExtPosBaseName);
      TPosBaseOperator.CreateRead(self, FALSE);
    end;
  _sExtPosBaseName := vsExtPosBaseName;
end;


function EstComape(item1, item2: pointer): integer;
begin
  Result := SmallInt(PMoveEst(item2).estimate and $FFFF) - SmallInt(PMoveEst(item1).estimate and $FFFF);
end;


procedure TPosBaseChessBoard.FReadFromBase;

  procedure ClasterMoves(var rlstMove: TList); // кластеризация ходов
  var
    i, j, num_clast, i_min, j_min, curr_assoc: integer;
    modus_min: double;
    clastWeights: array of record
      grav: double;
      assoc: integer;
    end;
    mp: PMovePrior;
    p: TPrior;
  begin
    if rlstMove.Count = 0 then
      exit;

    rlstMove.Sort(EstComape);
    SetLength(clastWeights, rlstMove.Count);

    num_clast := rlstMove.Count;
    for i := 0 to num_clast - 1 do
      begin
        clastWeights[i].assoc := i + 1;
        clastWeights[i].grav := SmallInt(PMoveEst(rlstMove[i]).estimate and $FFFF);
      end;

    repeat
      i_min := 0;
      j_min := 0;
      modus_min := $7FFF; // $7FFF - макс. значение для оценки
      curr_assoc := 0; // текущий просматриваемый кластер

      for i := 0 to length(clastWeights) - 2 do
        begin
          if curr_assoc = clastWeights[i].assoc then
            continue;
          curr_assoc := clastWeights[i].assoc;
          for j := i + 1 to length(clastWeights) - 1 do
            if (clastWeights[j].assoc <> clastWeights[j-1].assoc) and
               (curr_assoc <> clastWeights[j].assoc) and
               (abs(clastWeights[i].grav - clastWeights[j].grav) <= modus_min) then
              begin
                i_min := i;
                j_min := j;
                modus_min := abs(clastWeights[i].grav - clastWeights[j].grav);
              end;
         end;

       if (num_clast > Ord(High(TPrior))) or (modus_min = 0.0) then
         begin
           for i := High(clastWeights) downto j_min do
             if clastWeights[i].assoc = clastWeights[j_min].assoc then
               clastWeights[i].assoc := clastWeights[i_min].assoc;
           clastWeights[i_min].grav := (clastWeights[i_min].grav + clastWeights[j_min].grav) / 2;
         end;

       dec(num_clast);
    until (num_clast <= Ord(High(TPrior))) and ((modus_min <> 0.0) or (num_clast < 1));

    p := mpHigh;
    for i := 0 to rlstMove.Count - 1 do
      begin
        new(mp);
        if (i > 0) and (clastWeights[i].assoc > clastWeights[i-1].assoc) then
          p := Succ(p); 
        mp.move := PMoveEst(rlstMove[i]).move;
        mp.prior := p;
        dispose(rlstMove[i]);
        rlstMove[i] := mp;
      end;

    SetLength(clastWeights, 0);
  end;

var
  lstUsrMove, lstExtMove: TList;

  procedure MergeMoves;
    function NEqualMoves(i,j: integer): boolean;
    begin
      with PMovePrior(lstExtMove[i])^, PMovePrior(_lstMovePrior[j]).move do
        Result := (i0 = move.i0) and (j0 = move.j0) and (j = move.j) and (i = move.i) and
                  (prom_fig = move.prom_fig);
    end;

  var
    i, j, n: integer;
  const
    PRIOR_CALC: array[TPrior, TPrior] of TPrior =
      ((mpNo, mpNo, mpNo, mpNo),        // UsrPrior = mpNo - ?, т.к. ещё нигде не исп.
       (mpHigh, mpHigh, mpHigh, mpMid), // UsrPrior = mpHigh
       (mpMid, mpMid, mpMid, mpMid),    // UsrPrior = mpMid
       (mpLow, mpMid, mpLow, mpLow));   // UsrPrior = mpLow
  begin
    for i := 0 to lstUsrMove.Count - 1 do
      _lstMovePrior.Add(lstUsrMove[i]);

  // Сливание списков
  n := _lstMovePrior.Count;
  for i := 0 to lstExtMove.Count - 1 do
    begin
      j := n - 1;
      while (j >= 0) do
        begin
          if NEqualMoves(i,j) then
            begin
              PMovePrior(_lstMovePrior[j]).prior :=
                PRIOR_CALC[PMovePrior(_lstMovePrior[j]).prior, PMovePrior(lstExtMove[j]).prior];
              dispose(lstExtMove[i]);
              break;
            end;
          dec(j);
        end;
      if j < 0 then
        _lstMovePrior.Add(lstExtMove[i]);
    end; { for }
  end;

var
  i: integer;
begin
  for i := 0 to _lstMovePrior.Count - 1 do
    dispose(_lstMovePrior[i]);
  _lstMovePrior.Clear;

  lstExtMove := nil;
  lstUsrMove := TList.Create;
  try
    lstExtMove := TList.Create;

    if _bUseUserBase or (not Assigned(_oExtPosBase)) then
      _oPosBase.Find(Position^, lstUsrMove);
    if Assigned(_oExtPosBase) then
      _oExtPosBase.Find(Position^, lstExtMove);

    // TODO: Handle wrong DB

    ClasterMoves(lstUsrMove);
    ClasterMoves(lstExtMove);
    MergeMoves;

  finally
    lstExtMove.Free;
    lstUsrMove.Free;
  end;
end;


procedure TPosBaseChessBoard.WriteGameToBase(vGameResult: TGameResult);
begin
  if not _bTrainingMode then
    exit;
  gameResult := vGameResult;
  TPosBaseOperator.CreateWrite(self);
end;


procedure TPosBaseChessBoard.FWriteGameToBase;
var
  ply: integer;
begin
  gameID := Random($FFFF) + 1;

  if (PlayerColor = fcWhite) then
    ply := 0
  else
    ply := 1;

  while ((ply < PositionsList.Count) and ((MAX_PLY_TO_BASE < 0) or (ply <= MAX_PLY_TO_BASE))) do
    begin
      _oPosBase.Add(PPosMove(PositionsList[ply])^);
      inc(ply, 2);
    end;
end;


procedure TPosBaseChessBoard.UnsetExternalBase;
begin
  FreeAndNil(_oExtPosBase);
end;


procedure TPosBaseChessBoard.PPRandom;
var
  PosBaseOperator: TPosBaseOperator;
begin
  inherited;
  if _bTrainingMode then
    begin
      PosBaseOperator := TPosBaseOperator.CreateRead(self, FALSE, FALSE); // чтение из базы и вывод на скрытую доску
      PosBaseOperator.WaitFor;
      PosBaseOperator.Free;
    end;
end;

{------------- TPosBaseOperator --------------}

constructor TPosBaseOperator.CreateRead(voChessBoard: TPosBaseChessBoard; vbHidden: boolean; vbFreeOnTerminate: boolean = TRUE);
begin
  _enuOperation := opRead;
  _oChessBoard := voChessBoard;
  _bHidden := vbHidden;

  inherited Create(TRUE);
  Priority := tpNormal;
  FreeOnTerminate := vbFreeOnTerminate;
  Resume;
end;


constructor TPosBaseOperator.CreateWrite(voChessBoard: TPosBaseChessBoard);
begin
  _oChessBoard := voChessBoard;
  _enuOperation := opWrite;
  inherited Create(TRUE);
  Priority := tpNormal;
  FreeOnTerminate := TRUE;
  Resume;
end;


procedure TPosBaseOperator.Execute;
begin
  case _enuOperation of
    opRead:
      _oChessBoard.FReadFromBase;
    opWrite:
      _oChessBoard.FWriteGameToBase;
  end;
end;


initialization
  Randomize;

end.
