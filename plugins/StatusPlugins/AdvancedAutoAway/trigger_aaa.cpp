#include "../commonstatus.h"
#include "advancedautoaway.h"
#include <m_trigger.h>
#include "trigger_aaa.h"
#include "../resource.h"

extern HINSTANCE hInst;
static HANDLE hAAAStateChangedHook;

static int AAAStateChanged(WPARAM wParam, LPARAM lParam) {

	AUTOAWAYSETTING *aas;
	DWORD triggerID;
	REPORTINFO ri;
	TRIGGERDATA td;
	BOOL bReport;

	aas = (AUTOAWAYSETTING *)lParam;
	triggerID = 0;
	do {
		triggerID = (DWORD)CallService(MS_TRIGGER_FINDNEXTTRIGGERID, triggerID, (LPARAM)TRIGGERNAME);
		if (triggerID == 0) {
			continue;
		}
		bReport = FALSE;
		switch (aas->curState) {
		case STATUS1_SET:
			if (DBGetTriggerSettingByte(triggerID, NULL, MODULENAME, SETTING_ENTERFIRST, 0)) {
				bReport = TRUE;
			}
			break;
		case STATUS2_SET:
			if (DBGetTriggerSettingByte(triggerID, NULL, MODULENAME, SETTING_ENTERSECOND, 0)) {
				bReport = TRUE;
			}
			break;
		case SET_ORGSTATUS:
			if ( (aas->oldState == STATUS1_SET) && (DBGetTriggerSettingByte(triggerID, NULL, MODULENAME, SETTING_LEAVEFIRST, 0)) ) {
				bReport = TRUE;;
			}
			if ( (aas->oldState == STATUS2_SET) && (DBGetTriggerSettingByte(triggerID, NULL, MODULENAME, SETTING_LEAVESECOND, 0)) ) {
				bReport = TRUE;;
			}
			break;
		case HIDDEN_ACTIVE:
			if (DBGetTriggerSettingByte(triggerID, NULL, MODULENAME, SETTING_BECOMEACTIVE, 0)) {
				bReport = TRUE;
			}
			break;
		case ACTIVE:
			if (aas->oldState == HIDDEN_ACTIVE) {
				char setting[128];
				char *szProto;

				szProto = DBGetContactSettingByte(NULL, MODULENAME, SETTING_SAMESETTINGS, 0)?SETTING_ALL:aas->protocolSetting->szName;
				_snprintf(setting, sizeof(setting), "%s_Lv1Status", szProto);
				if ( (aas->protocolSetting->lastStatus == DBGetContactSettingWord(NULL, MODULENAME, setting, ID_STATUS_AWAY)) && DBGetTriggerSettingByte(triggerID, NULL, MODULENAME, SETTING_LEAVEFIRST, 0) ) {
					bReport = TRUE;
				}
				
				_snprintf(setting, sizeof(setting), "%s_Lv2Status", szProto);
				if ( (aas->protocolSetting->lastStatus == DBGetContactSettingWord(NULL, MODULENAME, setting, ID_STATUS_NA)) && DBGetTriggerSettingByte(triggerID, NULL, MODULENAME, SETTING_LEAVESECOND, 0) ) {
					bReport = TRUE;
				}
			}
			break;
		}
		if (!bReport) {
			continue;
		}
		ZeroMemory(&td, sizeof(td));
		td.cbSize = sizeof(td);
		td.dFlags = DF_PROTO|DF_STATUS|DF_TEXT;
		td.szProto = td.szText = aas->protocolSetting->szName;
		td.status = aas->protocolSetting->status;
		ZeroMemory(&ri, sizeof(REPORTINFO));
		ri.cbSize = sizeof(REPORTINFO);
		ri.triggerID = triggerID;
		ri.pszName = TRIGGERNAME;
		ri.flags = TRG_PERFORM;
		ri.td = &td;
		CallService(MS_TRIGGER_REPORTEVENT, 0, (LPARAM)&ri);
	} while (triggerID != 0);

	return 0;
}

static INT_PTR CALLBACK DlgProcOptsAAAStateChange(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch (msg) {
	case WM_INITDIALOG: {
		DWORD triggerID;

		TranslateDialogDefault(hwndDlg);		
		triggerID = (DWORD)lParam;
		CheckDlgButton(hwndDlg, IDC_ENTERFIRST, DBGetTriggerSettingByte(triggerID, NULL, MODULENAME, SETTING_ENTERFIRST, 0)?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ENTERSECOND, DBGetTriggerSettingByte(triggerID, NULL, MODULENAME, SETTING_ENTERSECOND, 0)?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LEAVEFIRST, DBGetTriggerSettingByte(triggerID, NULL, MODULENAME, SETTING_LEAVEFIRST, 0)?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LEAVESECOND, DBGetTriggerSettingByte(triggerID, NULL, MODULENAME, SETTING_LEAVESECOND, 0)?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_BECOMEACTIVE, DBGetTriggerSettingByte(triggerID, NULL, MODULENAME, SETTING_BECOMEACTIVE, 0)?BST_CHECKED:BST_UNCHECKED);
		break;
						}

	case TM_ADDTRIGGER: {
		// wParam = trigger ID
		DWORD triggerID;

		triggerID = (DWORD)wParam;
		DBWriteTriggerSettingByte(triggerID, NULL, MODULENAME, SETTING_ENTERFIRST, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ENTERFIRST));
		DBWriteTriggerSettingByte(triggerID, NULL, MODULENAME, SETTING_ENTERSECOND, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ENTERSECOND));
		DBWriteTriggerSettingByte(triggerID, NULL, MODULENAME, SETTING_LEAVEFIRST, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_LEAVEFIRST));
		DBWriteTriggerSettingByte(triggerID, NULL, MODULENAME, SETTING_LEAVESECOND, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_LEAVESECOND));
		DBWriteTriggerSettingByte(triggerID, NULL, MODULENAME, SETTING_BECOMEACTIVE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_BECOMEACTIVE));
		break;
						}
	
	case TM_DELTRIGGER: {
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
	}

    return FALSE;
}

int InitTrigger() {

	TRIGGERREGISTER tr;

	if (!ServiceExists(MS_TRIGGER_REGISTERTRIGGER)) {
		return -1;
	}
	ZeroMemory(&tr, sizeof(tr));
	tr.cbSize = sizeof(tr);
	tr.pszName = TRIGGERNAME;
	tr.hInstance = hInst;
	tr.pfnDlgProc = DlgProcOptsAAAStateChange;
	tr.pszTemplate = MAKEINTRESOURCEA(IDD_TRG_AAASTATECHANGE);
	tr.dFlags = DF_STATUS|DF_PROTO;
	CallService(MS_TRIGGER_REGISTERTRIGGER, 0, (LPARAM)&tr);
	hAAAStateChangedHook = HookEvent(ME_AAA_STATECHANGED, AAAStateChanged);

	return 0;
}

int DeInitTrigger() {

	UnhookEvent(hAAAStateChangedHook);

	return 0;
}