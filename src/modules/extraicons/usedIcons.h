/*

Copyright (C) 2009 Ricardo Pescuma Domenecci
Copyright (C) 2012-15 Miranda NG project

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

#ifndef __USEDICONS_H__
#define __USEDICONS_H__

HANDLE GetIcon(LPCSTR icolibName);

HANDLE AddIcon(LPCSTR icolibName);
HANDLE AddIcon(HANDLE hIcolib);

void RemoveIcon(LPCSTR icolibName);
void ResetIcons();


#endif // __USEDICONS_H__
