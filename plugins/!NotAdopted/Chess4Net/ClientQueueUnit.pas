////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit ClientQueueUnit;

interface

type
  TClientName = string[25];

  PClientNode = ^TClientNode;

  TClientNode = record
    n: word;
    name: TClientName;
    handler: pointer;
    next: PClientNode;
  end;

  TClientQueue = object
    Number: word;
    procedure Add(const handler: pointer; const name: TClientName = '');
    procedure Remove(const handler: pointer);
    function GetName(const handler: pointer): TClientName;
    function GetNum(const handler: pointer): word;
    function GetHandler(const num: word): pointer;
{
    function Contains(const handler: pointer): boolean;
}
    constructor Create(const max: word);
    destructor Free;
  private
    first: PClientNode;
    MaxClientsNum: word;
  end;

implementation

function TClientQueue.GetHandler(const num: word): pointer;
var
  n: PClientNode;
begin
  n:= first;
  while n <> nil do
    begin
      if num = n^.n then
        begin
          Result:= n^.handler;
          exit;
        end;
      n:= n^.next;
    end;
  Result:= nil;
end;

procedure TClientQueue.Add(const handler: pointer; const name: TClientName);
var
  n: PClientNode;
  p: ^PClientNode;
begin
  if Number = MaxClientsNum then exit;

  n:= first; p:= @first;
  while n <> nil do
    begin
      p:= addr(n^.next);
      n:= n^.next;
    end;
  new(n); inc(Number);
  n^.name:= name;
  n^.handler:= handler;
  n^.next:= nil;
  n^.n:= Number;
  p^:= n;
end;

procedure TClientQueue.Remove(const handler: pointer);
var
  n: PClientNode;
  p: ^PClientNode;
begin
  n:= first; p:= @first;
  while n <> nil do
    begin
      if n^.handler = handler then
        begin
          n:= n^.next;
          dispose(p^);
          p^:= n;
          break;
        end;
      p:= addr(n^.next);
      n:= n^.next;      
    end;

  while n <> nil do
    begin
      dec(n^.n);
      n:= n^.next;
    end;
  dec(Number);
end;


function TClientQueue.GetName(const handler: pointer): TClientName;
var
  n: PClientNode;
begin
  n:= first;
  while n <> nil do
    begin
      if handler = n^.handler then
        begin
          Result:= n^.name;
          exit;
        end;
      n:= n^.next;
    end;
  Result:= '';
end;

function TClientQueue.GetNum(const handler: pointer): word;
var
  n: PClientNode;
begin
  n:= first;
  while n <> nil do
    begin
      if handler = n^.handler then
        begin
          Result:= n^.n;
          exit;
        end;
      n:= n^.next;
    end;
  Result:= 0;
end;

{
function TClientQueue.Contains(const handler: pointer): boolean;
}

constructor TClientQueue.Create(const max: word);
begin
  MaxClientsNum:= max;
end;


destructor TClientQueue.Free;
var
  n: PClientNode;
begin
  while first <> nil do
    begin
      n:= first^.next;
      dispose(first);
      first:= n;
    end;
  Number:= 0;  
end;

end.
