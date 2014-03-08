{
Copyright (C) 2006 Ricardo Pescuma Domenecci, Nightwish

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

{$IFNDEF M_ACC}
{$DEFINE M_ACC}

const
  AVATAR_CONTROL_CLASS = 'MAvatarControlClass'; // Class of the control

{
 Sets the contact that this avatar represents. This invalidates
 a previous call of AVATAR_SETPROTCOL.
 wParam = not used
 lParam = (HANDLE) hContact
 Usage: SendMessage(hwnd, AVATAR_SETCONTACT, 0, (LPARAM) hContact);
 Only works on AVATAR_CONTROL_CLASS controls
}
  AVATAR_SETCONTACT = WM_USER+1;

{
 Sets the user protocol that this avatar represents. This invalidates
 a previous call of AVATAR_SETCONTACT.
 wParam = not used
 lParam = (AnsiChar *) protcol name
 Usage: SendMessage(hwnd, AVATAR_SETPROTOCOL, 0, (LPARAM) "MSN");
 Only works on AVATAR_CONTROL_CLASS controls
}
  AVATAR_SETPROTOCOL = WM_USER+2;

{
 Sets the background color of thr control
 Default: transparent
 wParam = not used
 lParam = (COLORREF) color or -1 to transparent
 Usage: SendMessage(hwnd, AVATAR_SETBKGCOLOR, 0, (LPARAM) RGB(0,0,0));
 Only works on AVATAR_CONTROL_CLASS controls
}
  AVATAR_SETBKGCOLOR = WM_USER+3;

{
 Sets the control border color
 Default: no border
 wParam = not used
 lParam = (COLORREF) color or -1 to no border
 Usage: SendMessage(hwnd, AVATAR_SETBKGCOLOR, 0, (LPARAM) RGB(0,0,0));
 Only works on AVATAR_CONTROL_CLASS controls
}
  AVATAR_SETBORDERCOLOR = WM_USER+4;

{
 Sets the avatar border color
 Default: no border
 wParam = not used
 lParam = (COLORREF) color or -1 to no border
 Usage: SendMessage(hwnd, AVATAR_SETBKGCOLOR, 0, (LPARAM) RGB(0,0,0));
 Only works on AVATAR_CONTROL_CLASS controls
}
  AVATAR_SETAVATARBORDERCOLOR = WM_USER+5;

{
 Sets the radius of the round corner of the avatar
 Default: 0
 wParam = not used
 lParam = (int) radius or 0 to not use round corners
 Usage: SendMessage(hwnd, AVATAR_SETAVATARROUNDCORNERRADIUS, 0, 4);
 Only works on AVATAR_CONTROL_CLASS controls
}
  AVATAR_SETAVATARROUNDCORNERRADIUS = WM_USER+6;

{
 Sets the text to be shown if no avatar is set. The font can be set using WM_SETFONT.
 Default: ""
 wParam = not used
 lParam = (AnsiChar *) text (it will be translated) - max 128 chars
 Usage: SendMessage(hwnd, AVATAR_SETNOAVATARTEXT, 0, (LPARAM) "Contact has no avatar");
 Only works on AVATAR_CONTROL_CLASS controls
}
  AVATAR_SETNOAVATARTEXT = WM_USER+7;

{
 Sets if the hidden setting must be respected and hidden avatars shouldn't be draw
 Default: TRUE
 wParam = not used
 lParam = (bool) respect?
 Usage: SendMessage(hwnd, AVATAR_RESPECTHIDDEN, 0, (LPARAM) FALSE);
 Only works on AVATAR_CONTROL_CLASS controls
}
  AVATAR_RESPECTHIDDEN = WM_USER+8;

{
 Get the space inside the control that is really beeing used to display the avatar (the rest
 is filled with background color). Set both to 0 if no avatar exists.
 wParam = (int *) width
 lParam = (int *) height
 Usage: SendMessage(hwnd, AVATAR_GETUSEDSPACE, (WPARAM) &width, (LPARAM) &height);
 Only works on AVATAR_CONTROL_CLASS controls
}
  AVATAR_GETUSEDSPACE = WM_USER+9;

{
 Sets if the avatar will be resized when its smaller then the control size
 Default: TRUE
 wParam = not used
 lParam = (bool) TRUE or FALSE
 Usage: SendMessage(hwnd, AVATAR_SETRESIZEIFSMALLER, 0, (LPARAM) FALSE);
 Only works on AVATAR_CONTROL_CLASS controls
}
  AVATAR_SETRESIZEIFSMALLER = WM_USER+10;

{
 tell acc to paint avatar on aero surface (must draw with alpha channel, not using BitBlt()
 wParam = not used
 lParam = (bool) TRUE -> enable, FALSE -> disable
}
  AVATAR_SETAEROCOMPATDRAWING = WM_USER+11;

// Set to the parent throught WM_NOTIFY to notify when the avatar shown has changed
  NM_AVATAR_CHANGED = -200;

{$ENDIF}
