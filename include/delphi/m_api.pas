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
{$A+,H+}
{$IFNDEF VER130} // skip for delphi 5
  {$IFDEF WIN32}{$A4}{$ENDIF}
  {$IFDEF WIN64}{$A8}{$ENDIF}
{$ENDIF}
{$IFDEF FPC}
  {$DEFINE AllowInline}
{$ELSE}
  {$IFDEF NativeCode} // Delphi.NET+
    {$DEFINE AllowInline}
  {$ENDIF}
{$ENDIF}

unit m_api;

interface

uses
  Windows;//, FreeImage;

// often used
const
  strCList:PAnsiChar = 'CList';
const
  WM_USER  = $0400; // from Messages
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
  DWORD_PTR = ULONG_PTR;
  size_t    = ULONG_PTR;
{$ENDIF}
  pint_ptr  = ^int_ptr;
  puint_ptr = ^uint_ptr;
  time_t    = ulong;
  int       = integer;
//  uint     = Cardinal;
//  pint     = ^int;
//  WPARAM   = Integer;
//  LPARAM   = Integer;
  TLPARAM   = LPARAM;
  TWPARAM   = WPARAM;

// My definitions
  TWNDPROC = function (Dialog:HWnd; hMessage:uint; wParam:WPARAM;lParam:LPARAM):lresult; stdcall;

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

// The UUID structure below is used to for plugin UUID's and module type definitions
type
  PMUUID = ^TMUUID;
  MUUID  = System.TGUID;
  TMUUID = MUUID;
{
  MUUID = record
    a:cardinal;
    b:word;
    c:word;
    d:array [0..7] of byte;
  end;
}

{$include interfaces.inc}

type
  PPLUGININFOEX = ^TPLUGININFOEX;
  TPLUGININFOEX = record
    cbSize     :int;
    shortName  :PAnsiChar;
    version    :DWORD;
    description:PAnsiChar;
    author     :PAnsiChar;
    authorEmail:PAnsiChar;
    copyright  :PAnsiChar;
    homepage   :PAnsiChar;
    flags      :Byte;  // right now the only flag, UNICODE_AWARE, is recognized here
    uuid       :MUUID; // plugin's unique identifier
  end;

//----- Fork enchancement -----
{
  Miranda/System/LoadModule event
  called when a plugin is being loaded dynamically
  wParam=PLUGININFOEX*
  //lParam=HINSTANCE of the loaded plugin
}
const
  ME_SYSTEM_MODULELOAD:pAnsiChar = 'Miranda/System/LoadModule';

{
  Miranda/System/UnloadModule event
  called when a plugin is being unloaded dynamically
  wParam=PLUGININFOEX*
  lParam=HINSTANCE of the loaded plugin
}
  ME_SYSTEM_MODULEUNLOAD:pAnsiChar = 'Miranda/System/UnloadModule';

  { Database plugin stuff  }

  // grokHeader() error codes
const
   EGROKPRF_NOERROR   = 0;
   EGROKPRF_CANTREAD  = 1; // can't open the profile for reading
   EGROKPRF_UNKHEADER = 2; // header not supported, not a supported profile
   EGROKPRF_VERNEWER  = 3; // header correct, version in profile newer than reader/writer
   EGROKPRF_DAMAGED   = 4; // header/version fine, other internal data missing, damaged.
// makeDatabase() error codes
   EMKPRF_CREATEFAILED = 1; // for some reason CreateFile() didnt like something

type
  PDATABASELINK = ^TDATABASELINK;
  TDATABASELINK = record
    cbSize : int;
    {
      returns what the driver can do given the flag
    }
    getCapability : function (flag:int):int; cdecl;
    {
       buf: pointer to a string buffer
       cch: length of buffer
       shortName: if true, the driver should return a short but descriptive name, e.g. "3.xx profile"
       Affect: The database plugin must return a "friendly name" into buf and not exceed cch bytes,
         e.g. "Database driver for 3.xx profiles"
       Returns: 0 on success, non zero on failure
    }
    getFriendlyName : function (buf:PAnsiChar; cch:size_t; shortName:int):int; cdecl;
    {
      profile: pointer to a string which contains full path + name
      Affect: The database plugin should create the profile, the filepath will not exist at
        the time of this call, profile will be C:\..\<name>.dat
      Note: Do not prompt the user in anyway about this operation.
      Note: Do not initialise internal data structures at this point!
      Returns: 0 on success, non zero on failure - error contains extended error information, see EMKPRF_
    }
    makeDatabase : function (profile:PAnsiChar; error:Pint):int; cdecl;
    {
      profile: [in] a null terminated string to file path of selected profile
      error: [in/out] pointer to an int to set with error if any
      Affect: Ask the database plugin if it supports the given profile, if it does it will
        return 0, if it doesnt return 1, with the error set in error -- EGROKPRF_  can be valid error
        condition, most common error would be [EGROKPRF_UNKHEADER]
      Note: Just because 1 is returned, doesnt mean the profile is not supported, the profile might be damaged
        etc.
      Returns: 0 on success, non zero on failure
    }
    grokHeader : function (profile:PAnsiChar; error:Pint):int; cdecl;
    {
      Affect: Tell the database to create all services/hooks that a 3.xx legecy database might support into link
      Returns: 0 on success, nonzero on failure
    }
    Load : function (profile:PAnsiChar):int; cdecl;
    {
      Affect: The database plugin should shutdown, unloading things from the core and freeing internal structures
      Returns: 0 on success, nonzero on failure
      Note: Unload() might be called even if Load() was never called, wasLoaded is set to 1 if Load() was ever called.
    }
    Unload : function (wasLoaded:int):int; cdecl;
  end;

{-- end newpluginapi --}

var
  { has to be returned via MirandaPluginInfo and has to be statically allocated,
  this means only one module can return info, you shouldn't be merging them anyway! }
  PLUGININFO: TPLUGININFOEX;

  {$include m_plugins.inc}
  {$include m_database.inc}
  {$include m_findadd.inc}
  {$include m_awaymsg.inc}
  {$include m_email.inc}
  {$include m_history.inc}
  {$include m_message.inc}
  {$include m_tabsrmm.inc}
  {$include m_url.inc}
  {$include m_clui.inc}
  {$include m_idle.inc}
  {$include m_ignore.inc}
  {$include m_skin.inc}
  {$include m_file.inc}
  {$include m_netlib.inc}
  {$include m_langpack.inc}
  {$include m_clist.inc}
  {$include m_clc.inc}
  {$include m_userinfo.inc}
  {$include m_protosvc.inc}
  {$include m_options.inc}
  {$include m_ssl.inc}
  {$include m_icq.inc}
  {$include m_protoint.inc}
  {$include m_protocols.inc}
  {$include m_protomod.inc}
  {$include m_utils.inc}
  {$include m_addcontact.inc}
  {$include statusmodes.inc}
  {$include m_contacts.inc}
  {$include m_genmenu.inc}
  {$include m_icolib.inc}
  {$include m_fontservice.inc}
  {$include m_chat.inc}
  {$include m_fingerprint.inc}
  {$include m_variables.inc}
  {$include m_cluiframes.inc}
  {$include m_popup.inc}
  {$include m_avatars.inc}
  {$include m_smileyadd.inc}
  {$include m_tipper.inc}
  {$include m_button.inc}
  {$include m_dbeditor.inc}
  {$include m_userinfoex.inc}
  {$include m_imgsrvc.inc}
  {$include m_hotkeys.inc}
  {$include m_acc.inc}
  {$include m_xml.inc}
  {$include m_modernopt.inc}
  {$include m_descbutton.inc}
  {$include m_iconheader.inc}
  {$include m_extraicons.inc}
  {$include m_errors.inc}
  {$include m_help.inc}
  {$include m_proto_listeningto.inc}
  {$include m_toptoolbar.inc}
  {$include m_msg_buttonsbar.inc}
  {$include m_json.inc}
{$define M_API_UNIT}
  {$include m_helpers.inc}
  {$include m_clistint.inc}
  {$include m_metacontacts.inc}
  {$include m_timezones.inc}
  {$include m_crypto.inc}

  {$include m_newawaysys.inc}

implementation

const
  hLangpack:THANDLE = 0;

{$undef M_API_UNIT}
  {$include m_helpers.inc}
  {$include m_clistint.inc}

end.
