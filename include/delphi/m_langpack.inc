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
 translates a single string into the user's local language    v0.1.1.0+
 wParam=LANG_* flags
 lParam=(LPARAM)(const AnsiChar*)szEnglish
 returns a pointer to the localised string. If there is no known translation
 it will return szEnglish. The return value does not need to be freed in any way
 Note that the Translate() macro as defined below will crash plugins that are
 loaded into Miranda 0.1.0.1 and earlier. If anyone's actually using one of
 these versions, I pity them.
}
  MS_LANGPACK_TRANSLATESTRING:PAnsiChar = 'LangPack/TranslateString';
(*
{
 translates a dialog into the user's local language    v0.1.1.0+
 wParam=0
 lParam=(LPARAM)(LANGPACKTRANSLATEDIALOG* )&lptd
 returns 0 on success, nonzero on failure
 This service only knows about the following controls:
 Window titles, STATIC, EDIT, Hyperlink, BUTTON
}
type
  IntArray = array [0..1000] of integer;
  PLANGPACKTRANSLATEDIALOG = ^TLANGPACKTRANSLATEDIALOG;
  TLANGPACKTRANSLATEDIALOG = record
    cbSize        :int;
    flags         :dword;
    hwndDlg       :HWND;
    ignoreControls:^IntArray; // zero-terminated list of control IDs *not* to translate
  end;

const
  LPTDF_NOIGNOREEDIT = 1; // translate all edit controls. By default
                          // non-read-only edit controls are not translated
  LPTDF_NOTITLE      = 2; //do not translate the title of the dialog

const
 MS_LANGPACK_TRANSLATEDIALOG:PAnsiChar = 'LangPack/TranslateDialog';
*)
{
 translates a menu into the user's local language    v0.1.1.0+
 wParam=(WPARAM)(HMENU)hMenu
 lParam=0
 returns 0 on success, nonzero on failure
}
  MS_LANGPACK_TRANSLATEMENU:PAnsiChar = 'LangPack/TranslateMenu';

{
 returns the codepage used in the language pack     v0.4.3.0+
 wParam=0
 lParam=0
 returns the codepage stated in the langpack, or CP_ACP if no langpack is present
}
  MS_LANGPACK_GETCODEPAGE:PAnsiChar = 'LangPack/GetCodePage';

{
  returns the locale id associated with the language pack    v0.4.3.0+
  wParam=0
  lParam=0
  returns the Windows locale id stated in the langpack, or LOCALE_USER_DEFAULT if no langpack is present
}
  MS_LANGPACK_GETLOCALE:PAnsiChar = 'LangPack/GetLocale';

{
  returns the strdup/wcsdup of lparam according to the langpack  v0.4.3.0+
  wParam=0
  lParam=(LPARAM)(AnsiChar*)source string
  returns a string converted from AnsiChar* to TCHAR* using the langpack codepage. 
  This string should be freed using mir_free() then
}
  MS_LANGPACK_PCHARTOTCHAR:PAnsiChar = 'LangPack/PcharToTchar';

{
  initializes the plugin-specific translation context  v0.10.0+
  wParam=pointer to the langpack handle
  lParam=PLUGININFOEX* of the caller plugin
  always returns 0
}
  MS_LANGPACK_REGISTER:PAnsiChar = 'LangPack/Register';

{
  reloads langpack
  wParam=0 (ignored)
  lParam=(LPARAM)(TCHAR*)langpack file name or NULL to reload the current one
  always returns 0
}
  MS_LANGPACK_RELOAD:PAnsiChar = 'LangPack/Reload';

  ME_LANGPACK_CHANGE:PAnsiChar = 'LangPack/Changed';

{
  retrieves the hLangpack of a plugin by its HINSTANCE
  wParam = 0 (ignored)
  lParam = (LPARAM)(HINSTANCE)plugin's base address
  returns hLangpack if found, or 0 if error occurred
}
  MS_LANGPACK_LOOKUPHANDLE:PAnsiChar = 'LangPack/LookupHandle';

{$ENDIF}
