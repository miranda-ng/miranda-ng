(*
    History++ plugin for Miranda IM: the free IM client for Microsoft* Windows*

    Copyright (C) 2006-2009 theMIROn, 2003-2006 Art Fedorov.
    History+ parts (C) 2001 Christian Kastner

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*)

  {$WEAKLINKRTTI ON}
  {.$RTTI EXPLICIT METHODS([]) PROPERTIES([]) FIELDS([])}
library historypp;

{$R 'hpp_resource.res' 'hpp_resource.rc'}
{$R 'version.res'}
{$R 'hpp_opt_dialog.res' 'hpp_opt_dialog.rc'}

uses
  Windows,
  SysUtils,
  CommCtrl,
  m_api,
  Forms,
  hpp_global in 'hpp_global.pas',
  hpp_contacts in 'hpp_contacts.pas',
  hpp_database in 'hpp_database.pas',
  hpp_events in 'hpp_events.pas',
  hpp_services in 'hpp_services.pas',
  hpp_itemprocess in 'hpp_itemprocess.pas',
  hpp_options in 'hpp_options.pas',
  hpp_messages in 'hpp_messages.pas',
  HistoryGrid in 'HistoryGrid.pas',
  VertSB in 'VertSB.pas',
  HistoryForm in 'HistoryForm.pas' {HistoryFrm},
  EventDetailForm in 'EventDetailForm.pas' {EventDetailsFrm},
  GlobalSearch in 'GlobalSearch.pas' {fmGlobalSearch},
  hpp_searchthread in 'hpp_searchthread.pas',
  hpp_bookmarks in 'hpp_bookmarks.pas',
  hpp_sessionsthread in 'hpp_sessionsthread.pas',
  hpp_arrays in 'hpp_arrays.pas',
  hpp_strparser in 'hpp_strparser.pas',
  hpp_forms in 'hpp_forms.pas',
  hpp_opt_dialog in 'hpp_opt_dialog.pas',
  hpp_eventfilters in 'hpp_eventfilters.pas',
  hpp_mescatcher in 'hpp_mescatcher.pas',
  CustomizeFiltersForm in 'CustomizeFiltersForm.pas' {fmCustomizeFilters},
  CustomizeToolbar in 'CustomizeToolbar.pas' {fmCustomizeToolbar},
  hpp_external in 'hpp_external.pas',
  hpp_externalgrid in 'hpp_externalgrid.pas',
  hpp_richedit in 'hpp_richedit.pas',
  hpp_olesmileys in 'hpp_olesmileys.pas',
  HistoryControls in 'HistoryControls.pas',
  Checksum in 'Checksum.pas',
  hpp_JclSysUtils in 'hpp_JclSysUtils.pas',
  hpp_puny in 'hpp_puny.pas';

var
  MenuCount: Integer = -1;
  PrevShowHistoryCount: Boolean = False;
  hmiContact: THandle;

const
  hLangpack:THANDLE = 0;
  histName:PWideChar = 'View &History';

var
  HookModulesLoad,
  HookOptInit,
  HookSettingsChanged,
  HookSmAddChanged,
  HookIconChanged,
  HookIcon2Changed,
  //hookContactChanged,
  HookContactDelete,
  HookFSChanged,
  HookTTBLoaded,
  HookBuildMenu,
  HookEventAdded,
  HookEventDeleted,
  HookMetaDefaultChanged,
  HookPreshutdown: THandle;

procedure RegisterHppLogger; stdcall; external AppDll;
procedure UnregisterHppLogger; stdcall; external AppDll;

function OnModulesLoad(awParam:WPARAM; alParam:LPARAM):int; cdecl; forward;
function OnSettingsChanged(wParam: WPARAM; lParam: LPARAM): Integer; cdecl; forward;
function OnSmAddSettingsChanged(wParam: WPARAM; lParam: LPARAM): Integer; cdecl; forward;
function OnIconChanged(wParam: WPARAM; lParam: LPARAM): Integer; cdecl; forward;
function OnIcon2Changed(awParam: WPARAM; alParam: LPARAM): Integer; cdecl; forward;
function OnOptInit(awParam: WPARAM; alParam: LPARAM): Integer; cdecl; forward;
function OnContactChanged(wParam: wParam; lParam: LPARAM): Integer; cdecl; forward;
function OnContactDelete(wParam: wParam; lParam: LPARAM): Integer; cdecl; forward;
function OnFSChanged(wParam: WPARAM; lParam: LPARAM): Integer; cdecl; forward;
function OnTTBLoaded(awParam: WPARAM; alParam: LPARAM): Integer; cdecl; forward;
function OnBuildContactMenu(hContact: WPARAM; alParam: LPARAM): Integer; cdecl; forward;
function OnEventAdded(wParam: WPARAM; lParam: LPARAM): Integer; cdecl; forward;
function OnEventDeleted(wParam: WPARAM; lParam: LPARAM): Integer; cdecl; forward;
function OnMetaDefaultChanged(wParam: WPARAM; lParam: LPARAM): Integer; cdecl; forward;
function OnPreshutdown(wParam: WPARAM; lParam: LPARAM): Integer; cdecl; forward;

var
  PluginInterfaces: array[0..1] of TGUID;

// tell Miranda about supported interfaces
function MirandaPluginInterfaces:PMUUID; cdecl;
begin
  PluginInterfaces[0]:=MIID_UIHISTORY;
  PluginInterfaces[1]:=MIID_LAST;
  Result := @PluginInterfaces;
end;

// load function called by miranda
function Load():Integer; cdecl;
begin

  Langpack_Register();
  RegisterHppLogger;

  // Getting langpack codepage for ansi translation
  hppCodepage := Langpack_GetDefaultCodePage;
  if hppCodepage = CP_ACP then
    hppCodepage := GetACP();

  // init history functions later
  HookModulesLoad := HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoad);
  HookOptInit := HookEvent(ME_OPT_INITIALISE, OnOptInit);
  hppRegisterServices;
{$IFNDEF NO_EXTERNALGRID}
  RegisterExtGridServices;
{$ENDIF}
  hppRegisterMainWindow;
 Result := 0;
end;

// unload
function Unload:Integer; cdecl;
begin
  UnregisterHppLogger;
  Result:=0;

  // unhook events
  UnhookEvent(hookOptInit);
  UnhookEvent(HookPreshutdown);
  UnhookEvent(HookModulesLoad);

  UnhookEvent(HookEventAdded);
  UnhookEvent(HookEventDeleted);
  UnhookEvent(HookSettingsChanged);
  UnhookEvent(HookIconChanged);
  UnhookEvent(HookContactDelete);
  UnhookEvent(HookBuildMenu);

  if SmileyAddEnabled then
    UnhookEvent(HookSmAddChanged);
  UnhookEvent(HookIcon2Changed);
  UnhookEvent(HookFSChanged);
  UnhookEvent(HookMetaDefaultChanged);

  try
    // destroy hidden main window
    hppUnregisterMainWindow;
    {$IFNDEF NO_EXTERNALGRID}
    UnregisterExtGridServices;
    {$ENDIF}
    // unregistering events
    hppUnregisterServices;
    // unregister bookmarks
    hppDeinitBookmarkServer;

  except
    on E: Exception do
      HppMessageBox(hppMainWindow,
        'Error while closing '+hppName+':'+#10#13+E.Message,
        hppName+' Error',MB_OK or MB_ICONERROR);
  end;

end;

// init plugin
function OnModulesLoad(awParam{0}:WPARAM; alParam{0}:LPARAM):integer; cdecl;
var
  menuItem:TMO_MenuItem;
begin
  // register
  hppRegisterGridOptions;
  // pretranslate strings
  hppPrepareTranslation;

  LoadIcons;
  LoadIcons2;

  // TopToolBar support
  HookTTBLoaded := HookEvent(ME_TTB_MODULELOADED,OnTTBLoaded);

  hppInitBookmarkServer;

  InitEventFilters;
  ReadEventFilters;

  // create contact item in contact menu
  ZeroMemory(@menuitem,SizeOf(menuItem));
  menuItem.flags := CMIF_UNICODE;

  SET_UID(@menuItem, '806B9223-0522-4B6B-AB2C-666D76C45D59');
  menuItem.Position := 1000090000;
  menuItem.szName.w := histName;
  menuItem.pszService := MS_HISTORY_SHOWCONTACTHISTORY;
  menuItem.hIcon := hppIcons[HPP_ICON_CONTACTHISTORY].handle;
  hmiContact := Menu_AddContactMenuItem(@menuItem);

  // create menu item in main menu for history search
  SET_UID(@menuItem, 'A5FE6AE6-7674-4DC0-9F22-9F56AE71E2D7');
  menuItem.Position := 500060001;
  menuItem.szName.w := 'His&tory Search';
  menuItem.pszService := MS_HPP_SHOWGLOBALSEARCH;
  menuItem.hIcon := hppIcons[HPP_ICON_GLOBALSEARCH].handle;
  Menu_AddMainMenuItem(@menuItem);

  LoadGridOptions;

  HookSettingsChanged := HookEvent(ME_DB_CONTACT_SETTINGCHANGED,OnSettingsChanged);
  HookIconChanged     := HookEvent(ME_SKIN_ICONSCHANGED,OnIconChanged);
  HookContactDelete   := HookEvent(ME_DB_CONTACT_DELETED,OnContactDelete);
  HookBuildMenu       := HookEvent(ME_CLIST_PREBUILDCONTACTMENU,OnBuildContactMenu);

  HookEventAdded   := HookEvent(ME_DB_EVENT_ADDED,OnEventAdded);
  HookEventDeleted := HookEvent(ME_DB_EVENT_DELETED,OnEventDeleted);
  HookPreshutdown  := HookEvent(ME_SYSTEM_PRESHUTDOWN,OnPreshutdown);

  if SmileyAddEnabled    then HookSmAddChanged := HookEvent(ME_SMILEYADD_OPTIONSCHANGED,OnSmAddSettingsChanged);
  HookIcon2Changed := HookEvent(ME_SKIN_ICONSCHANGED,OnIcon2Changed);
  HookFSChanged := HookEvent(ME_FONT_RELOAD,OnFSChanged);
  HookMetaDefaultChanged := HookEvent(ME_MC_DEFAULTTCHANGED,OnMetaDefaultChanged);

  // return successfully
  Result:=0;
end;

// Called when the toolbar services are available
// wParam = lParam = 0
function OnTTBLoaded(awParam: WPARAM; alParam: LPARAM): Integer; cdecl;
var
  ttb: TTBButton;
begin
  ZeroMemory(@ttb,SizeOf(ttb));
  ttb.hIconUp := hppIcons[HPP_ICON_GLOBALSEARCH].handle;
  ttb.pszService := MS_HPP_SHOWGLOBALSEARCH;
  ttb.dwFlags := TTBBF_VISIBLE or TTBBF_SHOWTOOLTIP;
  ttb.name := 'Global History Search';
  ttb.pszTooltipUp := ttb.name;
  CallService(MS_TTB_ADDBUTTON,WPARAM(@ttb), 0);
  Result := 0;
end;

// Called when setting in DB have changed
// wParam = hContact, lParam = PDbContactWriteSetting
function OnSettingsChanged(wParam: WPARAM; lParam: LPARAM): Integer; cdecl;
var
  cws: PDBContactWriteSetting;
  szProto: PAnsiChar;
begin
  Result := 0;
  // Log('OnSettChanged','Started. wParam: '+IntToStr(wParam)+', lParam: '+IntToStr(lParam));
  cws := PDBContactWriteSetting(lParam);

  if wParam = 0 then
  begin
    // check for own nick changed
    if (StrPos('Nick,yahoo_id', cws.szSetting) <> nil) then
    begin
      NotifyAllForms(HM_NOTF_NICKCHANGED, 0, 0)
    end
    else
      // check for history++ setings changed
      if StrComp(cws.szModule, hppDBName) = 0 then
      begin
        if GridOptions.Locked then
          exit;
        if StrComp(cws.szSetting, 'FormatCopy') = 0 then
          GridOptions.ClipCopyFormat := GetDBWideStr(hppDBName, 'FormatCopy', DEFFORMAT_CLIPCOPY)
        else if StrComp(cws.szSetting, 'FormatCopyText') = 0 then
          GridOptions.ClipCopyTextFormat := GetDBWideStr(hppDBName, 'FormatCopyText', DEFFORMAT_CLIPCOPYTEXT)
        else if StrComp(cws.szSetting, 'FormatReplyQuoted') = 0 then
          GridOptions.ReplyQuotedFormat := GetDBWideStr(hppDBName, 'FormatReplyQuoted', DEFFORMAT_REPLYQUOTED)
        else if StrComp(cws.szSetting, 'FormatReplyQuotedText') = 0 then
          GridOptions.ReplyQuotedTextFormat := GetDBWideStr(hppDBName, 'FormatReplyQuotedText', DEFFORMAT_REPLYQUOTEDTEXT)
        else if StrComp(cws.szSetting, 'FormatSelection') = 0 then
          GridOptions.SelectionFormat := GetDBWideStr(hppDBName, 'FormatSelection', DEFFORMAT_SELECTION)
        else if StrComp(cws.szSetting, 'ProfileName') = 0 then
          GridOptions.ProfileName := GetDBWideStr(hppDBName, 'ProfileName', '')
        else if StrComp(cws.szSetting, 'DateTimeFormat') = 0 then
          GridOptions.DateTimeFormat := GetDBWideStr(hppDBName, 'DateTimeFormat', DEFFORMAT_DATETIME);
      end;
    exit;
  end;

  szProto := Proto_GetBaseAccountName(wParam);
  if (StrComp(cws.szModule, 'CList') <> 0) and
    ((szProto = nil) or (StrComp(cws.szModule, szProto) <> 0)) then
    exit;

  if (StrComp(cws.szModule, META_PROTO) = 0) and
    (StrComp(cws.szSetting, 'Nick') = 0) then
    exit;

  // check for contact nick changed
  if (StrPos('MyHandle,Nick', cws.szSetting) <> nil) then
    NotifyAllForms(HM_NOTF_NICKCHANGED, wParam, 0);
end;

// Called when smilayadd settings have changed
//wParam = Contact handle which options have changed, NULL if global options changed
//lParam = (LPARAM) 0; not used
function OnSmAddSettingsChanged(wParam: WPARAM; lParam: LPARAM): Integer; cdecl;
begin
  Result := 0;
  if GridOptions.Locked then exit;
  LoadGridOptions;
end;

// Called when setting in FontService have changed
// wParam = 0, lParam = 0
function OnFSChanged(wParam: WPARAM; lParam: LPARAM): Integer; cdecl;
begin
  Result := 0;
  if GridOptions.Locked then exit;
  LoadGridOptions;
end;

// Called when setting in DB have changed
// wParam = hContact, lParam = PDbContactWriteSetting
function OnContactChanged(wParam: WPARAM; lParam: LPARAM): Integer; cdecl;
begin
  Result := 0;
  if GridOptions.Locked then exit;
  LoadGridOptions;
end;

// Called when contact is deleted
// wParam - hContact
function OnContactDelete(wParam: wParam; lParam: LPARAM): Integer; cdecl;
begin
  Result := 0;
  NotifyAllForms(HM_MIEV_CONTACTDELETED,wParam,lParam);
end;

function OnOptInit(awParam: WPARAM; alParam: LPARAM): Integer; cdecl;
var
  odp: TOPTIONSDIALOGPAGE;
begin
  ZeroMemory(@odp,SizeOf(odp));
  odp.pszTemplate := MakeIntResourceA(IDD_OPT_HISTORYPP);
  odp.szTitle.a := 'History';
  odp.szGroup.a := nil;
  odp.pfnDlgProc := @OptDialogProc;
  odp.flags := ODPF_BOLDGROUPS;
  Options_AddPage(awParam,@odp);
  Result:=0;
end;

//sent when the icons DLL has been changed in the options dialog, and everyone
//should re-make their image lists
//wParam=lParam=0
function OnIconChanged(wParam: WPARAM; lParam: LPARAM): Integer; cdecl;
begin
  Result := 0;
  if not GridOptions.ShowIcons then exit;
  LoadIcons;
  NotifyAllForms(HM_NOTF_ICONSCHANGED,0,0);
end;

function OnIcon2Changed(awParam: WPARAM; alParam: LPARAM): Integer; cdecl;
begin
  Result := 0;
  LoadIcons2;
  NotifyAllForms(HM_NOTF_ICONS2CHANGED,0,0);
end;

//the context menu for a contact is about to be built     v0.1.0.1+
//wParam=(WPARAM)(HANDLE)hContact
//lParam=0
//modules should use this to change menu items that are specific to the
//contact that has them
function OnBuildContactMenu(hContact: WPARAM; alParam: LPARAM): Integer; cdecl;
var
  hLast: THandle;
  count: Integer;
  text:  PWideChar;
begin
  Result := 0;
  count := db_event_count(hContact);
  hLast := db_event_last(hContact);
  if (PrevShowHistoryCount xor ShowHistoryCount) or (count <> MenuCount) then
  begin
    Menu_ShowItem(hmiContact, byte(hLast <> 0));

    if ShowHistoryCount then
      text := pWideChar(Format('%s [%u]',[TranslateW(histName),count]))
    else if PrevShowHistoryCount then
      text := TranslateW(histName)
    else
      text := nil;

    Menu_ModifyItem(hmiContact, text);
    MenuCount := count;
    PrevShowHistoryCount := ShowHistoryCount;
  end;
end;

//wParam : MCONTACT
//lParam : HDBCONTACT
//Called when a new event has been added to the event chain
//for a contact, MCONTACT contains the contact who added the event,
//HDBCONTACT a handle to what was added.
function OnEventAdded(wParam: WPARAM; lParam: LPARAM): Integer; cdecl;
begin
  Result := 0;
  NotifyAllForms(HM_MIEV_EVENTADDED,wParam,lParam);
end;

//wParam : MCONTACT
//lParam : HDBEVENT
//Affect : Called when an event is about to be deleted from the event chain
//for a contact, see notes
function OnEventDeleted(wParam: WPARAM; lParam: LPARAM): Integer; cdecl;
begin
  Result := 0;
  NotifyAllForms(HM_MIEV_EVENTDELETED,wParam,lParam);
end;

//wParam : hMetaContact
//lParam : hDefaultContact
//Affect : Called when a metacontact's default contact changes
function OnMetaDefaultChanged(wParam: WPARAM; lParam: LPARAM): Integer; cdecl;
begin
  Result := 0;
  NotifyAllForms(HM_MIEV_METADEFCHANGED,wParam,lParam);
end;

//wParam=0
//lParam=0
//This hook is fired just before the thread unwind stack is used,
//it allows MT plugins to shutdown threads if they have any special
//processing to do, etc.
function OnPreshutdown(wParam: WPARAM; lParam: LPARAM): Integer; cdecl;
begin
  Result := 0;
  NotifyAllForms(HM_MIEV_PRESHUTDOWN,0,0);
end;

exports
  MirandaPluginInterfaces,
  Load,
  Unload;

begin
  DisableThreadLibraryCalls(hInstance);

  PluginInfo.cbSize:= SizeOf(TPLUGININFOEX);
  PluginInfo.shortName:= hppShortNameV;
  PluginInfo.version:= hppVersion;
  PluginInfo.description:= hppDescription;
  PluginInfo.author:= hppAuthor;
  PluginInfo.copyright:= hppCopyright;
  PluginInfo.homepage:= hppHomePageURL;
  PluginInfo.flags:= UNICODE_AWARE;
  PluginInfo.uuid:= MIID_HISTORYPP;

  // decreasing ref count to oleaut32.dll as said
  // in plugins doc
  FreeLibrary(GetModuleHandle('oleaut32.dll'));
  // to use RTL on LTR systems
  SysLocale.MiddleEast := true;

  // shadow is back again...
  Forms.HintWindowClass := THppHintWindow;

end.
