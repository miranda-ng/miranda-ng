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
  public
    constructor Create(const ClientBoardSize: TSize);
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
  g_BitmapResInstance: TBitmapRes = nil;
  arrClientBoardSizes: array[1..7] of TSize;
  bClientBoardSizesCalculated: boolean = FALSE;

////////////////////////////////////////////////////////////////////////////////
// TBitmapRes

constructor TBitmapRes.Create(const ClientBoardSize: TSize);
begin
  if (not bClientBoardSizesCalculated) then
    FCalculateClientBoardSizes(ClientBoardSize);
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
begin
  Png := nil;
  ResBoard := nil;
  m_iSquareSize := 0;

  GetOptimalBoardSize(ClientBoardSize); // To refresh m_iSetNumber
  if (m_iSetNumber = 0) then
    exit;

  Bitmap := TBitMap.Create;
  with Bitmap do
  try
    Png := TPngObject.Create;
    Png.LoadFromResourceName(HInstance, 'BOARD' + IntToStr(m_iSetNumber));
    ResBoard := TBitmap.Create;
    ResBoard.Assign(Png);

    Width := arrClientBoardSizes[m_iSetNumber].cx;
    Height := arrClientBoardSizes[m_iSetNumber].cy;
    Canvas.Brush.Color := BackgroundColor;
    Canvas.FillRect(Bounds(0, 0, Width, Height));
    Canvas.Draw(CHB_X - CHB_RES_X, CHB_Y - CHB_RES_Y, ResBoard);

    // Load appropriate set
    FreeAndNil(m_ResSet);
    Png.LoadFromResourceName(HInstance, 'SET' + IntToStr(m_iSetNumber));
    m_ResSet := TBitmap.Create;
    m_ResSet.Assign(Png);

    m_iSquareSize := m_ResSet.Height;
  finally;
    ResBoard.Free;
    Png.Free;
  end;
end;


procedure TBitmapRes.CreateFigureBitmap(const Figure: TFigure; out Bitmap: TBitmap);
const
  PNG_SET_POS: array[TFigure] of integer = (2, 4, 6, 8, 10, 12, 0, 3, 5, 7, 9, 11, 13);
var
  iSquareSize, iWidth: integer;
begin
  if (m_iSetNumber = 0) then
    exit;

  iSquareSize := m_ResSet.Height;   

  iWidth := IfThen((Figure = ES), iSquareSize + iSquareSize, iSquareSize);

  Bitmap := TBitMap.Create;
  Bitmap.Width := iWidth;
  Bitmap.Height := iSquareSize;

  Bitmap.Canvas.CopyRect(Bounds(0, 0, iWidth, iSquareSize), m_ResSet.Canvas,
    Bounds(iSquareSize * PNG_SET_POS[Figure], 0, iWidth, iSquareSize));
  Bitmap.Transparent:= TRUE;
end;


function TBitmapRes.GetOptimalBoardSize(ClientSize: TSize): TSize;
var
  i: integer;
begin
  m_iSetNumber := 0;

  for i := High(arrClientBoardSizes) downto Low(arrClientBoardSizes) do
  begin
    if ((ClientSize.cx >= arrClientBoardSizes[i].cx) and
        (ClientSize.cy >= arrClientBoardSizes[i].cy)) then
    begin
      Result := arrClientBoardSizes[i];
      m_iSetNumber := i;
      exit;
    end;
  end; { for i }

  Result := Size(0, 0);
end;


procedure TBitmapRes.FCalculateClientBoardSizes(InitialSize: TSize);
var
  i: integer;
  strResName: string;
  iOptimal: integer;
  iAddX, iAddY: integer;
begin
  // Load board sizes from resources
  with TPngObject.Create do
  try
    for i := Low(arrClientBoardSizes) to High(arrClientBoardSizes) do
    begin
      strResName := 'BOARD' + IntToStr(i);
      LoadFromResourceName(HInstance, strResName);
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

  bClientBoardSizesCalculated := TRUE;
end;

end.
