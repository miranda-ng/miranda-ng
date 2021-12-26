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

struct Options {
	// Log
	BOOL log_per_contact_folders;
	BOOL log_keep_same_folder;
	BOOL log_store_as_hash;

	// Popup
	BOOL popup_show_changed;
	wchar_t popup_changed[1024];
	BOOL popup_show_removed;
	wchar_t popup_removed[1024];
	uint16_t popup_delay_type;
	uint16_t popup_timeout;
	uint8_t popup_use_win_colors;
	uint8_t popup_use_default_colors;
	COLORREF popup_bkg_color;
	COLORREF popup_text_color;
	uint16_t popup_left_click_action;
	uint16_t popup_right_click_action;
};

// Initializations needed by popups
void InitPopups();

// Deinitializations needed by popups
void DeInitPopups();


#define POPUP_TYPE_NORMAL	0
#define POPUP_TYPE_TEST		1
#define POPUP_TYPE_ERROR	2

// Show an popup
void ShowPopup(MCONTACT hContact, const wchar_t *title, const wchar_t *description);
void ShowDebugPopup(MCONTACT hContact, const wchar_t *title, const wchar_t *description);

// Show an test
void ShowTestPopup(MCONTACT hContact,const wchar_t *title, const wchar_t *description, const Options *op);

// Show an error popup
void ShowErrPopup(const char *description, const char *title = nullptr);

void ShowPopupEx(MCONTACT hContact, const wchar_t *title, const wchar_t *description, 
			   void *plugin_data, int type, const Options *op);



#endif // __POPUP_H__
