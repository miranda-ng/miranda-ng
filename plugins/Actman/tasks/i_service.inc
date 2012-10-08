{}
// wParam: 1/0 (enable/disable), lParam = task name
// works for all tasks with same started name
function TaskEnable(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
var
  i,j:integer;
begin
  result:=0;
  if lParam=0 then exit;
  j:=StrLenW(pWideChar(lParam));

  for i:=0 to MaxTasks-1 do
  begin
    if (TaskList[i].flags and ACF_ASSIGNED)<>0 then
    begin
      if StrCmpW(TaskList[i].name,pWideChar(lParam),j)=0 then
      begin
        if wParam=0 then // disable
        begin
          if (TaskList[i].flags and ACF_DISABLED)=0 then
          begin
            inc(result);
            TaskList[i].flags:=TaskList[i].flags or ACF_DISABLED;
            if TaskList[i].timer<>0 then
            begin
              KillTimer(0,TaskList[i].timer);
              TaskList[i].timer:=0;
            end;
          end;
        end
        else
        begin
          if (TaskList[i].flags and ACF_DISABLED)<>0 then
          begin
            inc(result);
            TaskList[i].flags:=TaskList[i].flags and not ACF_DISABLED;
            SetTask(TaskList[i]);
          end;
        end;
      end;
    end;
  end;
end;

function TaskDelete(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
var
  i,j:integer;
begin
  result:=0;
  if lParam=0 then exit;
  j:=StrLenW(pWideChar(lParam));

  for i:=0 to MaxTasks-1 do
  begin
    if (TaskList[i].flags and ACF_ASSIGNED)<>0 then
    begin
      if StrCmpW(TaskList[i].name,pWideChar(lParam),j)=0 then
      begin
        TaskList[i].flags:=TaskList[i].flags and not ACF_ASSIGNED;
      end;
    end;
  end;
end;

function TaskCount(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
var
  i,j:integer;
begin
  result:=0;
  if lParam=0 then exit;
  j:=StrLenW(pWideChar(lParam));

  for i:=0 to MaxTasks-1 do
  begin
    with TaskList[i] do
    begin
      if (flags and ACF_ASSIGNED)<>0 then
      begin
        if StrCmpW(name,pWideChar(lParam),j)=0 then
        begin
          result:=count;
          count:=wParam;
        end;
      end;
    end;
  end;
end;
