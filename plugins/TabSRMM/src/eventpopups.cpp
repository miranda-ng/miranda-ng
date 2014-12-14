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
// implements the event notification module for tabSRMM. The code
// is largely based on the NewEventNotify plugin for Miranda NG. See
// notices below.
//
//  Name: NewEventNotify - Plugin for Miranda ICQ
// 	Description: Notifies you when you receive a message
// 	Author: icebreaker, <icebreaker@newmail.net>
// 	Date: 18.07.02 13:59 / Update: 16.09.02 17:45
// 	Copyright: (C) 2002 Starzinger Michael

#include "commonheaders.h"

static int TSAPI PopupPreview(NEN_OPTIONS *pluginOptions);

static LIST<PLUGIN_DATAT> arPopupList(10, NumericKeySortT);

BOOL bWmNotify = TRUE;

static PLUGIN_DATAT* PU_GetByContact(const MCONTACT hContact)
{
	return arPopupList.find((PLUGIN_DATAT*)&hContact);
}

/**
 * remove stale popup data which has been marked for removal by the popup
 * window procedure.
 *
 */
static void PU_CleanUp()
{
	for (int i=arPopupList.getCount()-1; i >= 0; i--) {
		PLUGIN_DATAT *p = arPopupList[i];
		if (p->hContact != NULL)
			continue;

		mir_free(p->eventData);
		mir_free(p);
		arPopupList.remove(i);
	}
}

static void CheckForRemoveMask()
{
	if (!M.GetByte(MODULE, "firsttime", 0) && (nen_options.maskActL & MASK_REMOVE || nen_options.maskActR & MASK_REMOVE || nen_options.maskActTE & MASK_REMOVE)) {
		MessageBox(0, TranslateT("One of your popup actions is set to DISMISS EVENT.\nNote that this options may have unwanted side effects as it REMOVES the event from the unread queue.\nThis may lead to events not showing up as \"new\". If you don't want this behavior, please review the Event Notifications settings page."), TranslateT("TabSRMM warning message"), MB_OK | MB_ICONSTOP);
		db_set_b(0, MODULE, "firsttime", 1);
	}
}


int TSAPI NEN_ReadOptions(NEN_OPTIONS *options)
{
	options->bPreview = (BOOL)M.GetByte(MODULE, OPT_PREVIEW, TRUE);
	options->bDefaultColorMsg = (BOOL)M.GetByte(MODULE, OPT_COLDEFAULT_MESSAGE, TRUE);
	options->bDefaultColorOthers = (BOOL)M.GetByte(MODULE, OPT_COLDEFAULT_OTHERS, TRUE);
	options->bDefaultColorErr = (BOOL)M.GetByte(MODULE, OPT_COLDEFAULT_ERR, TRUE);
	options->colBackMsg = (COLORREF)M.GetDword(MODULE, OPT_COLBACK_MESSAGE, DEFAULT_COLBACK);
	options->colTextMsg = (COLORREF)M.GetDword(MODULE, OPT_COLTEXT_MESSAGE, DEFAULT_COLTEXT);
	options->colBackOthers = (COLORREF)M.GetDword(MODULE, OPT_COLBACK_OTHERS, DEFAULT_COLBACK);
	options->colTextOthers = (COLORREF)M.GetDword(MODULE, OPT_COLTEXT_OTHERS, DEFAULT_COLTEXT);
	options->colBackErr = (COLORREF)M.GetDword(MODULE, OPT_COLBACK_ERR, DEFAULT_COLBACK);
	options->colTextErr = (COLORREF)M.GetDword(MODULE, OPT_COLTEXT_ERR, DEFAULT_COLTEXT);
	options->maskActL = (UINT)M.GetByte(MODULE, OPT_MASKACTL, DEFAULT_MASKACTL);
	options->maskActR = (UINT)M.GetByte(MODULE, OPT_MASKACTR, DEFAULT_MASKACTR);
	options->maskActTE = (UINT)M.GetByte(MODULE, OPT_MASKACTTE, DEFAULT_MASKACTR) & (MASK_OPEN | MASK_DISMISS);
	options->bMergePopup = (BOOL)M.GetByte(MODULE, OPT_MERGEPOPUP, 0);
	options->iDelayMsg = (int)M.GetDword(MODULE, OPT_DELAY_MESSAGE, DEFAULT_DELAY);
	options->iDelayOthers = (int)M.GetDword(MODULE, OPT_DELAY_OTHERS, DEFAULT_DELAY);
	options->iDelayErr = (int)M.GetDword(MODULE, OPT_DELAY_ERR, DEFAULT_DELAY);
	options->iDelayDefault = (int)DBGetContactSettingRangedWord(NULL, "Popup", "Seconds", SETTING_LIFETIME_DEFAULT, SETTING_LIFETIME_MIN, SETTING_LIFETIME_MAX);
	options->bShowHeaders = (BYTE)M.GetByte(MODULE, OPT_SHOW_HEADERS, FALSE);
	options->bNoRSS = (BOOL)M.GetByte(MODULE, OPT_NORSS, FALSE);
	options->iDisable = (BYTE)M.GetByte(MODULE, OPT_DISABLE, 0);
	options->iMUCDisable = (BYTE)M.GetByte(MODULE, OPT_MUCDISABLE, 0);
	options->dwStatusMask = (DWORD)M.GetDword(MODULE, "statusmask", (DWORD) - 1);
	options->bTraySupport = (BOOL)M.GetByte(MODULE, "traysupport", 0);
	options->bWindowCheck = (BOOL)M.GetByte(MODULE, OPT_WINDOWCHECK, 0);
	options->bNoRSS = (BOOL)M.GetByte(MODULE, OPT_NORSS, 0);
	options->iLimitPreview = (int)M.GetDword(MODULE, OPT_LIMITPREVIEW, 0);
	options->wMaxFavorites = 15;
	options->wMaxRecent = 15;
	options->dwRemoveMask = M.GetDword(MODULE, OPT_REMOVEMASK, 0);
	options->bDisableNonMessage = M.GetByte(MODULE, "disablenonmessage", 0);
	CheckForRemoveMask();
	return 0;
}

int TSAPI NEN_WriteOptions(NEN_OPTIONS *options)
{
	db_set_b(0, MODULE, OPT_PREVIEW, (BYTE)options->bPreview);
	db_set_b(0, MODULE, OPT_COLDEFAULT_MESSAGE, (BYTE)options->bDefaultColorMsg);
	db_set_b(0, MODULE, OPT_COLDEFAULT_OTHERS, (BYTE)options->bDefaultColorOthers);
	db_set_b(0, MODULE, OPT_COLDEFAULT_ERR, (BYTE)options->bDefaultColorErr);
	db_set_dw(0, MODULE, OPT_COLBACK_MESSAGE, (DWORD)options->colBackMsg);
	db_set_dw(0, MODULE, OPT_COLTEXT_MESSAGE, (DWORD)options->colTextMsg);
	db_set_dw(0, MODULE, OPT_COLBACK_OTHERS, (DWORD)options->colBackOthers);
	db_set_dw(0, MODULE, OPT_COLTEXT_OTHERS, (DWORD)options->colTextOthers);
	db_set_dw(0, MODULE, OPT_COLBACK_ERR, (DWORD)options->colBackErr);
	db_set_dw(0, MODULE, OPT_COLTEXT_ERR, (DWORD)options->colTextErr);
	db_set_b(0, MODULE, OPT_MASKACTL, (BYTE)options->maskActL);
	db_set_b(0, MODULE, OPT_MASKACTR, (BYTE)options->maskActR);
	db_set_b(0, MODULE, OPT_MASKACTTE, (BYTE)options->maskActTE);
	db_set_b(0, MODULE, OPT_MERGEPOPUP, (BYTE)options->bMergePopup);
	db_set_dw(0, MODULE, OPT_DELAY_MESSAGE, (DWORD)options->iDelayMsg);
	db_set_dw(0, MODULE, OPT_DELAY_OTHERS, (DWORD)options->iDelayOthers);
	db_set_dw(0, MODULE, OPT_DELAY_ERR, (DWORD)options->iDelayErr);
	db_set_b(0, MODULE, OPT_SHOW_HEADERS, (BYTE)options->bShowHeaders);
	db_set_b(0, MODULE, OPT_DISABLE, (BYTE)options->iDisable);
	db_set_b(0, MODULE, OPT_MUCDISABLE, (BYTE)options->iMUCDisable);
	db_set_b(0, MODULE, "traysupport", (BYTE)options->bTraySupport);
	db_set_b(0, MODULE, OPT_WINDOWCHECK, (BYTE)options->bWindowCheck);
	db_set_b(0, MODULE, OPT_NORSS, (BYTE)options->bNoRSS);
	db_set_dw(0, MODULE, OPT_LIMITPREVIEW, options->iLimitPreview);
	db_set_dw(0, MODULE, OPT_REMOVEMASK, options->dwRemoveMask);
	db_set_b(0, MODULE, "disablenonmessage", options->bDisableNonMessage);
	return 0;
}

INT_PTR CALLBACK DlgProcPopupOpts(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	NEN_OPTIONS *options = &nen_options;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWnd);
		{
			SetWindowLongPtr(GetDlgItem(hWnd, IDC_EVENTOPTIONS), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hWnd, IDC_EVENTOPTIONS), GWL_STYLE) | (TVS_NOHSCROLL | TVS_CHECKBOXES));
			HIMAGELIST himl = (HIMAGELIST)SendDlgItemMessage(hWnd, IDC_EVENTOPTIONS, TVM_SETIMAGELIST, TVSIL_STATE, (LPARAM)CreateStateImageList());
			ImageList_Destroy(himl);

			if (!PluginConfig.g_PopupAvail) {
				HWND	hwndChild = FindWindowEx(hWnd, 0, 0, 0);
				while(hwndChild) {
					ShowWindow(hwndChild, SW_HIDE);
					hwndChild = FindWindowEx(hWnd, hwndChild, 0, 0);
				}
				Utils::showDlgControl(hWnd, IDC_NOPOPUPAVAIL, SW_SHOW);
			}
			else Utils::showDlgControl(hWnd, IDC_NOPOPUPAVAIL, SW_HIDE);

			/*
			* fill the tree view
			*/

			TOptionListGroup *lGroups = CTranslator::getGroupTree(CTranslator::TREE_NEN);
			for (int i=0; lGroups[i].szName != NULL; i++) {
				TVINSERTSTRUCT tvi = { 0 };
				tvi.hInsertAfter = TVI_LAST;
				tvi.item.mask = TVIF_TEXT | TVIF_STATE;
				tvi.item.pszText = TranslateTS(lGroups[i].szName);
				tvi.item.stateMask = TVIS_STATEIMAGEMASK | TVIS_EXPANDED | TVIS_BOLD;
				tvi.item.state = INDEXTOSTATEIMAGEMASK(0) | TVIS_EXPANDED | TVIS_BOLD;
				lGroups[i].handle = (LRESULT)TreeView_InsertItem(GetDlgItem(hWnd, IDC_EVENTOPTIONS), &tvi);
			}

			TOptionListItem *defaultItems = CTranslator::getTree(CTranslator::TREE_NEN);
			for (int i=0; defaultItems[i].szName != 0; i++) {
				TVINSERTSTRUCT tvi = { 0 };
				tvi.hParent = (HTREEITEM)lGroups[defaultItems[i].uGroup].handle;
				tvi.hInsertAfter = TVI_LAST;
				tvi.item.pszText = TranslateTS(defaultItems[i].szName);
				tvi.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
				tvi.item.lParam = i;
				tvi.item.stateMask = TVIS_STATEIMAGEMASK;
				if (defaultItems[i].uType == LOI_TYPE_SETTING)
					tvi.item.state = INDEXTOSTATEIMAGEMASK(*((BOOL *)defaultItems[i].lParam) ? 3 : 2);//2 : 1);
				else if (defaultItems[i].uType == LOI_TYPE_FLAG) {
					UINT uVal = *((UINT *)defaultItems[i].lParam);
					tvi.item.state = INDEXTOSTATEIMAGEMASK(uVal & defaultItems[i].id ? 3 : 2);//2 : 1);
				}
				defaultItems[i].handle = (LRESULT)TreeView_InsertItem(GetDlgItem(hWnd, IDC_EVENTOPTIONS), &tvi);
			}

			SendDlgItemMessage(hWnd, IDC_COLBACK_MESSAGE, CPM_SETCOLOUR, 0, options->colBackMsg);
			SendDlgItemMessage(hWnd, IDC_COLTEXT_MESSAGE, CPM_SETCOLOUR, 0, options->colTextMsg);
			SendDlgItemMessage(hWnd, IDC_COLBACK_OTHERS, CPM_SETCOLOUR, 0, options->colBackOthers);
			SendDlgItemMessage(hWnd, IDC_COLTEXT_OTHERS, CPM_SETCOLOUR, 0, options->colTextOthers);
			SendDlgItemMessage(hWnd, IDC_COLBACK_ERR, CPM_SETCOLOUR, 0, options->colBackErr);
			SendDlgItemMessage(hWnd, IDC_COLTEXT_ERR, CPM_SETCOLOUR, 0, options->colTextErr);
			CheckDlgButton(hWnd, IDC_CHKDEFAULTCOL_MESSAGE, options->bDefaultColorMsg ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_CHKDEFAULTCOL_OTHERS, options->bDefaultColorOthers ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_CHKDEFAULTCOL_ERR, options->bDefaultColorErr ? BST_CHECKED : BST_UNCHECKED);

			SendDlgItemMessage(hWnd, IDC_COLTEXT_MUC, CPM_SETCOLOUR, 0, g_Settings.crPUTextColour);
			SendDlgItemMessage(hWnd, IDC_COLBACK_MUC, CPM_SETCOLOUR, 0, g_Settings.crPUBkgColour);
			CheckDlgButton(hWnd, IDC_CHKDEFAULTCOL_MUC, g_Settings.iPopupStyle == 2 ? BST_CHECKED : BST_UNCHECKED);

			SendDlgItemMessage(hWnd, IDC_DELAY_MESSAGE_SPIN, UDM_SETRANGE, 0, MAKELONG(3600, -1));
			SendDlgItemMessage(hWnd, IDC_DELAY_OTHERS_SPIN, UDM_SETRANGE, 0, MAKELONG(3600, -1));
			SendDlgItemMessage(hWnd, IDC_DELAY_MESSAGE_MUC_SPIN, UDM_SETRANGE, 0, MAKELONG(3600, -1));
			SendDlgItemMessage(hWnd, IDC_DELAY_ERR_SPIN, UDM_SETRANGE, 0, MAKELONG(3600, -1));

			SendDlgItemMessage(hWnd, IDC_DELAY_MESSAGE_SPIN, UDM_SETPOS, 0, (LPARAM)options->iDelayMsg);
			SendDlgItemMessage(hWnd, IDC_DELAY_OTHERS_SPIN, UDM_SETPOS, 0, (LPARAM)options->iDelayOthers);
			SendDlgItemMessage(hWnd, IDC_DELAY_ERR_SPIN, UDM_SETPOS, 0, (LPARAM)options->iDelayErr);
			SendDlgItemMessage(hWnd, IDC_DELAY_MESSAGE_MUC_SPIN, UDM_SETPOS, 0, (LPARAM)g_Settings.iPopupTimeout);

			Utils::enableDlgControl(hWnd, IDC_COLBACK_MESSAGE, !options->bDefaultColorMsg);
			Utils::enableDlgControl(hWnd, IDC_COLTEXT_MESSAGE, !options->bDefaultColorMsg);
			Utils::enableDlgControl(hWnd, IDC_COLBACK_OTHERS, !options->bDefaultColorOthers);
			Utils::enableDlgControl(hWnd, IDC_COLTEXT_OTHERS, !options->bDefaultColorOthers);
			Utils::enableDlgControl(hWnd, IDC_COLBACK_ERR, !options->bDefaultColorErr);
			Utils::enableDlgControl(hWnd, IDC_COLTEXT_ERR, !options->bDefaultColorErr);
			Utils::enableDlgControl(hWnd, IDC_COLTEXT_MUC,  (g_Settings.iPopupStyle == 3) ? TRUE : FALSE);
			Utils::enableDlgControl(hWnd, IDC_COLBACK_MUC,  (g_Settings.iPopupStyle == 3) ? TRUE : FALSE);

			CheckDlgButton(hWnd, IDC_MUC_LOGCOLORS, g_Settings.iPopupStyle < 2 ? BST_CHECKED : BST_UNCHECKED);
			Utils::enableDlgControl(hWnd, IDC_MUC_LOGCOLORS, g_Settings.iPopupStyle != 2 ? TRUE : FALSE);

			SetDlgItemInt(hWnd, IDC_MESSAGEPREVIEWLIMIT, options->iLimitPreview, FALSE);
			CheckDlgButton(hWnd, IDC_LIMITPREVIEW, (options->iLimitPreview > 0) ? BST_CHECKED : BST_UNCHECKED);
			SendDlgItemMessage(hWnd, IDC_MESSAGEPREVIEWLIMITSPIN, UDM_SETRANGE, 0, MAKELONG(2048, options->iLimitPreview > 0 ? 50 : 0));
			SendDlgItemMessage(hWnd, IDC_MESSAGEPREVIEWLIMITSPIN, UDM_SETPOS, 0, (LPARAM)options->iLimitPreview);
			Utils::enableDlgControl(hWnd, IDC_MESSAGEPREVIEWLIMIT, IsDlgButtonChecked(hWnd, IDC_LIMITPREVIEW));
			Utils::enableDlgControl(hWnd, IDC_MESSAGEPREVIEWLIMITSPIN, IsDlgButtonChecked(hWnd, IDC_LIMITPREVIEW));

			bWmNotify = FALSE;
		}
		return TRUE;

	case DM_STATUSMASKSET:
		db_set_dw(0, MODULE, "statusmask", (DWORD)lParam);
		options->dwStatusMask = (int)lParam;
		break;

	case WM_COMMAND:
		if (!bWmNotify) {
			HWND hwndNew;
			switch (LOWORD(wParam)) {
			case IDC_PREVIEW:
				PopupPreview(options);
				break;

			case IDC_POPUPSTATUSMODES:
				hwndNew = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_CHOOSESTATUSMODES), hWnd, DlgProcSetupStatusModes, M.GetDword(MODULE, "statusmask", (DWORD) - 1));
				SendMessage(hwndNew, DM_SETPARENTDIALOG, 0, (LPARAM)hWnd);
				break;

			default:
				if (IsDlgButtonChecked(hWnd, IDC_CHKDEFAULTCOL_MUC))
					g_Settings.iPopupStyle = 2;
				else if (IsDlgButtonChecked(hWnd, IDC_MUC_LOGCOLORS))
					g_Settings.iPopupStyle = 1;
				else
					g_Settings.iPopupStyle = 3;

				Utils::enableDlgControl(hWnd, IDC_MUC_LOGCOLORS, g_Settings.iPopupStyle != 2 ? TRUE : FALSE);

				options->bDefaultColorMsg = IsDlgButtonChecked(hWnd, IDC_CHKDEFAULTCOL_MESSAGE);
				options->bDefaultColorOthers = IsDlgButtonChecked(hWnd, IDC_CHKDEFAULTCOL_OTHERS);
				options->bDefaultColorErr = IsDlgButtonChecked(hWnd, IDC_CHKDEFAULTCOL_ERR);

				options->iDelayMsg = SendDlgItemMessage(hWnd, IDC_DELAY_MESSAGE_SPIN, UDM_GETPOS, 0, 0);
				options->iDelayOthers = SendDlgItemMessage(hWnd, IDC_DELAY_OTHERS_SPIN, UDM_GETPOS, 0, 0);
				options->iDelayErr = SendDlgItemMessage(hWnd, IDC_DELAY_ERR_SPIN, UDM_GETPOS, 0, 0);

				g_Settings.iPopupTimeout = SendDlgItemMessage(hWnd, IDC_DELAY_MESSAGE_MUC_SPIN, UDM_GETPOS, 0, 0);

				if (IsDlgButtonChecked(hWnd, IDC_LIMITPREVIEW))
					options->iLimitPreview = GetDlgItemInt(hWnd, IDC_MESSAGEPREVIEWLIMIT, NULL, FALSE);
				else
					options->iLimitPreview = 0;
				Utils::enableDlgControl(hWnd, IDC_COLBACK_MESSAGE, !options->bDefaultColorMsg);
				Utils::enableDlgControl(hWnd, IDC_COLTEXT_MESSAGE, !options->bDefaultColorMsg);
				Utils::enableDlgControl(hWnd, IDC_COLBACK_OTHERS, !options->bDefaultColorOthers);
				Utils::enableDlgControl(hWnd, IDC_COLTEXT_OTHERS, !options->bDefaultColorOthers);
				Utils::enableDlgControl(hWnd, IDC_COLBACK_ERR, !options->bDefaultColorErr);
				Utils::enableDlgControl(hWnd, IDC_COLTEXT_ERR, !options->bDefaultColorErr);
				Utils::enableDlgControl(hWnd, IDC_COLTEXT_MUC,  (g_Settings.iPopupStyle == 3) ? TRUE : FALSE);
				Utils::enableDlgControl(hWnd, IDC_COLBACK_MUC,  (g_Settings.iPopupStyle == 3) ? TRUE : FALSE);

				Utils::enableDlgControl(hWnd, IDC_MESSAGEPREVIEWLIMIT, IsDlgButtonChecked(hWnd, IDC_LIMITPREVIEW));
				Utils::enableDlgControl(hWnd, IDC_MESSAGEPREVIEWLIMITSPIN, IsDlgButtonChecked(hWnd, IDC_LIMITPREVIEW));
				//disable delay textbox when infinite is checked

				Utils::enableDlgControl(hWnd, IDC_DELAY_MESSAGE, options->iDelayMsg != -1);
				Utils::enableDlgControl(hWnd, IDC_DELAY_OTHERS, options->iDelayOthers != -1);
				Utils::enableDlgControl(hWnd, IDC_DELAY_ERR, options->iDelayErr != -1);
				Utils::enableDlgControl(hWnd, IDC_DELAY_MUC, g_Settings.iPopupTimeout != -1);

				if (HIWORD(wParam) == CPN_COLOURCHANGED) {
					options->colBackMsg = SendDlgItemMessage(hWnd, IDC_COLBACK_MESSAGE, CPM_GETCOLOUR, 0, 0);
					options->colTextMsg = SendDlgItemMessage(hWnd, IDC_COLTEXT_MESSAGE, CPM_GETCOLOUR, 0, 0);
					options->colBackOthers = SendDlgItemMessage(hWnd, IDC_COLBACK_OTHERS, CPM_GETCOLOUR, 0, 0);
					options->colTextOthers = SendDlgItemMessage(hWnd, IDC_COLTEXT_OTHERS, CPM_GETCOLOUR, 0, 0);
					options->colBackErr = SendDlgItemMessage(hWnd, IDC_COLBACK_ERR, CPM_GETCOLOUR, 0, 0);
					options->colTextErr = SendDlgItemMessage(hWnd, IDC_COLTEXT_ERR, CPM_GETCOLOUR, 0, 0);
					g_Settings.crPUBkgColour = SendDlgItemMessage(hWnd, IDC_COLBACK_MUC, CPM_GETCOLOUR, 0, 0);
					g_Settings.crPUTextColour = SendDlgItemMessage(hWnd, IDC_COLTEXT_MUC, CPM_GETCOLOUR, 0, 0);
				}
				SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
				break;
			}
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR) lParam)->idFrom == IDC_EVENTOPTIONS && ((LPNMHDR)lParam)->code == NM_CLICK) {
			TVHITTESTINFO hti;
			TVITEM item = {0};

			item.mask = TVIF_HANDLE | TVIF_STATE;
			item.stateMask = TVIS_STATEIMAGEMASK | TVIS_BOLD;
			hti.pt.x = (short)LOWORD(GetMessagePos());
			hti.pt.y = (short)HIWORD(GetMessagePos());
			ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
			if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti)) {
				item.hItem = (HTREEITEM)hti.hItem;
				SendDlgItemMessageA(hWnd, IDC_EVENTOPTIONS, TVM_GETITEMA, 0, (LPARAM)&item);
				if (item.state & TVIS_BOLD && hti.flags & TVHT_ONITEMSTATEICON) {
					item.state = INDEXTOSTATEIMAGEMASK(0) | TVIS_BOLD;
					SendDlgItemMessageA(hWnd, IDC_EVENTOPTIONS, TVM_SETITEMA, 0, (LPARAM)&item);
				}
				else if (hti.flags&TVHT_ONITEMSTATEICON) {
					if (((item.state & TVIS_STATEIMAGEMASK) >> 12) == 3) {
						item.state = INDEXTOSTATEIMAGEMASK(1);
						SendDlgItemMessageA(hWnd, IDC_EVENTOPTIONS, TVM_SETITEMA, 0, (LPARAM)&item);
					}
					SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
				}
			}
		}

		switch (((LPNMHDR)lParam)->code) {
		case PSN_RESET:
			NEN_ReadOptions(&nen_options);
			break;

		case PSN_APPLY:
			TOptionListItem *defaultItems = CTranslator::getTree(CTranslator::TREE_NEN);
			for (int i=0; defaultItems[i].szName != NULL; i++) {
				TVITEM item = {0};
				item.mask = TVIF_HANDLE | TVIF_STATE;
				item.hItem = (HTREEITEM)defaultItems[i].handle;
				item.stateMask = TVIS_STATEIMAGEMASK;

				SendDlgItemMessageA(hWnd, IDC_EVENTOPTIONS, TVM_GETITEMA, 0, (LPARAM)&item);
				if (defaultItems[i].uType == LOI_TYPE_SETTING) {
					BOOL *ptr = (BOOL *)defaultItems[i].lParam;
					*ptr = (item.state >> 12) == 3/*2*/ ? TRUE : FALSE;
				}
				else if (defaultItems[i].uType == LOI_TYPE_FLAG) {
					UINT *uVal = (UINT *)defaultItems[i].lParam;
					*uVal = ((item.state >> 12) == 3/*2*/) ? *uVal | defaultItems[i].id : *uVal & ~defaultItems[i].id;
				}
			}

			db_set_b(0, CHAT_MODULE, "PopupStyle", (BYTE)g_Settings.iPopupStyle);
			db_set_w(NULL, CHAT_MODULE, "PopupTimeout", g_Settings.iPopupTimeout);

			g_Settings.crPUBkgColour = SendDlgItemMessage(hWnd, IDC_COLBACK_MUC, CPM_GETCOLOUR, 0, 0);
			db_set_dw(0, CHAT_MODULE, "PopupColorBG", (DWORD)g_Settings.crPUBkgColour);
			g_Settings.crPUTextColour = SendDlgItemMessage(hWnd, IDC_COLTEXT_MUC, CPM_GETCOLOUR, 0, 0);
			db_set_dw(0, CHAT_MODULE, "PopupColorText", (DWORD)g_Settings.crPUTextColour);

			NEN_WriteOptions(&nen_options);
			CheckForRemoveMask();
			CreateSystrayIcon(nen_options.bTraySupport);
			SetEvent(g_hEvent);                                 // wake up the thread which cares about the floater and tray
		}
		break;

	case WM_DESTROY:
		bWmNotify = TRUE;
		break;
	}

	return FALSE;
}

static int PopupAct(HWND hWnd, UINT mask, PLUGIN_DATAT* pdata)
{
	pdata->iActionTaken = TRUE;
	if (mask & MASK_OPEN) {
		for (int i=0; i < pdata->nrMerged; i++) {
			if (pdata->eventData[i].hEvent != 0) {
				PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_HANDLECLISTEVENT, (WPARAM)pdata->hContact, (LPARAM)pdata->eventData[i].hEvent);
				pdata->eventData[i].hEvent = 0;
			}
		}
	}
	if (mask & MASK_REMOVE) {
		for (int i=0; i < pdata->nrMerged; i++) {
			if (pdata->eventData[i].hEvent != 0) {
				PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_REMOVECLISTEVENT, (WPARAM)pdata->hContact, (LPARAM)pdata->eventData[i].hEvent);
				pdata->eventData[i].hEvent = 0;
			}
		}
	}
	if (mask & MASK_DISMISS) {
		PUDeletePopup(hWnd);
		if (pdata->hContainer) {
			FLASHWINFO fwi = { sizeof(fwi) };
			fwi.dwFlags = FLASHW_STOP;
			fwi.hwnd = pdata->hContainer;
			FlashWindowEx(&fwi);
		}
	}
	return 0;
}

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PLUGIN_DATAT *pdata = (PLUGIN_DATAT*)PUGetPluginData(hWnd);
	if (pdata == NULL)
		return FALSE;

	switch (message) {
	case WM_COMMAND:
		PopupAct(hWnd, pdata->pluginOptions->maskActL, pdata);
		break;
	case WM_CONTEXTMENU:
		PopupAct(hWnd, pdata->pluginOptions->maskActR, pdata);
		break;
	case UM_FREEPLUGINDATA:
		pdata->hContact = 0;								// mark as removeable
		pdata->hWnd = 0;
		return TRUE;
	case UM_INITPOPUP:
		pdata->hWnd = hWnd;
		if (pdata->iSeconds > 0)
			SetTimer(hWnd, TIMER_TO_ACTION, pdata->iSeconds * 1000, NULL);
		break;
	case WM_MOUSEWHEEL:
		break;
	case WM_SETCURSOR:
		break;
	case WM_TIMER:
		POINT	pt;
		RECT	rc;

		if (wParam != TIMER_TO_ACTION)
			break;

		GetCursorPos(&pt);
		GetWindowRect(hWnd, &rc);
		if (PtInRect(&rc, pt))
			break;

		if (pdata->iSeconds > 0)
			KillTimer(hWnd, TIMER_TO_ACTION);
		PopupAct(hWnd, pdata->pluginOptions->maskActTE, pdata);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/**
* Get a preview for the message
* caller must always mir_free() the return value
*
* @param eventType the event type
* @param dbe       DBEVENTINFO *: database event structure
*
* @return
*/

static TCHAR* ShortenPreview(DBEVENTINFO* dbe)
{
	bool	fAddEllipsis = false;
	size_t iPreviewLimit = nen_options.iLimitPreview;
	if (iPreviewLimit > 500 || iPreviewLimit == 0)
		iPreviewLimit = 500;

	TCHAR* buf = DbGetEventTextT(dbe, CP_ACP);
	if (mir_tstrlen(buf) > iPreviewLimit) {
		fAddEllipsis = true;
		size_t iIndex = iPreviewLimit;
		size_t iWordThreshold = 20;
		while(iIndex && buf[iIndex] != ' ' && iWordThreshold--)
			buf[iIndex--] = 0;

		buf[iIndex] = 0;
	}
	if (fAddEllipsis) {
		buf = (TCHAR*)mir_realloc(buf, (mir_tstrlen(buf) + 5) * sizeof(TCHAR));
		_tcscat(buf, _T("..."));
	}
	return buf;
}

static TCHAR* GetPreviewT(WORD eventType, DBEVENTINFO* dbe)
{
	char *pBlob = (char *)dbe->pBlob;

	switch (eventType) {
	case EVENTTYPE_MESSAGE:
		if (pBlob && nen_options.bPreview)
			return ShortenPreview(dbe);

		return mir_tstrdup( TranslateT("Message"));

	case EVENTTYPE_FILE:
		if (pBlob) {
			if (!nen_options.bPreview)
				return mir_tstrdup(TranslateT("Incoming file"));

			if (dbe->cbBlob > 5) { // min valid size = (sizeof(DWORD) + 1 character file name + terminating 0)
				char* szFileName = (char *)dbe->pBlob + sizeof(DWORD);
				char* szDescr = 0;
				size_t namelength = Utils::safe_strlen(szFileName, dbe->cbBlob - sizeof(DWORD));

				if (dbe->cbBlob > (sizeof(DWORD) + namelength + 1))
					szDescr = szFileName + namelength + 1;

				ptrT tszFileName( DbGetEventStringT(dbe, szFileName));
				TCHAR buf[1024];

				if (szDescr && Utils::safe_strlen(szDescr, dbe->cbBlob - sizeof(DWORD) - namelength - 1) > 0) {
					ptrT tszDescr( DbGetEventStringT(dbe, szDescr));
					if (tszFileName && tszDescr) {
						mir_sntprintf(buf, SIZEOF(buf), _T("%s: %s (%s)"), TranslateT("Incoming file"), tszFileName, tszDescr);
						return mir_tstrdup(buf);
					}
				}

				if (tszFileName) {
					mir_sntprintf(buf, SIZEOF(buf), _T("%s: %s (%s)"), TranslateT("Incoming file"), tszFileName, TranslateT("No description given"));
					return mir_tstrdup(buf);
				}
			}
		}
		return mir_tstrdup(TranslateT("Incoming file (invalid format)"));

	default:
		if (nen_options.bPreview)
			return ShortenPreview(dbe);

		return mir_tstrdup(TranslateT("Unknown event"));
	}
}

static int PopupUpdateT(MCONTACT hContact, HANDLE hEvent)
{
	PLUGIN_DATAT *pdata = const_cast<PLUGIN_DATAT *>(PU_GetByContact(hContact));
	if (!pdata)
		return 1;

	if (hEvent == NULL)
		return 0;

	TCHAR szHeader[256];
	if (pdata->pluginOptions->bShowHeaders)
		mir_sntprintf(szHeader, SIZEOF(szHeader), _T("%s %d\n"), TranslateT("New messages: "), pdata->nrMerged + 1);
	else
		szHeader[0] = 0;

	DBEVENTINFO dbe = { sizeof(dbe) };
	if (pdata->pluginOptions->bPreview && hContact) {
		dbe.cbBlob = db_event_getBlobSize(hEvent);
		dbe.pBlob = (PBYTE)mir_alloc(dbe.cbBlob);
	}
	db_event_get(hEvent, &dbe);

	TCHAR timestamp[MAX_DATASIZE];
	_tcsftime(timestamp, MAX_DATASIZE, _T("%Y.%m.%d %H:%M"), _localtime32((__time32_t *)&dbe.timestamp));
	mir_sntprintf(pdata->eventData[pdata->nrMerged].tszText, SIZEOF(pdata->eventData[pdata->nrMerged].tszText), _T("\n\n%s\n"), timestamp);

	TCHAR *szPreview = GetPreviewT(dbe.eventType, &dbe);
	if (szPreview) {
		_tcsncat(pdata->eventData[pdata->nrMerged].tszText, szPreview, MAX_SECONDLINE);
		mir_free(szPreview);
	}
	else _tcsncat(pdata->eventData[pdata->nrMerged].tszText, _T(" "), MAX_SECONDLINE);

	pdata->eventData[pdata->nrMerged].tszText[MAX_SECONDLINE - 1] = 0;

	/*
	* now, reassemble the popup text, make sure the *last* event is shown, and then show the most recent events
	* for which there is enough space in the popup text
	*/

	TCHAR lpzText[MAX_SECONDLINE];
	int i, available = MAX_SECONDLINE - 1;
	if (pdata->pluginOptions->bShowHeaders) {
		_tcsncpy(lpzText, szHeader, MAX_SECONDLINE);
		available -= (int)mir_tstrlen(szHeader);
	}
	else lpzText[0] = 0;

	for (i = pdata->nrMerged; i >= 0; i--) {
		available -= (int)mir_tstrlen(pdata->eventData[i].tszText);
		if (available <= 0)
			break;
	}
	i = (available > 0) ? i + 1 : i + 2;
	for (; i <= pdata->nrMerged; i++)
		_tcsncat(lpzText, pdata->eventData[i].tszText, SIZEOF(lpzText));

	pdata->eventData[pdata->nrMerged].hEvent = hEvent;
	pdata->eventData[pdata->nrMerged].timestamp = dbe.timestamp;
	pdata->nrMerged++;
	if (pdata->nrMerged >= pdata->nrEventsAlloced) {
		pdata->nrEventsAlloced += 5;
		pdata->eventData = (EVENT_DATAT *)mir_realloc(pdata->eventData, pdata->nrEventsAlloced * sizeof(EVENT_DATAT));
	}
	if (dbe.pBlob)
		mir_free(dbe.pBlob);

	PUChangeTextT(pdata->hWnd, lpzText);
	return 0;
}

static int PopupShowT(NEN_OPTIONS *pluginOptions, MCONTACT hContact, HANDLE hEvent, UINT eventType, HWND hContainer)
{
	//there has to be a maximum number of popups shown at the same time
	if (arPopupList.getCount() >= MAX_POPUPS)
		return 2;

	if (!PluginConfig.g_PopupAvail)
		return 0;

	DBEVENTINFO dbe = { sizeof(dbe) };
	// fix for a crash
	if (hEvent && (pluginOptions->bPreview || hContact == 0)) {
		dbe.cbBlob = db_event_getBlobSize(hEvent);
		dbe.pBlob = (PBYTE)mir_alloc(dbe.cbBlob);
	}
	db_event_get(hEvent, &dbe);

	if (hEvent == 0 && hContact == 0)
		dbe.szModule = Translate("Unknown module or contact");

	POPUPDATAT pud = {0};
	long iSeconds;
	switch (eventType) {
	case EVENTTYPE_MESSAGE:
		pud.lchIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
		pud.colorBack = pluginOptions->bDefaultColorMsg ? 0 : pluginOptions->colBackMsg;
		pud.colorText = pluginOptions->bDefaultColorMsg ? 0 : pluginOptions->colTextMsg;
		iSeconds = pluginOptions->iDelayMsg;
		break;

	default:
		pud.lchIcon = (HICON)CallService(MS_DB_EVENT_GETICON, LR_SHARED, (LPARAM)&dbe);
		pud.colorBack = pluginOptions->bDefaultColorOthers ? 0 : pluginOptions->colBackOthers;
		pud.colorText = pluginOptions->bDefaultColorOthers ? 0 : pluginOptions->colTextOthers;
		iSeconds = pluginOptions->iDelayOthers;
		break;
	}

	PLUGIN_DATAT *pdata = (PLUGIN_DATAT *)mir_calloc(sizeof(PLUGIN_DATAT));
	pdata->eventType = eventType;
	pdata->hContact = hContact;
	pdata->pluginOptions = pluginOptions;
	pdata->pud = &pud;
	pdata->iSeconds = iSeconds; // ? iSeconds : pluginOptions->iDelayDefault;
	pdata->hContainer = hContainer;
	pud.iSeconds = pdata->iSeconds ? -1 : 0;

	//finally create the popup
	pud.lchContact = hContact;
	pud.PluginWindowProc = PopupDlgProc;
	pud.PluginData = pdata;

	if (hContact)
		_tcsncpy_s(pud.lptzContactName, pcli->pfnGetContactDisplayName(hContact, 0), _TRUNCATE);
	else
		_tcsncpy_s(pud.lptzContactName, _A2T(dbe.szModule), _TRUNCATE);

	TCHAR *szPreview = GetPreviewT((WORD)eventType, &dbe);
	if (szPreview) {
		_tcsncpy_s(pud.lptzText, szPreview, _TRUNCATE);
		mir_free(szPreview);
	}
	else _tcsncpy(pud.lptzText, _T(" "), MAX_SECONDLINE);

	pdata->eventData = (EVENT_DATAT *)mir_alloc(NR_MERGED * sizeof(EVENT_DATAT));
	pdata->eventData[0].hEvent = hEvent;
	pdata->eventData[0].timestamp = dbe.timestamp;
	_tcsncpy(pdata->eventData[0].tszText, pud.lptzText, MAX_SECONDLINE);
	pdata->eventData[0].tszText[MAX_SECONDLINE - 1] = 0;
	pdata->nrEventsAlloced = NR_MERGED;
	pdata->nrMerged = 1;

	// fix for broken popups -- process failures
	if ( PUAddPopupT(&pud) < 0) {
		mir_free(pdata->eventData);
		mir_free(pdata);
	}
	else arPopupList.insert(pdata);

	if (dbe.pBlob)
		mir_free(dbe.pBlob);

	return 0;
}

static int TSAPI PopupPreview(NEN_OPTIONS *pluginOptions)
{
	PopupShowT(pluginOptions, NULL, NULL, EVENTTYPE_MESSAGE, NULL);
	return 0;
}

/*
* updates the menu entry...
* bForced is used to only update the status, nickname etc. and does NOT update the unread count
*/
void TSAPI UpdateTrayMenuState(TWindowData *dat, BOOL bForced)
{
	if (PluginConfig.g_hMenuTrayUnread == 0 || dat->hContact == NULL)
		return;

	MENUITEMINFO mii = {0};
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_DATA | MIIM_BITMAP;

	const TCHAR *tszProto = dat->cache->getRealAccount();
	assert(tszProto != 0);

	GetMenuItemInfo(PluginConfig.g_hMenuTrayUnread, (UINT_PTR)dat->hContact, FALSE, &mii);
	if (!bForced)
		PluginConfig.m_UnreadInTray -= (mii.dwItemData & 0x0000ffff);
	if (mii.dwItemData > 0 || bForced) {
		TCHAR szMenuEntry[80];
		mir_sntprintf(szMenuEntry, SIZEOF(szMenuEntry), _T("%s: %s (%s) [%d]"), tszProto,
			dat->cache->getNick(), dat->szStatus[0] ? dat->szStatus : _T("(undef)"), mii.dwItemData & 0x0000ffff);

		if (!bForced)
			mii.dwItemData = 0;
		mii.fMask |= MIIM_STRING;
		mii.dwTypeData = (LPTSTR)szMenuEntry;
		mii.cch = (int)mir_tstrlen(szMenuEntry) + 1;
	}
	mii.hbmpItem = HBMMENU_CALLBACK;
	SetMenuItemInfo(PluginConfig.g_hMenuTrayUnread, (UINT_PTR)dat->hContact, FALSE, &mii);
}

/*
 * if we want tray support, add the contact to the list of unread sessions in the tray menu
 */

int TSAPI UpdateTrayMenu(const TWindowData *dat, WORD wStatus, const char *szProto, const TCHAR *szStatus, MCONTACT hContact, DWORD fromEvent)
{
	if (!PluginConfig.g_hMenuTrayUnread || hContact == 0 || szProto == NULL)
		return 0;

	PROTOACCOUNT *acc = ProtoGetAccount(szProto);
	TCHAR *tszFinalProto = (acc && acc->tszAccountName ? acc->tszAccountName : 0);
	if (tszFinalProto == 0)
		return 0;

	WORD wMyStatus = (wStatus == 0) ? db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE) : wStatus;
	const TCHAR	*szMyStatus = (szStatus == NULL) ? pcli->pfnGetStatusModeDescription(wMyStatus, 0) : szStatus;

	MENUITEMINFO mii = { sizeof(mii) };
	mii.fMask = MIIM_DATA | MIIM_ID | MIIM_BITMAP;
	mii.wID = (UINT)hContact;
	mii.hbmpItem = HBMMENU_CALLBACK;

	TCHAR	szMenuEntry[80];
	const TCHAR *szNick = NULL;
	if (dat != 0) {
		szNick = dat->cache->getNick();
		GetMenuItemInfo(PluginConfig.g_hMenuTrayUnread, (UINT_PTR)hContact, FALSE, &mii);
		mii.dwItemData++;
		if (fromEvent == 2)                         // from chat...
			mii.dwItemData |= 0x10000000;
		DeleteMenu(PluginConfig.g_hMenuTrayUnread, (UINT_PTR)hContact, MF_BYCOMMAND);
		mir_sntprintf(szMenuEntry, SIZEOF(szMenuEntry), _T("%s: %s (%s) [%d]"), tszFinalProto, szNick, szMyStatus, mii.dwItemData & 0x0000ffff);
		AppendMenu(PluginConfig.g_hMenuTrayUnread, MF_BYCOMMAND | MF_STRING, (UINT_PTR)hContact, szMenuEntry);
		PluginConfig.m_UnreadInTray++;
		if (PluginConfig.m_UnreadInTray)
			SetEvent(g_hEvent);
		SetMenuItemInfo(PluginConfig.g_hMenuTrayUnread, (UINT_PTR)hContact, FALSE, &mii);
	}
	else {
		szNick = pcli->pfnGetContactDisplayName(hContact, 0);
		if (CheckMenuItem(PluginConfig.g_hMenuTrayUnread, (UINT_PTR)hContact, MF_BYCOMMAND | MF_UNCHECKED) == -1) {
			mir_sntprintf(szMenuEntry, SIZEOF(szMenuEntry), _T("%s: %s (%s) [%d]"), tszFinalProto, szNick, szMyStatus, fromEvent ? 1 : 0);
			AppendMenu(PluginConfig.g_hMenuTrayUnread, MF_BYCOMMAND | MF_STRING, (UINT_PTR)hContact, szMenuEntry);
			mii.dwItemData = fromEvent ? 1 : 0;
			PluginConfig.m_UnreadInTray += (mii.dwItemData & 0x0000ffff);
			if (PluginConfig.m_UnreadInTray)
				SetEvent(g_hEvent);
			if (fromEvent == 2)
				mii.dwItemData |= 0x10000000;
		}
		else {
			GetMenuItemInfo(PluginConfig.g_hMenuTrayUnread, (UINT_PTR)hContact, FALSE, &mii);
			mii.dwItemData += (fromEvent ? 1 : 0);
			PluginConfig.m_UnreadInTray += (fromEvent ? 1 : 0);
			if (PluginConfig.m_UnreadInTray)
				SetEvent(g_hEvent);
			mii.fMask |= MIIM_STRING;
			if (fromEvent == 2)
				mii.dwItemData |= 0x10000000;
			mir_sntprintf(szMenuEntry, SIZEOF(szMenuEntry), _T("%s: %s (%s) [%d]"), tszFinalProto, szNick, szMyStatus, mii.dwItemData & 0x0000ffff);
			mii.cch = (int)mir_tstrlen(szMenuEntry) + 1;
			mii.dwTypeData = (LPTSTR)szMenuEntry;
		}
		SetMenuItemInfo(PluginConfig.g_hMenuTrayUnread, (UINT_PTR)hContact, FALSE, &mii);
	}
	return 0;
}

int tabSRMM_ShowPopup(MCONTACT hContact, HANDLE hDbEvent, WORD eventType, int windowOpen, TContainerData *pContainer, HWND hwndChild, const char *szProto)
{
	if (nen_options.iDisable) // no popups at all. Period
		return 0;

	PU_CleanUp();

	if (nen_options.bDisableNonMessage && eventType != EVENTTYPE_MESSAGE)
		return 0;

	/*
	* check the status mode against the status mask
	*/

	if (nen_options.dwStatusMask != -1) {
		if (szProto != NULL) {
			DWORD dwStatus = (DWORD)CallProtoService(szProto, PS_GETSTATUS, 0, 0);
			if (!(dwStatus == 0 || dwStatus <= ID_STATUS_OFFLINE || ((1 << (dwStatus - ID_STATUS_ONLINE)) & nen_options.dwStatusMask)))           // should never happen, but...
				return 0;
		}
	}
	if (nen_options.bNoRSS && szProto != NULL && !strncmp(szProto, "RSS", 3))
		return 0;                                        // filter out RSS popups

	if (windowOpen && pContainer != 0) {               // message window is open, need to check the container config if we want to see a popup nonetheless
		if (nen_options.bWindowCheck && windowOpen)                  // no popups at all for open windows... no exceptions
			return 0;
		if (pContainer->dwFlags & CNT_DONTREPORT && (IsIconic(pContainer->hwnd)))        // in tray counts as "minimised"
			goto passed;

		if (pContainer->dwFlags & CNT_DONTREPORTUNFOCUSED)
			if (!IsIconic(pContainer->hwnd) && GetForegroundWindow() != pContainer->hwnd && GetActiveWindow() != pContainer->hwnd)
				goto passed;

		if (pContainer->dwFlags & CNT_ALWAYSREPORTINACTIVE) {
			if (pContainer->dwFlags & CNT_DONTREPORTFOCUSED)
				goto passed;

			if (pContainer->hwndActive != hwndChild)
				goto passed;
		}
		return 0;
	}
passed:
	if ( !PluginConfig.g_PopupAvail)
		return 0;

	if ( PU_GetByContact(hContact) && nen_options.bMergePopup && eventType == EVENTTYPE_MESSAGE) {
		if (PopupUpdateT(hContact, hDbEvent) != 0)
			PopupShowT(&nen_options, hContact, hDbEvent, eventType, pContainer ? pContainer->hwnd : 0);
	}
	else PopupShowT(&nen_options, hContact, hDbEvent, eventType, pContainer ? pContainer->hwnd : 0);

	return 0;
}

/**
* remove all popups for hContact, but only if the mask matches the current "mode"
*/

void TSAPI DeletePopupsForContact(MCONTACT hContact, DWORD dwMask)
{
	if (!(dwMask & nen_options.dwRemoveMask) || nen_options.iDisable || !PluginConfig.g_PopupAvail)
		return;

	PLUGIN_DATAT *_T = 0;
	while ((_T = PU_GetByContact(hContact)) != 0) {
		_T->hContact = 0;									// make sure, it never "comes back"
		if (_T->hWnd != 0 && IsWindow(_T->hWnd))
			PUDeletePopup(_T->hWnd);
	}
}
