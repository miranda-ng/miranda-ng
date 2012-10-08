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

{$IFNDEF M_LANGPACK}
{$DEFINE M_LANGPACK}

const

    {
        wParam : 0
        lParam : pointer to a null terminated string
        Affects: Returns a pointer to a localised string, if there is no known
                 translation it will return lParam, the return value does *not*
                 have to be freed in anyway (if successful) -- see notes
        Returns: a pointer to a null terminated string
        Notes  : No check is done to see if Miranda has the required version
        Version: v0.1.1.0+
    }
    MS_LANGPACK_TRANSLATESTRING         = 'LangPack/TranslateString';

    {
        wParam : 0
        lParam : Pointer to a LANGPACKTRANSLATEDIALOG initialised structure, see notes
        Affects: Translates a dialog into the user's local language
        Returns: 0 on successs [non zero] on failure
        Notes  : this service only knows about the following window classes/elements:
                 Window titles, STATIC, EDIT, Hyperlink, BUTTON.
        Version: v0.1.1.0+
    }

type

    PLANGPACKTRANSLATEDIALOG = ^TLANGPACKTRANSLATEDIALOG;
    TLANGPACKTRANSLATEDIALOG = record
        cbSize: int;
        flags: DWORD;
        hwndDlg: THandle;
        ignoreControls: ^Integer;       // pointer to an array of integers? mebbe?
    end;

const

    { translate all edit controls, by default non-read-only edit controls are not }
    LPTDF_NOIGNOREEDIT = 1;
    { don't translate the title of the dialog }
    LPTDF_NOTITLE      = 2;

    MS_LANGPACK_TRANSLATEDIALOG         = 'LangPack/TranslateDialog';

    {
        wParam : HMENU handle (WinAPI handle to a menu)
        lParam : 0
        Affects: Translates a menu into the user's local language -- see notes
        Returns: 0 on success [non zero] on failure
        Notes  : This doesn't work with owner draw menus that store their
                 captions in a structure known by the owner -- something to be aware of ;)
        version: v0.1.1.0+
    }
    MS_LANGPACK_TRANSLATEMENU           = 'LangPack/TranslateMenu';

{$ENDIF}
