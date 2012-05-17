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

static void parseStringThread(void *arg) {

	TCHAR *tszParsed;
	FORMATINFO *fi;

	fi = (FORMATINFO *)arg;
	if (arg == NULL) {
		return;
	}
	tszParsed = (TCHAR *)CallService(MS_VARS_FORMATSTRING, (WPARAM)fi, 0);
	log_debugA("parseStringThread: %s > %s", fi->tszFormat, tszParsed);
	if (tszParsed != NULL) {
		free(tszParsed);
	}
	if (fi->tszFormat != NULL) {
		free(fi->tszFormat);
	}
	if (fi->tszExtraText != NULL) {
		free(fi->tszExtraText);
	}
	free(fi);
}


int ParseStringAction(DWORD actionID, REPORTINFO *ri) {

	if (ri->flags&ACT_PERFORM) {
		DBVARIANT dbv;

		if (!DBGetActionSettingTString(actionID, NULL, MODULENAME, SETTING_PARSESTRING, &dbv)) {
			if (DBGetActionSettingByte(actionID, NULL, MODULENAME, SETTING_PARSEASYNC, 0)) {
				FORMATINFO *fi;
				
				fi = ( FORMATINFO* )malloc(sizeof(FORMATINFO));
				ZeroMemory(fi, sizeof(FORMATINFO));
				fi->cbSize = sizeof(FORMATINFO);
				fi->tszFormat = _tcsdup(dbv.ptszVal);
				fi->tszExtraText = ((ri->td!=NULL)&&(ri->td->dFlags&DF_TEXT))?_tcsdup(ri->td->tszText):NULL;
				fi->hContact = ((ri->td!=NULL)&&(ri->td->dFlags&DF_CONTACT))?ri->td->hContact:NULL;
				fi->flags |= FIF_TCHAR;
				//forkthread(parseStringThread, 0, fi);
				mir_forkthread(parseStringThread, fi);
			}
			else {
				free(variables_parsedup(dbv.ptszVal, ((ri->td!=NULL)&&(ri->td->dFlags&DF_TEXT))?ri->td->tszText:NULL, ((ri->td!=NULL)&&(ri->td->dFlags&DF_CONTACT))?ri->td->hContact:NULL));
			}
			DBFreeVariant(&dbv);
		}
	}
	if (ri->flags&ACT_CLEANUP) {
		REMOVETRIGGERSETTINGS ras;

		ras.cbSize = sizeof(REMOVETRIGGERSETTINGS);
		ras.prefix = PREFIX_ACTIONID;
		ras.id = actionID;
		ras.szModule = MODULENAME;
		ras.hContact = NULL;
		CallService(MS_TRIGGER_REMOVESETTINGS, 0, (LPARAM)&ras);
	}
	
	return 0;
}

INT_PTR CALLBACK DlgProcOptsParseString(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch (msg) {
	case WM_INITDIALOG: {
		DBVARIANT dbv;
		DWORD actionID;
		
		TranslateDialogDefault(hwndDlg);
		actionID = (DWORD)lParam;
		if (!DBGetActionSettingTString(actionID, NULL, MODULENAME, SETTING_PARSESTRING, &dbv)) {
			SetDlgItemText(hwndDlg, IDC_PARSESTRING, dbv.ptszVal);
			DBFreeVariant(&dbv);
		}
		CheckDlgButton(hwndDlg, IDC_PARSEASYNC, DBGetActionSettingByte(actionID, NULL, MODULENAME, SETTING_PARSEASYNC, 0)?BST_CHECKED:BST_UNCHECKED);
		variables_skin_helpbutton(hwndDlg, IDC_SHOWHELP);
        break;
						}

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_SHOWHELP: {
			int flags;
			VARHELPINFO vhi;
			TRIGGERINFO ti;

			ZeroMemory(&ti, sizeof(TRIGGERINFO));
			SendMessage(GetParent(hwndDlg), TM_GETTRIGGERINFO, 0, (LPARAM)&ti);
			ZeroMemory(&vhi, sizeof(VARHELPINFO));
			vhi.cbSize = sizeof(VARHELPINFO);
			vhi.flags = VHF_INPUT;
			if (ti.dFlags&DF_TEXT) {
				vhi.flags |= VHF_EXTRATEXT;
				vhi.szExtraTextDesc = "TriggerData: Text";
			}
			else {
				flags |= VHF_HIDESUBJECTTOKEN;
			}
			if (ti.dFlags&DF_CONTACT) {
				flags |= VHF_SUBJECT;
				vhi.szSubjectDesc = "TriggerData: Contact";
			}
			else {
				flags |= VHF_HIDEEXTRATEXTTOKEN;
			}
			vhi.hwndCtrl = GetDlgItem(hwndDlg, IDC_PARSESTRING);
			CallService(MS_VARS_SHOWHELPEX, (WPARAM)hwndDlg, (LPARAM)&vhi);
			break;		
					   }
		}
		break;

	case TM_ADDACTION: {
		// wParam = action ID
		// lParam = 0
		DWORD actionID;
		TCHAR *tszText;

		actionID = (DWORD)wParam;
		tszText = Hlp_GetDlgItemText(hwndDlg, IDC_PARSESTRING);
		if (tszText != NULL) {
			DBWriteActionSettingTString(actionID, NULL, MODULENAME, SETTING_PARSESTRING, tszText);
			free(tszText);
		}
		DBWriteActionSettingByte(actionID, NULL, MODULENAME, SETTING_PARSEASYNC, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_PARSEASYNC));
		break;
					   }

	case WM_DESTROY:
		break;
	}

	return FALSE;
}