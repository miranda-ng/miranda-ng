{
Copyright (C) 2006 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
}

{$IFNDEF M_SPELLCHECKER}
{$DEFINE M_SPELLCHECKER}

type
  SPELLCHECKER_ITEM = record
    cbSize     :int;
    hContact   :TMCONTACT; // The contact to get the settings from, or NULL
    hwnd       :HWND;      // The hwnd of the richedit
    window_name:PAnsiChar; // A name for this richedit
  end;

type
  SPELLCHECKER_POPUPMENU = record
    cbSize   :int;
    hwnd     :HWND;   // The hwnd of the richedit
    hMenu    :HMENU;  // The handle to the menu
    pt       :TPOINT; // The point, in screen coords
    hwndOwner:HWND;   // The hwnd of owner of the popup menu. If it is null, hwnd is used
  end;

const
{
Adds a richedit control for the spell checker to check

wParam: SPELLCHECKER_ITEM *
lParam: ignored
return: 0 on success
}
  MS_SPELLCHECKER_ADD_RICHEDIT:PAnsiChar = 'SpellChecker/AddRichedit';

{
Removes a richedit control for the spell checker to check

wParam: HWND
lParam: ignored
return: 0 on success
}
  MS_SPELLCHECKER_REMOVE_RICHEDIT:PAnsiChar = 'SpellChecker/RemoveRichedit';

{
Show context menu

wParam: SPELLCHECKER_POPUPMENU 
lParam: ignored
return: the control id selected by the user, 0 if no one was selected, < 0 on error
}
  MS_SPELLCHECKER_SHOW_POPUP_MENU:PAnsiChar = 'SpellChecker/ShowPopupMenu';

{$ENDIF}
