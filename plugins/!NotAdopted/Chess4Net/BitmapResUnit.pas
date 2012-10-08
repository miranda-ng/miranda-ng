////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit BitmapResUnit;

interface

uses
  Graphics, Types,
  // Chess4Net
  ChessBoardHeaderUnit, ChessRulesEngine;

type
  // Bitmap resources - introduced for 2009.1 (sizable board feature)
  TBitmapRes = class // parametrizable class factory
  private
    m_ResSet: TBitmap;
    m_iSetNumber: integer;
    m_iSquareSize: integer;
    procedure FCalculateClientBoardSizes(InitialSize: TSize);
    function FGetOptimalBoardSize(const ClientSize: TSize; out iSetNumber: integer): TSize;
    procedure FCalculateSetNumberFromSquareSize;
    procedure FLoadPieceSet(iSetNumber: integer);
    function FGetBoardResName(iSetNumber: integer): string;
    function FGetSetResName(iSetNumber: integer): string;
  public
    constructor Create(const ClientBoardSize: TSize);  overload;
    constructor Create(iSquareSize: integer); overload;
    destructor Destroy; override;
    procedure CreateBoardBitmap(ClientBoardSize: TSize; const BackgroundColor: TColor;
      out Bitmap: TBitmap);
    procedure CreateFigureBitmap(const Figure: TFigure; out Bitmap: TBitmap);
    function GetOptimalBoardSize(ClientSize: TSize): TSize;
    property SquareSize: integer read m_iSquareSize;
  end;

implementation

{$R ChessSet_PNG.RES}

uses
  SysUtils, Classes, Math, pngimage;

{$J+}

const
  CHB_RES_X = 4; CHB_RES_Y = 4; // starting coordinates of A8 field in resources

var
  arrClientBoardSizes: array[1..7] of TSize;
  g_bClientBoardSizesCalculated: boolean = FALSE;

////////////////////////////////////////////////////////////////////////////////
// TBitmapRes

constructor TBitmapRes.Create(const ClientBoardSize: TSize);
begin
  inherited Create;
  FCalculateClientBoardSizes(ClientBoardSize);
end;


constructor TBitmapRes.Create(iSquareSize: integer);
begin
  inherited Create;
  m_iSquareSize := iSquareSize;
end;


destructor TBitmapRes.Destroy;
begin
  m_ResSet.Free;
  inherited;
end;


procedure TBitmapRes.CreateBoardBitmap(ClientBoardSize: TSize; const BackgroundColor: TColor;
  out Bitmap: TBitmap);
var
  Png: TPngObject;
  ResBoard: TBitmap;
  iSetNumber: integer;
begin
  Png := nil;
  ResBoard := nil;

  FGetOptimalBoardSize(ClientBoardSize, iSetNumber);

  if (iSetNumber = 0) then
    exit;

  Bitmap := TBitMap.Create;
  with Bitmap do
  try
    Png := TPngObject.Create;
    Png.LoadFromResourceName(HInstance, FGetBoardResName(iSetNumber));
    ResBoard := TBitmap.Create;
    ResBoard.Assign(Png);

    Width := arrClientBoardSizes[iSetNumber].cx;
    Height := arrClientBoardSizes[iSetNumber].cy;
    Canvas.Brush.Color := BackgroundColor;
    Canvas.FillRect(Bounds(0, 0, Width, Height));
    Canvas.Draw(CHB_X - CHB_RES_X, CHB_Y - CHB_RES_Y, ResBoard);

    // Load appropriate set
    FLoadPieceSet(iSetNumber);

  finally;
    m_iSetNumber := iSetNumber;
    ResBoard.Free;
    Png.Free;
  end;
end;


procedure TBitmapRes.FLoadPieceSet(iSetNumber: integer);
var
  Png: TPngObject;
begin
  if (Assigned(m_ResSet) and (iSetNumber = m_iSetNumber)) then
    exit;

  FreeAndNil(m_ResSet);

  Png := TPngObject.Create;
  try
    Png.LoadFromResourceName(HInstance, FGetSetResName(iSetNumber));
    m_ResSet := TBitmap.Create;
    m_ResSet.Assign(Png);

    m_iSquareSize := m_ResSet.Height;
  finally
    Png.Free;
  end;
end;


procedure TBitmapRes.CreateFigureBitmap(const Figure: TFigure; out Bitmap: TBitmap);
const
  PNG_SET_POS: array[TFigure] of integer = (2, 4, 6, 8, 10, 12, 0, 3, 5, 7, 9, 11, 13);
var
  iWidth: integer;
begin
  if (m_iSetNumber = 0) then
  begin
    FCalculateSetNumberFromSquareSize;
    if (m_iSetNumber = 0) then
      exit;
  end;

  FLoadPieceSet(m_iSetNumber);

  iWidth := IfThen((Figure = ES), m_iSquareSize + m_iSquareSize, m_iSquareSize);

  Bitmap := TBitMap.Create;
  Bitmap.Width := iWidth;
  Bitmap.Height := m_iSquareSize;

  Bitmap.Canvas.CopyRect(Bounds(0, 0, iWidth, m_iSquareSize), m_ResSet.Canvas,
    Bounds(m_iSquareSize * PNG_SET_POS[Figure], 0, iWidth, m_iSquareSize));
  Bitmap.Transparent:= TRUE;
end;


procedure TBitmapRes.FCalculateSetNumberFromSquareSize;
var
  i: integer;
begin
  m_iSetNumber := 0;

  with TPngObject.Create do
  try
    for i := High(arrClientBoardSizes) downto Low(arrClientBoardSizes) do
    begin
      LoadFromResourceName(HInstance, FGetSetResName(i));

      if (Height <= m_iSquareSize) then
      begin
        m_iSetNumber := i;
        exit;
      end;
    end;

  finally
    Free;
  end;

end;


function TBitmapRes.GetOptimalBoardSize(ClientSize: TSize): TSize;
var
  iDummy: integer;
begin
  Result := FGetOptimalBoardSize(ClientSize, iDummy);
end;


function TBitmapRes.FGetOptimalBoardSize(const ClientSize: TSize; out iSetNumber: integer): TSize;
var
  i: integer;
begin
  iSetNumber := 0;

  for i := High(arrClientBoardSizes) downto Low(arrClientBoardSizes) do
  begin
    if ((ClientSize.cx >= arrClientBoardSizes[i].cx) and
        (ClientSize.cy >= arrClientBoardSizes[i].cy)) then
    begin
      Result := arrClientBoardSizes[i];
      iSetNumber := i;
      exit;
    end;
  end; { for i }

  Result := Size(0, 0);
end;


procedure TBitmapRes.FCalculateClientBoardSizes(InitialSize: TSize);
var
  i: integer;
  iOptimal: integer;
  iAddX, iAddY: integer;
begin
  if (g_bClientBoardSizesCalculated) then
    exit;

  // Load board sizes from resources
  with TPngObject.Create do
  try
    for i := Low(arrClientBoardSizes) to High(arrClientBoardSizes) do
    begin
      LoadFromResourceName(HInstance, FGetBoardResName(i));
      arrClientBoardSizes[i] := Size(Width, Height);
    end;
  finally
    Free;
  end;

  // Find optimal board size from resources
  iOptimal := 0;
  for i := High(arrClientBoardSizes) downto Low(arrClientBoardSizes) do
  begin
    if ((InitialSize.cx > (arrClientBoardSizes[i].cx + CHB_X - CHB_RES_X)) and
        (InitialSize.cy > (arrClientBoardSizes[i].cy + CHB_Y - CHB_RES_Y))) then
    begin
      iOptimal := i;
      break;
    end;
  end;
  Assert(iOptimal > 0);

  // Calculate board sizes for client
  iAddX := InitialSize.cx - arrClientBoardSizes[iOptimal].cx;
  iAddY := InitialSize.cy - arrClientBoardSizes[iOptimal].cy;
  for i := Low(arrClientBoardSizes) to High(arrClientBoardSizes) do
  begin
    inc(arrClientBoardSizes[i].cx, iAddX);
    inc(arrClientBoardSizes[i].cy, iAddY);
  end;

  g_bClientBoardSizesCalculated := TRUE;
end;


function TBitmapRes.FGetBoardResName(iSetNumber: integer): string;
begin
  Result := 'BOARD' + IntToStr(iSetNumber);
end;


function TBitmapRes.FGetSetResName(iSetNumber: integer): string;
begin
  Result := 'SET' + IntToStr(iSetNumber);
end;

end.
