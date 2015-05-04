{$include i_opt_0.inc}
{$include i_opt_1.inc}

function OnOptInitialise(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  odp:TOPTIONSDIALOGPAGE;
  ptr:pwModule;
  tmpl:pAnsiChar;
  name:pansiChar;
  proc:pointer;
  i:integer;
begin
  if hwndTooltip<>0 then
    DestroyWindow(hwndTooltip);
  hwndTooltip:=CreateWindowW(TOOLTIPS_CLASS,nil,TTS_ALWAYSTIP,
      integer(CW_USEDEFAULT),integer(CW_USEDEFAULT),
      integer(CW_USEDEFAULT),integer(CW_USEDEFAULT),
      0,0,hInstance,nil);

  SendMessage(hwndTooltip,TTM_SETMAXTIPWIDTH,0,300);

  FillChar(odp,SizeOf(odp),0);
  odp.Position   :=900003000;
  odp.hInstance  :=hInstance;
  odp.szTitle.a  :=PluginName;
  odp.szGroup.a  :='Plugins';

  odp.flags      :=ODPF_BOLDGROUPS;
  odp.pszTemplate:=PAnsiChar(IDD_OPT_PARTS);
  odp.pfnDlgProc :=@DlgProcOptions0;
  odp.szTab.a    :='Modules';
  CallService(MS_OPT_ADDPAGE,wParam,tlparam(@odp));

  odp.flags      :=ODPF_BOLDGROUPS;
  odp.pszTemplate:=PAnsiChar(IDD_OPT_BASIC);
  odp.pfnDlgProc :=@DlgProcOptions1;
  odp.szTab.a    :='Basic';
  CallService(MS_OPT_ADDPAGE,wParam,tlparam(@odp));

  ptr:=ModuleLink;
  while ptr<>nil do
  begin
    if (ptr^.ModuleStat<>0) and (@ptr^.AddOption<>nil) then
    begin
      i:=ptr^.AddOption(tmpl,proc,name);
      odp.pszTemplate:=tmpl;
      odp.pfnDlgProc :=proc;
      odp.szTab.a    :=name;
      CallService(MS_OPT_ADDPAGE,wParam,tlparam(@odp));
      if i>0 then continue;
    end;
    ptr:=ptr^.Next;
  end;

  result:=0;
end;
