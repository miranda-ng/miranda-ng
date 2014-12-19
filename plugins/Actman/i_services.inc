{Basic ActMan services}

function ActSelect(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
begin
  if odd(wParam) then
    result:=int_ptr(MacroList.GetMacro(lParam))
  else
    result:=int_ptr(MacroList.GetMacro(pWideChar(lParam)));

  if result<>0 then
    with pMacroRecord(result)^ do
    begin
      if (wParam and 4)<>0 then
        result:=ord((flags and ACF_SELECTED)<>0)
      else
      begin
        if (wParam and 2)<>0 then
          flags:=flags and not ACF_SELECTED
        else
          flags:=flags or      ACF_SELECTED;
      end;
    end;
end;

function ActFreeList(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
begin
  result:=0;
  mFreeMem(PAnsiChar(lParam));
end;

function ActGetList(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
var
  pc:^tChain;
  p:pMacroRecord;
  i,cnt:integer;
begin
  p:=MacroList[0];
  cnt:=0;
  for i:=0 to MacroList.Count-1 do
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
    pdword(pc)^:=SizeOf(tChain);
    inc(PByte(pc),4);

    p:=MacroList[0];
    for i:=0 to MacroList.Count-1 do
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
end;

//====================== Execute code =======================

{$IFDEF DEBUG}
procedure ActmanMacroDebug(txt:pWideChar);
begin
  OutputDebugStringW(txt);
end;
{$ENDIF}

procedure DoAction(Macro:pMacroRecord;var WorkData:tWorkData);
var
  res:LRESULT;
  i,cnt:integer;
{$IFDEF DEBUG}
  buf:array [0..1023] of WideChar;
  buf1,buf2:array [0..31] of WideChar;
  p:pWideChar;
{$ENDIF}
begin
  cnt:=Macro^.ActionCount;
  if cnt<>0 then
  begin
//!!    act:=CloneActions(action);
    i:=0;
    WorkData.ActionList :=Macro^.ActionList;
    WorkData.ActionCount:=Macro^.ActionCount;
    while i<cnt do
    begin
      if (Macro^.ActionList^[i].flags and ACF_DISABLED)=0 then
      begin
{$IFDEF DEBUG}
        if WorkData.ResultType=rtWide then
          p:=pWideChar(WorkData.LastResult)
        else
          p:=IntToStr(buf2,WorkData.LastResult);
        StrCopyEW(
          StrCopyEW(
            StrCopyEW(
              StrCopyEW(
                StrCopyEW(
                  StrCopyEW(
                    StrCopyEW(
                      StrCopyEW(buf,'ActMan: Macro "'),
                      Macro^.descr
                    ),'", Action #'
                  ),IntToStr(buf1,i)
                ),' "'
              ),Macro^.ActionList^[i].ActionDescr
            ),'", LR: '
          ),p{IIF(WorkData.ResultType=rtWide,
                pWideChar(WorkData.LastResult),
                IntToStr(buf1,WorkData.LastResult))}
        );
        ActmanMacroDebug(buf);
{$ENDIF}
        res:=Macro^.ActionList^[i].DoAction(WorkData);
        if res<0 then break
        else if res>0 then // res = next action number+1
        begin
          i:=res-1;
          continue;
        end;
      end;
      inc(i);
    end;
//!!    FreeActionsContinued(act_org);
  end;
end;

type
  pActStartData = ^tActStartData;
  tActStartData = record
    macro   :pMacroRecord;
    event   :THANDLE;
    WorkData:tWorkData;
    flags   :dword;
  end;

procedure ThDoAction(arg:pActStartData); cdecl;
var
  i:integer;
begin
  if (arg^.flags and ACTP_NOTIFY)<>0 then
  begin
    NotifyEventHooks(hevaction,arg^.macro.id,0); // started
    arg^.macro.flags:=arg^.macro.flags or ACF_USEDNOW;
  end;

  DoAction(arg^.macro,arg^.WorkData);

  if (arg^.flags and ACTP_NOTIFY)<>0 then
  begin
    arg^.macro.flags:=arg^.macro.flags and not ACF_USEDNOW;
    NotifyEventHooks(hevaction,arg^.macro.id,1); // finished
  end;

  if arg^.event<>0 then // service, waiting
    SetEvent(arg^.event)
  else if (arg^.flags and ACTP_SAMETHREAD)=0 then // no waiting
  begin
    ClearResult(arg^.WorkData); // free last result memory if needs
    // Free Storage
    for i:=0 to 9 do
      ClearResult(arg^.WorkData,i);
    mFreeMem(arg);              // free ActStartData (no time to free after)
  end;
end;

function ActionStarter(macro:pMacroRecord;wd:pWorkData;flags:dword):LPARAM;
var
  tmp:pActStartData;
  i:integer;
begin
  mGetMem (tmp ,SizeOf(tActStartData));
  FillChar(tmp^,SizeOf(tActStartData),0);

  if wd<>nil then
  begin
    tmp^.WorkData.ResultType:=wd^.ResultType;
    case wd^.ResultType of
      rtInt : tmp^.WorkData.LastResult:=wd^.LastResult;
      rtWide: StrDupW(pWideChar(tmp^.WorkData.LastResult),pWideChar(wd^.LastResult));
      rtAnsi: begin
        AnsiToWide(pAnsiChar(wd^.LastResult),pWideChar(tmp^.WorkData.LastResult));
        tmp^.WorkData.ResultType:=rtWide;
      end;
      rtUTF8: begin
        UTF8ToWide(pAnsiChar(wd^.LastResult),pWideChar(tmp^.WorkData.LastResult));
        tmp^.WorkData.ResultType:=rtWide;
      end;
    end;
    tmp^.WorkData.Parameter:=wd^.Parameter;
  end
  else
    tmp^.WorkData.ResultType:=rtInt;

  tmp^.macro :=macro;
  tmp^.flags :=flags;

  if (flags and ACTP_SAMETHREAD)<>0 then // with waiting, macro or service
  begin
    tmp^.event:=0;
    ThDoAction(tmp);
    // keep text result (for macro from macro)
    if (flags and ACTP_KEEPRESULT)<>0 then
    begin
      wd^.ResultType:=tmp^.WorkData.ResultType;
      if wd^.ResultType=rtInt then
        wd^.LastResult:=tmp^.WorkData.LastResult
      else
        StrDupW(pWideChar(wd^.LastResult),pWideChar(tmp^.WorkData.LastResult));
      result:=0;
    end
    else if tmp^.WorkData.ResultType=rtInt then
      result:=tmp^.WorkData.LastResult
    // result no needs or macro from service
    else
      result:=StrToInt(pWideChar(tmp^.WorkData.LastResult));
  end

  else if (flags and ACTP_WAIT)<>0 then // with waiting, service
  begin
    tmp^.event:=CreateEvent(nil,FALSE,FALSE,nil);
    {CloseHandle}(mir_forkthread(@ThDoAction,tmp));
    //!!!!!!!!! movetmp structure (event handle) to array, not stack?
    WaitForSingleObjectEx(tmp^.event,INFINITE,true);
    CloseHandle(tmp^.event);
    if tmp^.WorkData.ResultType=rtWide then
      result:=StrToInt(pWideChar(tmp^.WorkData.LastResult))
    else
      result:=tmp^.WorkData.LastResult;
  end

  else // no waiting, service or macro
  begin
    tmp^.event:=0;
    {CloseHandle}(mir_forkthread(@ThDoAction,tmp));
    result:=0;
    exit;
  end;

  ClearResult(tmp^.WorkData); // free last result memory if needs
  // Free Storage
  for i:=0 to 9 do
    ClearResult(tmp^.WorkData,i);
  mFreeMem(tmp);              // free ActStartData (no time to free after)
end;

//----- execute services -----

function ActRun(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
var
  p:pMacroRecord;
  w:tWorkData;
begin
  result:=-1;
  p:=MacroList.GetMacro(wParam);
  if p<>nil then
  begin
    FillChar(w,SizeOf(w),0);
    w.ResultType:=rtInt;
    w.Parameter:=lParam;
    result:=ActionStarter(p,@w,ACTP_SAMETHREAD);
  end;
end;

function ActRunGroup(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
var
  p:pMacroRecord;
  w:tWorkData;
begin
  result:=-1;
  p:=MacroList.GetMacro(pWideChar(wParam));
  if p<>nil then
  begin
    FillChar(w,SizeOf(w),0);
    w.ResultType:=rtInt;
    w.Parameter:=lParam;
    result:=ActionStarter(p,@w,ACTP_SAMETHREAD);
  end;
end;

function ActRunParam(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
var
  p:pMacroRecord;
  w:tWorkData;
begin
  result:=-1;
  if (pAct_Param(lParam)^.flags and ACTP_BYNAME)=0 then
    p:=MacroList.GetMacro(pAct_Param(lParam)^.Id)
  else
    p:=MacroList.GetMacro(pWideChar(pAct_Param(lParam)^.Id));

  if p<>nil then
  begin
    FillChar(w,SizeOf(w),0);
    w.Parameter :=pAct_Param(lParam)^.wParam;
    w.LastResult:=pAct_Param(lParam)^.lParam;
    w.ResultType:=pAct_Param(lParam)^.lPType;
    result:=ActionStarter(p,@w,pAct_Param(lParam)^.flags);
    if (pAct_Param(lParam)^.flags and (ACTP_KEEPRESULT or ACTP_SAMETHREAD))=
        (ACTP_KEEPRESULT or ACTP_SAMETHREAD) then
    begin
      pAct_Param(lParam)^.lParam:=w.LastResult;
      pAct_Param(lParam)^.lPType:=w.ResultType;
    end;
  end;
end;
