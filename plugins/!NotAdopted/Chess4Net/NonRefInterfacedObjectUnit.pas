////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit NonRefInterfacedObjectUnit;

interface

type
  TNonRefInterfacedObject = class(TObject, IInterface)
  protected
    function QueryInterface(const IID: TGUID; out Obj): HResult; stdcall;
    function _AddRef: Integer; stdcall;
    function _Release: Integer; stdcall;
  end;

implementation

////////////////////////////////////////////////////////////////////////////////
// TNonRefInterfacedObject

function TNonRefInterfacedObject.QueryInterface(const IID: TGUID; out Obj): HResult;
begin
  if GetInterface(IID, Obj) then
    Result := 0
  else
    Result := E_NOINTERFACE;
end;


function TNonRefInterfacedObject._AddRef: Integer;
begin
  Result := -1;
end;


function TNonRefInterfacedObject._Release: Integer;
begin
  Result := -1;
end;

end.
