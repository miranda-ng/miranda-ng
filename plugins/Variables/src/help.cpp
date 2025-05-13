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

#include "stdafx.h"

struct HELPDLGDATA
{
	VARHELPINFO *vhs;
	HWND hwndSubjectDlg;
	HWND hwndExtraTextDlg;
	HWND hwndInputDlg;
};

static INT_PTR CALLBACK inputDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam);

extern HCURSOR hCurSplitNS;

static HWND hwndHelpDialog = nullptr;

static HICON hHelpIcon = nullptr;

static int defaultHelpDialogResize(HWND, LPARAM, UTILRESIZECONTROL *urc) {

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
			SendDlgItemMessage(hwndDlg, IDC_EXTRATEXT, WM_SETTEXT, wParam, lParam));
		return TRUE;

	case VARM_GETEXTRATEXTLENGTH:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT,
			SendDlgItemMessage(hwndDlg, IDC_EXTRATEXT, WM_GETTEXTLENGTH, wParam, lParam));
		return TRUE;

	case VARM_GETEXTRATEXT:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT,
			GetDlgItemText(hwndDlg, IDC_EXTRATEXT, (LPWSTR)lParam, wParam));
		return TRUE;

	case WM_SIZE:
		if (!IsIconic( hwndDlg )) {
			Utils_ResizeDialog(hwndDlg, g_plugin.getInst(), MAKEINTRESOURCEA(IDD_EXTRATEXT_DIALOG), defaultHelpDialogResize);
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
	SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETUSEGROUPS, Clist::bUseGroups, 0);
	SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETHIDEEMPTYGROUPS, 1, 0);
}

static int clistDialogResize(HWND, LPARAM, UTILRESIZECONTROL *urc)
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
			log_debug(0, "VARM_SETSUBJECT: %u", hContact);
			if (hContact == INVALID_CONTACT_ID) {
				wchar_t *tszContact = g_plugin.getWStringA(SETTING_SUBJECT);
				log_debug(0, "VARM_SETSUBJECT: %s", tszContact);
				if (tszContact != nullptr) {
					hContact = getContactFromString(tszContact, CI_PROTOID);
					log_debug(0, "VARM_SETSUBJECT decoded: %u", hContact);
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
			Utils_ResizeDialog(hwndDlg, g_plugin.getInst(), MAKEINTRESOURCEA(IDD_CLIST_DIALOG), clistDialogResize);
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
		g_plugin.delSetting(SETTING_SUBJECT);

		MCONTACT hContact = (MCONTACT)SendMessage(hwndDlg, VARM_GETSUBJECT, 0, 0);
		if (hContact != NULL) {
			wchar_t *tszContact = encodeContactToString(hContact);
			if (tszContact != nullptr) {
				g_plugin.setWString(SETTING_SUBJECT, tszContact);
				mir_free(tszContact);
		}	}
		break;
	}

	return FALSE;
}

// dialog box for the tokens
static wchar_t* getTokenCategory(TOKENREGISTEREX *tr) {
	if (tr == nullptr) {
		return nullptr;
	}
	char *helpText = mir_strdup(tr->szHelpText);
	if (helpText == nullptr) {
		return nullptr;
	}
	char *cur = helpText;
	while (*cur != 0) {
		if (*cur == '\t') {
			*cur = 0;
			helpText = ( char* )mir_realloc(helpText, mir_strlen(helpText)+1);

			wchar_t *res = mir_a2u(helpText);
			mir_free(helpText);
			return res;
		}
		cur++;
	}

	wchar_t *res = mir_a2u(helpText);
	mir_free(helpText);
	return res;

}

static wchar_t* getHelpDescription(TOKENREGISTEREX *tr)
{
	if (tr == nullptr)
		return nullptr;

	char *cur = tr->szHelpText + mir_strlen(tr->szHelpText);
	while (cur > tr->szHelpText) {
		if (*cur == '\t') {

			cur = mir_strdup(cur+1);
			wchar_t *res = mir_a2u(cur);
			mir_free(cur);
			return res;

		}
		cur--;
	}

	return mir_a2u(tr->szHelpText);

}

static wchar_t* getTokenDescription(TOKENREGISTEREX *tr)
{
	if (tr == nullptr)
		return nullptr;

	if (tr->szHelpText == nullptr)
		return mir_wstrdup(tr->szTokenString.w);

	char *helpText = mir_strdup(tr->szHelpText);
	if (helpText == nullptr)
		return nullptr;

	char *cur = helpText;
	wchar_t *tArgs = nullptr;
	char *args = nullptr, *first = nullptr, *second = nullptr;
	while (*cur != 0) {
		if (*cur == '\t') {
			if (first == nullptr)
				first = cur;
			else if (second == nullptr)
				second = cur;
		}
		cur++;
	}

	if ((first != nullptr) && (second != nullptr)) {
		*second = 0;
		args = first+1;
	}
	else args = nullptr;

	size_t len = mir_wstrlen(tr->szTokenString.w) + (args!=nullptr?mir_strlen(args):0) + 3;
	wchar_t *desc = (wchar_t*)mir_calloc(len * sizeof(wchar_t));
	if (desc == nullptr) {
		mir_free(helpText);
		return nullptr;
	}

	if (tr->flags&TRF_FIELD)
		mir_snwprintf(desc,  len, L"%c%s%c", FIELD_CHAR, tr->szTokenString.w, FIELD_CHAR);
	else {
		if (args != nullptr)
			tArgs = mir_a2u(args);

		mir_snwprintf(desc,  len, L"%c%s%s", FUNC_CHAR, tr->szTokenString.w, (tArgs!=nullptr?tArgs:L""));
	}

	mir_free(tArgs);
	mir_free(helpText);
	return desc;
}

static int CALLBACK compareTokenHelp(LPARAM lParam1, LPARAM lParam2, LPARAM)
{
	TOKENREGISTEREX *tr1 = (TOKENREGISTEREX *)lParam1;
	TOKENREGISTEREX *tr2 = (TOKENREGISTEREX *)lParam2;
	if (tr1 == nullptr || tr2 == nullptr)
		return 0;

	ptrW cat1(getTokenCategory(tr1));
	ptrW cat2(getTokenCategory(tr2));
	if (cat1 == NULL || cat2 == NULL)
		return 0;

	int res = mir_wstrcmp(cat1, cat2);
	if (res != 0)
		return res;

	if (tr1->szTokenString.w == nullptr || tr2->szTokenString.w == nullptr)
		return 0;

	return mir_wstrcmp(tr1->szTokenString.w, tr2->szTokenString.w);
}

static BOOL CALLBACK processTokenListMessage(HWND hwndDlg, UINT msg, WPARAM, LPARAM lParam)
{
	/* hwndDlg must be a child of the help dialog */
	switch(msg) {
	case WM_INITDIALOG:
		{
			TOKENREGISTEREX *tr;
			wchar_t *tszTokenDesc, *tszHelpDesc, *last, *cat, *text;

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
				i++;
				tszHelpDesc = tszTokenDesc = nullptr;
				tr = getTokenRegister(i);
				if ((tr == nullptr) || (tr->szTokenString.w == nullptr))
					continue;

				else if (hdd != nullptr) {
					if (!mir_wstrcmp(tr->szTokenString.w, SUBJECT)) {
						if (hdd->vhs->flags&VHF_HIDESUBJECTTOKEN)
							continue;

						if (hdd->vhs->szSubjectDesc != nullptr)
							tszHelpDesc = mir_a2u(hdd->vhs->szSubjectDesc);
					}
					if (!mir_wstrcmp(tr->szTokenString.w, MIR_EXTRATEXT)) {
						if (hdd->vhs->flags & VHF_HIDEEXTRATEXTTOKEN)
							continue;

						if (hdd->vhs->szExtraTextDesc != nullptr)
							tszHelpDesc = mir_a2u(hdd->vhs->szExtraTextDesc);
					}
				}

				LVITEM lvItem = { 0 };
				lvItem.mask = LVIF_TEXT | LVIF_PARAM;
				lvItem.iItem = ListView_GetItemCount(hList);
				lvItem.lParam = (LPARAM)tr;
				tszTokenDesc = getTokenDescription(tr);
				if (tszTokenDesc == nullptr)
					continue;

				lvItem.pszText = tszTokenDesc;
				ListView_InsertItem(hList, &lvItem);
				mir_free(tszTokenDesc);

				lvItem.mask = LVIF_TEXT;
				if (tszHelpDesc == nullptr)
					tszHelpDesc = getHelpDescription(tr);

				if (tszHelpDesc == nullptr)
					tszHelpDesc = mir_wstrdup(L"unknown");

				lvItem.iSubItem = 1;
				lvItem.pszText = TranslateW(tszHelpDesc);
				ListView_SetItem(hList, &lvItem);
				mir_free(tszHelpDesc);
			}
				while (tr != nullptr);
	
			ListView_SetColumnWidth(hList, 0, LVSCW_AUTOSIZE);
			ListView_SetColumnWidth(hList, 1, LVSCW_AUTOSIZE);
			ListView_SortItems(hList, compareTokenHelp, 0);
			last = text = nullptr;
			for (i = 0; i < ListView_GetItemCount(hList); i++) {
				LVITEM lvItem = { 0 };
				lvItem.mask = LVIF_PARAM;
				lvItem.iItem = i;
				if (ListView_GetItem(hList, &lvItem) == FALSE)
					continue;

				cat = getTokenCategory((TOKENREGISTEREX *)lvItem.lParam);
				if (cat != nullptr) {
					text = mir_wstrdup(TranslateW(cat));
					mir_free(cat);
				}
				else text = nullptr;

				if (text != nullptr && (last == nullptr || mir_wstrcmpi(last, text))) {
					lvItem.mask = LVIF_TEXT;
					lvItem.pszText = text;
					ListView_InsertItem(hList, &lvItem);
					if (last != nullptr) {
						mir_free(last);
						lvItem.iSubItem = 0;
						lvItem.pszText = L"";
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
			if (hwndInputDlg == nullptr)
				break;

			HWND hList = GetDlgItem(hwndDlg, IDC_TOKENLIST);
			int item = ListView_GetNextItem(hList, -1, LVNI_SELECTED|LVNI_FOCUSED);

			LVITEM lvItem = { 0 };
			lvItem.mask = LVIF_PARAM;
			lvItem.iItem = item;
			if (ListView_GetItem(hList, &lvItem) == FALSE)
				break;

			TOKENREGISTER *tr = (TOKENREGISTER *)lvItem.lParam;
			if (tr == nullptr)
				break;

			size_t len = mir_wstrlen(tr->szTokenString.w) + 2;
			wchar_t *tokenString = (wchar_t*)mir_alloc((len+1)*sizeof(wchar_t));
			if (tokenString == nullptr)
				break;

			memset(tokenString, 0, ((len + 1) * sizeof(wchar_t)));
			mir_snwprintf(tokenString, len + 1, L"%c%s%c", (tr->flags & TRF_FIELD) ? FIELD_CHAR : FUNC_CHAR, tr->szTokenString.w, (tr->flags & TRF_FIELD) ? FIELD_CHAR : '(');
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
			Utils_ResizeDialog(hwndDlg, g_plugin.getInst(), MAKEINTRESOURCEA(IDD_TOKENS_DIALOG), defaultHelpDialogResize);
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

static int inputDialogResize(HWND, LPARAM lParam, UTILRESIZECONTROL *urc)
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
		dat->splitterPos = (INT_PTR)g_plugin.getDword(SETTING_SPLITTERPOS, -1);
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
		SetTimer(hwndDlg, IDT_PARSE, 1000, nullptr);

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
		if (!IsIconic(hwndDlg))
			Utils_ResizeDialog(hwndDlg, g_plugin.getInst(), MAKEINTRESOURCEA(IDD_INPUT_DIALOG), inputDialogResize, (LPARAM)dat);
		
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
			wchar_t *string = Hlp_GetDlgItemText(hwndDlg, IDC_TESTSTRING), *extraText;
			int len = SendMessage(GetParent(hwndDlg), VARM_GETEXTRATEXTLENGTH, 0, 0);
			if (len > 0) {
				extraText = (wchar_t*)mir_calloc((len + 1)* sizeof(wchar_t));
				SendMessage(GetParent(hwndDlg), VARM_GETEXTRATEXT, (WPARAM)len + 1, (LPARAM)extraText);
			}
			else extraText = nullptr;

			if (string != nullptr) {
				wchar_t *newString = variables_parsedup(string, extraText, (MCONTACT)SendMessage(GetParent(hwndDlg), VARM_GETSUBJECT, 0, 0));
				if (newString != nullptr) {
					wchar_t *oldString = Hlp_GetDlgItemText(hwndDlg, IDC_RESULT);
					if (oldString == nullptr || mir_wstrcmp(oldString, newString))
						SetDlgItemText(hwndDlg, IDC_RESULT, newString);

					mir_free(newString);
					if (oldString != nullptr)
						mir_free(oldString);
				}
				mir_free(string);
			}
		}
		break;

	case VARM_SETINPUTTEXT:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT,
			SendDlgItemMessage(hwndDlg, IDC_TESTSTRING, WM_SETTEXT, wParam, lParam));
		return TRUE;

	case VARM_GETINPUTTEXTLENGTH:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT,
			SendDlgItemMessage(hwndDlg, IDC_TESTSTRING, WM_GETTEXTLENGTH, wParam, lParam));
		return TRUE;

	case VARM_GETINPUTTEXT:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT,
			GetDlgItemText(hwndDlg, IDC_TESTSTRING, (LPWSTR)lParam, wParam));
		return TRUE;

	case WM_TIMER:
		SendMessage(hwndDlg, VARM_PARSE, 0, 0);
		break;

	case WM_DESTROY:
		KillTimer(hwndDlg, IDT_PARSE);
		if (dat == nullptr)
			break;
		g_plugin.setDword(SETTING_SPLITTERPOS, dat->splitterPos);
		mir_free(dat);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		break;
	}

	return FALSE;
}

// help info dialog
static INT_PTR CALLBACK helpInfoDlgProc(HWND hwndDlg,UINT msg,WPARAM,LPARAM)
{
	switch(msg) {
	case WM_INITDIALOG:
		SetDlgItemText(hwndDlg, IDC_HELPDESC, VAR_HELP_TEXT);
		TranslateDialogDefault(hwndDlg);
		break;

	case WM_SIZE:
		if (!IsIconic( hwndDlg )) {
			Utils_ResizeDialog(hwndDlg, g_plugin.getInst(), MAKEINTRESOURCEA(IDD_HELPINFO_DIALOG), defaultHelpDialogResize);
			SendMessage(hwndDlg, WM_MOVE, 0, 0);
		}
		break;
	}

	return FALSE;
}

// the parent page (tabs)
static int helpDialogResize(HWND, LPARAM, UTILRESIZECONTROL *urc)
{
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
			HWND hPage, hShow = nullptr;
			if (dat->vhs->flags&VHF_TOKENS) {
				// token tab
				tci.mask = TCIF_TEXT | TCIF_PARAM;
				tci.pszText = TranslateT("Tokens");
				hPage = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_TOKENS_DIALOG), hwndDlg, tokenHelpDlgProc, (LPARAM)GetParent(hwndDlg));
				EnableThemeDialogTexture(hPage, ETDT_ENABLETAB);

				tci.lParam = (LPARAM)hPage;
				MoveWindow(hPage, (rcTabs.left - rcParent.left), (rcTabs.top - rcParent.top), (rcTabs.right - rcTabs.left) - 2 * iFrameX, (rcTabs.bottom - rcTabs.top) - 2 * iFrameY, TRUE);
				ShowWindow(hPage, SW_HIDE);
				TabCtrl_InsertItem(hTab, tabCount++, &tci);
				hShow = hShow == nullptr ? hPage : hShow;
			}
			if (dat->vhs->flags&VHF_INPUT) {
				// input tab
				tci.mask = TCIF_TEXT | TCIF_PARAM;
				tci.pszText = TranslateT("Input");
				dat->hwndInputDlg = hPage = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_INPUT_DIALOG), hwndDlg, inputDlgProc, (LPARAM)GetParent(hwndDlg));
				EnableThemeDialogTexture(hPage, ETDT_ENABLETAB);

				tci.lParam = (LPARAM)hPage;
				MoveWindow(hPage, (rcTabs.left - rcParent.left), (rcTabs.top - rcParent.top), (rcTabs.right - rcTabs.left) - 2 * iFrameX, (rcTabs.bottom - rcTabs.top) - 2 * iFrameY, TRUE);
				ShowWindow(hPage, SW_HIDE);
				TabCtrl_InsertItem(hTab, tabCount++, &tci);
				hShow = hShow == nullptr ? hPage : hShow;
				if ((dat->vhs->fi != nullptr) && (dat->vhs->fi->szFormat.w != nullptr)) {
					if (dat->vhs->fi->flags & FIF_UNICODE)
						SendMessage(hwndDlg, VARM_SETINPUTTEXT, 0, (LPARAM)dat->vhs->fi->szFormat.w);
					else {

						wchar_t *wszFormatString = mir_a2u(dat->vhs->fi->szFormat.a);
						SendMessage(hwndDlg, VARM_SETINPUTTEXT, 0, (LPARAM)wszFormatString);
						mir_free(wszFormatString);

					}
				}
				else if (dat->vhs->hwndCtrl != nullptr) {
					wchar_t *tszText = Hlp_GetWindowText(dat->vhs->hwndCtrl);
					if (tszText != nullptr) {
						SendMessage(hwndDlg, VARM_SETINPUTTEXT, 0, (LPARAM)tszText);
						mir_free(tszText);
					}
				}
				if (dat->vhs->fi != nullptr || dat->vhs->hwndCtrl != nullptr) {
					SetDlgItemText(hwndDlg, IDC_CANCEL, TranslateT("Cancel"));
					ShowWindow(GetDlgItem(hwndDlg, IDC_OK), SW_SHOW);
				}
			}
			if ((dat->vhs->flags&VHF_SUBJECT) ||
				((dat->vhs->flags&VHF_INPUT) && (((dat->vhs->fi != nullptr) && (dat->vhs->fi->hContact != NULL)) || (dat->vhs->flags&VHF_SETLASTSUBJECT)))) {
				// subject window is created, but not necessarily shown
				dat->hwndSubjectDlg = hPage = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CLIST_DIALOG), hwndDlg, clistDlgProc, (LPARAM)GetParent(hwndDlg));
				EnableThemeDialogTexture(hPage, ETDT_ENABLETAB);

				MoveWindow(hPage, (rcTabs.left - rcParent.left), (rcTabs.top - rcParent.top), (rcTabs.right - rcTabs.left) - 2 * iFrameX, (rcTabs.bottom - rcTabs.top) - 2 * iFrameY, TRUE);
				ShowWindow(hPage, SW_HIDE);

				if ((dat->vhs->fi != nullptr) && (dat->vhs->fi->hContact != NULL))
					SendMessage(hwndDlg, VARM_SETSUBJECT, (WPARAM)dat->vhs->fi->hContact, 0);
				else if (dat->vhs->flags&VHF_SETLASTSUBJECT)
					SendMessage(hwndDlg, VARM_SETSUBJECT, (WPARAM)INVALID_HANDLE_VALUE, 0);

				if (dat->vhs->flags&VHF_SUBJECT) {
					// create subject tab
					tci.lParam = (LPARAM)hPage;
					tci.mask = TCIF_TEXT | TCIF_PARAM;
					tci.pszText = TranslateT("%subject%");
					TabCtrl_InsertItem(hTab, tabCount++, &tci);
					hShow = hShow == nullptr ? hPage : hShow;
				}
			}
			if ((dat->vhs->flags&VHF_EXTRATEXT) ||
				((dat->vhs->flags&VHF_INPUT) && (dat->vhs->fi != nullptr) && (dat->vhs->fi->szExtraText.w != nullptr))) {
				// extratext window is created, but not necessarily shown
				dat->hwndExtraTextDlg = hPage = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_EXTRATEXT_DIALOG), hwndDlg, extratextDlgProc, (LPARAM)GetParent(hwndDlg));
				EnableThemeDialogTexture(hPage, ETDT_ENABLETAB);

				MoveWindow(hPage, (rcTabs.left - rcParent.left), (rcTabs.top - rcParent.top), (rcTabs.right - rcTabs.left) - 2 * iFrameX, (rcTabs.bottom - rcTabs.top) - 2 * iFrameY, TRUE);
				ShowWindow(hPage, SW_HIDE);
				if ((dat->vhs->fi != nullptr) && (dat->vhs->fi->szExtraText.w != nullptr)) {
					if (dat->vhs->fi->flags & FIF_UNICODE)
						SendMessage(hwndDlg, VARM_SETEXTRATEXT, 0, (LPARAM)dat->vhs->fi->szExtraText.w);
					else {

						wchar_t *wszSource = mir_a2u(dat->vhs->fi->szExtraText.a);
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
					hShow = hShow == nullptr ? hPage : hShow;
				}
			}
			if (dat->vhs->flags&VHF_HELP) {
				// helpinfo tab
				tci.mask = TCIF_TEXT | TCIF_PARAM;
				tci.pszText = TranslateT("Help");
				hPage = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_HELPINFO_DIALOG), hwndDlg, helpInfoDlgProc, (LPARAM)GetParent(hwndDlg));
				EnableThemeDialogTexture(hPage, ETDT_ENABLETAB);

				tci.lParam = (LPARAM)hPage;
				MoveWindow(hPage, (rcTabs.left - rcParent.left), (rcTabs.top - rcParent.top), (rcTabs.right - rcTabs.left) - 2 * iFrameX, (rcTabs.bottom - rcTabs.top) - 2 * iFrameY, TRUE);
				ShowWindow(hPage, SW_HIDE);
				TabCtrl_InsertItem(hTab, tabCount++, &tci);
				hShow = hShow == nullptr ? hPage : hShow;
			}
			Utils_RestoreWindowPositionNoMove(hwndDlg, NULL, MODULENAME, "help");
			SetWindowText(hwndDlg, TranslateT("Variables Help"));
			ShowWindow(hShow, SW_SHOW);
			break;
		}

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_OK:
			if ((dat->vhs->fi != nullptr) && (!(dat->vhs->flags&VHF_DONTFILLSTRUCT))) {
				int len = SendMessage(hwndDlg, VARM_GETINPUTTEXTLENGTH, 0, 0);
				if (len > 0) {
					if ((dat->vhs->fi != nullptr) && (!(dat->vhs->flags&VHF_DONTFILLSTRUCT))) {
						if (dat->vhs->fi->flags&FIF_UNICODE) {
							dat->vhs->fi->szFormat.w = (wchar_t*)mir_calloc((len + 1)*sizeof(wchar_t));
							SendMessage(hwndDlg, VARM_GETINPUTTEXT, (WPARAM)len + 1, (LPARAM)dat->vhs->fi->szFormat.w);
						}
						else {
							dat->vhs->fi->szFormat.a = (char*)mir_calloc(len + 1);
							SendMessageA(hwndDlg, VARM_GETINPUTTEXT, (WPARAM)len + 1, (LPARAM)dat->vhs->fi->szFormat.a);
						}
					}
				}
			}

			if (dat->vhs->hwndCtrl != nullptr) {
				int len = SendMessage(hwndDlg, VARM_GETINPUTTEXTLENGTH, 0, 0);
				if (len > 0) {
					wchar_t *tszText;

					tszText = (wchar_t*)mir_calloc((len + 1)*sizeof(wchar_t));
					if (tszText != nullptr) {
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

			if ((dat->vhs->flags&VHF_FULLFILLSTRUCT) && (dat->vhs->fi != nullptr)) {
				int len = SendMessage(hwndDlg, VARM_GETEXTRATEXTLENGTH, 0, 0);
				if (len > 0) {
					if (dat->vhs->fi->flags&FIF_UNICODE) {
						dat->vhs->fi->szExtraText.w = (wchar_t*)mir_calloc((len + 1)*sizeof(wchar_t));
						SendMessage(hwndDlg, VARM_GETEXTRATEXT, (WPARAM)len + 1, (LPARAM)dat->vhs->fi->szExtraText.w);
					}
					else {
						dat->vhs->fi->szExtraText.a = (char*)mir_calloc(len + 1);
						SendMessageA(hwndDlg, VARM_GETEXTRATEXT, (WPARAM)len + 1, (LPARAM)dat->vhs->fi->szExtraText.a);
					}
				}
				dat->vhs->fi->hContact = (MCONTACT)SendMessage(hwndDlg, VARM_GETSUBJECT, 0, 0);
			}
			__fallthrough;

		case IDC_CANCEL:
			if (GetParent(hwndDlg) == nullptr)
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
			Utils_ResizeDialog(hwndDlg, g_plugin.getInst(), MAKEINTRESOURCEA(IDD_HELP_DIALOG), helpDialogResize);
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
		if (GetParent(hwndDlg) == nullptr)
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
		dat = nullptr;
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
		DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_HELP_DIALOG), (HWND)wParam, helpDlgProc, (LPARAM)lParam);
	else
		CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_HELP_DIALOG), nullptr, helpDlgProc, (LPARAM)lParam);

	return 0;
}

INT_PTR showHelpService(WPARAM wParam, LPARAM lParam)
{
	static VARHELPINFO *vhs = nullptr;
	static FORMATINFO *fi = nullptr;

	if (fi == nullptr)
		fi = (FORMATINFO*)mir_alloc(sizeof(FORMATINFO));

	memset(fi, 0, sizeof(FORMATINFO));
	fi->cbSize = sizeof(FORMATINFO);
	fi->szFormat.a = (char *)lParam;
	if (vhs == nullptr)
		vhs = (VARHELPINFO*)mir_alloc(sizeof(VARHELPINFO));

	memset(vhs, 0, sizeof(VARHELPINFO));
	vhs->cbSize = sizeof(VARHELPINFO);
	vhs->fi = fi;
	vhs->hwndCtrl = (HWND)wParam;
	vhs->flags = VHF_FULLDLG | VHF_DONTFILLSTRUCT;

	return showHelpExService(0, (LPARAM)vhs);
}

INT_PTR getSkinItemService(WPARAM, LPARAM lParam)
{
	int item = lParam;
	if (item == 0)
		return (INT_PTR)NULL;

	switch (item) {
	case VSI_HELPICON:
		if (hHelpIcon == nullptr)
			hHelpIcon = IcoLib_GetIcon("vars_help");
		return (INT_PTR)hHelpIcon;

	case VSI_HELPTIPTEXT:
		return (INT_PTR)Translate("Open String Formatting Help");
	}

	return (INT_PTR)NULL;
}

int iconsChanged(WPARAM, LPARAM)
{
	hHelpIcon = nullptr;
	return 0;
}
