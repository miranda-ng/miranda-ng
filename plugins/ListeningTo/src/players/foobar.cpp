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
		_T("{DA7CD0DE-1602-45e6-89A1-C2CA151E008E}/1"),	// Foobar 0.9.1
		_T("{DA7CD0DE-1602-45e6-89A1-C2CA151E008E}"),
		_T("{97E27FAA-C0B3-4b8e-A693-ED7881E99FC1}"),	// Foobar 0.9.5.3
		_T("{E7076D1C-A7BF-4f39-B771-BCBE88F2A2A8}"),	// Foobar Columns UI
};

Foobar::Foobar()
{ 
	name = _T("foobar2000");
	window_classes = wcs;
	num_window_classes = SIZEOF(wcs);
}
