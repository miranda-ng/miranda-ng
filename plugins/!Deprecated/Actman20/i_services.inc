{Basic ActMan services}

function ActFreeList(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
begin
  result:=0;
  mFreeMem(PAnsiChar(lParam));
end;

function ActGetList(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
var
  pc:^tChain;
  p:pHKRecord;
  i,cnt:integer;
begin
  p:=@GroupList[0];
  cnt:=0;
  for i:=0 to MaxGroups-1 do
  begin
    if (p^.flags and (ACF_ASSIGNED or ACF_VOLATILE))=ACF_ASSIGNED then inc(cnt);
    inc(p);
  end;
  result:=cnt;
  if lParam=0 then exit;
  if cnt>0 then
  begin
    mGetMem(pc,cnt*SizeOf(tChain)+4);
    puint_ptr(lParam)^:=uint_ptr(pc);
//    {$IFDEF WIN64}pqword{$ELSE}pdword{$ENDIF}(lParam)^:=uint_ptr(pc);
    pdword(pc)^:=SizeOf(tChain);
    inc(PByte(pc),4);

    p:=@GroupList[0];
    for i:=0 to MaxGroups-1 do
    begin
      if (p^.flags and (ACF_ASSIGNED or ACF_VOLATILE))=ACF_ASSIGNED then
      begin
        pc^.descr:=p^.descr;
        pc^.id   :=p^.id;
        pc^.flags:=p^.flags;
        inc(pc);
      end;
      inc(p);
    end;
  end
  else
    puint_ptr(lParam)^:=0;
//    {$IFDEF WIN64}pqword{$ELSE}pdword{$ENDIF}(lParam)^:=0;
end;

function ActRun(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
var
  i:integer;
  p:pHKRecord;
begin
  result:=-1;
  p:=@GroupList[0];
  for i:=0 to MaxGroups-1 do
  begin
    if ((p^.flags and ACF_ASSIGNED)<>0) and (p^.id=dword(wParam)) then
    begin
      result:=p^.firstAction;
      break;
    end;
    inc(p);
  end;
  if result>0 then
    result:=ActionStarter(result,lParam,p);
end;

function ActRunGroup(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
var
  i:integer;
  p:pHKRecord;
begin
  result:=-1;
  p:=@GroupList[0];
  for i:=0 to MaxGroups-1 do
  begin
    if ((p^.flags and ACF_ASSIGNED)<>0) and (StrCmpW(p^.descr,pWideChar(wParam))=0) then
    begin
      result:=p^.firstAction;
      break;
    end;
    inc(p);
  end;
  if result>0 then
    result:=ActionStarter(result,lParam,p);
end;

function ActRunParam(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
var
  i:integer;
  p:pHKRecord;
begin
  result:=-1;
  p:=@GroupList[0];
  
  if (pAct_Param(lParam)^.flags and ACTP_BYNAME)=0 then
  begin
    for i:=0 to MaxGroups-1 do
    begin
      if ((p^.flags and ACF_ASSIGNED)<>0) and (p^.id=pAct_Param(lParam)^.Id) then
      begin
        result:=p^.firstAction;
        break;
      end;
      inc(p);
    end;
  end
  else
  begin
    for i:=0 to MaxGroups-1 do
    begin
      if ((p^.flags and ACF_ASSIGNED)<>0) and
         (StrCmpW(p^.descr,pWideChar(pAct_Param(lParam)^.Id))=0) then
      begin
        result:=p^.firstAction;
        break;
      end;
      inc(p);
    end;
  end;

  if result>0 then
  begin
    if (pAct_Param(lParam)^.flags and ACTP_WAIT)=0 then
      result:=ActionStarter    (result,pAct_Param(lParam)^.wParam,p,pAct_Param(lParam)^.lParam)
    else
      result:=ActionStarterWait(result,pAct_Param(lParam)^.wParam,p,pAct_Param(lParam)^.lParam);
  end;
end;
