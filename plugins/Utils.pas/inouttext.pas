unit inouttext;

interface

type
  TTextExport = class
    private
      FCurrent:PAnsiChar;
      FItems  :array of PAnsiChar;
      FItem   :integer;
      FIndent :integer;
      FLineLen:integer;
      FDoTab  :boolean;

      function AddTab:boolean;
      function GetItems(i:integer):PAnsiChar;

    public
      constructor Create(cnt:integer);
      destructor  Destroy; override;

      function  NextItem:integer;
      procedure EndItem;
      procedure addText (name:PAnsichar; value:PAnsiChar);
      procedure addTextW(name:PAnsichar; value:PWideChar);
      procedure addDWord(name:PAnsichar; value:cardinal);
      procedure addFlag (name:PAnsichar; value:boolean=true);
      procedure addNewLine;
      procedure ShiftRight;
      procedure ShiftLeft;

      property Items[i:integer]:PAnsiChar read GetItems;
  end;

implementation

uses Common;

const
  BUFSIZE = 65536;

constructor TTextExport.Create(cnt:integer);
begin
  SetLength(FItems,cnt);
  FIndent :=0;
  FDoTab  :=true;
  FLineLen:=0;
  FItem   :=-1;
end;

destructor TTextExport.Destroy;
var
  i:integer;
begin
  for i:=0 to HIGH(FItems) do
    mFreeMem(FItems[i]);
  FItems:=nil;

  inherited;
end;

function TTextExport.GetItems(i:integer):PAnsiChar;
begin
  if (i>=0) and (i<HIGH(FItems)) then
    result:=FItems[i]
  else
    result:=nil;
end;

function TTextExport.NextItem:integer;
begin
  inc(FItem);
  mGetMem(FItems[FItem],BUFSIZE);
  FCurrent:=FItems[FItem];
  result:=FItem;
end;

procedure TTextExport.EndItem;
var
  p:PAnsiChar;
begin
  if FItem>=0 then
  begin
    FCurrent^:=#0;
    p:=FItems[FItem];
    StrDup(FItems[FItem],p);
    mFreeMem(p);
  end;
end;

function TTextExport.AddTab:boolean;
var
  i:integer;
begin
  result:=FDoTab;
  if FDoTab then
  begin
    for i:=0 to FIndent-1 do
    begin
      FCurrent^:=' '; inc(FCurrent); inc(FLineLen);
    end;
    FDoTab:=false;
  end;
end;

procedure TTextExport.addText(name:PAnsichar; value:PAnsiChar);
begin
  if not AddTab then
    if (FCurrent-1)^ <> #10 then //!!
    begin
      FCurrent^:=' '; inc(FCurrent); inc(FLineLen);
    end;

  FCurrent:=StrCopyE(FCurrent,name); FCurrent^:='='; inc(FCurrent); inc(FLineLen, StrLen(name)+1);
  // check and make borders ', " or []
  FCurrent^:='"'; inc(FCurrent); inc(FLineLen);
  // delim-start
  FCurrent:=StrCopyE(FCurrent,value);
  // delim-end
  FCurrent^:='"'; inc(FCurrent); inc(FLineLen);
end;

procedure TTextExport.addTextW(name:PAnsichar; value:PWideChar);
var
  pc:PAnsiChar;
begin
  if not AddTab then
    if (FCurrent-1)^ <> #10 then //!!
    begin
      FCurrent^:=' '; inc(FCurrent); inc(FLineLen);
    end;

  FCurrent:=StrCopyE(FCurrent,name); FCurrent^:='='; inc(FCurrent); inc(FLineLen, StrLen(name)+1);
  // check and make borders ', " or []
  FCurrent^:='"'; inc(FCurrent); inc(FLineLen);
  // delim-start
  FCurrent:=StrCopyE(FCurrent,WideToUTF8(value, pc));
  // delim-end
  FCurrent^:='"'; inc(FCurrent); inc(FLineLen);

  mFreeMem(pc);
end;

procedure TTextExport.addDWord(name:PAnsichar; value:cardinal);
var
  p:PAnsiChar;
begin
  if not AddTab then
  begin
    FCurrent^:=' '; inc(FCurrent); inc(FLineLen);
  end;
  FCurrent:=StrCopyE(FCurrent,name);
  inc(FLineLen,StrLen(name));
  FCurrent^:='='; inc(FCurrent); inc(FLineLen);
  p:=IntToStr(FCurrent,value);
  inc(FLineLen,StrLen(p));
  FCurrent:=StrEnd(p);
end;

procedure TTextExport.addFlag(name:PAnsichar; value:boolean=true);
begin
  // if set "name=0/1", change here
  if value then
  begin
    if not AddTab then
    begin
      FCurrent^:=' '; inc(FCurrent); inc(FLineLen);
    end;
    FCurrent:=StrCopyE(FCurrent,name);
    inc(FLineLen,StrLen(name));
  end;
end;

procedure TTextExport.addNewLine;
begin
  FCurrent^:=#13; inc(FCurrent);
  FCurrent^:=#10; inc(FCurrent);
  FDoTab  :=true;
  FLineLen:=0;
end;

procedure TTextExport.ShiftRight;
begin
  inc(FIndent,2);
end;

procedure TTextExport.ShiftLeft;
begin
  dec(FIndent,2);
  if FIndent<0 then
    FIndent:=0;
end;

end.
