{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2006 Miranda ICQ/IM project, 
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

{$IFNDEF M_LANGPACK}
{$DEFINE M_LANGPACK}

const

{
  reloads langpack
  wParam=0 (ignored)
  lParam=(LPARAM)(TCHAR*)langpack file name or NULL to reload the current one
  always returns 0
}
  MS_LANGPACK_RELOAD:PAnsiChar = 'LangPack/Reload';

  ME_LANGPACK_CHANGE:PAnsiChar = 'LangPack/Changed';

procedure mir_getLP(var pInfo:TPLUGININFOEX; var hLang:int); stdcall;
                 external CoreDLL name 'mir_getLP';

{$ENDIF}
