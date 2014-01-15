{
  Miranda IM
  Copyright (C) 2002 Robert Rainwater
  
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
  { Added in 0.3.0.0+ }
{$IFNDEF M_BUTTON}
{$DEFINE M_BUTTON}

const
  MIRANDABUTTONCLASS = 'MButtonClass';

const
  {
    Sets whether a dropdown arrow is used
    wParam = TRUE/FALSE turns arrow on or off
    lParam = 0 (unused)
    Usage: SendMessage(hwndbutton,BUTTONSETARROW,1,0);
    Only works on MButtonClass buttons
  }
  BUTTONSETARROW = WM_USER + 1;

  {
    Sets whether the button is a default button
    wParam = TRUE/FALSE default on/off
    lParam = 0 (unused)
    Usage: SendMessage(hwndbutton,BUTTONSETDEFAULT,1,0);
    Only works on MButtonClass buttons
  }
  BUTTONSETDEFAULT = WM_USER + 2;

  {
    Sets the button as a push button
    wParam = 1/0 as on/off
    lParam = 0 (unused)
    Usage: SendMessage(hwndbutton,BUTTONSETASPUSHBTN,1,0);
    Only works on MButtonClass buttons
  }
  BUTTONSETASPUSHBTN = WM_USER + 3;

  {
    Sets the button type as a flat button without borders v0.3.3+
    wParam = 1/0 as on/off
    lParam = 0 (unused)
    Usage: SendMessage(hwndbutton,BUTTONSETASFLATBTN,1,0);
    Only works on MButtonClass buttons
  }
  BUTTONSETASFLATBTN = WM_USER + 4;

  {
    Sets a tooltip for the button v0.3.3+
    wParam = PAnsiChar(szTip)
    lParam = BATF_* flags
    Usage: SendMessage(hwndButton,BUTTONADDTOOLTIP,(WPARAM)"My Tip",BATF_* flags);
  }
  BATF_UNICODE = 1;

  BUTTONADDTOOLTIP = WM_USER + 5;

  {
    Sets the button type as a themed button
    wParam = TRUE/FALSE default on/off
    lParam = 0 (unused)
    Usage: SendMessage(hwndbutton, BUTTONSETASTHEMEDBTN, 1, 0);
    Only works on MButtonClass buttons
  }
  BUTTONSETASTHEMEDBTN = WM_USER + 6;

{
    Sets the custom painting procedure for a button
    wParam = new data block length (inherited from MButtonCtrl)
    lParam = (pfnPainterFunc)pfnPainter (refer to mbutton_int.h for details)
    Usage: SendMessage(hwndbutton, BUTTONSETCUSTOM, 0, (LPARAM)MyPainter);
    Only works on MButtonClass buttons
}
  BUTTONSETCUSTOMPAINT = WM_USER + 7;

{
    Forces a button to send clicks on push
    wParam = TRUE/FALSE default on/off
    lParam = 0 (unused)
    Usage: SendMessage(hwndbutton, BUTTONSETSENDONDOWN, TRUE, 0);
}
  BUTTONSETSENDONDOWN = WM_USER + 8;

{$ENDIF}
