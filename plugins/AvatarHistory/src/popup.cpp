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

#include "stdafx.h"

struct PopupDataType
{
	PopupDataType(void *_1, HICON _2) :
		plugin_data(_1),
		hIcon(_2)
	{}

	~PopupDataType()
	{
		DestroyIcon(hIcon);
	}

	void *plugin_data;
	HICON hIcon;
};

static void DoAction(WPARAM wParam, LPARAM lParam)
{
	if (lParam == POPUP_ACTION_OPENAVATARHISTORY) {
		CallServiceSync("AvatarHistory/ShowDialog", wParam, 0);
	}
	else if (lParam == POPUP_ACTION_OPENHISTORY) {
		CallServiceSync(MS_HISTORY_SHOWCONTACTHISTORY, wParam, 0);
	}
}

// Handle to popup events
static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PopupDataType *popup = (PopupDataType *)PUGetPluginData(hWnd);

	switch (message) {
	case WM_COMMAND:
		DoAction(WPARAM(popup->plugin_data), opts.popup_left_click_action);

		if (opts.popup_left_click_action != POPUP_ACTION_DONOTHING)
			PUDeletePopup(hWnd);

		return TRUE;

	case WM_CONTEXTMENU:
		DoAction(WPARAM(popup->plugin_data), opts.popup_right_click_action);

		if (opts.popup_right_click_action != POPUP_ACTION_DONOTHING)
			PUDeletePopup(hWnd);

		return TRUE;

	case UM_FREEPLUGINDATA:
		delete popup;
		return FALSE; //the return value is ignored
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

// Handle to popup events
static LRESULT CALLBACK DumbPopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COMMAND:
		PUDeletePopup(hWnd);
		return TRUE;

	case WM_CONTEXTMENU:
		PUDeletePopup(hWnd);
		return TRUE;

	case UM_FREEPLUGINDATA:
		delete (PopupDataType *)PUGetPluginData(hWnd);
		return FALSE; //the return value is ignored
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void ShowTestPopup(MCONTACT hContact, const wchar_t *title, const wchar_t *description, const Options *op)
{
	ShowPopupEx(hContact, title, description, nullptr, POPUP_TYPE_TEST, op);
}

void ShowPopup(MCONTACT hContact, const wchar_t *title, const wchar_t *description)
{
	ShowPopupEx(hContact, title, description, (void*)hContact, POPUP_TYPE_NORMAL, &opts);
}

void ShowDebugPopup(MCONTACT hContact, const wchar_t *title, const wchar_t *description)
{
	if (g_plugin.getByte("Debug", 0))
		ShowPopup(hContact, title, description);
}

// Show an popup
void ShowPopupEx(MCONTACT hContact, const wchar_t *title, const wchar_t *description,
	void *plugin_data, int type, const Options *op)
{
	// Make popup
	POPUPDATAW ppd = {};
	ppd.lchContact = hContact;
	ppd.lchIcon = createProtoOverlayedIcon(hContact);
	ppd.PluginData = new PopupDataType(plugin_data, ppd.lchIcon);

	if (title != nullptr)
		mir_wstrncpy(ppd.lpwzContactName, title, _countof(ppd.lpwzContactName));
	else if (hContact != NULL)
		mir_wstrncpy(ppd.lpwzContactName, Clist_GetContactDisplayName(hContact), _countof(ppd.lpwzContactName));

	if (description != nullptr)
		mir_wstrncpy(ppd.lpwzText, description, _countof(ppd.lpwzText));

	if (type == POPUP_TYPE_NORMAL || type == POPUP_TYPE_TEST) {
		if (op->popup_use_default_colors) {
			ppd.colorBack = 0;
			ppd.colorText = 0;
		}
		else if (op->popup_use_win_colors) {
			ppd.colorBack = GetSysColor(COLOR_BTNFACE);
			ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
		}
		else {
			ppd.colorBack = op->popup_bkg_color;
			ppd.colorText = op->popup_text_color;
		}
	}
	else { // if (type == POPUP_TYPE_ERROR)
		ppd.colorBack = RGB(200, 0, 0);
		ppd.colorText = RGB(255, 255, 255);
	}

	if (type == POPUP_TYPE_NORMAL)
		ppd.PluginWindowProc = PopupDlgProc;
	else // if (type == POPUP_TYPE_TEST || type == POPUP_TYPE_ERROR)
		ppd.PluginWindowProc = DumbPopupDlgProc;

	if (type == POPUP_TYPE_NORMAL || type == POPUP_TYPE_TEST) {
		switch (op->popup_delay_type) {
		case POPUP_DELAY_CUSTOM:
			ppd.iSeconds = opts.popup_timeout;
			break;

		case POPUP_DELAY_PERMANENT:
			ppd.iSeconds = -1;
			break;
		}
	}

	// Now that every field has been filled, we want to see the popup.
	PUAddPopupW(&ppd);
}
