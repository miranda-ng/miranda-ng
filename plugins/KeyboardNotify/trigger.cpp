/*

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

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include "constants.h"
#include "utils.h"
#include "resource.h"
#include <newpluginapi.h>
#include <m_database.h>
#include <m_langpack.h>
#include "m_trigger.h"

extern HINSTANCE hInst;

void StartBlinkAction(char *, WORD);


int getCustomNro(DWORD actionID, HWND hwndDlg, int nItems)
{
	int i;
	DBVARIANT dbv;
	char theme[MAX_PATH+1];

	if (DBGetTriggerSetting(actionID, NULL, KEYBDMODULE, "custom", &dbv))
		return DEF_SETTING_CUSTOMTHEME;

	for (i=0; i < nItems; i++) {
		SendDlgItemMessage(hwndDlg, IDC_CUSTOMTHEME, CB_GETLBTEXT, (WPARAM)i, (LPARAM)theme);
		if (!strcmp(dbv.pszVal, theme)) {
			DBFreeVariant(&dbv);
			return i;
		}
	}

	DBFreeVariant(&dbv);
	return DEF_SETTING_CUSTOMTHEME;
}

static BOOL CALLBACK DlgProcOptsActionKbdNotify(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;
	DWORD actionID;
	int i, nItems;
	char theme[MAX_PATH+1];

	switch (msg) {
		case WM_INITDIALOG:
			actionID = (DWORD)lParam;
			TranslateDialogDefault(hwndDlg);

			for (i=0; !DBGetContactSetting(NULL, KEYBDMODULE, fmtDBSettingName("theme%d", i), &dbv); i++) {
				int index = SendDlgItemMessage(hwndDlg, IDC_CUSTOMTHEME, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)dbv.pszVal);
				DBFreeVariant(&dbv);
				if (index != CB_ERR && index != CB_ERRSPACE)
					SendDlgItemMessage(hwndDlg, IDC_CUSTOMTHEME, CB_SETITEMDATA, (WPARAM)index, (LPARAM)i);
			}

			nItems = SendDlgItemMessage(hwndDlg, IDC_CUSTOMTHEME, CB_GETCOUNT, 0, 0);

			CheckDlgButton(hwndDlg, IDC_USECUSTOM, nItems && DBGetTriggerSettingByte(actionID, NULL, KEYBDMODULE, "usecustom", 0) ? BST_CHECKED:BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_USECUSTOM), nItems);

			SendDlgItemMessage(hwndDlg, IDC_CUSTOMTHEME, CB_SETCURSEL, (WPARAM)getCustomNro(actionID, hwndDlg, nItems), 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CUSTOMTHEME), IsDlgButtonChecked(hwndDlg, IDC_USECUSTOM) == BST_CHECKED);

			SendDlgItemMessage(hwndDlg, IDC_FORCEOPEN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_SFORCEOPEN), 0);
			SendDlgItemMessage(hwndDlg, IDC_FORCEOPEN, UDM_SETRANGE32, 1, MAKELONG(UD_MAXVAL, 0));
			SendDlgItemMessage(hwndDlg, IDC_FORCEOPEN, UDM_SETPOS, 0, MAKELONG(DBGetTriggerSettingWord(actionID, NULL, KEYBDMODULE, "forceopen", DEF_SETTING_NBLINKS), 0));

			break;

		case TM_ADDACTION:	// save the settings
			actionID = (DWORD)wParam;
			DBWriteTriggerSettingByte(actionID, NULL, KEYBDMODULE, "usecustom", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_USECUSTOM) == BST_CHECKED ? 1:0));
			SendDlgItemMessage(hwndDlg, IDC_CUSTOMTHEME, CB_GETLBTEXT, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_CUSTOMTHEME, CB_GETCURSEL, 0, 0), (LPARAM)theme);
			DBWriteTriggerSettingString(actionID, NULL, KEYBDMODULE, "custom", theme);
			DBWriteTriggerSettingWord(actionID, NULL, KEYBDMODULE, "forceopen", (WORD)SendDlgItemMessage(hwndDlg, IDC_FORCEOPEN, UDM_GETPOS, 0, 0));
			break;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_USECUSTOM)
				EnableWindow(GetDlgItem(hwndDlg, IDC_CUSTOMTHEME), IsDlgButtonChecked(hwndDlg, IDC_USECUSTOM) == BST_CHECKED);
			break;

		case WM_DESTROY:
			break;
	}

	return FALSE;
}

char *getCustomString(DWORD actionID)
{
	int i;
	DBVARIANT dbv, dbv2;
	static char customString[MAX_PATH+1];
	
	if (DBGetTriggerSetting(actionID, NULL, KEYBDMODULE, "custom", &dbv))
		return NULL;

	for (i=0; !DBGetContactSetting(NULL, KEYBDMODULE, fmtDBSettingName("theme%d", i), &dbv2); i++) {
		if (!strcmp(dbv.pszVal, dbv2.pszVal)) {
			DBFreeVariant(&dbv);
			DBFreeVariant(&dbv2);
			if(DBGetContactSetting(NULL, KEYBDMODULE, fmtDBSettingName("custom%d", i), &dbv2))
				return NULL;
			strcpy(customString, dbv2.pszVal);
			DBFreeVariant(&dbv2);
			return customString;
		}
		DBFreeVariant(&dbv2);
	}
	
	DBFreeVariant(&dbv);
	return NULL;
}

static int TriggerStartBlinkFunction(DWORD actionID, REPORTINFO *ri)
{
	if (ri->flags&ACT_PERFORM) {
		char *customStr;

		if (!DBGetTriggerSettingByte(actionID, NULL, KEYBDMODULE, "usecustom", 0))
			customStr = NULL;
		else
			customStr = getCustomString(actionID);	
		StartBlinkAction(customStr, DBGetTriggerSettingWord(actionID, NULL, KEYBDMODULE, "forceopen", DEF_SETTING_NBLINKS));
	}
	if (ri->flags&ACT_CLEANUP) { // request to delete all associated settings
		RemoveAllActionSettings(actionID, KEYBDMODULE);
	}

	return 0; // all ok
}

int RegisterAction()
{
	ACTIONREGISTER ar;

	if (!ServiceExists(MS_TRIGGER_REGISTERACTION))
		return -1;

	ZeroMemory(&ar, sizeof(ar));
	ar.cbSize = sizeof(ar);
	ar.pszName = "Keyboard Flash: Start Blinking";
	ar.actionFunction = TriggerStartBlinkFunction;
	ar.hInstance = hInst;
	ar.pfnDlgProc = DlgProcOptsActionKbdNotify;
	ar.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_KN_ACTION);
	ar.flags = ARF_FUNCTION;
	return CallService(MS_TRIGGER_REGISTERACTION, 0, (LPARAM)&ar);
}

int DeInitAction()
{
	return 0;
}
