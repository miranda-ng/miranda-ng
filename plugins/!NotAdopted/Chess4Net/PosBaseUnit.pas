////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit PosBaseUnit;

interface

uses
  Classes,
  //
  ChessRulesEngine;

type
  PMoveEst = ^TMoveEst;
  TMoveEst = record
    move: TMoveAbs;
    estimate: LongWord;
  end;

  TReestimate = procedure(moveEsts: TList; nRec: integer);

  TPosBaseStream = class
  private
    m_iRecordSize: integer;
    m_iHeaderSize: integer;
    m_InnerStream: TStream;
    constructor Create(const strFileName: string; RecordSize: integer);
    function FGetSize: integer;
  public
    destructor Destroy; override;
    procedure SeekHeader;
    procedure SeekRec(lwRecordNumber: LongWord);
    procedure SeekEnd;
    procedure Write(const Buffer); overload;
    procedure Write(const Buffer; Count: integer); overload;
    procedure Read(var Buffer); overload;
    procedure Read(var Buffer; Count: integer); overload;
    property Size: integer read FGetSize;
    property HeaderSize: integer read m_iHeaderSize write m_iHeaderSize;
  end;

  TPosBase = class
  private
    m_iDBVersion: Integer;
    fPos: TPosBaseStream;
    fMov: TPosBaseStream;
    FReestimate: TReestimate;
    procedure FCreateStreams(const strPosFileName, strMovFileName: string);
    procedure FDestroyStreams;
    procedure FSetDBVersion;
    function FCheckDBVersion: Boolean;
  public
    procedure Add(const posMove: TPosMove); // добавление позиции и хода в базу
    function Find(const pos: TChessPosition): boolean; overload;
    function Find(const pos: TChessPosition; var moveEsts: TList): boolean; overload;
    constructor Create(fileNameNoExt: string; Reestimate: TReestimate = nil);
    destructor Destroy; override;
  end;

implementation

uses
  SysUtils;

type
  TFieldNode = packed object
  public
    btField: byte;
  private
    m_btNextNode: byte; // сл. узел
    m_wNextNode: word;
    m_btNextValue: byte; // сл. значение данных
    m_wNextValue: word;
    function FGetNextNode: LongWord;
    procedure FSetNextNode(lwValue: LongWord);
    function FGetNextValue: LongWord;
    procedure FSetNextValue(lwValue: LongWord);
  public
    property NextNode: LongWord read FGetNextNode write FSetNextNode;
    property NextValue: LongWord read FGetNextValue write FSetNextValue;
  end;

  TMoveNode = packed object
  public
    wMove: word;
    estimate: LongWord;
  private
    m_btNextValue: byte; // сл. значение данных
    m_wNextValue: word;
    function FGetNextValuePos: LongWord;
    procedure FSetNextValuePos(lwValue: LongWord);
  public
    procedure EmptyNode;
    property NextValue: LongWord read FGetNextValuePos write FSetNextValuePos;
  end;

  TCoord = record
    i, j: integer;
  end;

const
  POS_FILE_EXT = 'pos';
  MOV_FILE_EXT = 'mov';

  DB_VERSION = 1;

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

////////////////////////////////////////////////////////////////////////////////
// TPosBase

constructor TPosBase.Create(fileNameNoExt: string; Reestimate: TReestimate = nil);
begin
  inherited Create;

  self.FReestimate := Reestimate;

  FCreateStreams(fileNameNoExt + '.' + POS_FILE_EXT,
    fileNameNoExt + '.' + MOV_FILE_EXT);
  FSetDBVersion;
end;


destructor TPosBase.Destroy;
begin
  FDestroyStreams;
  inherited;
end;


procedure TPosBase.FSetDBVersion;
var
  btData: byte;
  wVersion: word;
begin
  m_iDBVersion := DB_VERSION; // default version

  if (fPos.Size > 0) then
  begin
    fPos.SeekHeader;
    fPos.Read(btData, SizeOf(btData));
    if (btData <> $FF) then
    begin
      m_iDBVersion := 0;
      fPos.HeaderSize := 0;
      exit;
    end;
    fPos.Read(wVersion, SizeOf(wVersion));
    m_iDBVersion := wVersion;
  end
  else
  begin
    btData := $FF;
    wVersion := m_iDBVersion;
    fPos.Write(btData, SizeOf(btData));
    fPos.Write(wVersion, SizeOf(wVersion));
  end;

  fPos.HeaderSize := SizeOf(byte) + SizeOf(word);
end;


function TPosBase.FCheckDBVersion: Boolean;
begin
  Result := (m_iDBVersion <= DB_VERSION);
end;


procedure TPosBase.FCreateStreams(const strPosFileName, strMovFileName: string);
begin
  fPos := TPosBaseStream.Create(strPosFileName, SizeOf(TFieldNode));
  fMov := TPosBaseStream.Create(strMovFileName, SizeOf(TMoveNode));
end;


procedure TPosBase.FDestroyStreams;
begin
  fMov.Free;
  fPos.Free;
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
    // ƒобавление узлов позиции
    if r >= 0 then
    begin
      nr := fPos.Size;
      fn.NextValue := nr;
      fPos.SeekRec(r);
      fPos.Write(fn);
      fPos.SeekRec(nr);
    end
    else
      nr := 0;
    for l := k to 66 do // 65 - доп. инф, 66 - цвет.
    begin
      if l = 66 then
      begin
        fn.btField := ord(posMove.pos.color);
        nr := fMov.Size;
      end
      else
      begin
        if l <= 64 then
          fn.btField := ord(posMove.pos.board[FIELD_SEQ[l].i, FIELD_SEQ[l].j])
        else // l = 65
          fn.btField := addInf;
        inc(nr);
      end;
      fn.NextNode := nr;
      fn.NextValue := 0;
      fPos.Write(fn);
    end;
    // формирование записи хода
    mn.EmptyNode;
    mn.wMove := EncodeMove(posMove.move);

    if Assigned(FReestimate) then
    begin
      estList := TList.Create;
      try
        estList.Add(Pointer(mn.estimate));
        FReestimate(estList, 0);
        mn.estimate := LongWord(estList[0]);
      finally
        estList.Free;
      end;

    end;
    fMov.SeekEnd;
    fMov.Write(mn);
  end;

var
  k, r, pr, rm, moveSet, moveCount: integer;
  mv: word;
  mn: TMoveNode;
  enc_mv: word;
  estList: TList;
begin
  if (not FCheckDBVersion) then
    exit;

  addInf := EncodeAddInf(posMove.pos);
  if (fPos.Size = 0) then
  begin
    AddPosNodes(1);
    exit;
  end;
  r := 0;
  for k := 1 to 66 do // 65 - доп. инф, 66 - цвет.
  begin
    fPos.SeekRec(r);
    fPos.Read(fn);

    while ((k <= 64) and (fn.btField <> ord(posMove.pos.board[FIELD_SEQ[k].i, FIELD_SEQ[k].j]))) or
          ((k = 65) and (fn.btField <> addInf)) or
          ((k = 66) and (fn.btField <> ord(posMove.pos.color))) do
    begin
      pr := r;
      r := fn.NextValue;
      if (r = 0) then
      begin
        AddPosNodes(k, pr);
        exit;
      end;
      fPos.SeekRec(r);
      fPos.Read(fn);
    end; { while }
    // значение в цепочке найдено
    r := fn.NextNode;
  end;

  moveCount := 0;
  moveSet := -1;
  estList := TList.Create;
  try
    rm := r;
    enc_mv := EncodeMove(posMove.move);
    repeat
      pr := r;
      fMov.SeekRec(r);
      fMov.Read(mn);

      mv := mn.wMove;
      if mv = enc_mv then
        moveSet := moveCount;

      if Assigned(FReestimate) then
        estList.Add(Pointer(mn.estimate));

      inc(moveCount);
      r := mn.NextValue;
    until r = 0;

    if moveSet < 0 then // хода нет в списке, добавл€ем
      begin
        // св€зывание нового узла с текущим узлом
        r := fMov.Size;
        mn.NextValue := r;
        fMov.SeekRec(pr);
        fMov.Write(mn);

        // ƒобавление нового узла ходов
        mn.EmptyNode;
        mn.wMove := enc_mv;
        fMov.SeekRec(r);
        fMov.Write(mn);

        if Assigned(FReestimate) then
          estList.Add(Pointer(mn.estimate));
        moveSet := moveCount;
      end;

      if Assigned(FReestimate) then
      begin
        FReestimate(estList, moveSet);
        for k := 0 to estList.Count - 1 do
        begin
          fMov.SeekRec(rm);
          fMov.Read(mn);
          if (mn.estimate <> LongWord(estList[k])) then
          begin
            mn.estimate := LongWord(estList[k]);
            fMov.SeekRec(rm);
            fMov.Write(mn);
          end;
          rm := mn.NextValue;
        end;
      end;
      
  finally
    estList.Free;
  end;  
end;


function TPosBase.Find(const pos: TChessPosition): boolean;
var
  lstDummy: TList;
begin
  lstDummy := nil;
  Result := Find(pos, lstDummy);
end;


function TPosBase.Find(const pos: TChessPosition; var moveEsts: TList): boolean;

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
begin // TPosBase.Find
  Result := FALSE;

  if (not FCheckDBVersion) then
    exit;

  if (Assigned(moveEsts)) then
  begin
    for k := 0 to moveEsts.Count - 1 do
      Dispose(moveEsts[k]);
    moveEsts.Clear;
  end;

  if (fPos.Size = 0) then
    exit;

  r := 0;
  for k := 1 to 66 do // 65 - доп. инф, 66 - цвет.
  begin
here:
    fPos.SeekRec(r);
    fPos.Read(fn);

    r := fn.NextNode;
    while ((k <= 64) and (fn.btField <> ord(pos.board[FIELD_SEQ[k].i, FIELD_SEQ[k].j]))) or
          ((k = 65) and (fn.btField <> EncodeAddInf(pos))) or
          ((k = 66) and (fn.btField <> ord(pos.color))) do
    begin
      r := fn.NextValue;
      if r = 0 then
        exit
      else
        goto here;
    end; { while }
  end; { for }

  Result := TRUE;
  if (not Assigned(moveEsts)) then
    exit;

  // Filling the moves list
  repeat
    fMov.SeekRec(r);
    fMov.Read(mn);

    new(pme);
    pme^.move := DecodeMove(mn.wMove);
    pme^.estimate := mn.estimate;
    moveEsts.Add(pme);

    r := mn.NextValue;
  until (r = 0);
  
end;

////////////////////////////////////////////////////////////////////////////////
// TFieldNode

function TFieldNode.FGetNextNode: LongWord;
begin
  Result := (m_wNextNode shl 8) or m_btNextNode;
end;


procedure TFieldNode.FSetNextNode(lwValue: LongWord);
begin
  m_btNextNode := lwValue and $FF;
  m_wNextNode := lwValue shr 8;
end;


function TFieldNode.FGetNextValue: LongWord;
begin
  Result := (m_wNextValue shl 8) or m_btNextValue;
end;


procedure TFieldNode.FSetNextValue(lwValue: LongWord);
begin
  m_btNextValue := lwValue and $FF;
  m_wNextValue := lwValue shr 8;
end;

////////////////////////////////////////////////////////////////////////////////
// TMoveNode


function TMoveNode.FGetNextValuePos: LongWord;
begin
  Result := (m_wNextValue shl 8) or m_btNextValue;
end;


procedure TMoveNode.FSetNextValuePos(lwValue: LongWord);
begin
  m_btNextValue := lwValue and $FF;
  m_wNextValue := lwValue shr 8;
end;


procedure TMoveNode.EmptyNode;
begin
  FillChar(self, SizeOf(self), 0);
end;

////////////////////////////////////////////////////////////////////////////////
// TPosBaseStream

constructor TPosBaseStream.Create(const strFileName: string; RecordSize: integer);
var
  FileHandle: Integer;
begin
  inherited Create;

  m_iRecordSize := RecordSize;

  if (not FileExists(strFileName)) then
  begin
    FileHandle := FileCreate(strFileName);
    FileClose(FileHandle);
  end;

  m_InnerStream := TFileStream.Create(strFileName, fmOpenReadWrite,
    fmShareDenyWrite);
end;


destructor TPosBaseStream.Destroy;
begin
  m_InnerStream.Free;
  inherited;
end;


function TPosBaseStream.FGetSize: integer;
begin
  Result := (m_InnerStream.Size - m_iHeaderSize) div m_iRecordSize;
end;


procedure TPosBaseStream.SeekHeader;
begin
  m_InnerStream.Seek(0, soFromBeginning);
end;


procedure TPosBaseStream.SeekRec(lwRecordNumber: LongWord);
begin
  m_InnerStream.Seek(m_iHeaderSize + lwRecordNumber * m_iRecordSize, soFromBeginning);
end;


procedure TPosBaseStream.SeekEnd;
begin
  m_InnerStream.Seek(0, soFromEnd);
end;


procedure TPosBaseStream.Write(const Buffer);
begin
  m_InnerStream.WriteBuffer(Buffer, m_iRecordSize);
end;


procedure TPosBaseStream.Write(const Buffer; Count: integer);
begin
  m_InnerStream.WriteBuffer(Buffer, Count);
end;


procedure TPosBaseStream.Read(var Buffer);
begin
  m_InnerStream.ReadBuffer(Buffer, m_iRecordSize);
end;


procedure TPosBaseStream.Read(var Buffer; Count: integer);
begin
  m_InnerStream.ReadBuffer(Buffer, Count);
end;

end.
