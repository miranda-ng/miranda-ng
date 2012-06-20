/*
    KeepStatus Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

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

#include "../commonstatus.h"
#include "keepstatus.h"
#include "../resource.h"
#include <m_trigger.h>

extern HINSTANCE hInst;
static HANDLE hKeepStatusHook;

static INT_PTR CALLBACK DlgProcOptsTriggerKeepStatus(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			/* lParam = (LPARAM)(DWORD)triggerID, or 0 if this is a new trigger */
			DWORD triggerID = (DWORD)lParam;
			CheckDlgButton(hwndDlg, IDC_CONNLOST, (DBGetTriggerSettingWord(triggerID, NULL, MODULENAME, SETTING_TRIGGERON, 0)&TRIGGER_CONNLOST)?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_LOGINERROR, (DBGetTriggerSettingWord(triggerID, NULL, MODULENAME, SETTING_TRIGGERON, 0)&TRIGGER_LOGINERROR)?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_OTHERLOC, (DBGetTriggerSettingWord(triggerID, NULL, MODULENAME, SETTING_TRIGGERON, 0)&TRIGGER_OTHERLOC)?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CONNRETRY, (DBGetTriggerSettingWord(triggerID, NULL, MODULENAME, SETTING_TRIGGERON, 0)&TRIGGER_CONNRETRY)?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CONNSUCCESS, (DBGetTriggerSettingWord(triggerID, NULL, MODULENAME, SETTING_TRIGGERON, 0)&TRIGGER_CONNSUCCESS)?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CONNGIVEUP, (DBGetTriggerSettingWord(triggerID, NULL, MODULENAME, SETTING_TRIGGERON, 0)&TRIGGER_CONNGIVEUP)?BST_CHECKED:BST_UNCHECKED);
			break;
		}

	case WM_COMMAND:
		break;

	case TM_ADDTRIGGER:
		{	
			// wParam = (WPARAM)(DWORD)triggerID
			DWORD triggerID;
			int flags;
			
			triggerID = (DWORD)wParam;
			flags = 0;
			flags |= IsDlgButtonChecked(hwndDlg, IDC_CONNLOST)?TRIGGER_CONNLOST:0;
			flags |= IsDlgButtonChecked(hwndDlg, IDC_LOGINERROR)?TRIGGER_LOGINERROR:0;
			flags |= IsDlgButtonChecked(hwndDlg, IDC_OTHERLOC)?TRIGGER_OTHERLOC:0;
			flags |= IsDlgButtonChecked(hwndDlg, IDC_CONNRETRY)?TRIGGER_CONNRETRY:0;
			flags |= IsDlgButtonChecked(hwndDlg, IDC_CONNSUCCESS)?TRIGGER_CONNSUCCESS:0;
			flags |= IsDlgButtonChecked(hwndDlg, IDC_CONNGIVEUP)?TRIGGER_CONNGIVEUP:0;
			DBWriteTriggerSettingWord(triggerID, NULL, MODULENAME, SETTING_TRIGGERON, (WORD)flags);
			break;
		}

	case TM_DELTRIGGER:
		{	
			/* wParam = (WPARAM)(DWORD)triggerID */
			// wParam = triggerID
			DWORD triggerID;
			REMOVETRIGGERSETTINGS rts;
			
			triggerID = (DWORD)wParam;
			rts.cbSize = sizeof(REMOVETRIGGERSETTINGS);
			rts.prefix = PREFIX_TRIGGERID;
			rts.id = triggerID;
			rts.hContact = NULL;
			rts.szModule = MODULENAME;
			CallService(MS_TRIGGER_REMOVESETTINGS, 0, (LPARAM)&rts);
			break;
		}

	case WM_DESTROY:
		break;
	}

	return FALSE;
}

static int KeepStatusEvent(WPARAM wParam, LPARAM lParam) {

	DWORD triggerID;
	TRIGGERDATA td;
	int flags;
	
	triggerID = 0;
	do {
		triggerID = (DWORD)CallService(MS_TRIGGER_FINDNEXTTRIGGERID, triggerID, (LPARAM)TRIGGERNAME);
		if (triggerID == 0)
			continue;

		ZeroMemory(&td, sizeof(TRIGGERDATA));
		td.cbSize = sizeof(TRIGGERDATA);
		flags = DBGetTriggerSettingWord(triggerID, NULL, MODULENAME, SETTING_TRIGGERON, 0);
		switch (wParam) {
		case KS_CONN_STATE_LOST:
			if (!(flags&TRIGGER_CONNLOST))
				continue;

			td.dFlags |= DF_PROTO;
			td.szProto = (char *)lParam;
			break;
		case KS_CONN_STATE_LOGINERROR:
			if (!(flags&TRIGGER_LOGINERROR))
				continue;

			td.dFlags |= DF_PROTO;
			td.szProto = (char *)lParam;
			break;
		case KS_CONN_STATE_OTHERLOCATION:
			if (!(flags&TRIGGER_OTHERLOC))
				continue;

			td.dFlags |= DF_PROTO;
			td.szProto = (char *)lParam;
			break;
		case KS_CONN_STATE_RETRY:
			if (!(flags&TRIGGER_CONNRETRY))
				continue;

			break;
		case KS_CONN_STATE_STOPPEDCHECKING:
			if ( (lParam) && (!(flags&TRIGGER_CONNSUCCESS)))
				continue;

			if ( (!lParam) && (!(flags&TRIGGER_CONNGIVEUP)))
				continue;

			break;

		default:
			continue;
		}

		REPORTINFO ri = { 0 };
		ri.cbSize = sizeof(REPORTINFO);
		ri.flags = TRG_PERFORM;
		ri.triggerID = triggerID;
		ri.td = &td;
		CallService(MS_TRIGGER_REPORTEVENT, 0, (LPARAM)&ri);
	}
		while (triggerID != 0);

	return 0;
}

int RegisterTrigger()
{
	if (!ServiceExists(MS_TRIGGER_REGISTERTRIGGER))
		return -1;

	hKeepStatusHook = HookEvent(ME_KS_CONNECTIONEVENT, KeepStatusEvent);

	TRIGGERREGISTER tr = { 0 };
	tr.cbSize = sizeof(TRIGGERREGISTER);
	tr.pszName = TRIGGERNAME;
	tr.hInstance = hInst;
	tr.pfnDlgProc = DlgProcOptsTriggerKeepStatus;
	tr.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_KS_TRIGGER);
	tr.dFlags = DF_PROTO;
	/* register the trigger at the TriggerPlugin */
	CallService(MS_TRIGGER_REGISTERTRIGGER, 0, (LPARAM)&tr);

	return 0;
}

int DeInitTrigger()
{
	UnhookEvent(hKeepStatusHook);
	return 0;
}
