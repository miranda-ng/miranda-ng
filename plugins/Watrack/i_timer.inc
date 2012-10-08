{Timer related procedures}

procedure TimerProc(wnd:HWND;uMsg:uint;idEvent:uint_ptr;dwTime:dword); stdcall;
begin
  case DisablePlugin of
    dsEnabled  : CallService(MS_WAT_GETMUSICINFO,WAT_INF_CHANGES,0);
    dsTemporary: DisablePlugin:=dsWait;
  end;
end;

procedure StartTimer;
begin
  if mTimer>0 then
    hTimer:=SetTimer(0,0,mTimer,@TimerProc)
  else
    hTimer:=0;
end;

procedure StopTimer;
begin
  if hTimer<>0 then
  begin
    KillTimer(0,hTimer);
    hTimer:=0;
  end;
end;
