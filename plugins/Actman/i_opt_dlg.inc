{}
const
  etMacro = 1; // Groups  changed
  etACT   = 2; // Actions changed

{$include i_opt_dlg2.inc}

function OnOptInitialise(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  odp:TOPTIONSDIALOGPAGE;
  ptr:pActionLink;
  tmpl:pAnsiChar;
  name:pansiChar;
  proc:pointer;
  i:integer;
begin
  result:=0;
  NoDescription:=TranslateW('No description');

  DoInitCommonControls(ICC_USEREX_CLASSES);

  FillChar(odp,SizeOf(odp),0);
  odp.flags      :=ODPF_BOLDGROUPS;
  odp.Position   :=900003000;
  odp.szGroup.a  :='Services';
  odp.szTitle.a  :='Actions';
  odp.szTab.a    :='Actions';
  odp.pfnDlgProc :=@DlgProcOpt2;
  odp.pszTemplate:=PAnsiChar(IDD_ACTION);
  Options_AddPage(wParam,@odp);

  ptr:=ActionLink;
  while ptr<>nil do
  begin
    if @ptr^.AddOption<>nil then
    begin
      i:=ptr^.AddOption(tmpl,proc,name);
      odp.pszTemplate:=tmpl;
      odp.pfnDlgProc :=proc;
      odp.szTab.a    :=name;
      Options_AddPage(wParam,@odp);
      if i>0 then continue;
    end;
    ptr:=ptr^.Next;
  end;
end;
