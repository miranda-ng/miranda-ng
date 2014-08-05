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

#include "common.h"

aPopups PopupsList[POPUPS];

void InitPopupList()
{
	int index = 0;
	PopupsList[index].ID = index;
	PopupsList[index].colorBack = db_get_dw(NULL, MODNAME, "Popups0Bg", COLOR_BG_FIRSTDEFAULT);
	PopupsList[index].colorText = db_get_dw(NULL, MODNAME, "Popups0Tx", COLOR_TX_DEFAULT);

	index = 1;
	PopupsList[index].ID = index;
	PopupsList[index].colorBack = db_get_dw(NULL, MODNAME, "Popups1Bg", COLOR_BG_SECONDDEFAULT);
	PopupsList[index].colorText = db_get_dw(NULL, MODNAME, "Popups1Tx", COLOR_TX_DEFAULT);

	index = 2;
	PopupsList[index].ID = index;
	PopupsList[index].colorBack = db_get_dw(NULL, MODNAME, "Popups2Bg", COLOR_BG_FIRSTDEFAULT);
	PopupsList[index].colorText = db_get_dw(NULL, MODNAME, "Popups2Tx", COLOR_TX_DEFAULT);
}

void PopupAction(HWND hPopup, BYTE action)
{
	switch (action) {
	case PCA_CLOSEPOPUP:
		break;
	case PCA_DONOTHING:
		return;
	}
	PUDeletePopup(hPopup);
}

static LRESULT CALLBACK PopupDlgProc(HWND hPopup, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_COMMAND:
		PopupAction(hPopup, PopupOptions.LeftClickAction);
		break;

	case WM_CONTEXTMENU:
		PopupAction(hPopup, PopupOptions.RightClickAction);
		break;

	case UM_FREEPLUGINDATA:
		break;
	}

	return DefWindowProc(hPopup, uMsg, wParam, lParam);
}

static void _stdcall RestartPrompt(void *) {
	TCHAR tszText[200];
	mir_sntprintf(tszText, SIZEOF(tszText), _T("%s\n\n%s"), TranslateT("You need to restart your Miranda to apply installed updates."), TranslateT("Would you like to restart it now?"));

	if (MessageBox(0, tszText, TranslateT("Plugin Updater"), MB_YESNO | MB_ICONQUESTION | MB_TOPMOST) == IDYES)
		CallService(MS_SYSTEM_RESTART, db_get_b(NULL,MODNAME,"RestartCurrentProfile",1) ? 1 : 0, 0);
}

static LRESULT CALLBACK PopupDlgProcRestart(HWND hPopup, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CONTEXTMENU:
		PUDeletePopup(hPopup);
		break;
	case WM_COMMAND:
		PUDeletePopup(hPopup);
		CallFunctionAsync(RestartPrompt,0);

		break;
	}

	return DefWindowProc(hPopup, uMsg, wParam, lParam);
}

void ShowPopup(LPCTSTR ptszTitle, LPCTSTR ptszText, int Number)
{
	if(Number != POPUP_TYPE_MSG) {
		char setting[100];
		mir_snprintf(setting, SIZEOF(setting), "Popups%d", Number);
		if(!db_get_b(NULL, MODNAME, setting, DEFAULT_POPUP_ENABLED))
			return;
	}

	if (ServiceExists(MS_POPUP_ADDPOPUPT) && db_get_b(NULL, "Popup", "ModuleIsEnabled", 1)) {
		POPUPDATAT pd = { 0 };
		pd.lchContact = NULL;
		pd.lchIcon = Skin_GetIcon("check_update");
		if(Number == POPUP_TYPE_MSG) {
			pd.PluginWindowProc = PopupDlgProcRestart;
			pd.iSeconds = -1;
		}
		else {
			pd.PluginWindowProc = PopupDlgProc;
			pd.iSeconds = PopupOptions.Timeout;
		}

		lstrcpyn(pd.lptzText, ptszText, MAX_SECONDLINE);
		lstrcpyn(pd.lptzContactName, ptszTitle, MAX_CONTACTNAME);

		switch (PopupOptions.DefColors) {
		case byCOLOR_WINDOWS:
			pd.colorBack = GetSysColor(COLOR_BTNFACE);
			pd.colorText = GetSysColor(COLOR_WINDOWTEXT);
			break;
		case byCOLOR_OWN:
			pd.colorBack = PopupsList[Number].colorBack;
			pd.colorText = PopupsList[Number].colorText;
			break;
		case byCOLOR_POPUP:
			pd.colorBack = pd.colorText = 0;
			break;
		}

		PUAddPopupT(&pd);
	} else  {
		if(Number != POPUP_TYPE_MSG) {
			char setting[100];
			mir_snprintf(setting, SIZEOF(setting), "Popups%dM", Number);
			if (!db_get_b(NULL, MODNAME, setting, DEFAULT_MESSAGE_ENABLED))
				return;
		}
		int iMsgType;
		switch( Number ) {
			case POPUP_TYPE_MSG: iMsgType = MB_ICONSTOP; break;
			case POPUP_TYPE_ERROR: iMsgType = MB_ICONINFORMATION; break;
			case POPUP_TYPE_INFO: iMsgType = MB_ICONQUESTION; break;
			default: iMsgType = 0;
		}
		MessageBox(0, ptszText, ptszTitle, iMsgType);
	}
}

int ImageList_AddIconFromIconLib(HIMAGELIST hIml, const char *name)
{
	HICON icon = Skin_GetIconByHandle(Skin_GetIconHandle(name));
	int res = ImageList_AddIcon(hIml, icon);
	Skin_ReleaseIcon(icon);
	return res;
}
