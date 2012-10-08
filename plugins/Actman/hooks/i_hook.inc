{}

const
  HWND_MESSAGE = HWND(-3);
const
  ACF_ASSIGNED = $80000000;  // hook assigned
  ACF_DISABLED = $10000000;  // hook disabled
const
  WM_RESETHOOKS = WM_USER+1312;
  WM_FIRSTHOOK  = WM_USER+1313;
  WM_LASTHOOK   = WM_FIRSTHOOK+1000;

type
  pHookRec = ^tHookRec;
  tHookRec = record
    flags  :dword;
    name   :PAnsiChar; // name for hook
    handle :THANDLE;   // handle of hook
    descr  :PWideChar; // name for list
    action :dword;     // assigned action
    message:uint;      // window message for hook
  end;
  pHookList = ^tHookList;
  tHookList  = array [0..1023] of tHookRec;

var
  HookList:pHookList = nil;
  MaxHooks:integer = 0;
  MessageWindow:HWND = 0;

function GetNextMessage:uint;
var
  i:uint;
  j:integer;
begin
  result:=0;
  for i:=WM_FIRSTHOOK to WM_LASTHOOK do
  begin
    for j:=0 to MaxHooks-1 do
    begin
      with HookList^[j] do
      begin
        if ((flags and ACF_ASSIGNED)<>0) and (i=message) then
        begin
          inc(result);
          break;
        end;
      end;
    end;
    if result=0 then
    begin
      result:=i;
      break;
    end
    else
      result:=0;
  end;
end;

procedure SetAllHooks;
var
  i:integer;
  msg:cardinal;
begin
  msg:=WM_FIRSTHOOK;
  for i:=0 to MaxHooks-1 do
  begin
    with HookList[i] do
    begin
      message:=msg;
      if (flags and ACF_ASSIGNED)<>0 then
      begin
        if (flags and ACF_DISABLED)<>0 then
        begin
          if handle<>0 then
          begin
            UnhookEvent(handle);
            handle:=0;
          end;
        end
        else
        begin
          if handle<>0 then
            UnhookEvent(handle);
          handle:=HookEventMessage(name,MessageWindow,message);
        end;
      end;
    end;
    inc(msg);
  end;
end;

function GetHookByMessage(msg:uint):pHookRec;
var
  i:integer;
begin
  result:=nil;
  for i:=0 to MaxHooks-1 do
  begin
    with HookList[i] do
    begin
      if ((flags and ACF_ASSIGNED)<>0) and (msg=message) then
      begin
        result:=@HookList[i];
        break;
      end;
    end;
  end;
end;

function HookWndProc(Dialog:HWnd;hMessage:UINT;wParam:WPARAM;lParam:LPARAM):lresult; stdcall;
var
  p:pHookRec;
  ap:tAct_Param;
begin
  result:=0;
  case hMessage of
    WM_FIRSTHOOK..WM_LASTHOOK: begin
      p:=GetHookByMessage(hMessage);
      if p<>nil then
      begin
        ap.flags :=ACTP_WAIT;
        ap.Id    :=p^.action;
        ap.wParam:=wParam;
        ap.lParam:=lParam;
        result:=CallService(MS_ACT_RUNPARAMS,0,TLPARAM(@ap));
      end;
    end;
  else
    result:=DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
end;

procedure ClearHooks;
var
  i:integer;
begin
  for i:=0 to MaxHooks-1 do
  begin
    with HookList[i] do
    begin
      if (flags and ACF_ASSIGNED)<>0 then
      begin
        mFreeMem(descr);
        mFreeMem(name);
        if handle<>0 then
          UnhookEvent(handle);
      end;
    end;
  end;
  FreeMem(HookList);
  MaxHooks:=0;
end;

