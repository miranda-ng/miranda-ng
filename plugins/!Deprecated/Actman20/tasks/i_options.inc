{}
const
  opt_task    :PAnsiChar = 'Task';
  opt_tasks   :PAnsiChar = 'Tasks';
  opt_count   :PAnsiChar = 'numtasks';

  opt_name    :PAnsiChar = 'name';
  opt_flags   :PAnsiChar = 'flags';
  opt_action  :PAnsiChar = 'action';
  opt_repeat  :PAnsiChar = 'repeat';
  opt_days    :PAnsiChar = 'dayoffset';
  opt_intdays :PAnsiChar = 'intdays';

  opt_time_lo    :PAnsiChar = 'starttime_lo';
  opt_time_hi    :PAnsiChar = 'starttime_hi';
  opt_interval_lo:PAnsiChar = 'interval_lo';
  opt_interval_hi:PAnsiChar = 'interval_hi';
  opt_lastcall_lo:PAnsiChar = 'lastcall_lo';
  opt_lastcall_hi:PAnsiChar = 'lastcall_hi';

procedure SaveTasks;
var
  section:array [0..63] of AnsiChar;
  p,p1:PAnsiChar;
  i,amount:integer;
begin
  DBDeleteGroup(0,DBBranch,opt_tasks);
  amount:=0;
  p1:=StrCopyE(section,opt_tasks);
  p1^:='/'; inc(p1);
  p1:=StrCopyE(p1,opt_task);
  for i:=0 to MaxTasks-1 do
  begin
    if (TaskList[i].flags and ACF_ASSIGNED)=0 then
      continue;

    p:=StrEnd(IntToStr(p1,amount));
    p^:='/'; inc(p);
    with TaskList[i] do
    begin
      StrCopy(p,opt_flags ); DBWriteDWord  (0,DBBranch,section,flags);
      StrCopy(p,opt_name  ); DBWriteUnicode(0,DBBranch,section,name);
      StrCopy(p,opt_action); DBWriteDWord  (0,DBBranch,section,action);
      StrCopy(p,opt_repeat); DBWriteWord   (0,DBBranch,section,count);
      StrCopy(p,opt_days  ); DBWriteByte   (0,DBBranch,section,dayoffset);
      //systemtime to filetime if needs
      StrCopy(p,opt_time_lo    ); DBWriteDWord(0,DBBranch,section,starttime.dwLowDateTime);
      StrCopy(p,opt_time_hi    ); DBWriteDWord(0,DBBranch,section,starttime.dwHighDateTime);
      StrCopy(p,opt_interval_lo); DBWriteDWord(0,DBBranch,section,interval .dwLowDateTime);
      StrCopy(p,opt_interval_hi); DBWriteDWord(0,DBBranch,section,interval .dwHighDateTime);
      StrCopy(p,opt_intdays    ); DBWriteByte (0,DBBranch,section,intdays);
      StrCopy(p,opt_lastcall_lo); DBWriteDWord(0,DBBranch,section,lastcall .dwLowDateTime);
      StrCopy(p,opt_lastcall_hi); DBWriteDWord(0,DBBranch,section,lastcall .dwHighDateTime);
    end;
    inc(amount);
  end;
  DBWriteByte(0,DBBranch,opt_count,amount);
end;

function LoadTasks:integer;
var
  section:array [0..63] of AnsiChar;
  p,p1:PAnsiChar;
  i:integer;
begin
  MaxTasks:=DBReadByte(0,DBBranch,opt_count);
  result:=MaxTasks;
  if MaxTasks>0 then
  begin
    GetMem  (TaskList ,MaxTasks*SizeOf(tTaskRec));
    FillChar(TaskList^,MaxTasks*SizeOf(tTaskRec),0);
    p1:=StrCopyE(section,opt_tasks);
    p1^:='/'; inc(p1);
    p1:=StrCopyE(p1,opt_task);
    for i:=0 to MaxTasks-1 do
    begin
      p:=StrEnd(IntToStr(p1,i));
      p^:='/'; inc(p);

      with TaskList[i] do
      begin
        StrCopy(p,opt_flags ); flags    :=DBReadDWord  (0,DBBranch,section);
        StrCopy(p,opt_name  ); name     :=DBReadUnicode(0,DBBranch,section);
        StrCopy(p,opt_action); action   :=DBReadDWord  (0,DBBranch,section);
        StrCopy(p,opt_days  ); dayoffset:=DBReadByte   (0,DBBranch,section);
        StrCopy(p,opt_repeat); count    :=Shortint(DBReadWord(0,DBBranch,section));

        StrCopy(p,opt_time_lo    ); starttime.dwLowDateTime :=DBReadDWord(0,DBBranch,section);
        StrCopy(p,opt_time_hi    ); starttime.dwHighDateTime:=DBReadDWord(0,DBBranch,section);
        StrCopy(p,opt_interval_lo); interval .dwLowDateTime :=DBReadDWord(0,DBBranch,section);
        StrCopy(p,opt_interval_hi); interval .dwHighDateTime:=DBReadDWord(0,DBBranch,section);
        StrCopy(p,opt_intdays    ); intdays:=DBReadByte(0,DBBranch,section);
        StrCopy(p,opt_lastcall_lo); lastcall .dwLowDateTime :=DBReadDWord(0,DBBranch,section);
        StrCopy(p,opt_lastcall_hi); lastcall .dwHighDateTime:=DBReadDWord(0,DBBranch,section);
        // filetime to systemtime if needs
      end;
    end;
  end;
end;
