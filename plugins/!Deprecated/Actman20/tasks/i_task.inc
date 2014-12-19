{}
const
  ACF_ASSIGNED = $80000000;  // Task assigned
  ACF_DISABLED = $10000000;  // Task disabled

  TCF_ABSOLUTE     = $00000001;
  TCF_IMMEDIATELY  = $00000002;
  TCF_NONZEROBREAK = $00000004;
  TCF_MAKEEVENT    = $00000008;
  TCF_EVENTONCE    = $00000010;

const
  WM_RESETTASKS = WM_USER+1312;
  WM_FIRSTTASK  = WM_USER+1313;
  WM_LASTTASK   = WM_FIRSTTASK+1000;

type
  pTaskRec = ^tTaskRec;
  tTaskRec = record
    // option values
    flags     :dword;
    name      :PWideChar; // name for task
    action    :dword;     // assigned action
    intdays,              // interval,days
    dayoffset :integer;   //!! offset, days
    starttime,            // task starttime
    interval  :TFileTime; // interval for repeat
    count     :integer;   // repeat count
    // support values
    lastcall  :TFileTime; // last timer event time
    nextcall  :TFileTime; // ?? next start time? 
    // runtime values
    timer     :uint_ptr;  // timer handle
    curcount  :integer;   // repeat count
    inprocess :bool;      // starting processing
    inaction  :bool;      // timer event processing
  end;
  pTaskList = ^tTaskList;
  tTaskList  = array [0..1023] of tTaskRec;

var
  TaskList:pTaskList = nil;
  MaxTasks:integer = 0;

procedure TimerProc(wnd:HWND;uMsg:uint;idEvent:uint_ptr;dwTime:dword); stdcall;
var
  ltime:uint;
  i:integer;
  res:int_ptr;
  st:tSystemTime;
begin
  for i:=0 to MaxTasks-1 do
  begin
    with TaskList[i] do
    begin
      if (flags and (ACF_ASSIGNED or ACF_DISABLED))=ACF_ASSIGNED then
        if timer=idEvent then
        begin
          inaction:=true;
          if ((flags and TCF_MAKEEVENT)<>0) and
            (((flags and TCF_EVENTONCE) =0) or (curcount=count)) then
            NotifyEventHooks(hevent,count-curcount,lParam(name));

          GetLocalTime(st);
          SystemTimeToFileTime(st,lastcall);

          res:=CallService(MS_ACT_RUNBYID,action,0);

          if ((res<>0) and ((flags and TCF_NONZEROBREAK)<>0)) or // non-zero result
             (count=0) or (curcount=0) then // no need to repeat or all repeats done
          begin
            KillTimer(0,idEvent);
            timer:=0;
            flags:=flags or ACF_DISABLED;
          end
          else
          begin
            if (count<>0) and (count=curcount) then // next timer - repeat interval
            begin
              KillTimer(0,idEvent);
              FileTimeToSystemTime(interval,st);
              ltime:={st.wMilliseconds+}st.wSecond*1000+st.wMinute*1000*60+st.wHour*60*60*1000;
              timer:=SetTimer(0,0,ltime,@TimerProc);
              if count=-1 then
                curcount:=1;
            end;
            if count>0 then
              dec(curcount);
          end;
          inaction:=false;
          break;
        end;
    end;
  end;
end;

procedure SetTask(var task:tTaskRec);
var
  ltime:uint;
  uli1,uli2:ULARGE_INTEGER;
  sft:tFileTime;
  st:tSystemTime;
  dif:int64;
begin
  task.inprocess:=true;
  // Check task time
  if (task.flags and TCF_IMMEDIATELY)<>0 then
  begin
    FileTimeToSystemTime(task.interval,st);
    ltime:={st.wMilliseconds+}st.wSecond*1000+st.wMinute*1000*60+
           st.wHour*60*60*1000;
  end
  else if (task.flags and TCF_ABSOLUTE)<>0 then
  begin
    uli1.LowPart :=task.starttime.dwLowDateTime;
    uli1.HighPart:=task.starttime.dwHighDateTime;
    GetLocalTime(st);
    SystemTimeToFileTime(st,sft);
    uli2.LowPart :=sft.dwLowDateTime;
    uli2.HighPart:=sft.dwHighDateTime;
    dif:=uli1.QuadPart-uli2.QuadPart;
    if dif>0 then // time in future
      ltime:=dif div 10000 // 100ns to 1 ms
    else // was in past
    begin
      task.flags:=task.flags or ACF_DISABLED;
      exit;
    end;
  end
  else
  begin
    // days+hours+minutes+seconds+millseconds
    FileTimeToSystemTime(task.starttime,st);
    ltime:={st.wMilliseconds+}st.wSecond*1000+st.wMinute*1000*60+
           st.wHour*60*60*1000+task.dayoffset*24*60*60*1000;
  end;
  // set timer
  task.curcount:=task.count;
  task.timer   :=SetTimer(0,0,ltime,@TimerProc);

  if (task.flags and TCF_IMMEDIATELY)<>0 then
    TimerProc(0,WM_TIMER,task.timer,0);
  task.inprocess:=false;
end;

procedure SetAllTasks;
var
  i:integer;
begin
  for i:=0 to MaxTasks-1 do
  begin
    if (TaskList[i].flags and ACF_ASSIGNED)<>0 then
    begin
      if (TaskList[i].flags and ACF_DISABLED)=0 then
        SetTask(TaskList[i])
      else if TaskList[i].timer<>0 then
      begin
        KillTimer(0,TaskList[i].timer);
        TaskList[i].timer:=0;
      end;
    end;
  end;
end;

procedure StopAllTasks;
var
  i:integer;
begin
  for i:=0 to MaxTasks-1 do
  begin
    if (TaskList[i].flags and (ACF_ASSIGNED or ACF_DISABLED))=ACF_ASSIGNED then
      if TaskList[i].timer<>0 then
      begin
        KillTimer(0,TaskList[i].timer);
        TaskList[i].timer:=0;
      end;
  end;
end;

procedure ClearTasks;
var
  i:integer;
begin
  for i:=0 to MaxTasks-1 do
  begin
    with TaskList[i] do
    begin
//!!      if (flags and ACF_ASSIGNED)<>0 then
        mFreeMem(name);
    end;
  end;
  FreeMem(TaskList);
  MaxTasks:=0;
end;

function CreateNewTask:integer;
var
  i:integer;
  tmp:pTaskList;
  st:tSystemTime;
begin
  result:=-1;
  // if list is not empty, search for hole
  if MaxTasks>0 then
  begin
    for i:=0 to MaxTasks-1 do
    begin
      if (TaskList[i].flags and ACF_ASSIGNED)=0 then
      begin
        FillChar(TaskList[i],SizeOf(tTaskRec),0);
        result:=i;
        break;
      end;
    end;
  end;
  if result<0 then
  begin
    // not found or empty list
    i:=(MaxTasks+16)*SizeOf(tTaskRec);
    GetMem  (tmp ,i);
    FillChar(tmp^,i,0);
    if MaxTasks>0 then
    begin
      move(TaskList^,tmp^,MaxTasks*SizeOf(tTaskRec));
      FreeMem(TaskList);
    end;
    TaskList:=tmp;
    result:=MaxTasks;
    inc(MaxTasks,16);
  end;
  with TaskList^[result] do
  begin
    flags:=flags or ACF_ASSIGNED or ACF_DISABLED or TCF_ABSOLUTE;
    GetLocalTime(st);
    SystemTimeToFileTime(st,starttime);
    //!!! CHEAT
    st.wHour  :=0;
    st.wMinute:=0;
    st.wSecond:=1;
    SystemTimeToFileTime(st,interval);
  end;
end;
