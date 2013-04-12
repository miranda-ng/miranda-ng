unit variants;

interface

var
  Null: Variant;          { Null standard constant }

implementation

initialization
  TVarData(Null).VType := varNull;
end.