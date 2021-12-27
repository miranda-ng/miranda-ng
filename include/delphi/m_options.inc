{
  Miranda IM: the free IM client for Microsoft  Windows

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
{$IFNDEF M_OPTIONS}
{$DEFINE M_OPTIONS}

const
  { Opt/Initialise
    The user opened the options dialog. Modules should do whatever initialisation
    they need and call opt/addpage one or more times if they want pages displayed
    in the options dialog
    wParam=addInfo
    lParam=0
    addInfo should be passed straight to the wParam of opt/addpage
  }
  ME_OPT_INITIALISE:PAnsiChar = 'Opt/Initialise';

  { Opt/AddPage
    Must only be called during an opt/initialise hook
    Adds a page to the options dialog
    wParam=addInfo
    lParam=(LPARAM)(OPTIONSDIALOGPAGE )odp
    addInfo must have come straight from the wParam of opt/initialise
    Pages in the options dialog operate just like pages in property sheets. See the
    Microsoft documentation for details on how they operate.
    Strings in the structure can be released as soon as the service returns, but
    icons must be kept around. This is not a problem if you're loading them from a
    resource.
    Prior to v0.1.2.1 the options dialog would resize to fit the largest page, but
    since then it is fixed in size. The largest page that fits neatly is 314x240
    DLUs.
  }

const
{
  show the User Details dialog box
  wParam=(WPARAM)(HANDLE)hContact
  lParam=0
}
  MS_USERINFO_SHOWDIALOG:PAnsiChar = 'UserInfo/ShowDialog';

type
  POPTIONSDIALOGPAGE = ^TOPTIONSDIALOGPAGE;
  TOPTIONSDIALOGPAGE = record
    position              :int;     // a position number, lower numbers are topmost
    szTitle               :TChar;   // [TRANSLATED-BY-CORE]
    szGroup               :TChar;   // [TRANSLATED-BY-CORE]
    szTab                 :TChar;   // [TRANSLATED-BY-CORE]
    pPlugin               :Pointer;
    flags                 :dword;
    pfnDlgProc            :pointer;
    pszTemplate           :PAnsiChar;
    dwInitParam           :LPARAM;  // a value to pass to lParam of WM_INITDIALOG message
  end;

const
  ODPF_BOLDGROUPS    =  4; // give group box titles a bold font
  ODPF_UNICODE       =  8; // string fields in OPTIONSDIALOGPAGE are WCHAR*
  ODPF_USERINFOTAB   = 16; // options page is tabbed
  ODPF_DONTTRANSLATE = 32; // do not translate option page title

const
  PSM_GETBOLDFONT   = WM_USER+102;   //returns HFONT used for group box titles

function _Options_AddPage(param:WPARAM; opts:POPTIONSDIALOGPAGE; lang:Pointer) : int; stdcall; external AppDll name 'Options_AddPage';

function _Options_Open(group,page,tab:PWideChar; lang:Pointer):int_ptr; stdcall; external AppDll name 'Options_Open';
function _Options_OpenPage(group,page,tab:PWideChar; lang:Pointer):HWND; stdcall; external AppDll name 'Options_OpenPage';

{$ENDIF}
