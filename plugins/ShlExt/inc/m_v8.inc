(*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2004 Miranda ICQ/IM project,
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

*)
{$IFNDEF V8}
{$DEFINE V8}

type
	
	PMUUID = ^TMUUID;
	TMUUID = record
		a: LongWord;
		b, c: Word;
		d: array[0..7] of Byte;
	end;

    PPLUGININFOEX = ^TPLUGININFOEX;
    TPLUGININFOEX = record
        cbSize: int;
        shortName: PChar;
        version: DWORD;
        description: PChar;
        author: PChar;
        authorEmail: PChar;
        copyright: PChar;
        homepage: PChar;
        isTransient: Byte;              // leave zero for now
        { one of the DEFMOD_* consts in m_plugin or zero, if non zero, this will
        suppress loading of the specified builtin module }
        replacesDefaultModule: int;
		uuid: TMUUID;	
    end;

    { any module must export the below functions to be valid plugin
    the export names MUST be 'MirandaPluginInfo' 'Load' 'Unload' }
	
	// defined in newpluginapi.inc

    //TMirandaPluginInfo      = function(mirandaVersion: DWORD): PPLUGININFO; cdecl;
    //TLoad                   = function(link: PPLUGINLINK): int; cdecl;
    //TUnload                 = function: int; cdecl;

{$ENDIF}
