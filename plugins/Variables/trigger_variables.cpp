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
#include "variables.h"
#include "m_trigger.h"
#include "trigger_variables.h"
#include "resource.h"

extern HINSTANCE hInst;

static TRG_VAR_CACHE *tvc = NULL;
static int tvcCount = 0;
static unsigned int stringChangeTimerID = 0;

static int addToCache(DWORD triggerID) {

	/* triggerID must be in the DB */
	DBVARIANT dbv;
	int i;

	for (i=0;i<tvcCount;i++) {
		if (tvc[i].triggerID == triggerID) {
			free(tvc[i].parsedText);
			MoveMemory(&tvc[i], &tvc[tvcCount-1], sizeof(TRG_VAR_CACHE));
			tvcCount -= 1;
		}
	}
	if (!DBGetTriggerSettingTString(triggerID, NULL, MODULENAME, SETTING_TRIGGERTEXT, &dbv)) {
		tvc = ( TRG_VAR_CACHE* )realloc(tvc, (tvcCount+1)*sizeof(TRG_VAR_CACHE));
		if (tvc == NULL) {
			return -1;
		}
		tvc[tvcCount].triggerID = triggerID;
		tvc[tvcCount].parsedText = variables_parsedup(dbv.ptszVal, NULL, NULL);
		// it stays in our own mem space!
		if (tvc[tvcCount].parsedText == NULL) {
			return -1;
		}
		tvcCount += 1;
		DBFreeVariant(&dbv);
	}
	else {
		return -1;
	}

	return 0;
}

static int removeFromCache(DWORD triggerID) {

	int i;

	for (i=0;i<tvcCount;i++) {
		if (tvc[i].triggerID == triggerID) {
			free(tvc[i].parsedText);
			MoveMemory(&tvc[i], &tvc[tvcCount-1], sizeof(TRG_VAR_CACHE));
			tvcCount -= 1;
		}
	}
	
	return 0;
}

static VOID CALLBACK checkStringsTimer(HWND hwnd,UINT message,UINT_PTR idEvent,DWORD dwTime) {

	int i;
	TCHAR *parsedText;
	REPORTINFO ri;
	TRIGGERDATA td;
	DBVARIANT dbv;
	DWORD triggerID;

	triggerID = 0;
	do {
		triggerID = (DWORD)CallService(MS_TRIGGER_FINDNEXTTRIGGERID, triggerID, (LPARAM)TRIGGERNAME);
		if (triggerID == 0) {
			continue;
		}
		for (i=0;i<tvcCount;i++) {
			if (triggerID != tvc[i].triggerID) {
				continue;
			}
			if (!DBGetTriggerSettingTString(tvc[i].triggerID, NULL, MODULENAME, SETTING_TRIGGERTEXT, &dbv)) {
				parsedText = variables_parsedup(dbv.ptszVal, NULL, NULL);
				if (parsedText == NULL) {
					continue;
				}
				if (!_tcscmp(tvc[i].parsedText, parsedText)) {
					free(parsedText);
					continue;
				}
				else {
					ZeroMemory(&td, sizeof(td));
					td.cbSize = sizeof(td);
					td.dFlags = DF_TEXT;
					td.tszText = parsedText;
					
					ZeroMemory(&ri, sizeof(REPORTINFO));
					ri.cbSize = sizeof(REPORTINFO);
					ri.triggerID = tvc[i].triggerID;
					ri.pszName = TRIGGERNAME;
					ri.flags = TRG_PERFORM;
					ri.td = &td;
					
					CallService(MS_TRIGGER_REPORTEVENT, 0, (LPARAM)&ri);
					free(tvc[i].parsedText);
					tvc[i].parsedText = parsedText;
				}
				DBFreeVariant(&dbv);
			}
		}
	} while (triggerID != 0);
}

INT_PTR CALLBACK DlgProcOptsStringChange(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch (msg) {
	case WM_INITDIALOG: {
		DBVARIANT dbv;
		DWORD triggerID;
		
		TranslateDialogDefault(hwndDlg);
		triggerID = (DWORD)lParam;
		if (!DBGetTriggerSetting(triggerID, NULL, MODULENAME, SETTING_TRIGGERTEXT, &dbv)) {
			SetDlgItemTextA(hwndDlg, IDC_FORMATTEXT, dbv.pszVal);
			DBFreeVariant(&dbv);
		}
		variables_skin_helpbutton(hwndDlg, IDC_SHOWHELP);
		break;
						}

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			case IDC_SHOWHELP: {
				variables_showhelp(hwndDlg, IDC_FORMATTEXT, VHF_FULLDLG|VHF_SETLASTSUBJECT, NULL, NULL);
				break;
							   }
		}
		break;

	case TM_ADDTRIGGER: {
		// wParam = trigger ID
		// lParam = (TRIGGERREGISTER *)
		DWORD triggerID;
		TCHAR *tszFormatText;

		triggerID = (DWORD)wParam;
		tszFormatText = Hlp_GetDlgItemText(hwndDlg, IDC_FORMATTEXT);
		if (tszFormatText != NULL) {
			DBWriteTriggerSettingTString(triggerID, NULL, MODULENAME, SETTING_TRIGGERTEXT, tszFormatText);
			free(tszFormatText);
		}
		addToCache(triggerID);
		break;
						}
	
	case TM_DELTRIGGER: {
		// wParam = triggerID
		// lParam = (TRIGGEREGISTER *) may be 0
		DWORD triggerID;
		REMOVETRIGGERSETTINGS rts;
		
		triggerID = (DWORD)wParam;
		removeFromCache(triggerID);
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

int initTriggerModule() {

	CONDITIONREGISTER cr;
	TRIGGERREGISTER tr;
	ACTIONREGISTER ar;
	int res;
	DWORD triggerID;

	log_debugA("Variables: initTriggerModule");
	if (!ServiceExists(MS_TRIGGER_REGISTERTRIGGER)) {
		log_debugA("Variables: %s does not exist", MS_TRIGGER_REGISTERTRIGGER);
		return -1;
	}
	ZeroMemory(&tr, sizeof(tr));
	tr.cbSize = sizeof(tr);
	tr.pszName = TRIGGERNAME;
	tr.hInstance = hInst;
	tr.pfnDlgProc = DlgProcOptsStringChange;
	tr.pszTemplate = MAKEINTRESOURCEA(IDD_TRG_STRINGCHANGE);
	tr.dFlags = DF_TEXT|DF_TCHAR;
	res = CallService(MS_TRIGGER_REGISTERTRIGGER, 0, (LPARAM)&tr);
	log_debugA("Variables: %s registered (%d)", TRIGGERNAME, res);

	ZeroMemory(&ar, sizeof(ACTIONREGISTER));
	ar.cbSize = sizeof(ACTIONREGISTER);
	ar.pszName = "Variables: Parse string";
	ar.hInstance = hInst;
	ar.pfnDlgProc = DlgProcOptsParseString;
	ar.pszTemplate = MAKEINTRESOURCEA(IDD_ACT_PARSESTRING);
	ar.actionFunction = ParseStringAction;
	ar.flags = ARF_FUNCTION|ARF_TCHAR;
	CallService(MS_TRIGGER_REGISTERACTION, 0, (LPARAM)&ar);

	ZeroMemory(&cr, sizeof(CONDITIONREGISTER));
	cr.cbSize = sizeof(CONDITIONREGISTER);
	cr.pszName = "Variables: Condition";
	cr.hInstance = hInst;
	cr.pfnDlgProc = DlgProcOptsCondition;
	cr.pszTemplate = MAKEINTRESOURCEA(IDD_CND_PARSESTRING);
	cr.conditionFunction = ParseStringCondition;
	cr.flags = CRF_FUNCTION|CRF_TCHAR;
	CallService(MS_TRIGGER_REGISTERCONDITION, 0, (LPARAM)&cr);

	// fill cache
	triggerID = 0;
	do {
		triggerID = (DWORD)CallService(MS_TRIGGER_FINDNEXTTRIGGERID, triggerID, (LPARAM)TRIGGERNAME);
		if (triggerID == 0) {
			continue;
		}
		addToCache(triggerID);
	} while (triggerID != 0);
	stringChangeTimerID = SetTimer(NULL, 0, CHECKSTRINGDELAY, checkStringsTimer);

	return res;
}
