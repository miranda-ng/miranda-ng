unit ChessBoardHeaderUnit;

interface

uses
  Types;

const
  CHB_X = 20; CHB_Y = 6;   // starting coordinates of A8 field

function Size(const iX, iY: integer): TSize;

implementation

function Size(const iX, iY: integer): TSize;
begin
  Result.cx := iX;
  Result.cy := iY;
end;

end.
