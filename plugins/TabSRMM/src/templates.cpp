/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (c) 2012-14 Miranda NG project,
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

#include "commonheaders.h"

/*
* hardcoded default set of templates for both LTR and RTL.
* cannot be changed and may be used at any time to "revert" to a working layout
*/

char *TemplateNames[] = {
	"Message In",
	"Message Out",
	"Group In (Start)",
	"Group Out (Start)",
	"Group In (Inner)",
	"Group Out (Inner)",
	"Status change",
	"Error message"
};

TTemplateSet LTR_Default = { TRUE,
							_T("%I %S %N  %?&D%\\&E%\\!, %\\T%\\!: %?n%?S %?T%?|%M"),
							_T("%I %S %N  %?&D%\\&E%\\!, %\\T%\\!: %?n%?S %?T%?|%M"),
							_T("%I %S %N  %?&D%\\&E%\\!, %\\T%\\!: %?n%?S %?T%?|%M"),
							_T("%I %S %N  %?&D%\\&E%\\!, %\\T%\\!: %?n%?S %?T%?|%M"),
							_T("%S %T%|%M"),
							_T("%S %T%|%M"),
							_T("%I %S %&D, %&T, %N %M%! "),
							_T("%I%S %D, %T, %e%l%M"),
							"Default LTR"
						  };

TTemplateSet RTL_Default = { TRUE,
							_T("%I %S %N  %D%n%S %T%|%M"),
							_T("%I %S %N  %D%n%S %T%|%M"),
							_T("%I %S %N  %D%n%S %T%|%M"),
							_T("%I %S %N  %D%n%S %T%|%M"),
							_T("%S %T%|%M"),
							_T("%S %T%|%M"),
							_T("%I%S %D, %T, %N %M%! "),
							_T("%I%S %D, %T, %e%l%M"),
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
	DBVARIANT dbv = {0};
	int i;

	for (i=0; i <= TMPL_ERRMSG; i++) {
		if (db_get_ts(hContact, rtl ? RTLTEMPLATES_MODULE : TEMPLATES_MODULE, TemplateNames[i], &dbv))
			continue;
		if (dbv.type == DBVT_ASCIIZ || dbv.type == DBVT_WCHAR)
			_tcsncpy_s(tSet->szTemplates[i], dbv.ptszVal, _TRUNCATE);
		db_free(&dbv);
	}
}

void LoadDefaultTemplates()
{
	int i;

	LTR_Active = LTR_Default;
	RTL_Active = RTL_Default;

	if (M.GetByte(RTLTEMPLATES_MODULE, "setup", 0) < 2) {
		for (i=0; i <= TMPL_ERRMSG; i++)
			db_set_ts(NULL, RTLTEMPLATES_MODULE, TemplateNames[i], RTL_Default.szTemplates[i]);
		db_set_b(0, RTLTEMPLATES_MODULE, "setup", 2);
	}
	if (M.GetByte(TEMPLATES_MODULE, "setup", 0) < 2) {
		for (i=0; i <= TMPL_ERRMSG; i++)
			db_set_ts(NULL, TEMPLATES_MODULE, TemplateNames[i], LTR_Default.szTemplates[i]);
		db_set_b(0, TEMPLATES_MODULE, "setup", 2);
	}
	LoadTemplatesFrom(&LTR_Active, 0, 0);
	LoadTemplatesFrom(&RTL_Active, 0, 1);
}

INT_PTR CALLBACK DlgProcTemplateEditor(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TemplateEditorInfo *teInfo = 0;
	TTemplateSet *tSet;
	int i;
	TWindowData *dat = (TWindowData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	/*
	* since this dialog needs a MessageWindowData * but has no container, we can store
	* the extended info struct in pContainer *)
	*/
	if (dat) {
		teInfo = (TemplateEditorInfo *)dat->pContainer;
		tSet = teInfo->rtl ? dat->pContainer->rtl_templates : dat->pContainer->ltr_templates;
	}
	else tSet = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			TemplateEditorNew *teNew = (TemplateEditorNew *)lParam;
			dat = (TWindowData*)mir_calloc( sizeof(TWindowData));
			dat->pContainer = (TContainerData*)mir_alloc(sizeof(TContainerData));
			memset(dat->pContainer, 0, sizeof(TContainerData));
			teInfo = (TemplateEditorInfo *)dat->pContainer;
			memset(teInfo, 0, sizeof(TemplateEditorInfo));
			teInfo->hContact = teNew->hContact;
			teInfo->rtl = teNew->rtl;
			teInfo->hwndParent = teNew->hwndParent;

			LoadOverrideTheme(dat->pContainer);
			/*
			* set hContact to the first found contact so that we can use the Preview window properly
			* also, set other parameters needed by the streaming function to display events
			*/

			SendDlgItemMessage(hwndDlg, IDC_PREVIEW, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_LINK);
			SendDlgItemMessage(hwndDlg, IDC_PREVIEW, EM_SETEDITSTYLE, SES_EXTENDBACKCOLOR, SES_EXTENDBACKCOLOR);
			SendDlgItemMessage(hwndDlg, IDC_PREVIEW, EM_EXLIMITTEXT, 0, 0x80000000);

			dat->hContact = db_find_first();
			dat->szProto = GetContactProto(dat->hContact);
			while(dat->szProto == 0 && dat->hContact != 0) {
				dat->hContact = db_find_next(dat->hContact);
				dat->szProto = GetContactProto(dat->hContact);
			}
			dat->dwFlags = dat->pContainer->theme.dwFlags;

			dat->cache = CContactCache::getContactCache(dat->hContact);
			dat->cache->updateState();
			dat->cache->updateUIN();
			dat->cache->updateStats(TSessionStats::INIT_TIMER);
			GetMYUIN(dat);

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) dat);
			ShowWindow(hwndDlg, SW_SHOW);
			SendDlgItemMessage(hwndDlg, IDC_EDITTEMPLATE, EM_LIMITTEXT, (WPARAM)TEMPLATE_LENGTH - 1, 0);
			SetWindowText(hwndDlg, TranslateT("Template Set Editor"));
			Utils::enableDlgControl(hwndDlg, IDC_SAVETEMPLATE, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_REVERT, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_FORGET, FALSE);
			for (i=0; i <= TMPL_ERRMSG; i++) {
				SendDlgItemMessageA(hwndDlg, IDC_TEMPLATELIST, LB_ADDSTRING, 0, (LPARAM)Translate(TemplateNames[i]));
				SendDlgItemMessage(hwndDlg, IDC_TEMPLATELIST, LB_SETITEMDATA, i, (LPARAM)i);
			}
			Utils::enableDlgControl(teInfo->hwndParent, IDC_MODIFY, FALSE);
			Utils::enableDlgControl(teInfo->hwndParent, IDC_RTLMODIFY, FALSE);

			SendDlgItemMessage(hwndDlg, IDC_COLOR1, CPM_SETCOLOUR, 0, M.GetDword("cc1", SRMSGDEFSET_BKGCOLOUR));
			SendDlgItemMessage(hwndDlg, IDC_COLOR2, CPM_SETCOLOUR, 0, M.GetDword("cc2", SRMSGDEFSET_BKGCOLOUR));
			SendDlgItemMessage(hwndDlg, IDC_COLOR3, CPM_SETCOLOUR, 0, M.GetDword("cc3", SRMSGDEFSET_BKGCOLOUR));
			SendDlgItemMessage(hwndDlg, IDC_COLOR4, CPM_SETCOLOUR, 0, M.GetDword("cc4", SRMSGDEFSET_BKGCOLOUR));
			SendDlgItemMessage(hwndDlg, IDC_COLOR5, CPM_SETCOLOUR, 0, M.GetDword("cc5", SRMSGDEFSET_BKGCOLOUR));
			SendMessage(GetDlgItem(hwndDlg, IDC_EDITTEMPLATE), EM_SETREADONLY, TRUE, 0);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;

		case IDC_RESETALLTEMPLATES:
			if (MessageBox(0, TranslateT("This will reset the template set to the default built-in templates. Are you sure you want to do this?"),
				TranslateT("Template Set Editor"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
					db_set_b(0, teInfo->rtl ? RTLTEMPLATES_MODULE : TEMPLATES_MODULE, "setup", 0);
					LoadDefaultTemplates();
					MessageBox(0, TranslateT("Template set was successfully reset, please close and reopen all message windows. This template editor window will now close."),
						TranslateT("Template Set Editor"), MB_OK);
					DestroyWindow(hwndDlg);
			}
			break;

		case IDC_TEMPLATELIST:
			switch (HIWORD(wParam)) {
			LRESULT iIndex;
			case LBN_DBLCLK:
				iIndex = SendDlgItemMessage(hwndDlg, IDC_TEMPLATELIST, LB_GETCURSEL, 0, 0);
				if (iIndex != LB_ERR) {
					SetDlgItemText(hwndDlg, IDC_EDITTEMPLATE, tSet->szTemplates[iIndex]);
					teInfo->inEdit = iIndex;
					teInfo->changed = FALSE;
					teInfo->selchanging = FALSE;
					SetFocus(GetDlgItem(hwndDlg, IDC_EDITTEMPLATE));
					SendMessage(GetDlgItem(hwndDlg, IDC_EDITTEMPLATE), EM_SETREADONLY, FALSE, 0);
				}
				break;

			case LBN_SELCHANGE:
				iIndex = SendDlgItemMessage(hwndDlg, IDC_TEMPLATELIST, LB_GETCURSEL, 0, 0);
				teInfo->selchanging = TRUE;
				if (iIndex != LB_ERR) {
					SetDlgItemText(hwndDlg, IDC_EDITTEMPLATE, tSet->szTemplates[iIndex]);
					teInfo->inEdit = iIndex;
					teInfo->changed = FALSE;
				}
				SendMessage(GetDlgItem(hwndDlg, IDC_EDITTEMPLATE), EM_SETREADONLY, TRUE, 0);
			}
			break;

		case IDC_VARIABLESHELP:
			CallService(MS_UTILS_OPENURL, 0, (LPARAM)"http://wiki.miranda.or.at/TabSRMM/Templates");
			break;

		case IDC_EDITTEMPLATE:
			if (HIWORD(wParam) == EN_CHANGE) {
				if (!teInfo->selchanging) {
					teInfo->changed = TRUE;
					teInfo->updateInfo[teInfo->inEdit] = TRUE;
					Utils::enableDlgControl(hwndDlg, IDC_SAVETEMPLATE, TRUE);
					Utils::enableDlgControl(hwndDlg, IDC_FORGET, TRUE);
					Utils::enableDlgControl(hwndDlg, IDC_TEMPLATELIST, FALSE);
					Utils::enableDlgControl(hwndDlg, IDC_REVERT, TRUE);
				}
				InvalidateRect(GetDlgItem(hwndDlg, IDC_TEMPLATELIST), NULL, FALSE);
			}
			break;
		case IDC_SAVETEMPLATE:
			{
				TCHAR newTemplate[TEMPLATE_LENGTH + 2];

				GetWindowText(GetDlgItem(hwndDlg, IDC_EDITTEMPLATE), newTemplate, SIZEOF(newTemplate));
				memcpy(tSet->szTemplates[teInfo->inEdit], newTemplate, sizeof(TCHAR) * TEMPLATE_LENGTH);
				teInfo->changed = FALSE;
				teInfo->updateInfo[teInfo->inEdit] = FALSE;
				Utils::enableDlgControl(hwndDlg, IDC_SAVETEMPLATE, FALSE);
				Utils::enableDlgControl(hwndDlg, IDC_FORGET, FALSE);
				Utils::enableDlgControl(hwndDlg, IDC_TEMPLATELIST, TRUE);
				Utils::enableDlgControl(hwndDlg, IDC_REVERT, FALSE);
				InvalidateRect(GetDlgItem(hwndDlg, IDC_TEMPLATELIST), NULL, FALSE);
				db_set_ts(teInfo->hContact, teInfo->rtl ? RTLTEMPLATES_MODULE : TEMPLATES_MODULE, TemplateNames[teInfo->inEdit], newTemplate);
				SendMessage(GetDlgItem(hwndDlg, IDC_EDITTEMPLATE), EM_SETREADONLY, TRUE, 0);
			}
			break;

		case IDC_FORGET:
			teInfo->changed = FALSE;
			teInfo->updateInfo[teInfo->inEdit] = FALSE;
			teInfo->selchanging = TRUE;
			SetDlgItemText(hwndDlg, IDC_EDITTEMPLATE, tSet->szTemplates[teInfo->inEdit]);
			SetFocus(GetDlgItem(hwndDlg, IDC_EDITTEMPLATE));
			InvalidateRect(GetDlgItem(hwndDlg, IDC_TEMPLATELIST), NULL, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_SAVETEMPLATE, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_FORGET, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_TEMPLATELIST, TRUE);
			Utils::enableDlgControl(hwndDlg, IDC_REVERT, FALSE);
			teInfo->selchanging = FALSE;
			SendMessage(GetDlgItem(hwndDlg, IDC_EDITTEMPLATE), EM_SETREADONLY, TRUE, 0);
			break;

		case IDC_REVERT:
			teInfo->changed = FALSE;
			teInfo->updateInfo[teInfo->inEdit] = FALSE;
			teInfo->selchanging = TRUE;
			memcpy(tSet->szTemplates[teInfo->inEdit], LTR_Default.szTemplates[teInfo->inEdit], sizeof(TCHAR) * TEMPLATE_LENGTH);
			SetDlgItemText(hwndDlg, IDC_EDITTEMPLATE, tSet->szTemplates[teInfo->inEdit]);
			db_unset(teInfo->hContact, teInfo->rtl ? RTLTEMPLATES_MODULE : TEMPLATES_MODULE, TemplateNames[teInfo->inEdit]);
			SetFocus(GetDlgItem(hwndDlg, IDC_EDITTEMPLATE));
			InvalidateRect(GetDlgItem(hwndDlg, IDC_TEMPLATELIST), NULL, FALSE);
			teInfo->selchanging = FALSE;
			Utils::enableDlgControl(hwndDlg, IDC_SAVETEMPLATE, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_REVERT, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_FORGET, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_TEMPLATELIST, TRUE);
			SendMessage(GetDlgItem(hwndDlg, IDC_EDITTEMPLATE), EM_SETREADONLY, TRUE, 0);
			break;

		case IDC_UPDATEPREVIEW:
			SendMessage(hwndDlg, DM_UPDATETEMPLATEPREVIEW, 0, 0);
			break;
		}
		break;

	case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *) lParam;
			int iItem = dis->itemData;
			HBRUSH bkg, oldBkg;
			SetBkMode(dis->hDC, TRANSPARENT);
			FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_WINDOW));
			if (dis->itemState & ODS_SELECTED) {
				if (teInfo->updateInfo[iItem] == TRUE) {
					bkg = CreateSolidBrush(RGB(255, 0, 0));
					oldBkg = (HBRUSH)SelectObject(dis->hDC, bkg);
					FillRect(dis->hDC, &dis->rcItem, bkg);
					SelectObject(dis->hDC, oldBkg);
					DeleteObject(bkg);
				} else
					FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));

				SetTextColor(dis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
			} else {
				if (teInfo->updateInfo[iItem] == TRUE)
					SetTextColor(dis->hDC, RGB(255, 0, 0));
				else
					SetTextColor(dis->hDC, GetSysColor(COLOR_WINDOWTEXT));
			}
			char *pszName = Translate(TemplateNames[iItem]);
			TextOutA(dis->hDC, dis->rcItem.left, dis->rcItem.top, pszName, (int)mir_strlen(pszName));
		}
		return TRUE;

	case DM_UPDATETEMPLATEPREVIEW: {
		DBEVENTINFO dbei = {0};
		int iIndex = SendDlgItemMessage(hwndDlg, IDC_TEMPLATELIST, LB_GETCURSEL, 0, 0);
		TCHAR szTemp[TEMPLATE_LENGTH + 2];

		if (teInfo->changed) {
			memcpy(szTemp, tSet->szTemplates[teInfo->inEdit], (TEMPLATE_LENGTH * sizeof(TCHAR)));
			GetDlgItemText(hwndDlg, IDC_EDITTEMPLATE, tSet->szTemplates[teInfo->inEdit], TEMPLATE_LENGTH);
		}
		dbei.szModule = dat->szProto;
		dbei.timestamp = time(NULL);
		dbei.eventType = (iIndex == 6) ? EVENTTYPE_STATUSCHANGE : EVENTTYPE_MESSAGE;
		dbei.eventType = (iIndex == 7) ? EVENTTYPE_ERRMSG : dbei.eventType;
		if (dbei.eventType == EVENTTYPE_ERRMSG)
			dbei.szModule = "Sample error message";
		dbei.cbSize = sizeof(dbei);
		dbei.pBlob = (iIndex == 6) ? (BYTE *)"is now offline (was online)" : (BYTE *)"The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog.";
		dbei.cbBlob = (int)mir_strlen((char *)dbei.pBlob) + 1;
		dbei.flags = (iIndex == 1 || iIndex == 3 || iIndex == 5) ? DBEF_SENT : 0;
		dbei.flags |= (teInfo->rtl ? DBEF_RTL : 0);
		dat->lastEventTime = (iIndex == 4 || iIndex == 5) ? time(NULL) - 1 : 0;
		dat->iLastEventType = MAKELONG(dbei.flags, dbei.eventType);
		SetWindowText(GetDlgItem(hwndDlg, IDC_PREVIEW), _T(""));
		dat->dwFlags = MWF_LOG_ALL;
		dat->dwFlags = (teInfo->rtl ? dat->dwFlags | MWF_LOG_RTL : dat->dwFlags & ~MWF_LOG_RTL);
		dat->dwFlags = (iIndex == 0 || iIndex == 1) ? dat->dwFlags & ~MWF_LOG_GROUPMODE : dat->dwFlags | MWF_LOG_GROUPMODE;
		mir_sntprintf(dat->szMyNickname, SIZEOF(dat->szMyNickname), _T("My Nickname"));
		StreamInEvents(hwndDlg, 0, 1, 1, &dbei);
		SendDlgItemMessage(hwndDlg, IDC_PREVIEW, EM_SETSEL, -1, -1);
		if (teInfo->changed)
			memcpy(tSet->szTemplates[teInfo->inEdit], szTemp, TEMPLATE_LENGTH * sizeof(TCHAR));
		break;
											 }
	case WM_DESTROY:
		Utils::enableDlgControl(teInfo->hwndParent, IDC_MODIFY, TRUE);
		Utils::enableDlgControl(teInfo->hwndParent, IDC_RTLMODIFY, TRUE);
		if (dat) {
			mir_free(dat->pContainer);
			mir_free(dat);
		}
		db_set_dw(0, SRMSGMOD_T, "cc1", SendDlgItemMessage(hwndDlg, IDC_COLOR1, CPM_GETCOLOUR, 0, 0));
		db_set_dw(0, SRMSGMOD_T, "cc2", SendDlgItemMessage(hwndDlg, IDC_COLOR2, CPM_GETCOLOUR, 0, 0));
		db_set_dw(0, SRMSGMOD_T, "cc3", SendDlgItemMessage(hwndDlg, IDC_COLOR3, CPM_GETCOLOUR, 0, 0));
		db_set_dw(0, SRMSGMOD_T, "cc4", SendDlgItemMessage(hwndDlg, IDC_COLOR4, CPM_GETCOLOUR, 0, 0));
		db_set_dw(0, SRMSGMOD_T, "cc5", SendDlgItemMessage(hwndDlg, IDC_COLOR5, CPM_GETCOLOUR, 0, 0));

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		break;
	}
	return FALSE;
}
