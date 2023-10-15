/* 
Copyright (C) 2005 Ricardo Pescuma Domenecci

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


#ifndef __POPUP_H__
# define __POPUP_H__

#define POPUP_TYPE_NORMAL	0
#define POPUP_TYPE_TEST		1
#define POPUP_TYPE_ERROR	2

// Show normal popup
void ShowPopup(MCONTACT hContact, const wchar_t *title, const wchar_t *description, void *pUserData);

// Show test popup
void ShowTestPopup(const wchar_t *title, const wchar_t *description, const Options *op);

// Show error popup
void ShowErrPopup(const wchar_t *description, const wchar_t *title = 0);

#endif // __POPUP_H__
