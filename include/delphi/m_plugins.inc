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

{$ENDIF}
