/* 
Copyright (C) 2005-2009 Ricardo Pescuma Domenecci

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
*/

#include "..\commons.h"

static TCHAR *wcs[] = {
		_T("Winamp v1.x")
};

Winamp::Winamp()
{ 
	name = _T("Winamp");
	window_classes = wcs;
	num_window_classes = SIZEOF(wcs);
	message_window_class = MIRANDA_WINDOWCLASS _T(".Winamp");
	dll_name = "gen_mlt";
}
