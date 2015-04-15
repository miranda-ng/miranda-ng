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

type
  POPTIONSDIALOGPAGE = ^TOPTIONSDIALOGPAGE;
  TOPTIONSDIALOGPAGE = record
    position              :int;     // a position number, lower numbers are topmost
    szTitle               :TChar;   // [TRANSLATED-BY-CORE]
    pfnDlgProc            :pointer;
    pszTemplate           :PAnsiChar;
    hInstance             :THANDLE;
    hIcon                 :HICON;   // v0.1.0.1+
    szGroup               :TChar;   // v0.1.0.1+ [TRANSLATED-BY-CORE]
    groupPosition         :int;     // v0.1.0.1+
    hGroupIcon            :HICON;   // v0.1.0.1+
    flags                 :dword;   // v0.1.2.1+
    szTab                 :TChar;   // v0.6.0.0+ [TRANSLATED-BY-CORE]
    dwInitParam           :LPARAM;  // v0.8.0.0+ a value to pass to lParam of WM_INITDIALOG message
    hLangpack             :int;
  end;

const
  ODPF_BOLDGROUPS    =  4; // give group box titles a bold font
  ODPF_UNICODE       =  8; // string fields in OPTIONSDIALOGPAGE are WCHAR*
  ODPF_USERINFOTAB   = 16; // options page is tabbed
  ODPF_DONTTRANSLATE = 32; // do not translate option page title

const
  PSM_GETBOLDFONT   = WM_USER+102;   //returns HFONT used for group box titles

  MS_OPT_ADDPAGE:PAnsiChar = 'Opt/AddPage';

  { Opens the options dialog, optionally at the specified page    v0.1.2.1+
    wParam=0
    lParam=(LPARAM)(OPENOPTIONSDIALOG*)&ood;
    Returns 0 on success, nonzero on failure
    The behaviour if the options dialog is already open is that it will just be
    activated, the page won't be changed. This may change in the future. }
type
  POPENOPTIONSDIALOG = ^TOPENOPTIONSDIALOG;
  TOPENOPTIONSDIALOG = record
    cbSize  :int;
    pszGroup:PAnsiChar;  // set to NULL if it's a root item
    pszPage :PAnsiChar;  // set to NULL to just open the options at no specific page
    pszTab  :PAnsiChar;  // set to NULL to just open the options at no specific tab
  end;

const
  MS_OPT_OPENOPTIONS:PAnsiChar = 'Opt/OpenOptions';

//Opens the options dialog, with only specified page    v0.8.0.x+
  MS_OPT_OPENOPTIONSPAGE = 'Opt/OpenOptionsPage';

{$ENDIF}
