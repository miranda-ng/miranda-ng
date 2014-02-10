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
	TCHAR popup_changed[1024];
	BOOL popup_show_removed;
	TCHAR popup_removed[1024];
	WORD popup_delay_type;
	WORD popup_timeout;
	BYTE popup_use_win_colors;
	BYTE popup_use_default_colors;
	COLORREF popup_bkg_color;
	COLORREF popup_text_color;
	WORD popup_left_click_action;
	WORD popup_right_click_action;
};

// Initializations needed by popups
void InitPopups();

// Deinitializations needed by popups
void DeInitPopups();


#define POPUP_TYPE_NORMAL	0
#define POPUP_TYPE_TEST		1
#define POPUP_TYPE_ERROR	2

// Show an popup
void ShowPopup(MCONTACT hContact, const TCHAR *title, const TCHAR *description);
void ShowDebugPopup(MCONTACT hContact, const TCHAR *title, const TCHAR *description);

// Show an test
void ShowTestPopup(MCONTACT hContact,const TCHAR *title, const TCHAR *description, const Options *op);

// Show an error popup
void ShowErrPopup(const char *description, const char *title = NULL);

void ShowPopupEx(MCONTACT hContact, const TCHAR *title, const TCHAR *description, 
			   void *plugin_data, int type, const Options *op);



#endif // __POPUP_H__
