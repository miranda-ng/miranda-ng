#include "common.h"
#include "trigger.h"

AlarmList alist;
//SPECIFICTRIGGERINFO sti;
TRIGGERDATA td;

unsigned short last_selected_id = 0;

static BOOL CALLBACK DlgProcTriggerOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		{
			copy_list(alist);
			ALARM *i;
			for(alist.reset(); i = alist.current(); alist.next()) {
				int pos = SendDlgItemMessage(hwndDlg, IDC_LST_TRIGLIST, LB_ADDSTRING, (WPARAM)-1, (LPARAM)i->szTitle);
				SendDlgItemMessage(hwndDlg, IDC_LST_TRIGLIST, LB_SETITEMDATA, (WPARAM)pos, (LPARAM)i->id);
				if(i->trigger_id == (unsigned int)lParam)
					SendDlgItemMessage(hwndDlg, IDC_LST_TRIGLIST, LB_SETCURSEL, (WPARAM)pos, 0);
			}
		}
		return FALSE;

	case WM_COMMAND:
		if (HIWORD( wParam ) == LBN_SELCHANGE) {
			int pos = SendDlgItemMessage(hwndDlg, IDC_LST_TRIGLIST, LB_GETCURSEL, 0, 0);
			if(pos != LB_ERR) {
				last_selected_id = (unsigned short)SendDlgItemMessage(hwndDlg, IDC_LST_TRIGLIST, LB_GETITEMDATA, pos, 0);
			}
			return TRUE;
		}
		break;
	case TM_GETTRIGGERINFO:
		{
			//memset(&sti, sizeof(sti), 0);
			memset(&td, sizeof(td), 0);

			ALARM *i;
			for(alist.reset(); i = alist.current(); alist.next()) {
				if(i->trigger_id == wParam) {
					//sti.cbSize = sizeof(sti);
					//sti.triggerID = wParam;
					//sti.pszSummary = i->szTitle;
					//sti.td = &td;

					td.cbSize = sizeof(td);
					td.dFlags = DF_TEXT;
					td.szText = i->szDesc;
					//*(SPECIFICTRIGGERINFO **)lParam = &sti;
					break;
				}
			}
		}
		return TRUE;
	case TM_ADDTRIGGER:
		{
			ALARM *i;
			for(alist.reset(); i = alist.current(); alist.next()) {
				if(i->id == last_selected_id) {
					i->trigger_id = wParam;
					alter_alarm_list(i);
					break;
				}
			}
		}
		return TRUE;
	case TM_DELTRIGGER:
		{
			ALARM *i;
			for(alist.reset(); i = alist.current(); alist.next()) {
				if(i->trigger_id == wParam) {
					i->trigger_id = 0;
					alter_alarm_list(i);
					break;
				}
			}
		}
		return TRUE;

	case WM_DESTROY:
		// it seems this is called before the TM_ADDTRIGGER messsage is passed to us....tsk,tsk :P
		//alist.clear();
		break;

	}
	return FALSE;
}

int LoadTriggerSupport() {
	if(ServiceExists(MS_TRIGGER_REGISTERTRIGGER)) {
		TRIGGERREGISTER treg = {0};
	
		treg.cbSize = sizeof(treg);
		treg.pszName = Translate("Alarms");
		treg.hInstance = hInst;
		treg.pfnDlgProc = DlgProcTriggerOptions;
		treg.pszTemplate = MAKEINTRESOURCE(IDD_OPTTRIGGER);

		CallService(MS_TRIGGER_REGISTERTRIGGER, 0, (LPARAM)&treg);
	}

	return 0;
}
