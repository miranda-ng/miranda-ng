{$include compilers.inc}
{$IFDEF COMPILER_16_UP}
  {$WEAKLINKRTTI ON}
  {.$RTTI EXPLICIT METHODS([]) PROPERTIES([]) FIELDS([])}
{$ENDIF}
{$IMAGEBASE $13100000}
library quicksearch;

{$R qs.res}
{$R version.res}

uses
//  FastMM4,
  Windows,
  Messages,
  m_api,
  sr_optdialog,
  sr_global,
  sr_window,
  sr_frame,
  mirutils,
  common;

function OnTTBLoaded(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  addtotoolbar;
  result:=0;
end;

function IconChanged(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  ttb:TTBButton;
begin
  result:=0;

// toptoolbar
  if ServiceExists(MS_TTB_GETBUTTONOPTIONS) then
  begin
    CallService(MS_TTB_GETBUTTONOPTIONS,(hTTBButton shl 16)+TTBO_ALLDATA,TLPARAM(@ttb));
    ttb.hIconUp:=IcoLib_GetIcon(QS_QS,0);
    ttb.hIconDn:=ttb.hIconUp;
    CallService(MS_TTB_SETBUTTONOPTIONS,(hTTBButton shl 16)+TTBO_ALLDATA,TLPARAM(@ttb));
  end;

end;

const
  IconAmount = 9;
const
  Icons:array [0..IconAmount-1] of tIconItem = (
    (szDescr: 'Quick Search' ; szName: 'QS'     ; defIconID: IDI_QS     ; size: 0; hIcolib: 0;),
    (szDescr: 'New Column'   ; szName: 'New'    ; defIconID: IDI_NEW    ; size: 0; hIcolib: 0;),
    (szDescr: 'Column Up'    ; szName: 'Up'     ; defIconID: IDI_UP     ; size: 0; hIcolib: 0;),
    (szDescr: 'Column Down'  ; szName: 'Down'   ; defIconID: IDI_DOWN   ; size: 0; hIcolib: 0;),
    (szDescr: 'Delete Column'; szName: 'Delete' ; defIconID: IDI_DELETE ; size: 0; hIcolib: 0;),
    (szDescr: 'Default'      ; szName: 'Default'; defIconID: IDI_DEFAULT; size: 0; hIcolib: 0;),
    (szDescr: 'Reload'       ; szName: 'Reload' ; defIconID: IDI_RELOAD ; size: 0; hIcolib: 0;),
    (szDescr: 'Male'         ; szName: 'Male'   ; defIconID: IDI_MALE   ; size: 0; hIcolib: 0;),
    (szDescr: 'Female'       ; szName: 'Female' ; defIconID: IDI_FEMALE ; size: 0; hIcolib: 0;)
  );

procedure RegisterIcons;
begin
  Icon_Register(hInstance, qs_module, @Icons, IconAmount, 'QS');
  HookEvent(ME_SKIN_ICONSCHANGED,@IconChanged);
end;

function OnOptInitialise(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  odp:TOPTIONSDIALOGPAGE;
begin
  ZeroMemory(@odp,sizeof(odp));
  odp.Position   :=900003000;
  odp.pszTemplate:=PAnsiChar(IDD_DIALOG1);
  odp.szTitle.a  :=qs_name;
  odp.szGroup.a  :='Contacts';
  odp.pfnDlgProc :=@sr_optdialog.DlgProcOptions;
  odp.flags      :=ODPF_BOLDGROUPS;
  Options_AddPage(wParam,@odp);
  Result:=0;
end;

function OpenSearchWindow(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
begin
  result:=0;
  OpenSrWindow(pointer(wParam),lParam)
end;

function OnModulesLoaded(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  RegisterIcons;
  RegisterColors;

  CreateServiceFunction(QS_SHOWSERVICE,@OpenSearchWindow);
  AddRemoveMenuItemToMainMenu;

  reghotkeys;

  HookEvent(ME_TTB_MODULELOADED,@OnTTBLoaded);

  CreateFrame(0);

  Result:=0;
end;

function Load():integer;cdecl;
begin
  Result:=0;
  Langpack_register;
  HookEvent(ME_OPT_INITIALISE      ,@OnOptInitialise);
  HookEvent(ME_SYSTEM_MODULESLOADED,@OnModulesLoaded);
  qsopt.numcolumns:=loadopt_db(qsopt.columns);
end;

function Unload:integer;cdecl;
begin
  result:=0;
  removetoolbar;
  DestroyFrame;

  CloseSrWindow;

  clear_columns(qsopt.columns);
end;

exports
  Load, Unload;

begin
  DisableThreadLibraryCalls(hInstance);

  PluginInfo.cbSize     :=SizeOf(TPLUGININFOEX);
  PluginInfo.shortName  :='Quick Search';
  PluginInfo.version    :=$01040200;
  PluginInfo.description:=
    'This plugin allows you to quick search for nickname, '+
    'firstname, lastname, email, uin in your contact list. '+
    'And now you may add any setting to display - for example '+
    'user''s version of Miranda, group or city.';
  PluginInfo.author     :='Awkward, based on Bethoven sources';
  PluginInfo.copyright  :='(c) 2004-05 Bethoven; 2006-13 Awkward';
  PluginInfo.homepage   :='https://miranda-ng.org/p/QuickSearch/';
  PluginInfo.flags      :=UNICODE_AWARE;
  PluginInfo.uuid       :=MIID_QUICKSEARCH;
end.
