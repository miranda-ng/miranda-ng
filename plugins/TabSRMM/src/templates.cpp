/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-17 Miranda NG project,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// Simple editor for the message log templates

#include "stdafx.h"

/*
* hardcoded default set of templates for both LTR and RTL.
* cannot be changed and may be used at any time to "revert" to a working layout
*/

char *TemplateNames[] =
{
	LPGEN("Message In"),
	LPGEN("Message Out"),
	LPGEN("Group In (Start)"),
	LPGEN("Group Out (Start)"),
	LPGEN("Group In (Inner)"),
	LPGEN("Group Out (Inner)"),
	LPGEN("Status change"),
	LPGEN("Error message")
};

TTemplateSet LTR_Default = 
{
	TRUE,
	L"%I %N  %?&r%\\&E%\\!, %\\T%\\!: %?n%?S %?T%?|%M",
	L"%I %N  %?&r%\\&E%\\!, %\\T%\\!: %?n%?S %?T%?|%M",
	L"%I %N  %?&r%\\&E%\\!, %\\T%\\!: %?n%?S %?T%?|%M",
	L"%I %N  %?&r%\\&E%\\!, %\\T%\\!: %?n%?S %?T%?|%M",
	L"%S %T%|%M",
	L"%S %T%|%M",
	L"%I %S %&r, %&T, %N %M%! ",
	L"%I%S %r, %T, %e%l%M",
	"Default LTR"
};

TTemplateSet RTL_Default =
{
	TRUE,
	L"%I %N  %r%n%S %T%|%M",
	L"%I %N  %r%n%S %T%|%M",
	L"%I %N  %r%n%S %T%|%M",
	L"%I %N  %r%n%S %T%|%M",
	L"%S %T%|%M",
	L"%S %T%|%M",
	L"%I%S %r, %T, %N %M%! ",
	L"%I%S %r, %T, %e%l%M",
	"Default RTL"
};

TTemplateSet LTR_Active, RTL_Active;
static int                      helpActive = 0;


/*
* loads template set overrides from hContact into the given set of already existing
* templates
*/

static void LoadTemplatesFrom(TTemplateSet *tSet, MCONTACT hContact, int rtl)
{
	for (int i = 0; i <= TMPL_ERRMSG; i++) {
		DBVARIANT dbv = { 0 };
		if (db_get_ws(hContact, rtl ? RTLTEMPLATES_MODULE : TEMPLATES_MODULE, TemplateNames[i], &dbv))
			continue;
		if (dbv.type == DBVT_ASCIIZ || dbv.type == DBVT_WCHAR)
			wcsncpy_s(tSet->szTemplates[i], dbv.ptszVal, _TRUNCATE);
		db_free(&dbv);
	}
}

void LoadDefaultTemplates()
{
	LTR_Active = LTR_Default;
	RTL_Active = RTL_Default;

	if (M.GetByte(RTLTEMPLATES_MODULE, "setup", 0) < 2) {
		for (int i = 0; i <= TMPL_ERRMSG; i++)
			db_set_ws(0, RTLTEMPLATES_MODULE, TemplateNames[i], RTL_Default.szTemplates[i]);
		db_set_b(0, RTLTEMPLATES_MODULE, "setup", 2);
	}
	if (M.GetByte(TEMPLATES_MODULE, "setup", 0) < 2) {
		for (int i = 0; i <= TMPL_ERRMSG; i++)
			db_set_ws(0, TEMPLATES_MODULE, TemplateNames[i], LTR_Default.szTemplates[i]);
		db_set_b(0, TEMPLATES_MODULE, "setup", 2);
	}
	LoadTemplatesFrom(&LTR_Active, 0, 0);
	LoadTemplatesFrom(&RTL_Active, 0, 1);
}

CTemplateEditDlg::CTemplateEditDlg(BOOL _rtl, HWND hwndParent) :
	CSuper(IDD_TEMPLATEEDIT),
	rtl(_rtl),
	edtText(this, IDC_EDITTEMPLATE),
	btnHelp(this, IDC_VARIABLESHELP),
	btnSave(this, IDC_SAVETEMPLATE),
	btnForget(this, IDC_FORGET),
	btnRevert(this, IDC_REVERT),
	btnPreview(this, IDC_UPDATEPREVIEW),
	btnResetAll(this, IDC_RESETALLTEMPLATES),
	listTemplates(this, IDC_TEMPLATELIST)
{
	SetParent(hwndParent);

	m_pContainer = (TContainerData*)mir_calloc(sizeof(TContainerData));
	LoadOverrideTheme(m_pContainer);
	tSet = rtl ? m_pContainer->rtl_templates : m_pContainer->ltr_templates;

	listTemplates.OnDblClick = Callback(this, &CTemplateEditDlg::onDblClick_List);
	listTemplates.OnSelChange = Callback(this, &CTemplateEditDlg::onSelChange_List);

	edtText.OnChange = Callback(this, &CTemplateEditDlg::onChange_Text);
	
	btnHelp.OnClick = Callback(this, &CTemplateEditDlg::onClick_Help);
	btnSave.OnClick = Callback(this, &CTemplateEditDlg::onClick_Save);
	btnForget.OnClick = Callback(this, &CTemplateEditDlg::onClick_Forget);
	btnRevert.OnClick = Callback(this, &CTemplateEditDlg::onClick_Revert);
	btnPreview.OnClick = Callback(this, &CTemplateEditDlg::onClick_Preview);
	btnResetAll.OnClick = Callback(this, &CTemplateEditDlg::onClick_Reset);
}

void CTemplateEditDlg::OnInitDialog()
{
	// set hContact to the first found contact so that we can use the Preview window properly
	// also, set other parameters needed by the streaming function to display events

	m_log.SendMsg(EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_LINK);
	m_log.SendMsg(EM_SETEDITSTYLE, SES_EXTENDBACKCOLOR, SES_EXTENDBACKCOLOR);
	m_log.SendMsg(EM_EXLIMITTEXT, 0, 0x80000000);

	m_hContact = db_find_first();
	m_szProto = GetContactProto(m_hContact);
	while (m_szProto == 0 && m_hContact != 0) {
		m_hContact = db_find_next(m_hContact);
		m_szProto = GetContactProto(m_hContact);
	}
	m_dwFlags = m_pContainer->theme.dwFlags;

	m_cache = CContactCache::getContactCache(m_hContact);
	m_cache->updateNick();
	m_cache->updateUIN();
	m_cache->updateStats(TSessionStats::INIT_TIMER);
	GetMYUIN();

	SendDlgItemMessage(m_hwnd, IDC_EDITTEMPLATE, EM_LIMITTEXT, (WPARAM)TEMPLATE_LENGTH - 1, 0);
	SetWindowText(m_hwnd, TranslateT("Template set editor"));
	Utils::enableDlgControl(m_hwnd, IDC_SAVETEMPLATE, FALSE);
	Utils::enableDlgControl(m_hwnd, IDC_REVERT, FALSE);
	Utils::enableDlgControl(m_hwnd, IDC_FORGET, FALSE);
	for (int i = 0; i <= TMPL_ERRMSG; i++) {
		SendDlgItemMessageA(m_hwnd, IDC_TEMPLATELIST, LB_ADDSTRING, 0, (LPARAM)Translate(TemplateNames[i]));
		SendDlgItemMessage(m_hwnd, IDC_TEMPLATELIST, LB_SETITEMDATA, i, (LPARAM)i);
	}
	Utils::enableDlgControl(m_hwndParent, IDC_MODIFY, FALSE);
	Utils::enableDlgControl(m_hwndParent, IDC_RTLMODIFY, FALSE);

	SendDlgItemMessage(m_hwnd, IDC_COLOR1, CPM_SETCOLOUR, 0, M.GetDword("cc1", SRMSGDEFSET_BKGCOLOUR));
	SendDlgItemMessage(m_hwnd, IDC_COLOR2, CPM_SETCOLOUR, 0, M.GetDword("cc2", SRMSGDEFSET_BKGCOLOUR));
	SendDlgItemMessage(m_hwnd, IDC_COLOR3, CPM_SETCOLOUR, 0, M.GetDword("cc3", SRMSGDEFSET_BKGCOLOUR));
	SendDlgItemMessage(m_hwnd, IDC_COLOR4, CPM_SETCOLOUR, 0, M.GetDword("cc4", SRMSGDEFSET_BKGCOLOUR));
	SendDlgItemMessage(m_hwnd, IDC_COLOR5, CPM_SETCOLOUR, 0, M.GetDword("cc5", SRMSGDEFSET_BKGCOLOUR));
	SendDlgItemMessage(m_hwnd, IDC_EDITTEMPLATE, EM_SETREADONLY, TRUE, 0);
}

void CTemplateEditDlg::OnDestroy()
{
	Utils::enableDlgControl(m_hwndParent, IDC_MODIFY, TRUE);
	Utils::enableDlgControl(m_hwndParent, IDC_RTLMODIFY, TRUE);

	mir_free(m_pContainer);

	db_set_dw(0, SRMSGMOD_T, "cc1", SendDlgItemMessage(m_hwnd, IDC_COLOR1, CPM_GETCOLOUR, 0, 0));
	db_set_dw(0, SRMSGMOD_T, "cc2", SendDlgItemMessage(m_hwnd, IDC_COLOR2, CPM_GETCOLOUR, 0, 0));
	db_set_dw(0, SRMSGMOD_T, "cc3", SendDlgItemMessage(m_hwnd, IDC_COLOR3, CPM_GETCOLOUR, 0, 0));
	db_set_dw(0, SRMSGMOD_T, "cc4", SendDlgItemMessage(m_hwnd, IDC_COLOR4, CPM_GETCOLOUR, 0, 0));
	db_set_dw(0, SRMSGMOD_T, "cc5", SendDlgItemMessage(m_hwnd, IDC_COLOR5, CPM_GETCOLOUR, 0, 0));
}

void CTemplateEditDlg::onChange_Text(CCtrlEdit*)
{
	if (!selchanging) {
		changed = TRUE;
		updateInfo[inEdit] = TRUE;
		Utils::enableDlgControl(m_hwnd, IDC_SAVETEMPLATE, TRUE);
		Utils::enableDlgControl(m_hwnd, IDC_FORGET, TRUE);
		Utils::enableDlgControl(m_hwnd, IDC_TEMPLATELIST, FALSE);
		Utils::enableDlgControl(m_hwnd, IDC_REVERT, TRUE);
	}
	InvalidateRect(GetDlgItem(m_hwnd, IDC_TEMPLATELIST), nullptr, FALSE);
}

void CTemplateEditDlg::onClick_Forget(CCtrlButton*)
{
	changed = FALSE;
	updateInfo[inEdit] = FALSE;
	selchanging = TRUE;
	SetDlgItemText(m_hwnd, IDC_EDITTEMPLATE, tSet->szTemplates[inEdit]);
	SetFocus(GetDlgItem(m_hwnd, IDC_EDITTEMPLATE));
	InvalidateRect(GetDlgItem(m_hwnd, IDC_TEMPLATELIST), nullptr, FALSE);
	Utils::enableDlgControl(m_hwnd, IDC_SAVETEMPLATE, FALSE);
	Utils::enableDlgControl(m_hwnd, IDC_FORGET, FALSE);
	Utils::enableDlgControl(m_hwnd, IDC_TEMPLATELIST, TRUE);
	Utils::enableDlgControl(m_hwnd, IDC_REVERT, FALSE);
	selchanging = FALSE;
	SendDlgItemMessage(m_hwnd, IDC_EDITTEMPLATE, EM_SETREADONLY, TRUE, 0);
}

void CTemplateEditDlg::onClick_Help(CCtrlButton*)
{
	Utils_OpenUrl("https://wiki.miranda-ng.org/index.php?title=Plugin:TabSRMM/en/Templates");
}

void CTemplateEditDlg::onClick_Preview(CCtrlButton*)
{
	int iIndex = SendDlgItemMessage(m_hwnd, IDC_TEMPLATELIST, LB_GETCURSEL, 0, 0);
	wchar_t szTemp[TEMPLATE_LENGTH + 2];

	if (changed) {
		memcpy(szTemp, tSet->szTemplates[inEdit], (TEMPLATE_LENGTH * sizeof(wchar_t)));
		GetDlgItemText(m_hwnd, IDC_EDITTEMPLATE, tSet->szTemplates[inEdit], TEMPLATE_LENGTH);
	}

	DBEVENTINFO dbei = {};
	dbei.szModule = m_szProto;
	dbei.timestamp = time(nullptr);
	dbei.eventType = (iIndex == 6) ? EVENTTYPE_STATUSCHANGE : EVENTTYPE_MESSAGE;
	dbei.eventType = (iIndex == 7) ? EVENTTYPE_ERRMSG : dbei.eventType;
	if (dbei.eventType == EVENTTYPE_ERRMSG)
		dbei.szModule = (char *)L"Sample error message";
	dbei.pBlob = (iIndex == 6) ? (BYTE *)"is now offline (was online)" : (BYTE *)"The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog.";
	dbei.cbBlob = (int)mir_strlen((char *)dbei.pBlob) + 1;
	dbei.flags = (iIndex == 1 || iIndex == 3 || iIndex == 5) ? DBEF_SENT : 0;
	dbei.flags |= (rtl ? DBEF_RTL : 0);
	m_lastEventTime = (iIndex == 4 || iIndex == 5) ? time(nullptr) - 1 : 0;
	m_iLastEventType = MAKELONG(dbei.flags, dbei.eventType);
	m_log.SetText(L"");
	m_dwFlags = MWF_LOG_ALL;
	m_dwFlags = (rtl ? m_dwFlags | MWF_LOG_RTL : m_dwFlags & ~MWF_LOG_RTL);
	m_dwFlags = (iIndex == 0 || iIndex == 1) ? m_dwFlags & ~MWF_LOG_GROUPMODE : m_dwFlags | MWF_LOG_GROUPMODE;
	mir_snwprintf(m_wszMyNickname, L"My Nickname");
	StreamInEvents(0, 1, 0, &dbei);
	m_log.SendMsg(EM_SETSEL, -1, -1);
	if (changed)
		memcpy(tSet->szTemplates[inEdit], szTemp, TEMPLATE_LENGTH * sizeof(wchar_t));
}

void CTemplateEditDlg::onClick_Reset(CCtrlButton*)
{
	if (MessageBox(m_hwnd, TranslateT("This will reset the template set to the default built-in templates. Are you sure you want to do this?"),
		TranslateT("Template set editor"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
		db_set_b(0, rtl ? RTLTEMPLATES_MODULE : TEMPLATES_MODULE, "setup", 0);
		LoadDefaultTemplates();
		MessageBox(m_hwnd,
			TranslateT("Template set was successfully reset, please close and reopen all message windows. This template editor window will now close."),
			TranslateT("Template set editor"), MB_OK);
		Close();
	}
}

void CTemplateEditDlg::onClick_Revert(CCtrlButton*)
{
	changed = FALSE;
	updateInfo[inEdit] = FALSE;
	selchanging = TRUE;
	memcpy(tSet->szTemplates[inEdit], LTR_Default.szTemplates[inEdit], sizeof(wchar_t) * TEMPLATE_LENGTH);
	SetDlgItemText(m_hwnd, IDC_EDITTEMPLATE, tSet->szTemplates[inEdit]);
	db_unset(0, rtl ? RTLTEMPLATES_MODULE : TEMPLATES_MODULE, TemplateNames[inEdit]);
	SetFocus(GetDlgItem(m_hwnd, IDC_EDITTEMPLATE));
	InvalidateRect(GetDlgItem(m_hwnd, IDC_TEMPLATELIST), nullptr, FALSE);
	selchanging = FALSE;
	Utils::enableDlgControl(m_hwnd, IDC_SAVETEMPLATE, FALSE);
	Utils::enableDlgControl(m_hwnd, IDC_REVERT, FALSE);
	Utils::enableDlgControl(m_hwnd, IDC_FORGET, FALSE);
	Utils::enableDlgControl(m_hwnd, IDC_TEMPLATELIST, TRUE);
	SendDlgItemMessage(m_hwnd, IDC_EDITTEMPLATE, EM_SETREADONLY, TRUE, 0);
}
void CTemplateEditDlg::onClick_Save(CCtrlButton*)
{
	wchar_t newTemplate[TEMPLATE_LENGTH + 2];

	GetDlgItemText(m_hwnd, IDC_EDITTEMPLATE, newTemplate, _countof(newTemplate));
	memcpy(tSet->szTemplates[inEdit], newTemplate, sizeof(wchar_t) * TEMPLATE_LENGTH);
	changed = FALSE;
	updateInfo[inEdit] = FALSE;
	Utils::enableDlgControl(m_hwnd, IDC_SAVETEMPLATE, FALSE);
	Utils::enableDlgControl(m_hwnd, IDC_FORGET, FALSE);
	Utils::enableDlgControl(m_hwnd, IDC_TEMPLATELIST, TRUE);
	Utils::enableDlgControl(m_hwnd, IDC_REVERT, FALSE);
	InvalidateRect(GetDlgItem(m_hwnd, IDC_TEMPLATELIST), nullptr, FALSE);
	db_set_ws(0, rtl ? RTLTEMPLATES_MODULE : TEMPLATES_MODULE, TemplateNames[inEdit], newTemplate);
	SendDlgItemMessage(m_hwnd, IDC_EDITTEMPLATE, EM_SETREADONLY, TRUE, 0);
}

void CTemplateEditDlg::onDblClick_List(CCtrlListBox*)
{
	LRESULT iIndex = SendDlgItemMessage(m_hwnd, IDC_TEMPLATELIST, LB_GETCURSEL, 0, 0);
	if (iIndex != LB_ERR) {
		SetDlgItemText(m_hwnd, IDC_EDITTEMPLATE, tSet->szTemplates[iIndex]);
		inEdit = iIndex;
		changed = FALSE;
		selchanging = FALSE;
		SetFocus(GetDlgItem(m_hwnd, IDC_EDITTEMPLATE));
		SendDlgItemMessage(m_hwnd, IDC_EDITTEMPLATE, EM_SETREADONLY, FALSE, 0);
	}
}

void CTemplateEditDlg::onSelChange_List(CCtrlListBox*)
{
	LRESULT iIndex = SendDlgItemMessage(m_hwnd, IDC_TEMPLATELIST, LB_GETCURSEL, 0, 0);
	selchanging = TRUE;
	if (iIndex != LB_ERR) {
		SetDlgItemText(m_hwnd, IDC_EDITTEMPLATE, tSet->szTemplates[iIndex]);
		inEdit = iIndex;
		changed = FALSE;
	}
	SendDlgItemMessage(m_hwnd, IDC_EDITTEMPLATE, EM_SETREADONLY, TRUE, 0);
}

INT_PTR CTemplateEditDlg::DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_DRAWITEM) {
		DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;
		int iItem = dis->itemData;
		SetBkMode(dis->hDC, TRANSPARENT);
		FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_WINDOW));
		if (dis->itemState & ODS_SELECTED) {
			if (updateInfo[iItem] == TRUE) {
				HBRUSH bkg = CreateSolidBrush(RGB(255, 0, 0));
				HBRUSH oldBkg = (HBRUSH)SelectObject(dis->hDC, bkg);
				FillRect(dis->hDC, &dis->rcItem, bkg);
				SelectObject(dis->hDC, oldBkg);
				DeleteObject(bkg);
			}
			else FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));

			SetTextColor(dis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
		}
		else {
			if (updateInfo[iItem] == TRUE)
				SetTextColor(dis->hDC, RGB(255, 0, 0));
			else
				SetTextColor(dis->hDC, GetSysColor(COLOR_WINDOWTEXT));
		}
		char *pszName = Translate(TemplateNames[iItem]);
		TextOutA(dis->hDC, dis->rcItem.left, dis->rcItem.top, pszName, (int)mir_strlen(pszName));
	}

	return CSuper::DlgProc(uMsg, wParam, lParam);
}
