unit scheduler;

interface

implementation

uses
  windows, commctrl, messages,
  mirutils, common, dbsettings, m_api, wrapper,
  global;

{$R tasks.res}

var
  hevent: THANDLE;

{$include i_task.inc}
{$include i_tconst.inc}
{$include i_options.inc}
{$include i_opt_dlg.inc}
{$include i_service.inc}

// ------------ base interface functions -------------

procedure Init;
begin

  if LoadTasks=0 then
  begin
    MaxTasks:=8;
    GetMem  (TaskList ,MaxTasks*SizeOf(tTaskRec));
    FillChar(TaskList^,MaxTasks*SizeOf(tTaskRec),0);
  end
  else
    SetAllTasks;

  CreateServiceFunction(MS_ACT_TASKCOUNT ,@TaskCount);
  CreateServiceFunction(MS_ACT_TASKENABLE,@TaskEnable);
  CreateServiceFunction(MS_ACT_TASKDELETE,@TaskDelete);

  hevent:=CreateHookableEvent(ME_ACT_BELL);

end;

procedure DeInit;
begin
  DestroyHookableEvent(hevent);
  StopAllTasks;
  ClearTasks;
end;

function AddOptionPage(var tmpl:pAnsiChar;var proc:pointer;var name:PAnsiChar):integer;
begin
  result:=0;
  tmpl:=PAnsiChar(IDD_TASKS);
  proc:=@DlgProcOpt;
  name:='Scheduler';
end;

var
  amLink:tActionLink;

procedure InitLink;
begin
  amLink.Next     :=ActionLink;
  amLink.Init     :=@Init;
  amLink.DeInit   :=@DeInit;
  amLink.AddOption:=@AddOptionPage;
  ActionLink      :=@amLink;
end;

initialization
  InitLink;
end.
