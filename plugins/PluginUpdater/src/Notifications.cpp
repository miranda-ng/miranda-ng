/*
Copyright (C) 2010 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt. If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

aPopups PopupsList[POPUPS] =
{
	{ IDC_MSG_BOXES,     IDC_MSG_BOXES_TX,     IDC_MSG_BOXES_BG, 0, 0 },
	{ IDC_ERRORS,        IDC_ERR_TX,           IDC_ERR_BG, 0, 0 },
	{ IDC_INFO_MESSAGES, IDC_INFO_MESSAGES_TX, IDC_INFO_MESSAGES_BG, 0, 0 }
};

void InitPopupList()
{
	PopupsList[0].colorBack = g_plugin.getDword("Popups0Bg", COLOR_BG_FIRSTDEFAULT);
	PopupsList[0].colorText = g_plugin.getDword("Popups0Tx", COLOR_TX_DEFAULT);

	PopupsList[1].colorBack = g_plugin.getDword("Popups1Bg", COLOR_BG_SECONDDEFAULT);
	PopupsList[1].colorText = g_plugin.getDword("Popups1Tx", COLOR_TX_DEFAULT);

	PopupsList[2].colorBack = g_plugin.getDword("Popups2Bg", COLOR_BG_FIRSTDEFAULT);
	PopupsList[2].colorText = g_plugin.getDword("Popups2Tx", COLOR_TX_DEFAULT);
}

static void PopupAction(HWND hPopup, uint8_t action)
{
	switch (action) {
	case PCA_CLOSEPOPUP:
		PUDeletePopup(hPopup);
		break;
	}
}

static LRESULT CALLBACK PopupDlgProc(HWND hPopup, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_COMMAND:
		PopupAction(hPopup, g_plugin.PopupLeftClickAction);
		break;

	case WM_CONTEXTMENU:
		PopupAction(hPopup, g_plugin.PopupRightClickAction);
		break;

	case UM_FREEPLUGINDATA:
		break;
	}

	return DefWindowProc(hPopup, uMsg, wParam, lParam);
}

void CALLBACK RestartPrompt(void *)
{
	if (!g_plugin.bAutoRestart) {
		wchar_t wszText[200];
		mir_snwprintf(wszText, L"%s\n\n%s", TranslateT("You need to restart your Miranda to apply installed updates."), TranslateT("Would you like to restart it now?"));
		if (MessageBox(nullptr, wszText, TranslateT("Plugin Updater"), MB_YESNO | MB_ICONQUESTION | MB_TOPMOST) != IDYES)
			return;
	}

	CallService(MS_SYSTEM_RESTART, g_plugin.getBool("RestartCurrentProfile", true), 0);
}

static LRESULT CALLBACK PopupDlgProcRestart(HWND hPopup, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CONTEXTMENU:
		PUDeletePopup(hPopup);
		break;

	case WM_COMMAND:
		PUDeletePopup(hPopup);
		CallFunctionAsync(RestartPrompt, nullptr);
		break;
	}

	return DefWindowProc(hPopup, uMsg, wParam, lParam);
}

void ShowPopup(LPCTSTR pwszTitle, LPCTSTR pwszText, int Number)
{
	if (Popup_Enabled()) {
		char setting[100];
		mir_snprintf(setting, "Popups%d", Number);

		if (g_plugin.getByte(setting, DEFAULT_POPUP_ENABLED)) {
			POPUPDATAW ppd;
			ppd.lchContact = NULL;
			ppd.lchIcon = g_plugin.getIcon(IDI_MENU);

			if (Number == POPUP_TYPE_MSG) {
				ppd.PluginWindowProc = PopupDlgProcRestart;
				ppd.iSeconds = -1;
			}
			else {
				ppd.PluginWindowProc = PopupDlgProc;
				ppd.iSeconds = g_plugin.PopupTimeout;
			}

			lstrcpyn(ppd.lpwzText, pwszText, MAX_SECONDLINE);
			lstrcpyn(ppd.lpwzContactName, pwszTitle, MAX_CONTACTNAME);

			switch (g_plugin.PopupDefColors) {
			case byCOLOR_WINDOWS:
				ppd.colorBack = GetSysColor(COLOR_BTNFACE);
				ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
				break;
			case byCOLOR_OWN:
				ppd.colorBack = PopupsList[Number].colorBack;
				ppd.colorText = PopupsList[Number].colorText;
				break;
			case byCOLOR_POPUP:
				ppd.colorBack = ppd.colorText = 0;
				break;
			}
			PUAddPopupW(&ppd);
			return;
		}
	}

	if (Number == POPUP_TYPE_ERROR)
		MessageBox(nullptr, pwszText, pwszTitle, MB_ICONINFORMATION);
}
