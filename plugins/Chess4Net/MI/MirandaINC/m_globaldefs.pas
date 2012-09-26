(*

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

*)

{$IFDEF FPC}
    {$PACKRECORDS C}
    {$MODE Delphi}
{$ENDIF}

unit m_globaldefs;

interface

uses

{$ifdef FPC}
    strings;
{$else}
    Windows;
{$endif}

type

    PByte                   = ^Byte;
    int                     = Integer;
    pint                    = ^int;
    WPARAM                  = Integer;
    LPARAM                  = Integer;
    DWORD                   = Integer;
    THandle                 = Integer;

    // strcpy()

    {$ifdef FPC}
        TStrCpy = function(Dst, Src: PChar): PChar;
    {$else}
        TStrCpy = function(Dst, Src: PChar): PChar; stdcall;
    {$endif}

    // strcat()

    {$ifdef FPC}
        TStrCat = function(Dst, Src: PChar): PChar;
    {$else}
        TStrCat = function(Dst, Src: PChar): PChar; stdcall;
    {$endif}

const

    {$ifdef FPC}
        strcpy: TStrCpy = strings.strcopy;
    {$else}
        strcpy: TStrCpy = lstrcpy;
    {$endif}

    {$ifdef FPC}
        strcat: TStrCat = strings.strcat;
    {$else}
        strcat: TStrCat = lstrcat;
    {$endif}

    {$include newpluginapi.inc}

var
    { this is now a pointer to a record of function pointers to match the C API,
    and to break old code and annoy you. }

    PLUGINLINK: PPLUGINLINK;

    { has to be returned via MirandaPluginInfo and has to be statically allocated,
    this means only one module can return info, you shouldn't be merging them anyway! }

    PLUGININFO: TPLUGININFO;
    PLUGININFOEX: TPLUGININFOEX;

implementation

end.
