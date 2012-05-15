(*
    ImportTXT plugin for Miranda IM: the free IM client for Microsoft* Windows*

    Copyright (Ñ) 2008 Abyss

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

{$IMAGEBASE $2080000}
library importtxt;

uses
  m_api,
  Windows,
  General,
  ImportTxtDlg,
  ImportTxtWiz;

{$R imptxt_ver.res}
{$include m_helpers.inc}


const
  PluginInfo:TPLUGININFOEX=(
    cbSize     :sizeof(TPLUGININFOEX);
    shortName  :'Import TXT';
    version    :$0000010A;
    description:'Imports history saved in TXT files from other clients.';
    author     :'Abyss';
    authorEmail:'abyss.andrey@gmail.com';
    copyright  :'(C)2008 Abyss';
    homepage   :'none';
    flags      :UNICODE_AWARE;
    replacesDefaultModule:0;
    uuid:'{6F376B33-D3F4-4c4f-A96B-77DA08043B06}';
  );

// Updater compatibility data
const
  VersionURL        = 'http://abyss.web.ur.ru/itxt_ver.htm';
  VersionPrefix     = 'Current version: ';
  UpdateURL         = 'http://abyss.web.ur.ru/importtxt.zip';
  BetaVersionURL    = nil;
  BetaVersionPrefix = nil;
  BetaUpdateURL     = nil;
  BetaChangelogURL  = nil;

var
  PluginInterfaces:array [0..1] of MUUID;
  hwndWizard:HWND;
  hwndDialog:HWND;
  //Services
  SrvITxt,SrvIWiz:Cardinal;
  //hooks
  onLoadHook:Cardinal;
  onAccChangHook:Cardinal;

function MirandaPluginInfo(mirandaVersion:DWORD):PPLUGININFOEX; cdecl;
begin
  MirVers:=mirandaVersion;
  result:=@PluginInfo;
  //PluginInfo.cbSize:=SizeOf(TPLUGININFO);
end;

function MirandaPluginInfoEx(mirandaVersion:DWORD):PPLUGININFOEX; cdecl;
begin
  MirVers:=mirandaVersion;
  result:=@PluginInfo;
  //PluginInfo.cbSize:=SizeOf(TPLUGININFOEX);
end;

function ContactMenuCommand(wParam: WPARAM; lParam: LPARAM): Integer; cdecl;
begin
  Result := 0;
  if IsWindow(hwndDialog) then
   begin
    SetForegroundWindow(hwndDialog);
		BringWindowToTop(hwndDialog);
   end
                          else
   hwndDialog:=CreateDialogParamW(hInstance,MAKEINTRESOURCEW(IDD_IMPDIALOG),0,@IDMainWndProc, wParam);
end;

function MainMenuCommand(wParam: WPARAM; lParam: LPARAM): Integer; cdecl;
begin
 Result:=0;
 if (IsWindow(hwndWizard)) then
   begin
		SetForegroundWindow(hwndWizard);
		BringWindowToTop(hwndWizard);
	 end
	                         else
   hwndWizard:=CreateDialogW(hInstance, MAKEINTRESOURCEW(IDD_IMPWIZARD), 0, @WizardDlgProc);
end;

function OnAccountChanged(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  Result:=0;
  EnumProtocols;
  if Assigned(OnAccountListChange) then OnAccountListChange;
end;

function OnModulesLoaded(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var upd: TUpdate;
    buf:array [0..63] of char;
begin
 PluginLink^.UnhookEvent(onloadhook);
 Result:=0;
 EnumProtocols;
 // Register in updater
 if Boolean(PluginLink.ServiceExists(MS_UPDATE_REGISTER)) then
  begin
   ZeroMemory(@upd,SizeOf(upd));
   upd.cpbVersion := SizeOf(upd);
   upd.szComponentName := PluginInfo.ShortName;;
   upd.pbVersion := CreateVersionStringPlugin(@pluginInfo,buf);
   upd.cpbVersion := lstrlen(upd.pbVersion);
   upd.szUpdateURL := UpdateURL;
   upd.szVersionURL := VersionURL;
   upd.pbVersionPrefix := VersionPrefix;
   upd.cpbVersionPrefix := Length(VersionPrefix);
   upd.szBetaUpdateURL := BetaUpdateURL;
   upd.szBetaVersionURL := BetaVersionURL;
   upd.pbBetaVersionPrefix := BetaVersionPrefix;
   upd.cpbBetaVersionPrefix := length(upd.pbBetaVersionPrefix);
   upd.szBetaChangelogURL := BetaChangelogURL;
   PluginLink.CallService(MS_UPDATE_REGISTER, 0, DWORD(@upd));
  end;
 //check for AutoStart
 if (DBReadByte(0,IMPORT_TXT_MODULE,IMPORT_TXT_AS)=1) and
    (ProtoCount>0) then
    begin
    pluginLink^.CallService(IMPORT_WIZ_SERVICE,0,0);
    DBWriteByte(0,IMPORT_TXT_MODULE,IMPORT_TXT_AS,0);
    end;
end;


function Load(link: PPLUGINLINK): int; cdecl;
var
   mi: TCListMenuItem;
begin
  PLUGINLINK := Pointer(link);
  IsMirandaUnicode:=fIsMirandaUnicode;
  cp:=pluginLink^.CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);
  SrvITxt:=pluginLink^.CreateServiceFunction(IMPORT_TXT_SERVICE, @ContactMenuCommand);
  SrvIWiz:=pluginLink^.CreateServiceFunction(IMPORT_WIZ_SERVICE, @MainMenuCommand);
  FillChar(mi, sizeof(mi), 0);
  mi.cbSize := sizeof(mi);
  mi.flags := 0;
  mi.position := 1000090050;
  mi.hIcon := LoadIcon(hInstance,MAKEINTRESOURCE(IDI_DEFAULT));
  mi.szName.a := 'Import history';
  mi.pszService := IMPORT_TXT_SERVICE;
  mi.pszContactOwner:=nil;  //All contacts
  pluginLink^.CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, int(@mi));
  mi.position := 500050010;
  mi.pszService := IMPORT_WIZ_SERVICE;
  mi.pszContactOwner:=nil;
  pluginLink^.CallService(MS_CLIST_ADDMAINMENUITEM, 0, int(@mi));
  onloadhook:=PluginLink^.HookEvent(ME_SYSTEM_MODULESLOADED,@OnModulesLoaded);
  if MirVers>080000 then onAccChangHook:=pluginLink^.HookEvent(ME_PROTO_ACCLISTCHANGED,@OnAccountChanged);
  Result := 0;
end;

function Unload: int; cdecl;
begin
  if MirVers>080000 then pluginLink^.UnhookEvent(onAccChangHook);
  pluginlink^.DestroyServiceFunction(SrvITxt);
  pluginlink^.DestroyServiceFunction(SrvIWiz);
  Result := 0;
end;

function MirandaPluginInterfaces:PMUUID; cdecl;
begin
  PluginInterfaces[0]:=PluginInfo.uuid;
  PluginInterfaces[1]:=MIID_LAST;
  result:=@PluginInterfaces;
end;


exports

  Load, Unload,
  MirandaPluginInfo,
  MirandaPluginInfoEx,
  MirandaPluginInterfaces;

begin
end.
