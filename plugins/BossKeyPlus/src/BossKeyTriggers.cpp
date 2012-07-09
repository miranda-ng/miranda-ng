/*
	BossKey - Hide Miranda from your boss :)
	Copyright (C) 2002-2003 Goblineye Entertainment, (C) 2007-2010 Billy_Bons

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "BossKey.h"
extern HANDLE g_hIsHiddenService;

static INT_PTR CALLBACK DlgProcOptsBossKeyTrigger(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {
	case WM_INITDIALOG: {
		/* lParam = (LPARAM)(DWORD)triggerID, or 0 if this is a new trigger */
		DWORD triggerID;

		triggerID = (DWORD)lParam;
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_BK_HIDE, (DBGetTriggerSettingWord(triggerID, NULL, MOD_NAME, SETTING_TRIGGERON, 0)&TRIGGER_BK_HIDE)?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_BK_SHOW, (DBGetTriggerSettingWord(triggerID, NULL, MOD_NAME, SETTING_TRIGGERON, 0)&TRIGGER_BK_SHOW)?BST_CHECKED:BST_UNCHECKED);
		break;
		}

	case TM_ADDTRIGGER: {	
		DWORD triggerID;
		int flags = 0;
		
		triggerID = (DWORD)wParam;
		flags |= IsDlgButtonChecked(hwndDlg, IDC_BK_HIDE)?TRIGGER_BK_HIDE:0;
		flags |= IsDlgButtonChecked(hwndDlg, IDC_BK_SHOW)?TRIGGER_BK_SHOW:0;
		DBWriteTriggerSettingWord(triggerID, NULL, MOD_NAME, SETTING_TRIGGERON, (WORD)flags);
		break;
		}

	case TM_DELTRIGGER: {	
		DWORD triggerID;
		REMOVETRIGGERSETTINGS rts;
		
		triggerID = (DWORD)wParam;
		rts.cbSize = sizeof(REMOVETRIGGERSETTINGS);
		rts.prefix = PREFIX_TRIGGERID;
		rts.id = triggerID;
		rts.hContact = NULL;
		rts.szModule = MOD_NAME;
		CallService(MS_TRIGGER_REMOVESETTINGS, 0, (LPARAM)&rts);
		break;
		}
	}

	return FALSE;
}

int BossKeyEvent(WPARAM wParam, LPARAM lParam) {

	DWORD triggerID = 0;
	REPORTINFO ri;
	TRIGGERDATA td;
	int flags;

	do {
		triggerID = (DWORD)CallService(MS_TRIGGER_FINDNEXTTRIGGERID, triggerID, (LPARAM)TRIGGERNAME);
		if (triggerID == 0) {
			continue;
		}
		ZeroMemory(&td, sizeof(TRIGGERDATA));
		td.cbSize = sizeof(TRIGGERDATA);
		flags = DBGetTriggerSettingWord(triggerID, NULL, MOD_NAME, SETTING_TRIGGERON, 0);
		switch (wParam) {
		case 0:
			if (!(flags&TRIGGER_BK_HIDE)) {
				continue;
			}
			break;
		case 1:
			if (!(flags&TRIGGER_BK_SHOW)) {
				continue;
			}
			break;
		default:
			continue;
			break;
		}
		ZeroMemory(&ri, sizeof(REPORTINFO));
		ri.cbSize = sizeof(REPORTINFO);
		ri.flags = TRG_PERFORM;
		ri.triggerID = triggerID;
		ri.td = &td;
		CallService(MS_TRIGGER_REPORTEVENT, 0, (LPARAM)&ri);
	} while (triggerID != 0);

	return 0;
}

INT_PTR IsMirandaHidden(WPARAM wParam, LPARAM lParam) // for trigger plugin
{
	return (INT_PTR)g_bWindowHidden;
}

int RegisterTrigger() {

	g_hIsHiddenService = CreateServiceFunction(MS_BOSSKEY_ISMIRANDAHIDDEN, IsMirandaHidden);

	TRIGGERREGISTER tr;
	ZeroMemory(&tr, sizeof(TRIGGERREGISTER));
	tr.cbSize = sizeof(TRIGGERREGISTER);
	tr.dFlags = DF_TCHAR;
	tr.pszName = Translate(TRIGGERNAME);
	tr.hInstance = g_hInstance;
	tr.pfnDlgProc = DlgProcOptsBossKeyTrigger;
	tr.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_TRIGGER);

	CallService(MS_TRIGGER_REGISTERTRIGGER, 0, (LPARAM)&tr);

	CONDITIONREGISTER cr;
	ZeroMemory(&cr, sizeof(CONDITIONREGISTER));
	cr.cbSize = sizeof(CONDITIONREGISTER);
	cr.flags = CRF_TCHAR;
	cr.pszName = "BossKey: Miranda is hidden";
	cr.pszService = MS_BOSSKEY_ISMIRANDAHIDDEN;

	CallService(MS_TRIGGER_REGISTERCONDITION, 0, (LPARAM)&cr);

	ACTIONREGISTER ar;
	ZeroMemory(&ar, sizeof(ACTIONREGISTER));
	ar.cbSize = sizeof(ACTIONREGISTER);
	ar.flags = ARF_TCHAR;
	ar.pszName = "BossKey: Hide/Show Miranda";
	ar.pszService = MS_BOSSKEY_HIDE;

	CallService(MS_TRIGGER_REGISTERACTION, 0, (LPARAM)&ar);

	return 0;
}
