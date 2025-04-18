{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
{$A+,H+,A8}
{$UNDEF AllowInline}

unit m_api;

interface

{$IFDEF FPC}
  {$DEFINE AllowInline}
{$ELSE}
  {$IFDEF ConditionalExpressions}
    // CompilerVersion defined in SYSTEM module, need to use not earlier
    {$IF CompilerVersion >= 22}
      {$DEFINE AllowInline}
    {$IFEND}
  {$ENDIF}
{$ENDIF}

uses
  Windows,CommCtrl,Messages;//, FreeImage;

// often used
const
  strCList:PAnsiChar = 'CList';
const
  NM_FIRST = 0;     // from CommCtrl

// RichEdit definitions
type
  PCHARRANGE = ^TCHARRANGE;
  TCHARRANGE = record
    cpMin:integer;
    cpMax:integer;
  end;
  CHARRANGE = TCHARRANGE;

// C translations
type
{$IFNDEF FPC}
  {$IFDEF WIN32}
  // delphi 64 must have these types anyway
  int_ptr   = integer;
  uint_ptr  = cardinal;

  {$ENDIF}
  long      = longint;
  plong     = ^long;
  {$IFDEF VER150}
  UnicodeString = WideString;
  ULONG_PTR = longword;
  {$ENDIF}
  DWORD_PTR = ULONG_PTR;
  size_t    = ULONG_PTR;
{$ENDIF}
  pint_ptr  = ^int_ptr;
  puint_ptr = ^uint_ptr;
  time_t    = ulong;
  int       = integer;
//  uint     = Cardinal;
//  pint     = ^int;
//  WPARAM   = integer;
//  LPARAM   = integer;
  TLPARAM   = LPARAM;
  TWPARAM   = WPARAM;

// My definitions
  TWNDPROC = function (Dialog:HWND; hMessage:uint; wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;

type
  PTChar = ^TChar;
  TChar = record
    case boolean of
      false: (a:PAnsiChar); // ANSI or UTF8
      true:  (w:PWideChar); // Unicode
  end;

{$include m_core.inc}
{$include m_system.inc}

{-- start newpluginapi --}
const
  UNICODE_AWARE = 1;

type
  PPLUGININFOEX = ^TPLUGININFOEX;
  TPLUGININFOEX = record
    cbSize     : int;
    shortName  : PAnsiChar;
    version    : dword;
    description: PAnsiChar;
    author     : PAnsiChar;
    copyright  : PAnsiChar;
    homepage   : PAnsiChar;
    flags      : integer;    // right now the only flag, UNICODE_AWARE, is recognized here
    uuid       : TGUID;      // plugin's unique identifier
  end;

type
  PCMPlugin = ^CMPlugin;
  CMPlugin = packed record
    vft        : pointer;  // virtual function table

    m_hInst    : THANDLE;
    m_szModule : PAnsiChar;
    m_pInfo    : PPLUGININFOEX;
    m_hLogger  : THANDLE;

    blabla1    : pointer;  // emulator of icon's list
    blabla2    : integer;
    blabla3    : integer;
    blabla4    : integer;
    blabla5    : pointer;
  end;

//----- Fork enchancement -----
{
  Miranda/System/LoadModule event
  called when a plugin is being loaded dynamically
  wParam=PLUGININFOEX*
  //lParam=HINSTANCE of the loaded plugin
}
const
  ME_SYSTEM_MODULELOAD:PAnsiChar = 'Miranda/System/LoadModule';

{
  Miranda/System/UnloadModule event
  called when a plugin is being unloaded dynamically
  wParam=PLUGININFOEX*
  lParam=HINSTANCE of the loaded plugin
}
  ME_SYSTEM_MODULEUNLOAD:PAnsiChar = 'Miranda/System/UnloadModule';

{
  Each service mode plugin must implement MS_SERVICEMODE_LAUNCH
   This service might return one of the following values:
	SERVICE_CONTINUE - load Miranda normally, like there's no service plugins at all
	SERVICE_ONLYDB - load database and then execute service plugin only
	SERVICE_MONOPOLY - execute only service plugin, even without database
	SERVICE_FAILED - terminate Miranda execution
}
const
  SERVICE_CONTINUE = 0;
  SERVICE_ONLYDB   = 1;
  SERVICE_MONOPOLY = 2;
  SERVICE_FAILED   = (-1);

  MS_SERVICEMODE_LAUNCH:PansiChar = 'ServiceMode/Launch';

{-- end newpluginapi --}

var
  { has to be returned via MirandaPluginInfo and has to be statically allocated,
  this means only one module can return info, you shouldn't be merging them anyway! }
  PluginInfo: TPLUGININFOEX;
  g_plugin: CMPlugin;

  {$include m_clist.inc}
  {$include m_genmenu.inc}
  {$include m_protosvc.inc}
  {$include m_protocols.inc}

  {$include m_acc.inc}
  {$include m_avatars.inc}
  {$include m_awaymsg.inc}
  {$include m_button.inc}
  {$include m_clc.inc}
  {$include m_clistint.inc}
  {$include m_cluiframes.inc}
  {$include m_contacts.inc}
  {$include m_crypto.inc}
  {$include m_database.inc}
  {$include m_descbutton.inc}
  {$include m_email.inc}
  {$include m_extraicons.inc}
  {$include m_findadd.inc}
  {$include m_fontservice.inc}
  {$include m_history.inc}
  {$include m_hotkeys.inc}
  {$include m_icolib.inc}
  {$include m_ignore.inc}
  {$include m_imgsrvc.inc}
  {$include m_json.inc}
  {$include m_langpack.inc}
  {$include m_message.inc}
  {$include m_metacontacts.inc}
  {$include m_netlib.inc}
  {$include m_options.inc}
  {$include m_popup.inc}
  {$include m_protomod.inc}
  {$include m_skin.inc}
  {$include m_ssl.inc}
  {$include statusmodes.inc}
  {$include m_timezones.inc}
  {$include m_utils.inc}
  {$include m_xstatus.inc}
  {$include m_zlib.inc}

  {$include custom_api.inc}

{$define M_API_UNIT}
  {$include m_helpers.inc}

implementation

{$undef M_API_UNIT}
  {$include m_helpers.inc}

initialization
  g_plugin.m_hInst := hInstance;
  g_plugin.m_pInfo := @PluginInfo;
  g_plugin.blabla4 := 10;
  RegisterPlugin(g_plugin);

finalization
  UnregisterPlugin(g_plugin);

end.
