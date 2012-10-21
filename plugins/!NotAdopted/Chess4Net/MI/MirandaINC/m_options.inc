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

{$IFNDEF M_OPTIONS}
{$DEFINE M_OPTIONS}

const

    {
        wParam : addinfo
        lParam : 0
        Affects: The user opened the options dialog, see notes
        Notes  : Modules should do whatever initalisation they need and call
                 MS_OPT_ADDPAGE with the wParam -- MS_OPT_ADDPAGE
                 can be called one or more times
                 if more than one page wants to be displayed.
    }
    ME_OPT_INITIALISE           = 'Opt/Initialise';

    {
        wParam : wParam from ME_OPT_INITIALISE
        lParam : Pointer to an initialised TOPTIONSDIALOGPAGE
        Affects: Adds a page to the options dialog, see notes
        Notes  : Strings in the structure can be released as soon as the
                 service returns -- but icons must be kept around, this iss
                 not a problem if you're loading theem from a resource.
                 -
                 This service should only be called within the ME_OPT_INITIALISE
                 event hook.
                 -
                 Pages in the options dialog operate just like pages in property
                 sheets, See the WinAPI documentation for details on how they operate.
        Version: Prior to v0.1.2.1 the options dialog would resize
                 to fit the largest page, but since then it's a fixed size
                 The largest page that fits neatly is 314x240 DLU's
                 -
                 Some of OPTIONSDIALOGPAGE's fields are version dependant.
    }
    MS_OPT_ADDPAGE                  = 'Opt/AddPage';

    { defacto size }

    OPTIONSDIALOGPAGE_V0100_SIZE    = $18;
    OPTIONSDIALOGPAGE_V0120_SIZE    = $28;

    { give group box titles a bold font }
    ODPF_BOLDGROUPS                 = 4;

type

    POPTIONSDIALOGPAGE = ^TOPTIONSDIALOGPAGE;
    TOPTIONSDIALOGPAGE = record
        cbSize: int;
        position: int;                  // position number, lower numbers are top most
        pszTitle: PChar;
        pfnDlgProc: Pointer;            // DLGPROC prototype
        pszTemplate: PChar;
        hInstance: THandle;
        hIcon: THandle;                 // v0.1.0.1+
        pszGroup: PChar;                // v0.1.0.1+
        groupPosition: int;             // v0.1.0.1+
        hGroupIcon: THandle;            // v0.1.0.1+
        flags: DWORD;                   // v0.1.2.1+
    end;

const

    PSM_GETBOLDFONT                 = ($0400 + 102);

{$ENDIF}
