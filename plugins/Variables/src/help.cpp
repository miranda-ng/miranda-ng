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

struct HELPDLGDATA
{
	VARHELPINFO *vhs;
	HWND hwndSubjectDlg;
	HWND hwndExtraTextDlg;
	HWND hwndInputDlg;
};

static INT_PTR CALLBACK inputDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam);

extern HINSTANCE hInst;

extern HCURSOR hCurSplitNS;

static HWND hwndHelpDialog = NULL;

static HICON hHelpIcon = NULL;

static int defaultHelpDialogResize(HWND hwndDlg, LPARAM lParam, UTILRESIZECONTROL *urc) {

	switch(urc->wId) {
	case IDC_ABOUT:
	case IDC_ABOUTFRAME:
		return RD_ANCHORX_WIDTH|RD_ANCHORY_TOP;
	}

	return RD_ANCHORX_WIDTH|RD_ANCHORY_HEIGHT;
}

// dialog box for the %extratext% input
static INT_PTR CALLBACK extratextDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam) {

	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		break;

	case VARM_SETEXTRATEXT:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT,
			SendMessage(GetDlgItem(hwndDlg, IDC_EXTRATEXT), WM_SETTEXT, wParam, lParam));
		return TRUE;

	case VARM_GETEXTRATEXTLENGTH:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT,
			SendMessage(GetDlgItem(hwndDlg, IDC_EXTRATEXT), WM_GETTEXTLENGTH, wParam, lParam));
		return TRUE;

	case VARM_GETEXTRATEXT:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT,
			SendMessage(GetDlgItem(hwndDlg, IDC_EXTRATEXT), WM_GETTEXT, wParam, lParam));
		return TRUE;

	case WM_SIZE:
		if (!IsIconic( hwndDlg )) {
			UTILRESIZEDIALOG urd = { 0 };
			urd.cbSize = sizeof(urd);
			urd.hInstance = hInst;
			urd.hwndDlg = hwndDlg;
			urd.lpTemplate = MAKEINTRESOURCEA(IDD_EXTRATEXT_DIALOG);
			urd.pfnResizer = defaultHelpDialogResize;
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);

			SendMessage(hwndDlg, WM_MOVE, 0, 0);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EXTRATEXT:
			SendMessage(GetParent(hwndDlg), VARM_PARSE, 0, 0);
			break;
		}
		break;
	}

	return FALSE;
}

// dialog box for the %subject% selection
void ResetCList(HWND hwndDlg)
{
	if ((CallService(MS_CLUI_GETCAPS, 0, 0) & CLUIF_DISABLEGROUPS && !db_get_b(NULL, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT)) || !(GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CLIST), GWL_STYLE)&CLS_USEGROUPS))
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETUSEGROUPS, FALSE, 0);
	else
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETUSEGROUPS, TRUE, 0);

	SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETHIDEEMPTYGROUPS, 1, 0);
}

static int clistDialogResize(HWND hwndDlg, LPARAM lParam, UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_ABOUT:
	case IDC_ABOUTFRAME:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;

	case IDC_NULL:
	case IDC_CONTACT:
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
	}

	return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
}

static INT_PTR CALLBACK clistDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CLIST), GWL_STYLE, (GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CLIST), GWL_STYLE) | (CLS_SHOWHIDDEN) | (CLS_NOHIDEOFFLINE)) & ~CLS_CHECKBOXES & ~CLS_USEGROUPS );
		ResetCList(hwndDlg);
		CheckRadioButton(hwndDlg, IDC_NULL, IDC_CONTACT, IDC_NULL);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CLIST), IsDlgButtonChecked(hwndDlg, IDC_CONTACT));
		break;

	case VARM_SETSUBJECT:
		{
			LPARAM res = 0;
			MCONTACT hItem, hContact = wParam;
			log_debugA("VARM_SETSUBJECT: %u", hContact);
			if (hContact == INVALID_CONTACT_ID) {
				TCHAR *tszContact = db_get_tsa(NULL, MODULENAME, SETTING_SUBJECT);
				log_debugA("VARM_SETSUBJECT: %s", tszContact);
				if (tszContact != NULL) {
					hContact = decodeContactFromString(tszContact);
					log_debugA("VARM_SETSUBJECT decoded: %u", hContact);
					mir_free(tszContact);
			}	}

			if ((hContact != INVALID_CONTACT_ID) && (hContact != NULL))
				hItem = (MCONTACT)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
			else
				hItem = NULL;

			if (hItem != NULL)
				res = (LPARAM)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SELECTITEM, (WPARAM)hItem, 0);

			CheckRadioButton(hwndDlg, IDC_NULL, IDC_CONTACT, hItem==NULL?IDC_NULL:IDC_CONTACT);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CLIST), IsDlgButtonChecked(hwndDlg, IDC_CONTACT));
			SetFocus(GetDlgItem(hwndDlg, IDC_CLIST));
			SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)res);
		}
		return TRUE;

	case VARM_GETSUBJECT:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT,
			(IsDlgButtonChecked(hwndDlg, IDC_CONTACT) ? SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETSELECTION, 0, 0) : 0));
		return TRUE;

	case WM_SIZE:
		if (!IsIconic(hwndDlg)) {
			UTILRESIZEDIALOG urd = { sizeof(urd) };
			urd.hInstance = hInst;
			urd.hwndDlg = hwndDlg;
			urd.lParam = 0;
			/* ! uses ANSI version ! */
			urd.lpTemplate = MAKEINTRESOURCEA(IDD_CLIST_DIALOG);
			urd.pfnResizer = clistDialogResize;
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);

			SendMessage(hwndDlg, WM_MOVE, 0, 0);
		}
		break;

	case WM_SHOWWINDOW:
		if ((wParam) && (IsDlgButtonChecked(hwndDlg, IDC_CONTACT)))
			SetFocus(GetDlgItem(hwndDlg, IDC_CLIST));
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_NULL:
		case IDC_CONTACT:
			CheckRadioButton(hwndDlg, IDC_NULL, IDC_CONTACT, LOWORD(wParam));
			EnableWindow(GetDlgItem(hwndDlg, IDC_CLIST), IsDlgButtonChecked(hwndDlg, IDC_CONTACT));
			if (IsDlgButtonChecked(hwndDlg, IDC_CONTACT))
				SetFocus(GetDlgItem(hwndDlg, IDC_CLIST));
			break;
		}
		break;

	case WM_NOTIFY:
		switch (((NMHDR *) lParam)->idFrom) {
		case IDC_CLIST:
			switch (((NMHDR *) lParam)->code) {
			case CLN_OPTIONSCHANGED:
				ResetCList(hwndDlg);
				break;
			}
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		db_unset(NULL, MODULENAME, SETTING_SUBJECT);

		MCONTACT hContact = (MCONTACT)SendMessage(hwndDlg, VARM_GETSUBJECT, 0, 0);
		if (hContact != NULL) {
			TCHAR *tszContact = encodeContactToString(hContact);
			if (tszContact != NULL) {
				db_set_ts(NULL, MODULENAME, SETTING_SUBJECT, tszContact);
				mir_free(tszContact);
		}	}
		break;
	}

	return FALSE;
}

// dialog box for the tokens
static TCHAR *getTokenCategory(TOKENREGISTEREX *tr) {
	if (tr == NULL) {
		return NULL;
	}
	char *cat = NULL;
	char *helpText = mir_strdup(tr->szHelpText);
	if (helpText == NULL) {
		return NULL;
	}
	char *cur = helpText;
	while (*cur != 0) {
		if (*cur == '\t') {
			*cur = 0;
			helpText = ( char* )mir_realloc(helpText, strlen(helpText)+1);

			TCHAR *res = mir_a2t(helpText);
			mir_free(helpText);
			return res;
		}
		cur++;
	}

	TCHAR *res = mir_a2t(helpText);
	mir_free(helpText);
	return res;

}

static TCHAR *getHelpDescription(TOKENREGISTEREX *tr)
{
	if (tr == NULL)
		return NULL;

	char *cur = tr->szHelpText + strlen(tr->szHelpText);
	while (cur > tr->szHelpText) {
		if (*cur == '\t') {

			cur = mir_strdup(cur+1);
			TCHAR *res = mir_a2t(cur);
			mir_free(cur);
			return res;

		}
		cur--;
	}

	return mir_a2t(tr->szHelpText);

}

static TCHAR *getTokenDescription(TOKENREGISTEREX *tr)
{
	char *args, *first, *second;

	if (tr == NULL)
		return NULL;

	args = NULL;
	TCHAR *tArgs = NULL;
	if (tr->szHelpText == NULL)
		return mir_tstrdup(tr->tszTokenString);

	char *helpText = mir_strdup(tr->szHelpText);
	if (helpText == NULL)
		return NULL;

	char *cur = helpText;
	first = second = NULL;
	while (*cur != 0) {
		if (*cur == '\t') {
			if (first == NULL)
				first = cur;
			else if (second == NULL)
				second = cur;
		}
		cur++;
	}

	if ((first != NULL) && (second != NULL)) {
		*second = 0;
		args = first+1;
	}
	else args = NULL;

	size_t len = _tcslen(tr->tszTokenString) + (args!=NULL?strlen(args):0) + 3;
	TCHAR *desc = (TCHAR*)mir_calloc(len * sizeof(TCHAR));
	if (desc == NULL)
		return NULL;

	if (tr->flags&TRF_FIELD)
		mir_sntprintf(desc, len, _T("%c%s%c"), FIELD_CHAR, tr->szTokenString, FIELD_CHAR);
	else {
		if (args != NULL)
			tArgs = mir_a2t(args);

		mir_sntprintf(desc, len, _T("%c%s%s"), FUNC_CHAR, tr->tszTokenString, (tArgs!=NULL?tArgs:_T("")));
	}

	mir_free(tArgs);
	mir_free(helpText);
	return desc;
}

static int CALLBACK compareTokenHelp(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	TOKENREGISTEREX *tr1 = (TOKENREGISTEREX *)lParam1;
	TOKENREGISTEREX *tr2 = (TOKENREGISTEREX *)lParam2;
	if (tr1 == NULL || tr2 == NULL)
		return 0;

	ptrT cat1(getTokenCategory(tr1));
	ptrT cat2(getTokenCategory(tr2));
	if (cat1 == NULL || cat2 == NULL)
		return 0;

	int res = _tcscmp(cat1, cat2);
	if (res != 0)
		return res;

	if (tr1->tszTokenString == NULL || tr2->tszTokenString == NULL)
		return 0;

	return _tcscmp(tr1->tszTokenString, tr2->tszTokenString);
}

static BOOL CALLBACK processTokenListMessage(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	/* hwndDlg must be a child of the help dialog */
	switch(msg) {
	case WM_INITDIALOG:
		{
			TOKENREGISTEREX *tr;
			TCHAR *tszTokenDesc, *tszHelpDesc, *last, *cat, *text;

			// token list things
			HWND hList = GetDlgItem(hwndDlg, IDC_TOKENLIST);

			LVCOLUMN lvCol = { 0 };
			lvCol.mask = LVCF_TEXT;
			lvCol.pszText = TranslateT("Token");
			ListView_InsertColumn(hList, 0, &lvCol);

			lvCol.pszText = TranslateT("Description");
			ListView_InsertColumn(hList, 1, &lvCol);

			HELPDLGDATA *hdd = (HELPDLGDATA *)GetWindowLongPtr(GetParent(hwndDlg), GWLP_USERDATA);
			int i = -1;
			do {
				i += 1;
				tszHelpDesc = tszTokenDesc = NULL;
				tr = getTokenRegister(i);
				if ((tr == NULL) || (tr->tszTokenString == NULL))
					continue;

				else if (hdd != NULL) {
					if (!_tcscmp(tr->tszTokenString, SUBJECT)) {
						if (hdd->vhs->flags&VHF_HIDESUBJECTTOKEN)
							continue;

						if (hdd->vhs->szSubjectDesc != NULL)
							tszHelpDesc = mir_a2t(hdd->vhs->szSubjectDesc);
					}
					if (!_tcscmp(tr->tszTokenString, MIR_EXTRATEXT)) {
						if (hdd->vhs->flags & VHF_HIDEEXTRATEXTTOKEN)
							continue;

						if (hdd->vhs->szExtraTextDesc != NULL)
							tszHelpDesc = mir_a2t(hdd->vhs->szExtraTextDesc);
					}
				}

				LVITEM lvItem = { 0 };
				lvItem.mask = LVIF_TEXT | LVIF_PARAM;
				lvItem.iItem = ListView_GetItemCount(hList);
				lvItem.lParam = (LPARAM)tr;
				tszTokenDesc = getTokenDescription(tr);
				if (tszTokenDesc == NULL)
					continue;

				lvItem.pszText = tszTokenDesc;
				ListView_InsertItem(hList, &lvItem);
				mir_free(tszTokenDesc);

				lvItem.mask = LVIF_TEXT;
				if (tszHelpDesc == NULL)
					tszHelpDesc = getHelpDescription(tr);

				if (tszHelpDesc == NULL)
					tszHelpDesc = mir_tstrdup(_T("unknown"));

				lvItem.iSubItem = 1;
				lvItem.pszText = TranslateTS(tszHelpDesc);
				ListView_SetItem(hList, &lvItem);
				mir_free(tszHelpDesc);
			}
				while (tr != NULL);
	
			ListView_SetColumnWidth(hList, 0, LVSCW_AUTOSIZE);
			ListView_SetColumnWidth(hList, 1, LVSCW_AUTOSIZE);
			ListView_SortItems(hList, compareTokenHelp, 0);
			last = text = NULL;
			for (i = 0; i < ListView_GetItemCount(hList); i++) {
				LVITEM lvItem = { 0 };
				lvItem.mask = LVIF_PARAM;
				lvItem.iItem = i;
				if (ListView_GetItem(hList, &lvItem) == FALSE)
					continue;

				cat = getTokenCategory((TOKENREGISTEREX *)lvItem.lParam);
				if (cat != NULL) {
					text = mir_tstrdup(TranslateTS(cat));
					mir_free(cat);
				}
				else text = NULL;

				if (text != NULL && (last == NULL || _tcsicmp(last, text))) {
					lvItem.mask = LVIF_TEXT;
					lvItem.pszText = text;
					ListView_InsertItem(hList, &lvItem);
					if (last != NULL) {
						mir_free(last);
						lvItem.iSubItem = 0;
						lvItem.pszText = _T("");
						ListView_InsertItem(hList, &lvItem);
					}
					last = text;
				}
				else mir_free(text);
			}
			mir_free(last);
		}
		break;

	case WM_NOTIFY:
		if ((((NMHDR*)lParam)->idFrom == IDC_TOKENLIST) && (((NMHDR*)lParam)->code == NM_DBLCLK)) {
			HWND hwndInputDlg = (HWND)SendMessage(GetParent(hwndDlg), VARM_GETDIALOG, (WPARAM)VHF_INPUT, 0);
			if (hwndInputDlg == NULL)
				break;

			HWND hList = GetDlgItem(hwndDlg, IDC_TOKENLIST);
			int item = ListView_GetNextItem(hList, -1, LVNI_SELECTED|LVNI_FOCUSED);

			LVITEM lvItem = { 0 };
			lvItem.mask = LVIF_PARAM;
			lvItem.iItem = item;
			if (ListView_GetItem(hList, &lvItem) == FALSE)
				break;

			TOKENREGISTER *tr = (TOKENREGISTER *)lvItem.lParam;
			if (tr == NULL)
				break;

			size_t len = _tcslen(tr->tszTokenString) + 2;
			if (len < 0)
				break;

			TCHAR *tokenString = (TCHAR*)mir_alloc((len+1)*sizeof(TCHAR));
			if (tokenString == NULL)
				break;

			memset(tokenString, 0, ((len + 1) * sizeof(TCHAR)));
			mir_sntprintf(tokenString, len + 1, _T("%c%s%c"), (tr->flags & TRF_FIELD) ? FIELD_CHAR : FUNC_CHAR, tr->tszTokenString, (tr->flags & TRF_FIELD) ? FIELD_CHAR : '(');
			SendDlgItemMessage(hwndInputDlg, IDC_TESTSTRING, EM_REPLACESEL, TRUE, (LPARAM)tokenString);
			mir_free(tokenString);
			SetFocus(GetDlgItem(hwndInputDlg, IDC_TESTSTRING));
		}
		break;
	}

	return FALSE;
}

// "token only" dialog proc
static INT_PTR CALLBACK tokenHelpDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	processTokenListMessage(hwndDlg, msg, wParam, lParam);
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		break;

	case WM_SIZE:
		if (!IsIconic(hwndDlg)) {
			UTILRESIZEDIALOG urd = { 0 };
			urd.cbSize = sizeof(urd);
			urd.hInstance = hInst;
			urd.hwndDlg = hwndDlg;
			urd.lpTemplate = MAKEINTRESOURCEA(IDD_TOKENS_DIALOG);
			urd.pfnResizer = defaultHelpDialogResize;
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);

			SendMessage(hwndDlg, WM_MOVE, 0, 0);
		}
		break;
	}

	return FALSE;
}

// token + input dialog proc
// from SRMM
static LRESULT CALLBACK SplitterSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;

	switch (msg) {
	case WM_NCHITTEST:
		return HTCLIENT;

	case WM_SETCURSOR:
		GetClientRect(hwnd, &rc);
		//SetCursor(rc.right > rc.bottom ? hCurSplitNS : hCurSplitWE);
		if (rc.right > rc.bottom)
			SetCursor(hCurSplitNS);

		return TRUE;

	case WM_LBUTTONDOWN:
		SetCapture(hwnd);
		return 0;

	case WM_MOUSEMOVE:
		if (GetCapture() == hwnd) {
			GetClientRect(hwnd, &rc);
			SendMessage(GetParent(hwnd), DM_SPLITTERMOVED, rc.right > rc.bottom ? (short)HIWORD(GetMessagePos()) + rc.bottom / 2 : (short)LOWORD(GetMessagePos()) + rc.right / 2, (LPARAM)hwnd);
		}
		return 0;

	case WM_LBUTTONUP:
		ReleaseCapture();
		return 0;
	}
	return mir_callNextSubclass(hwnd, SplitterSubclassProc, msg, wParam, lParam);
}

struct INPUTDLGDATA
{
	int splitterPos;
	int originalSplitterPos;
	POINT minInputSize;
	POINT minResultSize;
	HWND hwndHelpDlg;
};

static int iFrameX, iFrameY;

static int inputDialogResize(HWND hwndDlg, LPARAM lParam, UTILRESIZECONTROL *urc)
{
	INPUTDLGDATA *dat = (INPUTDLGDATA *)lParam;

	switch (urc->wId) {
	case IDC_TOKENLIST:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;

	case IDC_TESTSTRING:
		urc->rcItem.bottom -= dat->splitterPos - dat->originalSplitterPos;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

	case IDC_SPLITTER:
		urc->rcItem.top -= dat->splitterPos - dat->originalSplitterPos;
		urc->rcItem.bottom -= dat->splitterPos - dat->originalSplitterPos;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM;

	case IDC_RESULT:
		urc->rcItem.top -= dat->splitterPos - dat->originalSplitterPos;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM;
	}

	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

static INT_PTR CALLBACK inputDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static INPUTDLGDATA *dat = (INPUTDLGDATA *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	processTokenListMessage(hwndDlg, msg, wParam, lParam);
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		dat = (INPUTDLGDATA*)mir_alloc(sizeof(INPUTDLGDATA));
		memset(dat, 0, sizeof(INPUTDLGDATA));
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
		// splitter things
		dat->splitterPos = (INT_PTR)db_get_dw(NULL, MODULENAME, SETTING_SPLITTERPOS, -1);
		{
			RECT rc;
			POINT pt;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_SPLITTER), &rc);
			pt.y = (rc.top + rc.bottom) / 2;
			pt.x = 0;
			ScreenToClient(hwndDlg, &pt);
			dat->originalSplitterPos = pt.y;

			if (dat->splitterPos == -1)
				dat->splitterPos = dat->originalSplitterPos;

			GetWindowRect(GetDlgItem(hwndDlg, IDC_SHOWHELP), &rc);
			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_SPLITTER), SplitterSubclassProc);

			GetWindowRect(GetDlgItem(hwndDlg, IDC_TESTSTRING), &rc);
			dat->minInputSize.x = rc.right - rc.left;
			dat->minInputSize.y = rc.bottom - rc.top;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_RESULT), &rc);
			dat->minResultSize.x = rc.right - rc.left;
			dat->minResultSize.y = rc.bottom - rc.top;
		}

		dat->hwndHelpDlg = GetParent(hwndDlg);
		SendMessage(hwndDlg, VARM_PARSE, 0, 0);
		SetTimer(hwndDlg, IDT_PARSE, 1000, NULL);

		SetFocus(GetDlgItem(hwndDlg, IDC_TESTSTRING));
		break;

	case DM_SPLITTERMOVED:
	{
		POINT pt;
		RECT rc, rcTeststring, rcResult;
		int oldSplitterY;

		if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_SPLITTER)) {
			GetClientRect(hwndDlg, &rc);
			pt.x = 0;
			pt.y = wParam;
			ScreenToClient(hwndDlg, &pt);
			oldSplitterY = dat->splitterPos;
			dat->splitterPos = rc.bottom - pt.y + 223;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_TESTSTRING), &rcTeststring);
			GetWindowRect(GetDlgItem(hwndDlg, IDC_RESULT), &rcResult);
			if ((((rcTeststring.bottom - rcTeststring.top) - (dat->splitterPos - oldSplitterY)) < dat->minInputSize.y) || (((rcResult.bottom - rcResult.top) + (dat->splitterPos - oldSplitterY)) < dat->minResultSize.y))
				dat->splitterPos = oldSplitterY;
		}	}

		SendMessage(hwndDlg, WM_SIZE, 0, 0);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_TESTSTRING:
			if (HIWORD(wParam) == EN_CHANGE)
				SendMessage(hwndDlg, VARM_PARSE, 0, 0);
			break;
		}
		break;

	case WM_GETMINMAXINFO:
	{
		RECT rc, rcTeststring;
		GetWindowRect(GetParent(hwndDlg), &rc);
		GetWindowRect(GetDlgItem(hwndDlg, IDC_TESTSTRING), &rcTeststring);
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = (rc.bottom - rc.top) - (((rcTeststring.bottom - rcTeststring.top) - dat->minResultSize.y));
	}
		break;

	case WM_SIZE:
		if (!IsIconic(hwndDlg)) {
			UTILRESIZEDIALOG urd = { 0 };
			urd.cbSize = sizeof(urd);
			urd.hInstance = hInst;
			urd.hwndDlg = hwndDlg;
			urd.lParam = (LPARAM)dat;
			urd.lpTemplate = MAKEINTRESOURCEA(IDD_INPUT_DIALOG);
			urd.pfnResizer = inputDialogResize;
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);
		}
		SendMessage(hwndDlg, WM_MOVE, 0, 0);
		break;

	case WM_SHOWWINDOW:
		if (wParam)
			SendMessage(hwndDlg, VARM_PARSE, 0, 0);
		break;

	case WM_NOTIFY:
		break;

	case VARM_PARSE:
		{
			TCHAR *string = Hlp_GetDlgItemText(hwndDlg, IDC_TESTSTRING), *extraText;
			int len = SendMessage(GetParent(hwndDlg), VARM_GETEXTRATEXTLENGTH, 0, 0);
			if (len > 0) {
				extraText = (TCHAR*)mir_calloc((len + 1)* sizeof(TCHAR));
				SendMessage(GetParent(hwndDlg), VARM_GETEXTRATEXT, (WPARAM)len + 1, (LPARAM)extraText);
			}
			else extraText = NULL;

			if (string != NULL) {
				TCHAR *newString = variables_parsedup(string, extraText, (MCONTACT)SendMessage(GetParent(hwndDlg), VARM_GETSUBJECT, 0, 0));
				if (newString != NULL) {
					TCHAR *oldString = Hlp_GetDlgItemText(hwndDlg, IDC_RESULT);
					if (oldString == NULL || _tcscmp(oldString, newString))
						SetWindowText(GetDlgItem(hwndDlg, IDC_RESULT), newString);

					mir_free(newString);
					if (oldString != NULL)
						mir_free(oldString);
				}
				mir_free(string);
			}
		}
		break;

	case VARM_SETINPUTTEXT:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT,
			SendMessage(GetDlgItem(hwndDlg, IDC_TESTSTRING), WM_SETTEXT, wParam, lParam));
		return TRUE;

	case VARM_GETINPUTTEXTLENGTH:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT,
			SendMessage(GetDlgItem(hwndDlg, IDC_TESTSTRING), WM_GETTEXTLENGTH, wParam, lParam));
		return TRUE;

	case VARM_GETINPUTTEXT:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT,
			SendMessage(GetDlgItem(hwndDlg, IDC_TESTSTRING), WM_GETTEXT, wParam, lParam));
		return TRUE;

	case WM_TIMER:
		SendMessage(hwndDlg, VARM_PARSE, 0, 0);
		break;

	case WM_DESTROY:
		KillTimer(hwndDlg, IDT_PARSE);
		if (dat == NULL)
			break;
		db_set_dw(NULL, MODULENAME, SETTING_SPLITTERPOS, dat->splitterPos);
		mir_free(dat);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		break;
	}

	return FALSE;
}

// help info dialog
static INT_PTR CALLBACK helpInfoDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		SetDlgItemText(hwndDlg, IDC_HELPDESC, VAR_HELP_TEXT);
		TranslateDialogDefault(hwndDlg);
		break;

	case WM_SIZE:
		if (!IsIconic( hwndDlg )) {
			UTILRESIZEDIALOG urd = { 0 };
			urd.cbSize = sizeof(urd);
			urd.hInstance = hInst;
			urd.hwndDlg = hwndDlg;
			urd.lParam = 0;
			urd.lpTemplate = MAKEINTRESOURCEA(IDD_HELPINFO_DIALOG);
			urd.pfnResizer = defaultHelpDialogResize;
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);

			SendMessage(hwndDlg, WM_MOVE, 0, 0);
		}
		break;
	}

	return FALSE;
}

// the parent page (tabs)
static int helpDialogResize(HWND hwndDlg, LPARAM lParam, UTILRESIZECONTROL *urc)
{
	INPUTDLGDATA *dat = (INPUTDLGDATA *)lParam;
	switch(urc->wId) {
	case IDC_OK:
	case IDC_CANCEL:
		return RD_ANCHORX_RIGHT|RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_WIDTH|RD_ANCHORY_HEIGHT;
}

static INT_PTR CALLBACK helpDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	HELPDLGDATA *dat = (HELPDLGDATA *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			hwndHelpDialog = hwndDlg;
			dat = (HELPDLGDATA*)mir_calloc(sizeof(HELPDLGDATA));
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			dat->vhs = (VARHELPINFO *)lParam;

			// set tabs
			int tabCount = 0;
			RECT rcTabs, rcParent;
			HWND hTab = GetDlgItem(hwndDlg, IDC_TABS);
			GetWindowRect(hTab, &rcTabs);
			GetWindowRect(hwndDlg, &rcParent);

			TCITEM tci = { 0 };
			HWND hPage, hShow = 0;
			if (dat->vhs->flags&VHF_TOKENS) {
				// token tab
				tci.mask = TCIF_TEXT | TCIF_PARAM;
				tci.pszText = TranslateT("Tokens");
				hPage = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_TOKENS_DIALOG), hwndDlg, tokenHelpDlgProc, (LPARAM)GetParent(hwndDlg));
				EnableThemeDialogTexture(hPage, ETDT_ENABLETAB);

				tci.lParam = (LPARAM)hPage;
				MoveWindow(hPage, (rcTabs.left - rcParent.left), (rcTabs.top - rcParent.top), (rcTabs.right - rcTabs.left) - 2 * iFrameX, (rcTabs.bottom - rcTabs.top) - 2 * iFrameY, TRUE);
				ShowWindow(hPage, SW_HIDE);
				TabCtrl_InsertItem(hTab, tabCount++, &tci);
				hShow = hShow == 0 ? hPage : hShow;
			}
			if (dat->vhs->flags&VHF_INPUT) {
				// input tab
				tci.mask = TCIF_TEXT | TCIF_PARAM;
				tci.pszText = TranslateT("Input");
				dat->hwndInputDlg = hPage = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_INPUT_DIALOG), hwndDlg, inputDlgProc, (LPARAM)GetParent(hwndDlg));
				EnableThemeDialogTexture(hPage, ETDT_ENABLETAB);

				tci.lParam = (LPARAM)hPage;
				MoveWindow(hPage, (rcTabs.left - rcParent.left), (rcTabs.top - rcParent.top), (rcTabs.right - rcTabs.left) - 2 * iFrameX, (rcTabs.bottom - rcTabs.top) - 2 * iFrameY, TRUE);
				ShowWindow(hPage, SW_HIDE);
				TabCtrl_InsertItem(hTab, tabCount++, &tci);
				hShow = hShow == 0 ? hPage : hShow;
				if ((dat->vhs->fi != NULL) && (dat->vhs->fi->szFormat != NULL)) {
					if (dat->vhs->fi->flags & FIF_UNICODE)
						SendMessage(hwndDlg, VARM_SETINPUTTEXT, 0, (LPARAM)dat->vhs->fi->tszFormat);
					else {

						WCHAR *wszFormatString = mir_a2t(dat->vhs->fi->szFormat);
						SendMessage(hwndDlg, VARM_SETINPUTTEXT, 0, (LPARAM)wszFormatString);
						mir_free(wszFormatString);

					}
				}
				else if (dat->vhs->hwndCtrl != NULL) {
					TCHAR *tszText = Hlp_GetWindowText(dat->vhs->hwndCtrl);
					if (tszText != NULL) {
						SendMessage(hwndDlg, VARM_SETINPUTTEXT, 0, (LPARAM)tszText);
						mir_free(tszText);
					}
				}
				if (dat->vhs->fi != NULL || dat->vhs->hwndCtrl != NULL) {
					SetWindowText(GetDlgItem(hwndDlg, IDC_CANCEL), TranslateT("Cancel"));
					ShowWindow(GetDlgItem(hwndDlg, IDC_OK), SW_SHOW);
				}
			}
			if ((dat->vhs->flags&VHF_SUBJECT) ||
				((dat->vhs->flags&VHF_INPUT) && (((dat->vhs->fi != NULL) && (dat->vhs->fi->hContact != NULL)) || (dat->vhs->flags&VHF_SETLASTSUBJECT)))) {
				// subject window is created, but not necessarily shown
				dat->hwndSubjectDlg = hPage = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_CLIST_DIALOG), hwndDlg, clistDlgProc, (LPARAM)GetParent(hwndDlg));
				EnableThemeDialogTexture(hPage, ETDT_ENABLETAB);

				MoveWindow(hPage, (rcTabs.left - rcParent.left), (rcTabs.top - rcParent.top), (rcTabs.right - rcTabs.left) - 2 * iFrameX, (rcTabs.bottom - rcTabs.top) - 2 * iFrameY, TRUE);
				ShowWindow(hPage, SW_HIDE);

				if ((dat->vhs->fi != NULL) && (dat->vhs->fi->hContact != NULL))
					SendMessage(hwndDlg, VARM_SETSUBJECT, (WPARAM)dat->vhs->fi->hContact, 0);
				else if (dat->vhs->flags&VHF_SETLASTSUBJECT)
					SendMessage(hwndDlg, VARM_SETSUBJECT, (WPARAM)INVALID_HANDLE_VALUE, 0);

				if (dat->vhs->flags&VHF_SUBJECT) {
					// create subject tab
					tci.lParam = (LPARAM)hPage;
					tci.mask = TCIF_TEXT | TCIF_PARAM;
					tci.pszText = TranslateT("%subject%");
					TabCtrl_InsertItem(hTab, tabCount++, &tci);
					hShow = hShow == 0 ? hPage : hShow;
				}
			}
			if ((dat->vhs->flags&VHF_EXTRATEXT) ||
				((dat->vhs->flags&VHF_INPUT) && (dat->vhs->fi != NULL) && (dat->vhs->fi->tszExtraText != NULL))) {
				// extratext window is created, but not necessarily shown
				dat->hwndExtraTextDlg = hPage = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_EXTRATEXT_DIALOG), hwndDlg, extratextDlgProc, (LPARAM)GetParent(hwndDlg));
				EnableThemeDialogTexture(hPage, ETDT_ENABLETAB);

				MoveWindow(hPage, (rcTabs.left - rcParent.left), (rcTabs.top - rcParent.top), (rcTabs.right - rcTabs.left) - 2 * iFrameX, (rcTabs.bottom - rcTabs.top) - 2 * iFrameY, TRUE);
				ShowWindow(hPage, SW_HIDE);
				if ((dat->vhs->fi != NULL) && (dat->vhs->fi->tszExtraText != NULL)) {
					if (dat->vhs->fi->flags & FIF_UNICODE)
						SendMessage(hwndDlg, VARM_SETEXTRATEXT, 0, (LPARAM)dat->vhs->fi->tszExtraText);
					else {

						WCHAR *wszSource = mir_a2t(dat->vhs->fi->szExtraText);
						SendMessage(hwndDlg, VARM_SETEXTRATEXT, 0, (LPARAM)wszSource);
						mir_free(wszSource);

					}
				}
				if (dat->vhs->flags&VHF_EXTRATEXT) {
					// create extratext tab
					tci.mask = TCIF_TEXT | TCIF_PARAM;
					tci.pszText = TranslateT("%extratext%");
					tci.lParam = (LPARAM)hPage;
					TabCtrl_InsertItem(hTab, tabCount++, &tci);
					hShow = hShow == 0 ? hPage : hShow;
				}
			}
			if (dat->vhs->flags&VHF_HELP) {
				// helpinfo tab
				tci.mask = TCIF_TEXT | TCIF_PARAM;
				tci.pszText = TranslateT("Help");
				hPage = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_HELPINFO_DIALOG), hwndDlg, helpInfoDlgProc, (LPARAM)GetParent(hwndDlg));
				EnableThemeDialogTexture(hPage, ETDT_ENABLETAB);

				tci.lParam = (LPARAM)hPage;
				MoveWindow(hPage, (rcTabs.left - rcParent.left), (rcTabs.top - rcParent.top), (rcTabs.right - rcTabs.left) - 2 * iFrameX, (rcTabs.bottom - rcTabs.top) - 2 * iFrameY, TRUE);
				ShowWindow(hPage, SW_HIDE);
				TabCtrl_InsertItem(hTab, tabCount++, &tci);
				hShow = hShow == 0 ? hPage : hShow;
			}
			Utils_RestoreWindowPositionNoMove(hwndDlg, NULL, MODULENAME, "help");
			SetWindowText(hwndDlg, TranslateT("Variables Help"));
			ShowWindow(hShow, SW_SHOW);
			break;
		}

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_OK:
			if ((dat->vhs->fi != NULL) && (!(dat->vhs->flags&VHF_DONTFILLSTRUCT))) {
				int len = SendMessage(hwndDlg, VARM_GETINPUTTEXTLENGTH, 0, 0);
				if (len > 0) {
					if ((dat->vhs->fi != NULL) && (!(dat->vhs->flags&VHF_DONTFILLSTRUCT))) {
						if (dat->vhs->fi->flags&FIF_UNICODE) {
							dat->vhs->fi->tszFormat = (TCHAR*)mir_calloc((len + 1)*sizeof(WCHAR));
							SendMessage(hwndDlg, VARM_GETINPUTTEXT, (WPARAM)len + 1, (LPARAM)dat->vhs->fi->tszFormat);
						}
						else {
							dat->vhs->fi->szFormat = (char*)mir_calloc(len + 1);
							SendMessageA(hwndDlg, VARM_GETINPUTTEXT, (WPARAM)len + 1, (LPARAM)dat->vhs->fi->szFormat);
						}
					}
				}
			}

			if (dat->vhs->hwndCtrl != NULL) {
				int len = SendMessage(hwndDlg, VARM_GETINPUTTEXTLENGTH, 0, 0);
				if (len > 0) {
					TCHAR *tszText;

					tszText = (TCHAR*)mir_calloc((len + 1)*sizeof(TCHAR));
					if (tszText != NULL) {
						SendMessage(hwndDlg, VARM_GETINPUTTEXT, (WPARAM)len + 1, (LPARAM)tszText);
						SetWindowText(dat->vhs->hwndCtrl, tszText);
						mir_free(tszText);
					}
				}
				SendMessage(GetParent(dat->vhs->hwndCtrl),
					WM_COMMAND,
					MAKEWPARAM(GetDlgCtrlID(dat->vhs->hwndCtrl),
					EN_CHANGE),
					(LPARAM)dat->vhs->hwndCtrl);
			}

			if ((dat->vhs->flags&VHF_FULLFILLSTRUCT) && (dat->vhs->fi != NULL)) {
				int len = SendMessage(hwndDlg, VARM_GETEXTRATEXTLENGTH, 0, 0);
				if (len > 0) {
					if (dat->vhs->fi->flags&FIF_UNICODE) {
						dat->vhs->fi->tszExtraText = (TCHAR*)mir_calloc((len + 1)*sizeof(WCHAR));
						SendMessage(hwndDlg, VARM_GETEXTRATEXT, (WPARAM)len + 1, (LPARAM)dat->vhs->fi->tszExtraText);
					}
					else {
						dat->vhs->fi->szExtraText = (char*)mir_calloc(len + 1);
						SendMessageA(hwndDlg, VARM_GETEXTRATEXT, (WPARAM)len + 1, (LPARAM)dat->vhs->fi->szExtraText);
					}
				}
				dat->vhs->fi->hContact = (MCONTACT)SendMessage(hwndDlg, VARM_GETSUBJECT, 0, 0);
			}
			// fall through

		case IDC_CANCEL:
			if (GetParent(hwndDlg) == NULL)
				DestroyWindow(hwndDlg);
			else
				EndDialog(hwndDlg, 0);
			break;
		}
		break;

	case VARM_SETSUBJECT:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT,
			SendMessage(dat->hwndSubjectDlg, VARM_SETSUBJECT, wParam, lParam));
		return TRUE;

	case VARM_GETSUBJECT:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT,
			SendMessage(dat->hwndSubjectDlg, VARM_GETSUBJECT, wParam, lParam));
		return TRUE;

	case VARM_SETEXTRATEXT:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT,
			SendMessage(dat->hwndExtraTextDlg, VARM_SETEXTRATEXT, wParam, lParam));
		return TRUE;

	case VARM_GETEXTRATEXTLENGTH:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT,
			SendMessage(dat->hwndExtraTextDlg, VARM_GETEXTRATEXTLENGTH, wParam, lParam));
		return TRUE;

	case VARM_GETEXTRATEXT:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT,
			SendMessage(dat->hwndExtraTextDlg, VARM_GETEXTRATEXT, wParam, lParam));
		return TRUE;

	case VARM_SETINPUTTEXT:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT,
			SendMessage(dat->hwndInputDlg, VARM_SETINPUTTEXT, wParam, lParam));
		return TRUE;

	case VARM_GETINPUTTEXTLENGTH:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT,
			SendMessage(dat->hwndInputDlg, VARM_GETINPUTTEXTLENGTH, wParam, lParam));
		return TRUE;

	case VARM_GETINPUTTEXT:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT,
			SendMessage(dat->hwndInputDlg, VARM_GETINPUTTEXT, wParam, lParam));
		return TRUE;

	case VARM_GETDIALOG:
		switch (wParam) {
		case VHF_INPUT:
			SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)dat->hwndInputDlg);
			return TRUE;
		}
		break;

	case WM_GETMINMAXINFO:
		{
			int i, count;
			TCITEM tci;
			RECT rcParent;
			HWND hTab;
			MINMAXINFO pageMinMax;

			GetWindowRect(hwndDlg, &rcParent);
			// defaults
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 400;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 400;
			hTab = GetDlgItem(hwndDlg, IDC_TABS);
			tci.mask = TCIF_PARAM;
			count = TabCtrl_GetItemCount(hTab);
			// return the largest of all pages
			for (i = 0; i < count; i++) {
				TabCtrl_GetItem(hTab, i, &tci);
				memset(&pageMinMax, 0, sizeof(pageMinMax));
				SendMessage((HWND)tci.lParam, WM_GETMINMAXINFO, wParam, (LPARAM)&pageMinMax);
				((MINMAXINFO*)lParam)->ptMinTrackSize.x = max(((MINMAXINFO*)lParam)->ptMinTrackSize.x, pageMinMax.ptMinTrackSize.x);
				((MINMAXINFO*)lParam)->ptMinTrackSize.y = max(((MINMAXINFO*)lParam)->ptMinTrackSize.y, pageMinMax.ptMinTrackSize.y);
			}
		}
		break;

	case WM_SIZE:
		if (!IsIconic(hwndDlg)) {
			UTILRESIZEDIALOG urd = { 0 };
			urd.cbSize = sizeof(urd);
			urd.hInstance = hInst;
			urd.hwndDlg = hwndDlg;
			urd.lParam = 0;
			// ! uses ANSI version !
			urd.lpTemplate = MAKEINTRESOURCEA(IDD_HELP_DIALOG);
			urd.pfnResizer = helpDialogResize;
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);

			SendMessage(hwndDlg, WM_MOVE, 0, 0);

			RECT rcTabs, rcParent;
			HWND hTab = GetDlgItem(hwndDlg, IDC_TABS);
			GetWindowRect(hTab, &rcTabs);
			GetWindowRect(hwndDlg, &rcParent);

			TCITEM tci;
			tci.mask = TCIF_PARAM;
			int count = TabCtrl_GetItemCount(hTab);
			for (int i = 0; i < count; i++) {
				TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_TABS), i, &tci);
				MoveWindow((HWND)tci.lParam, (rcTabs.left - rcParent.left), (rcTabs.top - rcParent.top), (rcTabs.right - rcTabs.left) - 2 * iFrameX, (rcTabs.bottom - rcTabs.top) - 2 * iFrameY, TRUE);
			}
		}
		break;

	case WM_NOTIFY:
		if ((((NMHDR*)lParam)->idFrom == IDC_TABS)) {
			if (((NMHDR*)lParam)->code == TCN_SELCHANGING) {
				TCITEM tci;

				tci.mask = TCIF_PARAM;
				TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_TABS), TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_TABS)), &tci);
				ShowWindow((HWND)tci.lParam, SW_HIDE);
			}
			else if (((NMHDR*)lParam)->code == TCN_SELCHANGE) {
				TCITEM tci;

				tci.mask = TCIF_PARAM;
				TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_TABS), TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_TABS)), &tci);
				ShowWindow((HWND)tci.lParam, SW_SHOW);
			}
		}
		break;

	case WM_CLOSE:
		if (GetParent(hwndDlg) == NULL)
			DestroyWindow(hwndDlg);
		else
			EndDialog(hwndDlg, 0);
		break;

	case WM_DESTROY:
		Utils_SaveWindowPosition(hwndDlg, NULL, MODULENAME, "help");

		HWND hTab = GetDlgItem(hwndDlg, IDC_TABS);

		TCITEM tci;
		tci.mask = TCIF_PARAM;
		int count = TabCtrl_GetItemCount(hTab);
		for (int i = 0; i < count; i++) {
			TabCtrl_GetItem(hTab, i, &tci);
			if (((HWND)tci.lParam != dat->hwndSubjectDlg) && ((HWND)tci.lParam != dat->hwndExtraTextDlg))
				DestroyWindow((HWND)tci.lParam);
		}

		// these windows might have been created, but not inserted as tabs
		if (IsWindow(dat->hwndSubjectDlg))
			DestroyWindow(dat->hwndSubjectDlg);

		if (IsWindow(dat->hwndExtraTextDlg))
			DestroyWindow(dat->hwndExtraTextDlg);

		mir_free(dat);
		dat = NULL;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		break;
	}

	return FALSE;
}

INT_PTR showHelpExService(WPARAM wParam, LPARAM lParam)
{
	if (IsWindow(hwndHelpDialog)) {
		SetFocus(hwndHelpDialog);
		return 0;
	}
	if (lParam == 0)
		return -1;

	iFrameX = GetSystemMetrics(SM_CXSIZEFRAME);
	iFrameY = 3 * GetSystemMetrics(SM_CYSIZEFRAME);

	if (wParam)
		DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_HELP_DIALOG), (HWND)wParam, helpDlgProc, (LPARAM)lParam);
	else
		CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_HELP_DIALOG), NULL, helpDlgProc, (LPARAM)lParam);

	return 0;
}

INT_PTR showHelpService(WPARAM wParam, LPARAM lParam)
{
	static VARHELPINFO *vhs = NULL;
	static FORMATINFO *fi = NULL;

	if (fi == NULL)
		fi = (FORMATINFO*)mir_alloc(sizeof(FORMATINFO));

	memset(fi, 0, sizeof(FORMATINFO));
	fi->cbSize = sizeof(FORMATINFO);
	fi->szFormat = (char *)lParam;
	if (vhs == NULL)
		vhs = (VARHELPINFO*)mir_alloc(sizeof(VARHELPINFO));

	memset(vhs, 0, sizeof(VARHELPINFO));
	vhs->cbSize = sizeof(VARHELPINFO);
	vhs->fi = fi;
	vhs->hwndCtrl = (HWND)wParam;
	vhs->flags = VHF_FULLDLG | VHF_DONTFILLSTRUCT;

	return showHelpExService(0, (LPARAM)vhs);
}

INT_PTR getSkinItemService(WPARAM wParam, LPARAM lParam)
{
	int item = lParam;
	if (item == 0)
		return (INT_PTR)NULL;

	switch (item) {
	case VSI_HELPICON:
		if (hHelpIcon == NULL)
			hHelpIcon = Skin_GetIcon("vars_help");
		return (INT_PTR)hHelpIcon;

	case VSI_HELPTIPTEXT:
		return (INT_PTR)Translate("Open String Formatting Help");
	}

	return (INT_PTR)NULL;
}

int iconsChanged(WPARAM wParam, LPARAM lParam)
{
	hHelpIcon = NULL;
	return 0;
}
