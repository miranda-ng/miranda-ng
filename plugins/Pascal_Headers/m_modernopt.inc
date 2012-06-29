{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2007 Artem Shpynov
Copyright 2000-2007 Miranda ICQ/IM project,

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

{$IFNDEF M_MODERNOPT}
{$DEFINE M_MODERNOPT}

{
	260x210 - subsection
	368x210 - section
}
const
  MS_MODERNOPT_SHOW      :PAnsiChar = 'ModernOpt/Show';

  ME_MODERNOPT_INITIALIZE:PAnsiChar = 'ModernOpt/Initialize';

  MODEROPT_FLG_UNICODE  = $01;
  MODEROPT_FLG_NORESIZE = $02;

type
  moType = (
    MODERNOPT_TYPE_SECTIONPAGE,
    MODERNOPT_TYPE_SUBSECTIONPAGE,
    MODERNOPT_TYPE_IGNOREOBJECT,
    MODERNOPT_TYPE_SELECTORPAGE,
    MODERNOPT_TYPE_COUNT);

type
  moPage = (
    MODERNOPT_PAGE_HOME,
    MODERNOPT_PAGE_ACCOUNTS,
    MODERNOPT_PAGE_SKINS,
    MODERNOPT_PAGE_CLIST,
    MODERNOPT_PAGE_MSGS,
    MODERNOPT_PAGE_IGNORE,
    MODERNOPT_PAGE_STATUS,
    MODERNOPT_PAGE_ADVANCED,
    MODERNOPT_PAGE_MODULES,
    MODERNOPT_PAGE_COUNT);

const
  MODERNOPT_CTRL_LAST = 0;

type
  TMODERNOPTOBJECT = record
    cbSize            :int;
    dwFlags           :dword;
    iType             :int;
    iSection          :int;

    hIcon             :HICON;
    szSubsection      :TChar;

    // Generic dialog page
    hInstance         :HINST;
    lpzTemplate       :PAnsiChar;
    pfnDlgProc        :pointer;
    iBoldControls     :pint;

    // General page support
    lpzClassicGroup   :PAnsiChar;
    lpzClassicPage    :PAnsiChar;
    lpzClassicTab     :PAnsiChar;
    lpzHelpUrl        :PAnsiChar;

    // Ingore page
    lpzIgnoreModule   :PAnsiChar;
    lpzIgnoreSetting  :PAnsiChar;
    dwIgnoreBit       :dword;

    // Selector type page
    lpzThemeExtension :PAnsiChar;
    lpzThemeModuleName:PAnsiChar;
  end;

const
  MS_MODERNOPT_ADDOBJECT :PAnsiChar = 'ModernOpt/AddObject';
  MS_MODERNOPT_SELECTPAGE:PAnsiChar = 'ModernOpt/SelectPage';
  ME_MODERNINGORE_RELOAD :PAnsiChar = 'ModernOpt/Ignore/Reload';

  TS_SKIN_ACTIVE          = '/Active';

  TS_SKIN_PREVIEW         = '/Preview';

  TS_SKIN_APPLY           = '/Apply';

// used by core options module to switch back to modernopt module
  MS_MODERNOPT_RESTORE:PAnsiChar = 'ModernOpt/Restore';

{$ENDIF}
