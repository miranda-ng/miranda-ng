unit PosBaseUnit;

interface

uses
  ChessBoardHeaderUnit, ChessRulesEngine, ChessBoardUnit, Classes;

type
  PMoveEst = ^TMoveEst;
  TMoveEst = record
    move: TMoveAbs;
    estimate: LongWord;
  end;

  TFieldNode = packed record
    bField: byte;
    bNextNode: byte; // сл. узел
    wNextNode: word;
    bNextValue: byte; // сл. значение данных
    wNextValue: word;
  end;

  TMoveNode = packed record
    wMove: word;
    estimate: LongWord;
    bNextValue: byte; // сл. значение данных
    wNextValue: word;
  end;

  TReestimate = procedure(moveEsts: TList; nRec: integer);

  TPosBase = class
  private
    fPos: file of TFieldNode;
    fMov: file of TMoveNode;
    Reestimate: TReestimate;
  public
    procedure Add(const posMove: TPosMove); // добавление позиции и хода в базу
    function Find(const pos: TChessPosition; moveEsts: TList = nil): boolean;
    constructor Create(fileNameNoExt: string; Reestimate: TReestimate = nil);
    destructor Destroy; override;
  end;

implementation

uses
  SysUtils;

type
  TCoord = record
    i,j: integer;
  end;

const
  POS_FILE_EXT = 'pos';
  MOV_FILE_EXT = 'mov';
  EMPTY_MOVE_NODE: TMoveNode =
    (wMove: 0; estimate: 0; bNextValue: 0; wNextValue: 0);

  FIELD_SEQ: array[1..64] of TCoord = // 13617 kb
    ((i: 1; j: 1), (i: 1; j: 2), (i: 1; j: 3), (i: 1; j: 4),
     (i: 1; j: 5), (i: 1; j: 6), (i: 1; j: 7), (i: 1; j: 8),
     (i: 8; j: 8), (i: 8; j: 7), (i: 8; j: 6), (i: 8; j: 5),
     (i: 8; j: 4), (i: 8; j: 3), (i: 8; j: 2), (i: 8; j: 1),
     (i: 2; j: 1), (i: 2; j: 2), (i: 2; j: 3), (i: 2; j: 4),
     (i: 2; j: 5), (i: 2; j: 6), (i: 2; j: 7), (i: 2; j: 8),
     (i: 7; j: 8), (i: 7; j: 7), (i: 7; j: 6), (i: 7; j: 5),
     (i: 7; j: 4), (i: 7; j: 3), (i: 7; j: 2), (i: 7; j: 1),
     (i: 3; j: 1), (i: 3; j: 2), (i: 3; j: 3), (i: 3; j: 4),
     (i: 3; j: 5), (i: 3; j: 6), (i: 3; j: 7), (i: 3; j: 8),
     (i: 6; j: 8), (i: 6; j: 7), (i: 6; j: 6), (i: 6; j: 5),
     (i: 6; j: 4), (i: 6; j: 3), (i: 6; j: 2), (i: 6; j: 1),
     (i: 4; j: 1), (i: 4; j: 2), (i: 4; j: 3), (i: 4; j: 4),
     (i: 4; j: 5), (i: 4; j: 6), (i: 4; j: 7), (i: 4; j: 8),
     (i: 5; j: 1), (i: 5; j: 2), (i: 5; j: 3), (i: 5; j: 4),
     (i: 5; j: 5), (i: 5; j: 6), (i: 5; j: 7), (i: 5; j: 8));

constructor TPosBase.Create(fileNameNoExt: string; Reestimate: TReestimate = nil);
begin
  AssignFile(fPos, fileNameNoExt + '.' + POS_FILE_EXT);
{$I-}
  Reset(fPos);
{$I+}
  if IOResult <> 0 then
    Rewrite(fPos);

  AssignFile(fMov, fileNameNoExt + '.' + MOV_FILE_EXT);    
{$I-}
  Reset(fMov);
{$I+}
  try
    if IOResult <> 0 then
      Rewrite(fMov);
  except
    Close(fPos);
    raise;
  end;
  self.Reestimate := Reestimate;
end;

destructor TPosBase.Destroy;
begin
  CloseFile(fPos); // TODO: Here occurs an error if client is closed unforced
  CloseFile(fMov);
end;


function EncodeAddInf(const pos: TChessPosition): byte;
begin
  Result := pos.en_passant;
  if WhiteKingSide in pos.castling then
    Result := Result or $80;
  if WhiteQueenSide in pos.castling then
    Result := Result or $40;
  if BlackKingSide in pos.castling then
    Result := Result or $20;
  if BlackQueenSide in pos.castling then
    Result := Result or $10;
end;


function EncodeMove(const move: TMoveAbs): word;
begin
  with move do
    Result := ((((((((i0-1) shl 3) or (j0-1)) shl 3) or (i-1)) shl 3) or (j-1)) shl 3) or Ord(prom_fig);
end;


procedure TPosBase.Add(const posMove: TPosMove);
var
  addInf: byte;
  fn: TFieldNode;

  procedure AddPosNodes(k: integer; r: integer = -1);
  var
    l, nr: integer;
    mn: TMoveNode;
    estList: TList;
  begin
    // Добавление узлов позиции
    if r >= 0 then
      begin
        nr := FileSize(fPos);
        fn.bNextValue := nr and $FF;
        fn.wNextValue := nr shr 8;
        Seek(fPos, r);
        write(fPos, fn);
        Seek(fPos, nr);
      end
    else
      nr := 0;
    for l := k to 66 do // 65 - доп. инф, 66 - цвет.
      begin
        if l = 66 then
          begin
            fn.bField := ord(posMove.pos.color);
            nr := FileSize(fMov);
          end
        else
          begin
            if l <= 64 then
              fn.bField := ord(posMove.pos.board[FIELD_SEQ[l].i, FIELD_SEQ[l].j])
            else // l = 65
              fn.bField := addInf;
            inc(nr);
          end;
        fn.bNextNode := nr and $FF;
        fn.wNextNode := nr shr 8;
        fn.bNextValue := 0;
        fn.wNextValue := 0;
        write(fPos, fn);
      end;
    // формирование записи хода
    mn := EMPTY_MOVE_NODE;
    mn.wMove := EncodeMove(posMove.move);
    if Assigned(Reestimate) then
    begin
      estList := TList.Create;
      try
        estList.Add(Pointer(mn.estimate));
        Reestimate(estList, 0);
        mn.estimate := LongWord(estList[0]);
      finally
        estList.Free;
      end;  
    end;
    Seek(fMov, FileSize(fMov));
    write(fMov, mn);
  end;

var
  k, r, pr, rm, moveSet, moveCount: integer;
  mv: word;
  mn: TMoveNode;
  enc_mv: word;
  estList: TList;
begin
  addInf := EncodeAddInf(posMove.pos);
  if FileSize(fPos) = 0 then
    begin
      AddPosNodes(1);
      exit;
    end;
  r := 0;
  for k := 1 to 66 do // 65 - доп. инф, 66 - цвет.
    begin
      Seek(fPos, r);
      read(fPos, fn);
      while ((k <= 64) and (fn.bField <> ord(posMove.pos.board[FIELD_SEQ[k].i, FIELD_SEQ[k].j]))) or
            ((k = 65) and (fn.bField <> addInf)) or
            ((k = 66) and (fn.bField <> ord(posMove.pos.color))) do
        begin
          pr := r;
          r := (fn.wNextValue shl 8) or fn.bNextValue;
          if r = 0 then
            begin
              AddPosNodes(k, pr);
              exit;
            end;
          Seek(fPos, r);
          read(fPos, fn);
        end; { while }
      // значение в цепочке найдено
      r := (fn.wNextNode shl 8) or fn.bNextNode;
    end;

  moveCount := 0;
  moveSet := -1;
  estList := TList.Create;
  try
    rm := r;
    enc_mv := EncodeMove(posMove.move);
    repeat
      pr := r;
      Seek(fMov, r);
      read(fMov, mn);

      mv := mn.wMove;
      if mv = enc_mv then
        moveSet := moveCount;

      if Assigned(Reestimate) then
        estList.Add(Pointer(mn.estimate));

      inc(moveCount);
      r := (mn.wNextValue shl 8) or mn.bNextValue;
    until r = 0;

    if moveSet < 0 then // хода нет в списке, добавляем
      begin
        // связывание нового узла с текущим узлом
        r := FileSize(fMov);
        mn.bNextValue := r and $FF;
        mn.wNextValue := r shr 8;
        Seek(fMov, pr);
        write(fMov, mn);
        // Добавление нового узла ходов
        mn := EMPTY_MOVE_NODE;
        mn.wMove := enc_mv;
        Seek(fMov, r);
        write(fMov, mn);

        if Assigned(Reestimate) then
          estList.Add(Pointer(mn.estimate));
        moveSet := moveCount;
      end;

      if Assigned(Reestimate) then
        begin
          Reestimate(estList, moveSet);
          for k := 0 to estList.Count - 1 do
            begin
              Seek(fMov, rm);
              read(fMov, mn);
              if mn.estimate <> LongWord(estList[k]) then
                begin
                  mn.estimate := LongWord(estList[k]);
                  Seek(fMov, rm);
                  write(fMov, mn);
                end;
              rm := (mn.wNextValue shl 8) or mn.bNextValue;
            end;
        end;
  finally
    estList.Free;
  end;  
end;


function TPosBase.Find(const pos: TChessPosition; moveEsts: TList = nil): boolean;

  function DecodeMove(enc_move: word): TMoveAbs;
  begin
   with Result do
     begin
       prom_fig := TFigureName(enc_move and $07);
       enc_move := enc_move shr 3;
       j := (enc_move and $07) + 1;
       enc_move := enc_move shr 3;
       i := (enc_move and $07) + 1;
       enc_move := enc_move shr 3;
       j0 := (enc_move and $07) + 1;
       enc_move := enc_move shr 3;
       i0 := (enc_move and $07) + 1;
     end;
  end;

var
  k, r: integer;
  fn: TFieldNode;
  mn: TMoveNode;
  pme: PMoveEst;
label
  here;
begin
  Result := FALSE;
  for k := 0 to moveEsts.Count - 1 do
    dispose(moveEsts[k]);
  moveEsts.Clear;
  if FileSize(fPos) = 0 then
    exit;

  r := 0;
  for k := 1 to 66 do // 65 - доп. инф, 66 - цвет.
    begin
here:
      Seek(fPos, r);
      read(fPos, fn);
      r := (fn.wNextNode shl 8) or fn.bNextNode;
      while ((k <= 64) and (fn.bField <> ord(pos.board[FIELD_SEQ[k].i, FIELD_SEQ[k].j]))) or
            ((k = 65) and (fn.bField <> EncodeAddInf(pos))) or
            ((k = 66) and (fn.bField <> ord(pos.color))) do
        begin
          r := (fn.wNextValue shl 8) or fn.bNextValue;
          if r = 0 then
            exit
          else
            goto here;
        end; { while }
    end; { for }

  Result := TRUE;
  if not Assigned(moveEsts) then
    exit;

  // Заполнение списка ходов
  repeat
    Seek(fMov, r);
    read(fMov, mn);

    new(pme);
    pme^.move := DecodeMove(mn.wMove);
    pme^.estimate := mn.estimate;
    moveEsts.Add(pme);

    r := (mn.wNextValue shl 8) or mn.bNextValue;
  until r = 0;
end;

end.
