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
// Added in 0.3.0.0+

#ifndef M_BUTTON_H__
#define M_BUTTON_H__ 1

#define MIRANDABUTTONCLASS	_T( "MButtonClass" ) // Class of the control


// Sets whether a dropdown arrow is used
// wParam = TRUE/FALSE turns arrow on or off
// lParam = not used
// Usage: SendMessage(hwndbutton,BUTTONSETARROW,1,0);
// Only works on MButtonClass buttons
#define BUTTONSETARROW       (WM_USER+1)

// Sets whether the button is a default button
// wParam = TRUE/FALSE default on/off
// lParam = not used
// Usage: SendMessage(hwndbutton,BUTTONSETDEFAULT,1,0);
// Only works on MButtonClass buttons
#define BUTTONSETDEFAULT     (WM_USER+2)

// Sets the button as a push button
// wParam = lParam = not used
// Usage: SendMessage(hwndbutton,BUTTONSETASPUSHBTN,0,0);
// Only works on MButtonClass buttons
#define BUTTONSETASPUSHBTN   (WM_USER+3)

// Sets the button type as a flat button without borders v0.3.3+
// Usage: SendMessage(hwndbutton,BUTTONSETASFLATBTN,0,0);
// Only works on MButtonClass buttons
#define BUTTONSETASFLATBTN   (WM_USER+4)

// Sets a tooltip for the button v0.3.3+
// wParam = (WPARAM)(char *)szTip
// lParam = not used
// Usage: SendMessage(hwndButton,BUTTONADDTOOLTIP,(WPARAM)"My Tip",BATF_* flags);
#define BATF_UNICODE 1
#if defined( _UNICODE )
	#define BATF_TCHAR BATF_UNICODE
#else
	#define BATF_TCHAR 0
#endif

#define BUTTONADDTOOLTIP     (WM_USER+5)

#endif // M_BUTTON_H__

