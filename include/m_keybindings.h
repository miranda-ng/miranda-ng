/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project, 
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
*/

#define KB_CTRL_FLAG  0x10000
#define KB_SHIFT_FLAG 0x20000
#define KB_ALT_FLAG   0x40000
#define KBDF_UNICODE 1

typedef struct {
  int cbSize;				//size of the structure
  union {
    char *pszSection;		// section name used to display key bindings in the tree view
    TCHAR *ptszSection;		// [TRANSLATED-BY-CORE]
    WCHAR *pwszSection;
  };
  union {
    char *pszActionName;	// action name used to display key bindings in the tree view
    TCHAR *ptszActionName;	// [TRANSLATED-BY-CORE]
    WCHAR *pwszActionName;
  };
  char *pszActionGroup;		// action group name used to group unique shortcuts, shortcuts cannot be duplicated within a group
  DWORD key[5];				// virtual key + KB_* flags, up to 5 different shortcuts may be defined for each action
  DWORD flags;				// flags (KBDF_*)
  int	action;				// id of the action
} KEYBINDINGDESC;

//Registers action with default key bindings assigned to it.
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (KEYBINDINGDESC*)
//return: 0 on success, error code otherwise
#define MS_KEYBINDINGS_REGISTER  "KeyBindings/Register"
//Gets action assigned to the given key
//key[0] and pszActionGroup in KEYBINDINGDESC should be set before calling this service
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (KEYBINDINGDESC*)
//return: 0 if action assigned to the given key was found, 1 otherwise
#define MS_KEYBINDINGS_GET  "KeyBindings/Get"
