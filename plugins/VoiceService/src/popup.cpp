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

// Handle to popup events

struct PopupAction
{
	PopupAction(void *_1, int _2) :
		pCall((VoiceCall*)_1),
		iAction(_2)
	{}

	VoiceCall *pCall;
	int iAction;
};

static void CALLBACK ExecuteAction(void *param)
{
	auto *pParam = (PopupAction *)param;

	if (pParam->iAction == POPUP_ACTION_OPENWINDOW)
		ShowCallWindow(pParam->pCall);

	delete pParam;
}

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COMMAND:
		CallFunctionAsync(ExecuteAction, new PopupAction(PUGetPluginData(hWnd), opts.popup_left_click_action));

		if (opts.popup_left_click_action != POPUP_ACTION_DONOTHING)
			PUDeletePopup(hWnd);

		return TRUE;

	case WM_CONTEXTMENU:
		CallFunctionAsync(ExecuteAction, new PopupAction(PUGetPluginData(hWnd), opts.popup_right_click_action));

		if (opts.popup_right_click_action != POPUP_ACTION_DONOTHING)
			PUDeletePopup(hWnd);

		return TRUE;

	case UM_FREEPLUGINDATA:
		return TRUE;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Show an popup

static void ShowPopupEx(MCONTACT hContact, const wchar_t *title, const wchar_t *description, void *plugin_data, int type, const Options *op)
{
	// Make popup
	POPUPDATAW ppd;
	ppd.lchContact = hContact;
	ppd.lchIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(174), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

	if (title != NULL)
		wcsncpy_s(ppd.lpwzContactName, title, _TRUNCATE);
	else if (hContact != NULL)
		wcsncpy_s(ppd.lpwzContactName, Clist_GetContactDisplayName(hContact, 0), _TRUNCATE);

	if (description != NULL)
		wcsncpy_s(ppd.lpwzText, description, _TRUNCATE);

	if (type == POPUP_TYPE_NORMAL || type == POPUP_TYPE_TEST) {
		if (op->popup_use_win_colors) {
			ppd.colorBack = GetSysColor(COLOR_BTNFACE);
			ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
		}
		else if (!op->popup_use_default_colors) {
			ppd.colorBack = op->popup_bkg_color;
			ppd.colorText = op->popup_text_color;
		}
	}
	else { // if (type == POPUP_TYPE_ERROR)
		ppd.colorBack = RGB(200, 0, 0);
		ppd.colorText = RGB(255, 255, 255);
	}

	if (type == POPUP_TYPE_NORMAL) {
		ppd.PluginWindowProc = PopupDlgProc;
		ppd.PluginData = plugin_data;
	}

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

// Show normal popup
void ShowPopup(MCONTACT hContact, const wchar_t *title, const wchar_t *description, void *pUserData)
{
	ShowPopupEx(hContact, title, description, pUserData, POPUP_TYPE_NORMAL, &opts);
}

// Show error popup
void ShowErrPopup(const wchar_t *description, const wchar_t *title)
{
	ShowPopupEx(NULL, title == NULL ? _T(MODULE_NAME) _T(" Error") : title, description,
		NULL, POPUP_TYPE_ERROR, NULL);
}

// Show test popup
void ShowTestPopup(const wchar_t *title, const wchar_t *description, const Options *op)
{
	ShowPopupEx(NULL, title, description, NULL, POPUP_TYPE_TEST, op);
}
