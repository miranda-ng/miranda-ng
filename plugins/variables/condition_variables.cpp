/*
    Variables Plugin for Miranda-IM (www.miranda-im.org)
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
// This file has not been converted to unicode yet
#include "variables.h"
#include "m_trigger.h"
#include "trigger_variables.h"
#include "resource.h"

int ParseStringCondition(DWORD conditionID, REPORTINFO *ri) {

	int res;

	res = CRV_TRUE;
	if (ri->flags&CND_PERFORM) {
		DBVARIANT dbv;

		if (!DBGetConditionSettingTString(conditionID, NULL, MODULENAME, SETTING_PARSESTRING, &dbv)) {
			FORMATINFO fi;

			ZeroMemory(&fi, sizeof(FORMATINFO));
			fi.cbSize = sizeof(FORMATINFO);
			fi.tszFormat = dbv.ptszVal;
			fi.tszExtraText = ((ri->td!=NULL)&&(ri->td->dFlags&DF_TEXT))?ri->td->tszText:NULL;
			fi.hContact = ((ri->td!=NULL)&&(ri->td->dFlags&DF_CONTACT))?ri->td->hContact:NULL;
			fi.flags |= FIF_TCHAR;
			free((TCHAR *)CallService(MS_VARS_FORMATSTRING, (WPARAM)&fi, 0));
			log_debugA("err: %d", fi.eCount);
			res = fi.eCount==0?CRV_TRUE:CRV_FALSE;
			DBFreeVariant(&dbv);
		}
	}
	if (ri->flags&CND_CLEANUP) {
		REMOVETRIGGERSETTINGS ras;

		ras.cbSize = sizeof(REMOVETRIGGERSETTINGS);
		ras.prefix = PREFIX_CONDITIONID;
		ras.id = conditionID;
		ras.szModule = MODULENAME;
		ras.hContact = NULL;
		CallService(MS_TRIGGER_REMOVESETTINGS, 0, (LPARAM)&ras);
	}
	
	return res;
}

INT_PTR CALLBACK DlgProcOptsCondition(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch (msg) {
	case WM_INITDIALOG: {
		DBVARIANT dbv;
		DWORD conditionID;
		
		TranslateDialogDefault(hwndDlg);
		conditionID = (DWORD)lParam;
		if (!DBGetConditionSettingTString(conditionID, NULL, MODULENAME, SETTING_PARSESTRING, &dbv)) {
			SetDlgItemText(hwndDlg, IDC_PARSESTRING, dbv.ptszVal);
			DBFreeVariant(&dbv);
		}
        break;
						}

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_SHOWHELP: {
			int flags;
			VARHELPINFO vhi;
			TRIGGERINFO ti;

			ZeroMemory(&vhi, sizeof(VARHELPINFO));
			vhi.cbSize = sizeof(VARHELPINFO);
			ZeroMemory(&ti, sizeof(TRIGGERINFO));
			SendMessage(GetParent(hwndDlg), TM_GETTRIGGERINFO, 0, (LPARAM)&ti);
			vhi.flags = VHF_INPUT;
			if (ti.dFlags&DF_TEXT) {
				vhi.flags |= VHF_EXTRATEXT;
				vhi.szExtraTextDesc = "TriggerData: Text";
			}
			else {
				flags |= VHF_HIDEEXTRATEXTTOKEN;
			}
			if (ti.dFlags&DF_CONTACT) {
				flags |= VHF_SUBJECT;
				vhi.szSubjectDesc = "TriggerData: Contact";
			}
			else {
				flags |= VHF_HIDESUBJECTTOKEN;
			}
			vhi.hwndCtrl = GetDlgItem(hwndDlg, IDC_PARSESTRING);
			CallService(MS_VARS_SHOWHELPEX, (WPARAM)hwndDlg, (LPARAM)&vhi);
			break;		
					   }
		}
		break;

	case TM_ADDCONDITION: {
		DWORD conditionID;
		TCHAR *tszText;

		conditionID = (DWORD)wParam;
		tszText = Hlp_GetDlgItemText(hwndDlg, IDC_PARSESTRING);
		if (tszText != NULL) {
			DBWriteConditionSettingTString(conditionID, NULL, MODULENAME, SETTING_PARSESTRING, tszText);
			free(tszText);
		}
		break;
					   }

	case WM_DESTROY:
		break;
	}

	return FALSE;
}