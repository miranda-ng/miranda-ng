(*
    ImportTXT plugin for Miranda IM: the free IM client for Microsoft* Windows*

    Copyright (С) 2008 Abyss

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
  Windows,
  Messages,
  SysUtils,
  Inifiles,
  m_api,// in '.\inc\m_api.pas',
{ используются в ImportThrd
  KOL in '.\kol\kol.pas',
  err in '.\kol\err.pas',
  KOLedb in '.\kol\KOLEdb.pas',
}
  General in 'general.pas',
  ImportT in 'ImportT.pas',
  ImportTU in 'ImportTU.pas',
  ImportThrd in 'ImportThrd.pas',
  PerlRegEx in 'PerlRegEx.pas',
  ImportTxtDlg in 'ImportTxtDlg.pas',
  ImportTxtWiz in 'ImportTxtWiz.pas';

{$R imptxt_ver.res}

const MIID_IMPORTTEXT:TGUID = '{6F376B33-D3F4-4c4f-A96B-77DA08043B06}';


var
  hwndWizard:HWND;
  hwndDialog:HWND;
  //Services
  SrvITxt,SrvIWiz:THANDLE;
  //hooks
  onLoadHook:THANDLE;
  onAccChangHook:THANDLE;

function MirandaPluginInfoEx(mirandaVersion:DWORD):PPLUGININFOEX; cdecl;
begin
  PluginInfo.cbSize     :=sizeof(TPLUGININFOEX);
  PluginInfo.shortName  :='Import TXT';
  PluginInfo.version    :=$0000010A;
  PluginInfo.description:='Imports history saved in TXT files from other clients.';
  PluginInfo.author     :='Abyss';
  PluginInfo.authorEmail:='abyss.andrey@gmail.com';
  PluginInfo.copyright  :='(C)2008 Abyss';
  PluginInfo.homepage   :='none';
  PluginInfo.flags      :=UNICODE_AWARE;
  PluginInfo.uuid       :=MIID_IMPORTTEXT;

  result := @PluginInfo;
end;

function ContactMenuCommand(wParam: wParam; lParam: lParam): int_ptr; cdecl;
begin
  result := 0;
  if IsWindow(hwndDialog) then
  begin
    SetForegroundWindow(hwndDialog);
    BringWindowToTop(hwndDialog);
  end
  else
    hwndDialog := CreateDialogParamW(hInstance, MAKEINTRESOURCEW(IDD_IMPDIALOG),
      0, @IDMainWndProc, wParam);
end;

function MainMenuCommand(wParam: wParam; lParam: lParam): int_ptr; cdecl;
begin
  result := 0;
  if (IsWindow(hwndWizard)) then
  begin
    SetForegroundWindow(hwndWizard);
    BringWindowToTop(hwndWizard);
  end
  else
    hwndWizard := CreateDialogW(hInstance, MAKEINTRESOURCEW(IDD_IMPWIZARD), 0, @WizardDlgProc);
end;

function OnAccountChanged(wParam: wParam; lParam: lParam): int; cdecl;
begin
  result := 0;
  EnumProtocols;
  if Assigned(OnAccountListChange) then
    OnAccountListChange;
end;

function OnModulesLoaded(wParam: wParam; lParam: lParam): int; cdecl;
begin
  UnhookEvent(onLoadHook);
  result := 0;
  EnumProtocols;
  // check for AutoStart
  if (DBReadByte(0, IMPORT_TXT_MODULE, IMPORT_TXT_AS) = 1) and (ProtoCount > 0) then
  begin
    CallService(IMPORT_WIZ_SERVICE, 0, 0);
    DBWriteByte(0, IMPORT_TXT_MODULE, IMPORT_TXT_AS, 0);
  end;
end;

function Load(): int; cdecl;
var
  mi: TCListMenuItem;
begin
  cp := CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);
  SrvITxt := CreateServiceFunction(IMPORT_TXT_SERVICE, @ContactMenuCommand);
  SrvIWiz := CreateServiceFunction(IMPORT_WIZ_SERVICE, @MainMenuCommand);
  FillChar(mi, sizeof(mi), 0);
  mi.cbSize := sizeof(mi);
  mi.flags := 0;
  mi.position := 1000090050;
  mi.hIcon := LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DEFAULT));
  mi.szName.a := 'Import history';
  mi.pszService := IMPORT_TXT_SERVICE;
  mi.pszContactOwner := nil; // All contacts
  Menu_AddContacTMenuItem(@mi);

  mi.position := 500050010;
  mi.pszService := IMPORT_WIZ_SERVICE;
  mi.pszContactOwner := nil;
  Menu_AddMainMenuItem(@mi);

  onLoadHook := HookEvent(ME_SYSTEM_MODULESLOADED, @OnModulesLoaded);
  onAccChangHook := HookEvent(ME_PROTO_ACCLISTCHANGED, @OnAccountChanged);
  result := 0;
end;

function Unload: int; cdecl;
begin
  UnhookEvent(onAccChangHook);
  DestroyServiceFunction(SrvITxt);
  DestroyServiceFunction(SrvIWiz);
  Result := 0;
end;

exports
  Load, Unload, MirandaPluginInfoEx;

initialization
  DisableThreadLibraryCalls(hInstance);

end.
