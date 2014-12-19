{}
const
  opt_hook  :PAnsiChar = 'Hook';
  opt_hooks :PAnsiChar = 'Hooks';
  opt_count :PAnsiChar = 'numhooks';
  opt_flags :PAnsiChar = 'flags';
  opt_descr :PAnsiChar = 'descr';
  opt_name  :PAnsiChar = 'name';
  opt_action:PAnsiChar = 'action';

procedure SaveHooks;
var
  section:array [0..63] of AnsiChar;
  p,p1:PAnsiChar;
  i,amount:integer;
begin
  DBDeleteGroup(0,DBBranch,opt_hooks);
  amount:=0;
  p1:=StrCopyE(section,opt_hooks);
  p1^:='/'; inc(p1);
  p1:=StrCopyE(p1,opt_hook);
  for i:=0 to MaxHooks-1 do
  begin
    if (HookList[i].flags and ACF_ASSIGNED)=0 then
      continue;

    p:=StrEnd(IntToStr(p1,amount));
    p^:='/'; inc(p);

    with HookList[i] do
    begin
      StrCopy(p,opt_flags ); DBWriteDWord  (0,DBBranch,section,flags);
      StrCopy(p,opt_descr ); DBWriteUnicode(0,DBBranch,section,descr);
      StrCopy(p,opt_name  ); DBWriteString (0,DBBranch,section,name);
      StrCopy(p,opt_action); DBWriteDWord  (0,DBBranch,section,action);
    end;
    inc(amount);
  end;
  DBWriteByte(0,DBBranch,opt_count,amount);
end;

function LoadHooks:integer;
var
  section:array [0..63] of AnsiChar;
  p,p1:PAnsiChar;
  i:integer;
begin
  MaxHooks:=DBReadByte(0,DBBranch,opt_count);
  result:=MaxHooks;
  if MaxHooks>0 then
  begin
    GetMem  (HookList ,MaxHooks*SizeOf(tHookRec));
    FillChar(HookList^,MaxHooks*SizeOf(tHookRec),0);
    p1:=StrCopyE(section,opt_hooks);
    p1^:='/'; inc(p1);
    p1:=StrCopyE(p1,opt_hook);
    for i:=0 to MaxHooks-1 do
    begin
      p:=StrEnd(IntToStr(p1,i));
      p^:='/'; inc(p);

      with HookList[i] do
      begin
        StrCopy(p,opt_flags ); flags :=DBReadDWord  (0,DBBranch,section);
        StrCopy(p,opt_descr ); descr :=DBReadUnicode(0,DBBranch,section);
        StrCopy(p,opt_name  ); name  :=DBReadString (0,DBBranch,section);
        StrCopy(p,opt_action); action:=DBReadDWord  (0,DBBranch,section);
      end;
    end;
  end;
end;
