unit uRect;

interface

uses windows;

{
type
  PPoint = ^TPoint;
  TPoint = packed record
    X: Longint;
    Y: Longint;
  end;

  PSmallPoint = ^TSmallPoint;
  TSmallPoint = packed record
    x: SmallInt;
    y: SmallInt;
  end;

  PRect = ^TRect;
  TRect = packed record
    case integer of
      0: (Left, Top, Right, Bottom: Longint);
      1: (TopLeft, BottomRight: TPoint);
  end;
}
{
-function SetRect(var lprc: TRect; xLeft, yTop, xRight, yBottom: integer): bool; stdcall;
-function CopyRect(var lprcDst: TRect; const lprcSrc: TRect): bool; stdcall;
-function InflateRect(var lprc: TRect; dx, dy: integer): bool; stdcall;
function SubtractRect(var lprcDst: TRect; const lprcSrc1, lprcSrc2: TRect): bool; stdcall;
-function SetRectEmpty(var lprc: TRect): bool; stdcall;
-function IntersectRect(var lprcDst: TRect; const lprcSrc1, lprcSrc2: TRect): bool; stdcall;
-function UnionRect(var lprcDst: TRect; const lprcSrc1, lprcSrc2: TRect): bool; stdcall;
-function OffsetRect(var lprc: TRect; dx, dy: integer): bool; stdcall;
-function IsRectEmpty(const lprc: TRect): bool; stdcall;
-function EqualRect(const lprc1, lprc2: TRect): bool; stdcall;
-function PtInRect(const lprc: TRect; pt: TPoint): bool; stdcall;
}

function  Point(X, Y: integer): TPoint; overload;
procedure Point(var pt:TPoint; X, Y: integer); overload;

function SmallPoint(X, Y: integer): TSmallPoint; overload;
function SmallPoint(XY: longword): TSmallPoint; overload;

function  CenterPoint(const Rect: TRect): TPoint; overload;
procedure CenterPoint(const Rect: TRect; var pt:TPoint); overload;
function  PointInRect(const P: TPoint; const Rect: TRect): Boolean;
function  PtInRect   (const Rect: TRect; const P: TPoint): Boolean;

function  Rect   (Left, Top, Right, Bottom: integer): TRect; overload;
procedure Rect   (var Rect:TRect; Left, Top, Right, Bottom: integer); overload;
procedure SetRect(var Rect:TRect; Left, Top, Right, Bottom: integer);
function  CopyRect(var Rect: TRect; const R1: TRect):Boolean;
function  SetRectEmpty(var Rect: TRect): Boolean;

function  Bounds(ALeft, ATop, AWidth, AHeight: integer): TRect; overload;
procedure Bounds(var Rect:TRect; ALeft, ATop, AWidth, AHeight: integer); overload;

function EqualRect(const R1, R2: TRect): Boolean;
function IsRectInside(const R1, R2: TRect): Boolean;
function IsRectEmpty(const Rect: TRect): Boolean;
function InflateRect(var Rect: TRect; dx, dy: integer): Boolean;
function OffsetRect(var Rect: TRect; DX: integer; DY: integer): Boolean;
function IntersectRect(var Rect: TRect; const R1, R2: TRect): Boolean; overload;
function IntersectRect(const R1, R2: TRect): Boolean; overload;
function UnionRect(var Rect: TRect; const R1, R2: TRect): Boolean;


implementation

function Point(X, Y: integer): TPoint;
begin
  Result.X := X;
  Result.Y := Y;
end;

procedure Point(var pt:TPoint; X, Y: integer);
begin
  pt.X := X;
  pt.Y := Y;
end;

function SmallPoint(X, Y: integer): TSmallPoint;
begin
  Result.X := X;
  Result.Y := Y;
end;

function SmallPoint(XY: longword): TSmallPoint;
begin
  Result.X := SmallInt(XY and $0000FFFF);
  Result.Y := SmallInt(XY shr 16);
end;

function PointInRect(const P: TPoint; const Rect: TRect): Boolean;
begin
  Result :=
    (P.X >= Rect.Left)  and
    (P.X <  Rect.Right) and
    (P.Y >= Rect.Top)   and
    (P.Y <  Rect.Bottom);
end;

function PtInRect(const Rect: TRect; const P: TPoint): Boolean;
begin
  Result :=
    (P.X >= Rect.Left)  and
    (P.X <  Rect.Right) and
    (P.Y >= Rect.Top)   and
    (P.Y <  Rect.Bottom);
end;

function CenterPoint(const Rect: TRect): TPoint;
begin
  with Rect do
  begin
    Result.X := (Right  - Left) div 2 + Left;
    Result.Y := (Bottom - Top ) div 2 + Top;
  end;
end;

procedure CenterPoint(const Rect: TRect; var pt:TPoint);
begin
  with Rect do
  begin
    pt.X := (Right  - Left) div 2 + Left;
    pt.Y := (Bottom - Top ) div 2 + Top;
  end;
end;

//----- TRect -----

function Rect(Left, Top, Right, Bottom: integer): TRect;
begin
  Result.Left   := Left;
  Result.Top    := Top;
  Result.Bottom := Bottom;
  Result.Right  := Right;
end;

procedure Rect(var Rect:TRect; Left, Top, Right, Bottom: integer);
begin
  Rect.Left   := Left;
  Rect.Top    := Top;
  Rect.Bottom := Bottom;
  Rect.Right  := Right;
end;

procedure SetRect(var Rect:TRect; Left, Top, Right, Bottom: integer);
begin
  Rect.Left   := Left;
  Rect.Top    := Top;
  Rect.Bottom := Bottom;
  Rect.Right  := Right;
end;

function CopyRect(var Rect: TRect; const R1: TRect): Boolean;
begin
  Rect := R1;
  Result := not IsRectEmpty(Rect);
end;

function SetRectEmpty(var Rect: TRect): Boolean;
begin
  FillChar(Rect,SizeOf(Rect),0);
  Result := True;
end;

function EqualRect(const R1, R2: TRect): Boolean;
begin
  Result :=
    (R1.Left   = R2.Left)  and
    (R1.Right  = R2.Right) and
    (R1.Top    = R2.Top)   and
    (R1.Bottom = R2.Bottom);
end;

function IsRectInside(const R1, R2: TRect): Boolean;
begin
  Result :=
    (R1.Left   >= R2.Left)  and
    (R1.Right  <= R2.Right) and
    (R1.Top    >= R2.Top)   and
    (R1.Bottom <= R2.Bottom);
end;

function IsRectEmpty(const Rect: TRect): Boolean;
begin
  Result := (Rect.Right <= Rect.Left) or (Rect.Bottom <= Rect.Top);
end;

function IntersectRect(const R1, R2: TRect): Boolean;
var
  ml,mr,mt,mb:longint;
begin
  if R1.Left   > R2.Left   then ml := R1.Left   else ml := R2.Left;
  if R1.Right  < R2.Right  then mr := R1.Right  else mr := R2.Right;
  if R1.Top    > R2.Top    then mt := R1.Top    else mt := R2.Top;
  if R1.Bottom < R2.Bottom then mb := R1.Bottom else mb := R2.Bottom;

  Result := (ml < mr) and (mt < mb);
end;

function IntersectRect(var Rect: TRect; const R1, R2: TRect): Boolean;
var
  lRect:TRect;
begin
  lRect := R1;
  if R2.Left   > R1.Left   then lRect.Left   := R2.Left;
  if R2.Top    > R1.Top    then lRect.Top    := R2.Top;
  if R2.Right  < R1.Right  then lRect.Right  := R2.Right;
  if R2.Bottom < R1.Bottom then lRect.Bottom := R2.Bottom;
  Result := not IsRectEmpty(lRect);
  if not Result then FillChar(lRect, SizeOf(lRect), 0);
  Rect:=lRect;
end;

function UnionRect(var Rect: TRect; const R1, R2: TRect): Boolean;
begin
  Rect := R1;
  if not IsRectEmpty(R2) then
  begin
    if R2.Left   < R1.Left   then Rect.Left   := R2.Left;
    if R2.Top    < R1.Top    then Rect.Top    := R2.Top;
    if R2.Right  > R1.Right  then Rect.Right  := R2.Right;
    if R2.Bottom > R1.Bottom then Rect.Bottom := R2.Bottom;
  end;
  Result := not IsRectEmpty(Rect);
  if not Result then FillChar(Rect, SizeOf(Rect), 0);
end;

function InflateRect(var Rect: TRect; dx, dy: integer): Boolean;
begin
  with Rect do
  begin
    Left   := Left   - dx;
    Right  := Right  + dx;
    Top    := Top    - dy;
    Bottom := Bottom + dy;
  end;
  Result := not IsRectEmpty(Rect);
end;

function OffsetRect(var Rect: TRect; DX: integer; DY: integer): Boolean;
begin
  if @Rect <> nil then // Test to increase compatiblity with Windows
  begin
    Inc(Rect.Left  , DX);
    Inc(Rect.Right , DX);
    Inc(Rect.Top   , DY);
    Inc(Rect.Bottom, DY);
    Result := True;
  end
  else
    Result := False;
end;

function Bounds(ALeft, ATop, AWidth, AHeight: integer): TRect;
begin
  with Result do
  begin
    Left   := ALeft;
    Top    := ATop;
    Right  := ALeft + AWidth;
    Bottom := ATop  + AHeight;
  end;
end;

procedure Bounds(var Rect:TRect; ALeft, ATop, AWidth, AHeight: integer);
begin
  with Rect do
  begin
    Left   := ALeft;
    Top    := ATop;
    Right  := ALeft + AWidth;
    Bottom := ATop  + AHeight;
  end;
end;

end.
