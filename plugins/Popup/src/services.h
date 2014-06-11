/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
			© 2004-2007 Victor Pavlychko
			© 2010 MPK
			© 2010 Merlin_de

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

#ifndef __services_h__
#define __services_h__

extern int num_classes;			//for core class api support

INT_PTR Popup_AddPopup(WPARAM, LPARAM);
INT_PTR Popup_AddPopupW(WPARAM, LPARAM);
INT_PTR Popup_AddPopup2(WPARAM, LPARAM);

INT_PTR Popup_GetContact(WPARAM, LPARAM);
INT_PTR Popup_IsSecondLineShown(WPARAM, LPARAM);

INT_PTR Popup_ChangeTextW(WPARAM, LPARAM);

INT_PTR Popup_ChangeW(WPARAM, LPARAM);
INT_PTR Popup_Change2(WPARAM, LPARAM);

INT_PTR Popup_ShowMessage(WPARAM, LPARAM);
INT_PTR Popup_ShowMessageW(WPARAM, LPARAM);

INT_PTR Popup_Query(WPARAM, LPARAM);
INT_PTR Popup_GetPluginData(WPARAM, LPARAM);
INT_PTR Popup_RegisterActions(WPARAM wParam, LPARAM lParam);
INT_PTR Popup_RegisterNotification(WPARAM wParam, LPARAM lParam);
INT_PTR Popup_UnhookEventAsync(WPARAM wParam, LPARAM lParam);
INT_PTR Popup_RegisterVfx(WPARAM wParam, LPARAM lParam);

INT_PTR Popup_RegisterPopupClass(WPARAM wParam, LPARAM lParam);
INT_PTR Popup_UnregisterPopupClass(WPARAM wParam, LPARAM lParam);
INT_PTR Popup_CreateClassPopup(WPARAM wParam, LPARAM lParam);

#endif // __services_h__
