////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit ChessRulesEngine;

interface

uses
  Classes;

type
  TFigureName = (K, Q, R, B, N, P);
  TFigure = (WK, WQ, WR, WB, WN, WP, ES,
              BK, BQ, BR, BB, BN, BP); // ES - Empty Square
  TFigureColor = (fcWhite, fcBlack);

  TCastlingCapability = set of (
    WhiteKingSide, WhiteQueenSide, BlackKingSide, BlackQueenSide);

  PChessPosition = ^TChessPosition;
  TChessPosition = object // Chess position
    board: array[1..8, 1..8] of TFigure;
    color: TFigureColor; // Who moves
    castling: TCastlingCapability;
    en_passant: 0..8; // possibility of e.p 0 - no e.p.
  private
    procedure FUpdateKingSideCastling(AColor: TFigureColor);
    procedure FUpdateQueenSideCastling(AColor: TFigureColor);
  public
    function SetPiece(i, j: integer; APiece: TFigure): boolean;
  end;

  PMoveAbs = ^TMoveAbs;
  TMoveAbs = record
    i0, j0, i, j: byte;
    prom_fig: TFigureName;
  end;

  IChessRulesEngineable = interface
    function AskPromotionFigure(FigureColor: TFigureColor): TFigureName;
  end;

  TEvaluation = (evInGame, evMate, evStaleMate);

  TMoveNotationFormat = (mnfCh4N, mnfCh4NEx); // TODO: mnfPGN

  TChessRulesEngine = class
  private
    m_ChessRulesEngineable: IChessRulesEngineable;
    m_Position: TChessPosition;
    m_iMovesOffset: integer;
    m_i0, m_j0: integer;  // Previous position of piece
    m_fig: TFigure;    // Piece that moves
    m_lastMove: TMoveAbs; // Last move done
    m_strLastMoveStr: string; // last move in algebraic notation
    m_MoveNotationFormat: TMoveNotationFormat;
    m_bFENFormat: boolean;
    m_lstPosition: TList;

    function FGetPosition: PChessPosition;
    function FAskPromotionFigure(FigureColor: TFigureColor): TFigureName;
    procedure FAddPosMoveToList; // Add position and its move to the list
    function FMove2Str(const pos: TChessPosition): string;

    function FCheckMove(const chp: TChessPosition; var chp_res: TChessPosition;
      i0, j0, i, j: integer; var prom_fig: TFigureName): boolean;

    function FGetLastMove: PMoveAbs;
    procedure FDeleteLastPositionFromPositionList;

    function FDoMove(i, j: integer; prom_fig: TFigureName = K): boolean;

    class function FFieldUnderAttack(const pos: TChessPosition; i0,j0: integer): boolean;
    class function FCheckCheck(const pos: TChessPosition): boolean;
    function FCanMove(pos: TChessPosition): boolean;

    procedure FSetMovesOffset(iValue: integer);

    property i0: integer read m_i0 write m_i0;
    property j0: integer read m_j0 write m_j0;
    property fig: TFigure read m_fig write m_fig;

  public
    constructor Create(ChessRulesEngineable: IChessRulesEngineable = nil);
    destructor Destroy; override;

    function DoMove(move_str: string): boolean; overload;
    function DoMove(i0, j0, i, j: integer; prom_fig: TFigureName = K): boolean; overload;
    function TakeBack: boolean;
    function SetPosition(strValue: string): boolean;
    function GetPosition: string;
    function GetColorStarts: TFigureColor;
    procedure InitNewGame;
    procedure InitNewPPRandomGame;
    procedure ResetMoveList; // Clears positions list
    function NMovesDone: integer; // amount of moves done
    function NPlysDone: integer; // amount of plys done
    function GetFENMoveNumber: integer;
    function GetEvaluation: TEvaluation;

    property Position: PChessPosition read FGetPosition;
    property lastMove: PMoveAbs read FGetLastMove;
    property lastMoveStr: string read m_strLastMoveStr;
    property MovesOffset: integer read m_iMovesOffset write FSetMovesOffset;
    property PositionsList: TList read m_lstPosition;
    property MoveNotationFormat: TMoveNotationFormat
      read m_MoveNotationFormat write m_MoveNotationFormat;
    property FENFormat: boolean read m_bFENFormat write m_bFENFormat;
  end;

  PPosMove = ^TPosMove;
  TPosMove = record
    pos: TChessPosition;
    move: TMoveAbs;
  end;

const
  INITIAL_CHESS_POSITION = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -';
  EMPTY_CHESS_POSITION = '8/8/8/8/8/8/8/8 w - -';
  INITIAL_FEN = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1';
  EMPTY_FEN = '8/8/8/8/8/8/8/8 w - - 0 1';

implementation

{$J+}

uses
  SysUtils, StrUtils;

type
  TDeltaMove = array [TFigureName] of
    record
      longRange: boolean;
      dx,dy: array[1..8] of Integer;
    end;

const
  DELTA_MOVE: TDeltaMove = ((longRange: FALSE; // Король
                            dx: (1,0,-1,0, 1,-1,-1,1); dy: (0,1,0,-1, 1,1,-1,-1)),
                            (longRange: TRUE; // Ферзь
                            dx: (1,0,-1,0, 1,-1,-1,1); dy: (0,1,0,-1, 1,1,-1,-1)),
                            (longRange: TRUE; // Ладья
                            dx: (1,0,-1,0, 0,0,0,0); dy: (0,1,0,-1, 0,0,0,0)),
                            (longRange: TRUE; // Слон
                            dx: (1,-1,-1,1, 0,0,0,0); dy: (1,1,-1,-1, 0,0,0,0)),
                            (longRange: FALSE; // Конь
                            dx: (2,1,-1,-2, 2,1,-1,-2); dy: (1,2,2,1, -1,-2,-2,-1)),
                            (longRange: FALSE; // Пешка
                            dx: (0,0,-1,1, 0,0,0,0); dy: (2,1,1,1, 0,0,0,0)));

////////////////////////////////////////////////////////////////////////////////
// TChessRulesEngine

constructor TChessRulesEngine.Create(ChessRulesEngineable: IChessRulesEngineable = nil);
begin
  inherited Create;

  m_ChessRulesEngineable := ChessRulesEngineable;
  m_lstPosition := TList.Create;

  InitNewGame;
end;


destructor TChessRulesEngine.Destroy;
begin
  ResetMoveList;
  m_lstPosition.Free;
  
  inherited;
end;


function TChessRulesEngine.FGetPosition: PChessPosition;
begin
  Result := @m_Position;
end;


function TChessRulesEngine.FAskPromotionFigure(FigureColor: TFigureColor): TFigureName;
begin
  if (Assigned(m_ChessRulesEngineable)) then
    Result := m_ChessRulesEngineable.AskPromotionFigure(FigureColor)
  else
    Result := Q;
end;


class function TChessRulesEngine.FCheckCheck(const pos: TChessPosition): boolean;
label
  l;
const
  _i0: integer = 1; // для увеличения скорости обработки
  _j0: integer = 1;
var
  i, j: integer;
begin
  with pos do
  begin
    if ((color = fcWhite) and (board[_i0, _j0] = WK)) or
       ((color = fcBlack) and (board[_i0, _j0] = BK)) then goto l;
    // поиск короля на доске
    for i:= 1 to 8 do
    begin
      for j:= 1 to 8 do
      begin
        if ((color = fcWhite) and (board[i,j] = WK)) or
           ((color = fcBlack) and (board[i,j] = BK)) then
        begin
          _i0 := i;
          _j0 := j;
          goto l;
        end;
      end; // for j
    end; // for i
l:
    Result := FFieldUnderAttack(pos, _i0, _j0);
  end;
end;


class function TChessRulesEngine.FFieldUnderAttack(const pos: TChessPosition; i0, j0: integer): boolean;
var
  f: TFigureName;
  ef: TFigure;
  l: byte;
  ti,tj: Integer;
  locLongRange: boolean;
begin
  for f:= R to N do
    for l:= 1 to 8 do
      with DELTA_MOVE[f], pos do
        begin
          if (dx[l] = 0) and (dy[l] = 0) then break; // Все ходы просмотрены
          ti:= i0; tj:= j0;
          locLongRange:= FALSE;
          repeat
            ti:= ti + dx[l]; tj:= tj + dy[l];
            if not(ti in [1..8]) or not(tj in [1..8]) then break;
            ef:= board[ti,tj];
            if ((color = fcWhite) and (ef < ES)) or ((color = fcBlack) and (ef > ES))
              then break;
            case ef of
              WK,BK:
                if locLongRange or (f = N) then break;
              WQ,BQ:
                if f = N then break;
              WR,BR:
                if f <> R then break;
              WB,BB:
                if f <> B then break;
              WN,BN:
                if f <> N then break;
              WP,BP:
                if locLongRange or (f <> B) or
                   ((color = fcWhite) and not(tj > j0)) or
                   ((color = fcBlack) and not(tj < j0))
                          then break;
              ES:
                begin
                  locLongRange:= TRUE;
                  continue;
                end;
            end;
            Result:= TRUE;
            exit;
          until (not longRange);
        end;

  Result := FALSE;
end;


function TChessRulesEngine.FCheckMove(const chp: TChessPosition; var chp_res: TChessPosition;
  i0, j0, i, j: integer; var prom_fig: TFigureName): boolean;
label
  here;
var
  ti,tj: integer;
  l: byte;
  f: TFigureName;
  _fig: TFigure;
  pos: TChessPosition;
begin
  Result := FALSE;
  
  if (not ((i0 in [1..8]) and (j0 in [1..8]) and
           (i in [1..8]) and (j in [1..8]))) then
    exit;

  _fig := chp.board[i0, j0];
  if (((chp.color = fcWhite) and (_fig > ES)) or
      ((chp.color = fcBlack) and (_fig < ES))) then
    exit;

  f := TFigureName(ord(_fig) - ord(chp.color) * ord(BK));

  for l := 1 to 8 do
    with DELTA_MOVE[f], chp do
      begin
        if (dx[l] = 0) and (dy[l] = 0) then
          break; // All moves have been viewed

        ti := i0;
        tj := j0;

        case f of
          P:
          begin
            if ((l = 1) and
                (not (((color = fcWhite) and (j0 = 2) and (board[i0,3] = ES)) or
                      ((color = fcBlack) and (j0 = 7) and (board[i0,6] = ES))))) then
              continue; // Pawn is not on 2/7 row - long move is impossible

            case color of
              fcWhite:
              begin
                inc(ti, dx[l]);
                inc(tj, dy[l]);
              end;

              fcBlack:
              begin
                dec(ti, dx[l]);
                dec(tj, dy[l]);
              end;
            end;
              
            if (not (ti in [1..8]) or not(tj in [1..8])) then
              continue;
            if ((l <= 2) and (board[ti,tj] <> ES)) then
              continue; // There's a piece before the pawn -> exit

            if ((l >= 3) and (not (((color = fcWhite) and ((board[ti,tj] > ES) or
                                   ((j0 = 5) and (en_passant = ti)))) or
                                  ((color = fcBlack) and ((board[ti,tj] < ES) or
                                   ((j0 = 4) and (en_passant = ti))))))) then
              continue;

            if ((ti = i) and (tj = j)) then
              goto here;
          end;
          
        else
          begin
            repeat
              inc(ti, dx[l]);
              inc(tj, dy[l]);

              if ((not (ti in [1..8])) or (not (tj in [1..8])) or
                  ((color = fcWhite) and ((board[ti,tj] < ES) or
                   ((board[ti,tj] > ES) and ((ti <> i) or (tj <> j))))) or
                  ((color = fcBlack) and ((board[ti,tj] > ES) or
                   ((board[ti,tj] < ES) and ((ti <> i) or (tj <> j)))))) then
                break;

              if ((ti = i) and (tj = j)) then
                goto here;

            until (not longRange);
          end;

        end; { case }
      end;

      if (f = K) then // Checking against castling
      begin
        with chp do
        begin
          if (i-i0 = 2) and (j = j0) and
             (((color = fcWhite) and (WhiteKingSide in castling)) or
              ((color = fcBlack) and (BlackKingSide in castling))) then
          begin
            if ((board[6,j0] <> ES) or (board[7,j0] <> ES) or // 0-0
                FFieldUnderAttack(chp,5,j0) or
                FFieldUnderAttack(chp,6,j0)) then exit;
          end
          else if ((i-i0 = -2) and (j = j0) and
             (((color = fcWhite) and (WhiteQueenSide in castling)) or
              ((color = fcBlack) and (BlackQueenSide in castling)))) then
          begin
            if ((board[4,j0] <> ES) or (board[3,j0] <> ES) or // 0-0-0
                (board[2,j0] <> ES) or
                FFieldUnderAttack(chp,5,j0) or
                FFieldUnderAttack(chp,4,j0)) then
              exit;
          end
          else
            exit;

          goto here;
        end;
      end;

      exit; // The piece was moved not according to rules

here:
  // Making move on pos
  pos := chp;
  
  with pos do
  begin
    case f of
      P:
      begin
        if (((color = fcWhite) and (j0 = 5)) or
            ((color = fcBlack) and (j0 = 4))) and (i = en_passant) then
          board[i,j0]:= ES; // remove enemy pawn with e.p.
      end;

      K:
      begin
        if i-i0 = 2 then
          begin
            board[6,j0]:= board[8,j0]; // 0-0
            board[8,j0]:= ES;
          end
        else
        if i0-i = 2 then
          begin
            board[4,j0]:= board[1,j0]; // 0-0-0
            board[1,j0]:= ES;
          end;
        case color of
          fcWhite:
            castling:= castling - [WhiteKingSide, WhiteQueenSide];
          fcBlack:
            castling:= castling - [BlackKingSide, BlackQueenSide];
        end;
      end;

      R:
        begin
          if ((i0 = 8) and (j0 = 1)) or ((i = 8) and (j = 1)) then
            castling := castling - [WhiteKingSide]
          else if ((i0 = 1) and (j0 = 1)) or ((i = 1) and (j = 1)) then
            castling := castling - [WhiteQueenSide]
          else if ((i0 = 8) and (j0 = 8)) or ((i = 8) and (j = 8)) then
            castling := castling - [BlackKingSide]
          else if ((i0 = 1) and (j0 = 8)) or ((i = 1) and (j = 8)) then
            castling := castling - [BlackQueenSide];
        end;
    end;

    if ((f = P) and (abs(j-j0) = 2) and
        (((i > 1) and (((color = fcWhite) and (board[i-1,j] = BP)) or
                       ((color = fcBlack) and (board[i-1,j] = WP)))) or
         ((i < 8) and (((color = fcWhite) and (board[i+1,j] = BP)) or
                       ((color = fcBlack) and (board[i+1,j] = WP)))))) then
      en_passant := i0 // e.p. on
    else
      en_passant := 0; // e.p. off

    // make the move

    board[i0, j0]:= ES;
    board[i, j] := _fig;
    
    if (FCheckCheck(pos)) then
      exit; // move is impossible because of check
      
    if (f = P) and ((j = 1) or (j = 8)) then
    begin
      case prom_fig of
        Q..N: ;
      else
        prom_fig := FAskPromotionFigure(pos.color);
      end; // case
      board[i, j] := TFigure(ord(color) * ord(BK) + ord(prom_fig));
    end;

    if (color = fcWhite) then
      color := fcBlack
    else
      color := fcWhite;
  end; // with

  chp_res := pos;
  
  Result:= TRUE;
end;


function TChessRulesEngine.FCanMove(pos: TChessPosition): boolean;
var
  i,j: integer;
  ti,tj: integer;
  l: byte;
  f: TFigureName;
  prom_fig: TFigureName;
begin
  with pos do
    for i:= 1 to 8 do
      for j:= 1 to 8 do
        begin
          if ((color = fcWhite) and (board[i,j] >= ES)) or
             ((color = fcBlack) and (board[i,j] <= ES)) then continue;

          f:= TFigureName(ord(board[i,j]) - ord(color) * ord(BK));
          for l:= 1 to 8 do
            with DELTA_MOVE[f] do
              begin
                if (dx[l] = 0) and (dy[l] = 0) then break; // Все ходы просмотрены
                ti:= i; tj:= j;
                repeat
                  case color of
                    fcWhite:
                      begin
                        ti:= ti + dx[l]; tj:= tj + dy[l];
                      end;
                    fcBlack:
                      begin
                        ti:= ti - dx[l]; tj:= tj - dy[l];
                      end;
                  end;
                  if not ((ti in [1..8]) and (tj in [1..8])) then break;
                  prom_fig := Q;
                  if FCheckMove(pos, pos, i, j, ti, tj, prom_fig) then
                  begin
                    Result:= TRUE;
                    exit;
                  end;
                until not longRange;
              end;
        end;

  Result := FALSE;
end;


function TChessRulesEngine.DoMove(move_str: string): boolean;
label
  l1;
var
  l: byte;
  f, prom_f: TFigureName;
  i, j: integer;
  ti, tj: integer;
  saved_i, saved_j: integer;
begin
  Result := FALSE;

  l := length(move_str);
  if ((l <= 1)) then // at least two characters
    exit;

  if ((move_str[l] in ['+', '#'])) then
    move_str := LeftStr(move_str, l - 1);

  // Проверка на рокировку
  if (move_str = '0-0') then
  begin
    if (Position.color = fcWhite) then
      move_str:= 'Ke1g1'
    else
      move_str:= 'Ke8g8'
  end
  else if (move_str = '0-0-0') then
  begin
    if (Position.color = fcWhite) then
      move_str:= 'Ke1c1'
    else
      move_str:= 'Ke8c8';
  end;

  l := length(move_str);

  i0 := 0;
  j0 := 0;
  i := 0;
  j := 0;

  prom_f := K;
  case move_str[l] of
    'Q': prom_f := Q;
    'R': prom_f := R;
    'B': prom_f := B;
    'N': prom_f := N;
  else
    inc(l);
  end;

  dec(l);

  if move_str[l] in ['1'..'8'] then
  begin
    j := StrToInt(move_str[l]);
    dec(l);
  end;
  if move_str[l] in ['a'..'h'] then
  begin
    i := ord(move_str[l]) - ord('a') + 1;
    dec(l);
  end;
  if (l > 0) and (move_str[l] in ['1'..'8']) then
  begin
    j0 := StrToInt(move_str[l]);
    dec(l);
  end;
  if (l > 0) and (move_str[l] in ['a'..'h']) then
  begin
    i0 := ord(move_str[l]) - ord('a') + 1;
    dec(l);
  end;

  if (l = 0) then
    f := P
  else
  begin
    case move_str[l] of
      'K': f:= K;
      'Q': f:= Q;
      'R': f:= R;
      'B': f:= B;
      'N': f:= N;
    end;
  end;

  with m_Position do
  begin
    fig := TFigure(ord(f) + ord(Position.color) * ord(BK));

    case f of
      K..N: // Ход Кр - К
      begin
        if ((i0 > 0) and (j0 > 0)) then
        begin
          Result := FDoMove(i, j, prom_f);
          exit;
        end;

        for l := 1 to 8 do
        begin
          with DELTA_MOVE[f] do
          begin
            if (dx[l] = 0) and (dy[l] = 0) then break; // Все ходы просмотрены
            ti:= i;
            tj:= j;
            repeat
              ti:= ti + dx[l];
              tj:= tj + dy[l];
              if not ((ti in [1..8]) and (tj in [1..8])) or
                 ((board[ti,tj] <> ES) and (board[ti,tj] <> fig)) then
                break;

              if ((i0 = 0) or (i0 = ti)) and ((j0 = 0) or (j0 = tj)) and
                 (board[ti, tj] = fig) then
                begin // Ходящая фигура найдена
                  saved_i := i0;
                  saved_j := j0;

                  i0 := ti;
                  j0 := tj;
                  Result := FDoMove(i, j, prom_f);
                  if (Result) then
                    exit;

                  i0 := saved_i;
                  j0 := saved_j;
                end;
            until (f = K) or (f = N); // Если Кр или К, то выход
          end; // with
        end; // for
      end; // K..N

      P:    // Ход пешкой
      begin
        if (i0 <> 0) and (i0 <> i) then // взятие пешкой
        begin
          for l := 2 to 7 do
          begin
            if (board[i0, l] = fig) and ((j0 = 0) or (j0 = l)) then
            begin
              if color = fcWhite then
              begin
                if ((board[i, l + 1] > ES) or
                    ((l = 5) and (en_passant = i))) and
                   ((j = 0) or (j = l+1)) and (abs(i - i0) = 1) then
                begin
                  j0 := l;
                  j := l + 1;
                  Result := FDoMove(i, j, prom_f);
                  if (Result) then
                    exit;
                end;
              end
              else // color = fcBlack
              if ((board[i,l - 1] < ES) or
                  ((l = 4) and (en_passant = i))) and
                 ((j = 0) or (j = l-1)) and (abs(i - i0) = 1) then
              begin
                j0 := l;
                j := l - 1;
                Result := FDoMove(i, j, prom_f);
                if (Result) then
                  exit;
              end;
            end; // if
          end; // for
        end
        else  // Ход прямо
        begin
          i0 := i;
          if color = fcWhite then
          begin
            if board[i, j - 1] = fig then
              j0 := j - 1
            else if (j = 4) and (board[i, 3] = ES) and
                    (board[i,2] = fig) then
              j0 := 2;
          end
          else // color = fcBlack
          begin
            if (board[i, j + 1] = fig) then
              j0 := j + 1
            else if (j = 5) and (board[i,6] = ES) and (board[i, 7] = fig) then
              j0 := 7;
          end;

          Result := FDoMove(i, j, prom_f);
        end; // if
      end; // P:

    end;  // case
  end;
end;


function TChessRulesEngine.FDoMove(i, j: integer; prom_fig: TFigureName = K): boolean;
var
  newPosition: TChessPosition;
begin
  Result := FCheckMove(m_Position, newPosition, i0, j0, i, j, prom_fig);
  if (Result) then
  begin
    // Store the move done
    lastMove.i0 := i0;
    lastMove.j0 := j0;
    lastMove.i := i;
    lastMove.j := j;
    lastMove.prom_fig := prom_fig;

    FAddPosMoveToList;

    m_strLastMoveStr := FMove2Str(newPosition);
    
    m_Position := newPosition;
  end;
end;


function TChessRulesEngine.DoMove(i0, j0, i, j: integer; prom_fig: TFigureName = K): boolean;
begin
  self.i0 := i0;
  self.j0 := j0;
  Result := FDoMove(i, j, prom_fig);
end;


function TChessRulesEngine.FGetLastMove: PMoveAbs;
begin
  Result := @m_lastMove;
end;


procedure TChessRulesEngine.FAddPosMoveToList;
var
  pm: PPosMove;
begin
  new(pm);
  pm.pos := m_Position;
  pm.move := lastMove^;
  PositionsList.Add(pm);
end;


function TChessRulesEngine.FMove2Str(const pos: TChessPosition): string;

  procedure NExtendWithCheckOrMate(var strMove: string);
  begin
    if (strMove = '') then
      exit;

    if (FCheckCheck(pos)) then
    begin
      if (FCanMove(pos)) then
        strMove := strMove + '+'
      else
        strMove := strMove + '#';      
    end;
  end;

var
  f: TFigureName;
  l: byte;
  ti, tj: integer;
  ambig, hor, ver: boolean;
  _fig: TFigure;
  DummyPosition: TChessPosition;
begin // .FMove2Str
  if (lastMove.i0 = 0) then // No move
  begin
    Result:= '';
    exit;
  end;

  _fig := Position.board[lastMove.i0, lastMove.j0];
  f := TFigureName(ord(_fig) + (ord(pos.color) - 1) * ord(BK));

  // Pawn moves
  if (f = P) then
  begin
    with pos do
    begin
      if ((lastMove.i - lastMove.i0) = 0) then // move
        Result := chr(ord('a') + lastMove.i - 1) + IntToStr(lastMove.j)
      else // capturing
      begin
        Result := chr(ord('a') + lastMove.i0 - 1) + chr(ord('a') + lastMove.i - 1);

        for l := 2 to 7 do // Checking against ambiguity of capturing
        begin
          if (board[lastMove.i0, l] = WP) then
            tj := l + 1
          else if (board[lastMove.i0, l] = BP) then
            tj := l - 1
          else
            continue;

          if ((((tj > l)  and ((Position.board[lastMove.i, tj] > ES) or
               ((Position.en_passant = lastMove.i) and (l = 5)))) and (color = fcBlack)) or
              (((tj < l)  and ((Position.board[lastMove.i, tj] < ES) or
               ((Position.en_passant = lastMove.i) and (l = 4)))) and (color = fcWhite))) then
          begin
            if ((MoveNotationFormat <> mnfCh4NEx) or
                FCheckMove(m_Position, DummyPosition, lastMove.i0, l, lastMove.i, tj,
                           lastMove.prom_fig)) then
            begin
              Result := Result + IntToStr(lastMove.j);
            end;
          end;
          
        end;
      end;

      if (lastMove.j = 8) or (lastMove.j = 1) then // The pawn has been promoted
      begin
        case board[lastMove.i,lastMove.j] of
          WQ,BQ: Result := Result + 'Q';
          WR,BR: Result := Result + 'R';
          WB,BB: Result := Result + 'B';
          WN,BN: Result := Result + 'N';
        end;
      end;

      if (m_MoveNotationFormat = mnfCh4NEx) then
        NExtendWithCheckOrMate(Result);

      exit;
      
    end; // with
  end; // if

  // <Piece>
  case f of
    K: Result:= 'K';
    Q: Result:= 'Q';
    R: Result:= 'R';
    B: Result:= 'B';
    N: Result:= 'N';
  end;

  // [<Line>][<Row>]
  ambig:= FALSE;
  hor:= FALSE;
  ver:= FALSE;

  for l := 1 to 8 do
  begin
    with pos, DELTA_MOVE[f] do
    begin
      if (dx[l] = 0) and (dy[l] = 0) then
        break; // All moves have been viewed

      ti := lastMove.i;
      tj := lastMove.j;

      repeat
        inc(ti, dx[l]);
        inc(tj, dy[l]);

        if ((not (ti in [1..8])) or (not (tj in [1..8]))) then
          break;

        if (board[ti,tj] = ES) then
          continue;

        if (board[ti, tj] <> _fig) then
          break;

        if ((m_MoveNotationFormat <> mnfCh4NEx) or
            FCheckMove(m_Position, DummyPosition, ti, tj, lastMove.i, lastMove.j,
                       lastMove.prom_fig)) then
        begin
          ambig := TRUE;
          ver := (ver or (ti = lastMove.i0));
          hor := (hor or (tj = lastMove.j0));

          break;
        end;

      until (f = K) or (f = N); // If K or N -> exit
      
    end;
  end; // for l

  if (ambig) then
  begin
    if ((not ver) or hor) then
      Result := Result + chr(ord('a') + lastMove.i0 - 1);
    if (ver) then
      Result := Result + IntToStr(lastMove.j0);
  end;

  // <Destination field>
  Result := Result + chr(ord('a') + lastMove.i - 1) + IntToStr(lastMove.j);

  // <Short castling> | <Long castling>
  if (f = K) then
  begin
    if ((lastMove.i - lastMove.i0) = 2) then
      Result := '0-0'
    else if (lastMove.i0 - lastMove.i = 2) then
      Result := '0-0-0';
  end;

  if (m_MoveNotationFormat = mnfCh4NEx) then
    NExtendWithCheckOrMate(Result);
end;


function TChessRulesEngine.TakeBack: boolean;
begin
  Result := (PositionsList.Count > 0);
  if (Result) then
  begin
    FDeleteLastPositionFromPositionList;
    lastMove.i0 := 0;
  end;
end;


procedure TChessRulesEngine.FDeleteLastPositionFromPositionList;
var
  i: integer;
begin
  i := PositionsList.Count - 1;
  if (i >= 0) then
  begin
    m_Position := PPosMove(PositionsList[i]).pos;
    Dispose(PositionsList[i]);
    PositionsList.Delete(i);
  end;
end;


function TChessRulesEngine.SetPosition(strValue: string): boolean;

  function NNextToken(var str: string): string;
  var
    iPos: integer;
  begin
    str := TrimLeft(str);
    if (str = '') then
      Result := ''
    else
    begin
      iPos := Pos(' ', str);
      if (iPos > 0) then
      begin
        Result := LeftStr(str, Pred(iPos));
        str := Copy(str, iPos, MaxInt);
      end
      else
      begin
        Result := str;
        str := '';
      end;
    end;
  end;

var
  pos: TChessPosition;

  function NSetPlacingOfPieces: boolean;
  var
    strPos: string;
    iPos: integer;
    j, i, k: integer;
  begin
    Result := FALSE;

    strPos := NNextToken(strValue);

    iPos := 1;

    for j := 8 downto 1 do
    begin
      i := 1;
      repeat
        if (iPos > Length(strPos)) then
          exit;

        case strPos[iPos] of
          'K': pos.board[i,j]:= WK;
          'Q': pos.board[i,j]:= WQ;
          'R': pos.board[i,j]:= WR;
          'B': pos.board[i,j]:= WB;
          'N': pos.board[i,j]:= WN;
          'P': pos.board[i,j]:= WP;

          'k': pos.board[i,j]:= BK;
          'q': pos.board[i,j]:= BQ;
          'r': pos.board[i,j]:= BR;
          'b': pos.board[i,j]:= BB;
          'n': pos.board[i,j]:= BN;
          'p': pos.board[i,j]:= BP;

          '1'..'8': // Insert empty fields
            begin
              k := StrToInt(strPos[iPos]);
              repeat
                pos.board[i,j]:= ES;
                dec(k); inc(i);
              until k = 0;
              dec(i);
            end;

          ' ': break; // Position is read -> exit from loop

          else
            exit; // Error in strPos
        end;

        inc(i);
        inc(iPos);

      until ((i > 8) or (strPos[iPos] = '/')); // Repeat until '/' or if not on the row

      inc(iPos);
      
    end; // for j

    Result := TRUE;
  end;

var
  i: integer;
  iMovesOffset: integer;
  strToken: string;
begin // .SetPosition
  Result := NSetPlacingOfPieces;
  if (not Result) then
    exit;

  // Defaults
  pos.color := fcWhite;
  pos.castling := [];
  pos.en_passant := 0;
  iMovesOffset := 0;

  try
    strToken := NNextToken(strValue);
    if (strToken = '') then
      exit;

    case strToken[1] of
      'w':
        pos.color := fcWhite;
      'b':
        pos.color := fcBlack;
      else
        exit;
    end;

    strToken := NNextToken(strValue);
    if (strToken = '') then
      exit;

    for i := 1 to Length(strToken) do
    begin
      with pos do
      begin
        case strToken[i] of
          'K':
            Include(castling, WhiteKingSide);
          'Q':
            Include(castling, WhiteQueenSide);
          'k':
            Include(castling, BlackKingSide);
          'q':
            Include(castling, BlackQueenSide);
          '-':
            castling := [];
        else
          exit;
        end;
      end;

    end; // for

    strToken := NNextToken(strValue);
    if (strToken = '') then
      exit;

    with pos do
    begin
      case strToken[1] of
        'a'..'h':
          en_passant := ord(strToken[1]) - ord('a') + 1;
      end;
    end;

    strToken := NNextToken(strValue);
    if (strToken = '') then
      exit;

    // Skip 50-moves counter

    strToken := NNextToken(strValue);
    if (strToken = '') then
      exit;

    iMovesOffset := StrToIntDef(strToken, 1) - 1;
    if (iMovesOffset < 0) then
      iMovesOffset := 0;

  finally
    ResetMoveList;
    m_Position := pos;
    m_iMovesOffset := iMovesOffset;
  end;

end;


procedure TChessRulesEngine.InitNewGame;
var
  bRes: boolean;
begin
  bRes := SetPosition(INITIAL_CHESS_POSITION);
  Assert(bRes);
end;


procedure TChessRulesEngine.ResetMoveList;
var
  i: integer;
begin
  for i := 0 to PositionsList.Count - 1 do
    Dispose(PositionsList[i]);
  PositionsList.Clear;

  lastMove.i0 := 0;
end;


function TChessRulesEngine.GetPosition: string;

  function NGetPlacingOfPieces: string;
  var
    i, j: Integer;
    k: byte;
    chFig: char;
  begin
    Result := '';

    // Placing of pieces
    for j := 8 downto 1 do
    begin
      k := 0;

      for i := 1 to 8 do
      begin
        case Position.board[i, j] of
          WK: chFig := 'K';
          WQ: chFig := 'Q';
          WR: chFig := 'R';
          WB: chFig := 'B';
          WN: chFig := 'N';
          WP: chFig := 'P';
          BK: chFig := 'k';
          BQ: chFig := 'q';
          BR: chFig := 'r';
          BB: chFig := 'b';
          BN: chFig := 'n';
          BP: chFig := 'p';
          ES:
          begin
            inc(k);
            continue;
          end;
        end;

        if (k > 0) then
        begin
          Result := Result + IntToStr(k);
          k := 0;
        end;

        Result := Result + chFig;
      end; // for i

      if (k > 0) then
        Result := Result + IntToStr(k);
      if (j = 1) then
        Result := Result + ' '
      else
        Result := Result + '/'; // i <= 7
    end; // for j

  end;

begin // .GetPosition
  Result := NGetPlacingOfPieces;

  if (Position.color = fcWhite) then
    Result := Result + 'w '
  else
    Result := Result + 'b '; // color = fcBlack

  // Castling
  if (Position.castling = []) then
    Result := Result + '-'
  else
  begin
    if (WhiteKingSide in Position.castling) then
      Result := Result + 'K';
    if (WhiteQueenSide in Position.castling) then
      Result := Result + 'Q';
    if (BlackKingSide in Position.castling) then
      Result := Result + 'k';
    if (BlackQueenSide in Position.castling) then
      Result := Result + 'q';
  end;

  // en-passant
  if (Position.en_passant = 0) then
    Result := Result + ' -'
  else
  begin
    Result := Result + ' ' + Chr(Ord('a') - 1 + Position.en_passant);
  end;

  if (not FENFormat) then
    exit;

  if (Position.en_passant <> 0) then
  begin
    if (Position.color = fcWhite) then
      Result := Result + '6'
    else
      Result := Result + '3'; // Black
  end;

  Result := Result + ' 0'; // TODO: 50-moves rule

  Result := Result + ' ' + IntToStr(GetFENMoveNumber);
end;


function TChessRulesEngine.GetFENMoveNumber: integer;
begin
  Result := NMovesDone;
  if ((m_Position.color = fcWhite) or (Result = 0)) then
    inc(Result);
end;


procedure TChessRulesEngine.FSetMovesOffset(iValue: integer);
begin
  Assert(iValue >= 0);
  m_iMovesOffset := iValue;
end;


procedure TChessRulesEngine.InitNewPPRandomGame;
const
  FIG: array[0..5] of TFigureName = (B, B, Q, R, N, N);
  SQR: array[0..5] of byte = (2, 3, 4, 6, 7, 0);
var
  rnd_sqr: array[0..5] of byte;
  i,j: integer;
  f: boolean;
begin
  InitNewGame;

  if (Random(2) = 0) then
    SQR[5] := 1 // с какой стороны оставляем ладью
  else
    SQR[5] := 8;

  for i := 0 to 5 do
  begin
    repeat
      rnd_sqr[i] := SQR[Random(6)];
      f := FALSE;
      for j := 0 to i-1 do f := f or (rnd_sqr[i] = rnd_sqr[j]);
    until not (f or ((i = 1) and (((rnd_sqr[0] xor rnd_sqr[1]) and 1) = 0)));
    m_Position.board[rnd_sqr[i], 1] := TFigure(ord(FIG[i]));
    m_Position.board[rnd_sqr[i], 8] := TFigure(ord(BK) + ord(FIG[i]));
  end;
end;


function TChessRulesEngine.NMovesDone: integer;
var
  iMovesCount: integer;
begin
  if ((PositionsList.Count = 0) and (m_iMovesOffset = 0)) then
    iMovesCount := 0
  else
  begin
    if (GetColorStarts = fcWhite) then
      iMovesCount := ((PositionsList.Count + 1) div 2)
    else // GetColorStarts = fcBlack
      iMovesCount := ((PositionsList.Count + 2) div 2);
  end;

  Result := m_iMovesOffset + iMovesCount;
end;


function TChessRulesEngine.GetColorStarts: TFigureColor;
begin
  if (Odd(PositionsList.Count) and (m_Position.color = fcBlack)) or
      (not Odd(PositionsList.Count) and (m_Position.color = fcWhite)) then
    Result := fcWhite
  else
    Result := fcBlack;
end;


function TChessRulesEngine.NPlysDone: integer; // amount of plys done
begin
  Result := (2 * m_iMovesOffset) + PositionsList.Count;
end;


function TChessRulesEngine.GetEvaluation: TEvaluation;
begin
  Result := evInGame;
  if (not FCanMove(m_Position)) then
  begin
    if (FCheckCheck(m_Position)) then
      Result := evMate
    else
      Result := evStaleMate;
  end;
  // TODO: Evaluate position for possible technical draw
end;


function TChessPosition.SetPiece(i, j: integer; APiece: TFigure): boolean;
var
  SavedPiece: TFigure;
begin
  Result := ((i in [1..8]) and (j in [1..8]));
  if (not Result) then
    exit;

  SavedPiece := board[i, j];
  board[i, j] := APiece;

  if (SavedPiece = APiece) then
    exit;

  if ((i = 5) and (j = 1)) then
  begin
    FUpdateKingSideCastling(fcWhite);
    FUpdateQueenSideCastling(fcWhite);
  end
  else if ((i = 8) and (j = 1)) then
    FUpdateKingSideCastling(fcWhite)
  else if ((i = 1) and (j = 1)) then
    FUpdateQueenSideCastling(fcWhite)
  else if ((i = 5) and (j = 8)) then
  begin
    FUpdateKingSideCastling(fcBlack);
    FUpdateQueenSideCastling(fcBlack);
  end
  else if ((i = 8) and (j = 8)) then
    FUpdateKingSideCastling(fcBlack)
  else if ((i = 1) and (j = 8)) then
    FUpdateQueenSideCastling(fcBlack);
end;


procedure TChessPosition.FUpdateKingSideCastling(AColor: TFigureColor);
var
  j: integer;
  King, Rook: TFigure;
begin
  if (AColor = fcWhite) then
  begin
    j := 1;
    King := WK;
    Rook := WR;
  end
  else // fcBlack
  begin
    j := 8;
    King := BK;
    Rook := BR;
  end;

  if ((board[5, j] = King) and (board[8, j] = Rook)) then
  begin
    if (AColor = fcWhite) then
      Include(castling, WhiteKingSide)
    else
      Include(castling, BlackKingSide);
  end
  else
  begin
    if (AColor = fcWhite) then
      Exclude(castling, WhiteKingSide)
    else
      Exclude(castling, BlackKingSide);
  end;

end;


procedure TChessPosition.FUpdateQueenSideCastling(AColor: TFigureColor);
var
  j: integer;
  King, Rook: TFigure;
begin
  if (AColor = fcWhite) then
  begin
    j := 1;
    King := WK;
    Rook := WR;
  end
  else // fcBlack
  begin
    j := 8;
    King := BK;
    Rook := BR;
  end;

  if ((board[5, j] = King) and (board[1, j] = Rook)) then
  begin
    if (AColor = fcWhite) then
      Include(castling, WhiteQueenSide)
    else
      Include(castling, BlackQueenSide);
  end
  else
  begin
    if (AColor = fcWhite) then
      Exclude(castling, WhiteQueenSide)
    else
      Exclude(castling, BlackQueenSide);
  end;

end;

initialization

begin
  Randomize; // It's for PP Random
end;

finalization

end.
