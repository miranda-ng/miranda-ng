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

{$IFNDEF M_PLUGINS}
{$DEFINE M_PLUGINS}

{
  Undocumented: Do NOT use.
  Version: 0.3.4.1+ (2004/10/04)
}
const
  DBPE_DONE = 1;
  DBPE_CONT = 0;
  DBPE_HALT = -1;
type
  PPLUGIN_DB_ENUM = ^TPLUGIN_DB_ENUM;
  TPLUGIN_DB_ENUM = record
    cbSize : int;
// 0 = continue, 1 = found, -1 = stop now
    pfnEnumCallback : function (pluginname:PAnsiChar; link:pointer; lParam:LPARAM):int;cdecl;
    lParam : LPARAM;
  end;

const
  MS_PLUGINS_ENUMDBPLUGINS:PAnsiChar = 'Plugins/DbEnumerate';

const
//  DEFMOD_PROTOCOLICQ    = 1;  // removed from v0.3.0.0 alpha
//  DEFMOD_PROTOCOLMSN    = 2;  // removed from v0.1.2.0+
  DEFMOD_UIFINDADD      = 3;
  DEFMOD_UIUSERINFO     = 4;
  DEFMOD_SRMESSAGE      = 5;  // removed from v0.3.3a (temp)
  DEFMOD_SRURL          = 6;
  DEFMOD_SREMAIL        = 7;
  DEFMOD_SRAUTH         = 8;
  DEFMOD_SRFILE         = 9;
  DEFMOD_UIHELP         = 10;
  DEFMOD_UIHISTORY      = 11;
//  DEFMOD_RNDCHECKUPD    = 12; // removed from v0.3.1 alpha
//  DEFMOD_RNDICQIMPORT   = 13; // not built in to v0.1.0.1+ (removed from v0.3 alpha)
  DEFMOD_RNDAUTOAWAY    = 14;
  DEFMOD_RNDUSERONLINE  = 15;
//  DEFMOD_RNDCRYPT       = 16; // v0.1.0.1-v0.1.2.0
  DEFMOD_SRAWAY         = 17; // v0.1.0.1+
  DEFMOD_RNDIGNORE      = 18; // v0.1.0.1+
  DEFMOD_UIVISIBILITY   = 19; // v0.1.1.0+, options page only
  DEFMOD_UICLUI         = 20; // v0.1.1.0+
//  DEFMOD_UIPLUGINOPTS   = 21; // v0.1.2.1+ removed from 0.4.0.1
//  DEFMOD_PROTOCOLNETLIB = 22; // v0.1.2.2+ removed from 0.8.0.5
  DEFMOD_RNDIDLE        = 23; // v0.3.4a+
  DEFMOD_CLISTALL       = 24; // v0.3.4a+ (2004/09/28)
  DEFMOD_DB             = 25; // v0.3.4.3+ (2004/10/11)
//  DEFMOD_FONTSERVICE    = 26; // v0.7.0+ (2006/11/17)
  DEFMOD_UPDATENOTIFY   = 27;
//  DEFMOD_ICOLIB         = 27; // v0.7.0+ (2006/11/24)
  DEFMOD_SSL            = 28;  // v0.8.0+

  DEFMOD_HIGHEST        = 28;

  {
    wParam : 0
    lParam : 0
    Affect : Gets an array of modules that the plugins report they want to replace
    Returns: Returns a pointer to an array of int_ptr, with elements 1 or 0,
             indexed by the DEFMOD_* constants, 1 is to mark that the default
             module shouldn't be loaded, see notes
    Notes  : this is primarily for use by the core's module initialiser,
             but could also be used by modules that are doing
             naughty things that are very feature-dependent.
  }
  MS_PLUGINS_GETDISABLEDEFAULTARRAY:PAnsiChar = 'Plugins/GetDisableDefaultArray';

{$ENDIF}
