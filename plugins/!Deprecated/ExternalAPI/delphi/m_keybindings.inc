{
Miranda IM: the free IM client for Microsoft* Windows*

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

{$IFNDEF M_KEYBINDINGS}

const
  KB_CTRL_FLAG  = $10000;
  KB_SHIFT_FLAG = $20000;
  KB_ALT_FLAG   = $40000;

  KBDF_UNICODE = 1;

type
  TKEYBINDINGDESC = record
    cbSize        :int;   //size of the structure
    szSection     :TChar; // section name used to display key bindings in the tree view [TRANSLATED-BY-CORE]
    pszActionName :TChar; // action  name used to display key bindings in the tree view [TRANSLATED-BY-CORE]
    pszActionGroup:PAnsiChar; // action group name used to group unique shortcuts, shortcuts cannot be duplicated within a group
    key           :array [0..4] of dword; // virtual key + KB_* flags, up to 5 different shortcuts may be defined for each action
    flags         :dword; // flags (KBDF_*)
    action        :int;   // id of the action
  end;

{Registers action with default key bindings assigned to it.
  wParam = (WPARAM) 0; not used
  lParam = (LPARAM) (KEYBINDINGDESC*)
  return: 0 on success, error code otherwise
}
  MS_KEYBINDINGS_REGISTER :PAnsiChar = 'KeyBindings/Register';

{Gets action assigned to the given key
  key[0] and pszActionGroup in KEYBINDINGDESC should be set before calling this service
  wParam = (WPARAM) 0; not used
  lParam = (LPARAM) (KEYBINDINGDESC*)
  return: 0 if action assigned to the given key was found, 1 otherwise
}
  MS_KEYBINDINGS_GET :PAnsiChar = 'KeyBindings/Get';

{$ENDIF}
