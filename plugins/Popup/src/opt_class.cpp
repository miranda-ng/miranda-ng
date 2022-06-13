/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
© 2004-2007 Victor Pavlychko
© 2010 MPK
© 2010 Merlin_de

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

//---------------------------------------------------------------------------
// Workaround for MS bug ComboBox_SelectItemData
int ComboBox_SelectItem(HWND hwndCtl, char *data) {
	int i = 0;
	for (i; i < ComboBox_GetCount(hwndCtl); i++) {
		if (mir_strcmp(data, (char*)ComboBox_GetItemData(hwndCtl, i)) == 0) {
			ComboBox_SetCurSel(hwndCtl, i);
			return i;
		}
	}
	return CB_ERR;
}

//---------------------------------------------------------------------------
// Dialog Proc

struct {
	UINT idCtrl;
	uint8_t onTyp0;
	uint8_t onTyp1;
	uint8_t onTyp2;
}
static ctrlsAll[] = {
	{ IDC_TXT_TITLE1, 0, 1, 1 },
	{ IDC_ENABLE, 0, 1, 1 },

	{ IDC_TXT_TIMEOUT, 0, 1, 1 },
	{ IDC_TIMEOUT, 0, 1, 1 },
	{ IDC_TIMEOUT_SPIN, 0, 1, 1 },
	{ IDC_TXT_TIMEOUT_SEC, 0, 1, 1 },
	{ IDC_TXT_TIMEOUT_DEFAULT, 0, 1, 1 },
	{ IDC_TXT_TIMEOUT_INFINITE, 0, 1, 1 },

	{ IDC_TXT_LACTION, 0, 1, 1 },
	{ IDC_LACTION, 0, 1, 1 },

	{ IDC_TXT_RACTION, 0, 1, 1 },
	{ IDC_RACTION, 0, 1, 1 },

	{ IDC_CHECKWINDOW, 0, 0, 0 },		// may be delete ??

	{ IDC_TXT_TITLE3, 0, 1, 0 },
	{ IDC_SOFFLINE, 0, 1, 0 },
	{ IDC_SONLINE, 0, 1, 0 },
	{ IDC_SAWAY, 0, 1, 0 },
	{ IDC_SNA, 0, 1, 0 },
	{ IDC_SOCCUPIED, 0, 1, 0 },
	{ IDC_SDND, 0, 1, 0 },
	{ IDC_SFREE4CHAT, 0, 1, 0 },
	{ IDC_SINVISIBLE, 0, 1, 0 },

	{ IDC_TXT_TITLE4, 0, 0, 0 },
	{ IDC_SOFFLINE2, 0, 0, 0 },
	{ IDC_SONLINE2, 0, 0, 0 },
	{ IDC_SAWAY2, 0, 0, 0 },
	{ IDC_SNA2, 0, 0, 0 },
	{ IDC_SOCCUPIED2, 0, 0, 0 },
	{ IDC_SDND2, 0, 0, 0 },
	{ IDC_SFREE4CHAT2, 0, 0, 0 },
	{ IDC_SINVISIBLE2, 0, 0, 0 },

	{ IDC_ICO_INFO, 0, 1, 1 },
	{ IDC_TXT_COLORS, 0, 1, 1 },
	{ IDC_MORE, 0, 1, 1 },

	{ IDC_PREVIEW, 0, 1, 1 },
};

static UINT ctrlsClass[] = {
	IDC_TXT_TITLE1,		/*IDC_TXT_TITLE3,*/
	IDC_TXT_TIMEOUT, IDC_TIMEOUT,
	IDC_TIMEOUT_SPIN, IDC_TXT_TIMEOUT_SEC,
	IDC_TXT_TIMEOUT_DEFAULT, IDC_TXT_TIMEOUT_INFINITE,
	IDC_TXT_LACTION, IDC_LACTION,
	IDC_TXT_RACTION, IDC_RACTION,
	IDC_ICO_INFO, IDC_TXT_COLORS,
};

static UINT ctrlsEnable[] = {
	IDC_TXT_TIMEOUT, IDC_TIMEOUT, IDC_TIMEOUT_SPIN,
	IDC_TXT_TIMEOUT_SEC, IDC_TXT_TIMEOUT_DEFAULT, IDC_TXT_TIMEOUT_INFINITE,
	IDC_TXT_LACTION, IDC_LACTION,
	IDC_TXT_RACTION, IDC_RACTION,
	IDC_TXT_TITLE3, IDC_TXT_TITLE4,
	IDC_ICO_INFO, IDC_TXT_COLORS, IDC_MORE,

	IDC_SOFFLINE, IDC_SONLINE, IDC_SAWAY, IDC_SNA,
	IDC_SOCCUPIED, IDC_SDND, IDC_SFREE4CHAT, IDC_SINVISIBLE,

	IDC_SOFFLINE2, IDC_SONLINE2, IDC_SAWAY2, IDC_SNA2,
	IDC_SOCCUPIED2, IDC_SDND2, IDC_SFREE4CHAT2, IDC_SINVISIBLE2,
};

static UINT ctrlsContact[] = {
	IDC_TXT_TITLE4,
	IDC_SOFFLINE2, IDC_SONLINE2, IDC_SAWAY2, IDC_SNA2,
	IDC_SOCCUPIED2, IDC_SDND2, IDC_SFREE4CHAT2, IDC_SINVISIBLE2,
};

static int titleIds[] = {
	IDC_TXT_TITLE1,
	IDC_TXT_TITLE3,
	IDC_TXT_TITLE4,
};

struct {
	int idCtrl;
	int iconId;
	char *title;
	uint32_t statusFlag;
	uint32_t disableWhenFlag;
}
static statusButtons[] = {
	{ IDC_SOFFLINE, SKINICON_STATUS_OFFLINE, "Offline", PF2_IDLE, PF2_IDLE },
	{ IDC_SONLINE, SKINICON_STATUS_ONLINE, "Online", PF2_ONLINE, PF2_ONLINE },
	{ IDC_SAWAY, SKINICON_STATUS_AWAY, "Away", PF2_SHORTAWAY, PF2_SHORTAWAY },
	{ IDC_SNA, SKINICON_STATUS_NA, "Not available", PF2_LONGAWAY, PF2_LONGAWAY },
	{ IDC_SOCCUPIED, SKINICON_STATUS_OCCUPIED, "Occupied", PF2_LIGHTDND, PF2_LIGHTDND },
	{ IDC_SDND, SKINICON_STATUS_DND, "Do not disturb", PF2_HEAVYDND, PF2_HEAVYDND },
	{ IDC_SFREE4CHAT, SKINICON_STATUS_FREE4CHAT, "Free for chat", PF2_FREECHAT, PF2_FREECHAT },
	{ IDC_SINVISIBLE, SKINICON_STATUS_INVISIBLE, "Invisible", PF2_INVISIBLE, PF2_INVISIBLE },

	{ IDC_SOFFLINE2, SKINICON_STATUS_OFFLINE, "Offline", PF2_IDLE, PF2_IDLE << 16 },
	{ IDC_SONLINE2, SKINICON_STATUS_ONLINE, "Online", PF2_ONLINE, PF2_ONLINE << 16 },
	{ IDC_SAWAY2, SKINICON_STATUS_AWAY, "Away", PF2_SHORTAWAY, PF2_SHORTAWAY << 16 },
	{ IDC_SNA2, SKINICON_STATUS_NA, "Not available", PF2_LONGAWAY, PF2_LONGAWAY << 16 },
	{ IDC_SOCCUPIED2, SKINICON_STATUS_OCCUPIED, "Occupied", PF2_LIGHTDND, PF2_LIGHTDND << 16 },
	{ IDC_SDND2, SKINICON_STATUS_DND, "Do not disturb", PF2_HEAVYDND, PF2_HEAVYDND << 16 },
	{ IDC_SFREE4CHAT2, SKINICON_STATUS_FREE4CHAT, "Free for chat", PF2_FREECHAT, PF2_FREECHAT << 16 },
	{ IDC_SINVISIBLE2, SKINICON_STATUS_INVISIBLE, "Invisible", PF2_INVISIBLE, PF2_INVISIBLE << 16 },
};

INT_PTR CALLBACK DlgProcOptsClasses(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			// Treeview
			HWND hwndTree = GetDlgItem(hwnd, IDC_TREE1);

			TreeView_DeleteAllItems(hwndTree);
			// Treeview create image list
			HIMAGELIST hImgLst = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR | ILC_COLOR32 | ILC_MASK, 5, num_classes + 1);
			ImageList_ReplaceIcon(hImgLst, -1, g_plugin.getIcon(IDI_OPT_GROUP));
			TreeView_SetImageList(hwndTree, hImgLst, TVSIL_NORMAL);

			for (auto &p : gTreeData) {
				wchar_t itemName[MAXMODULELABELLENGTH];
				int iconIndex;

				if (p->typ == 1) { // Treeview part for typ 1 (notification)
					iconIndex = ImageList_ReplaceIcon(hImgLst, -1, IcoLib_GetIconByHandle(p->hIcoLib));
					mir_snwprintf(itemName, L"%s/%s", p->pszTreeRoot, p->pszDescription);
				}
				else { // Treeview part typ 2 (popup class api)
					iconIndex = ImageList_ReplaceIcon(hImgLst, -1, p->pupClass.hIcon);
					mir_snwprintf(itemName, L"%s/%s", LPGENW("CLASS Plugins"), p->pszDescription);
				}
				OptTree_AddItem(hwndTree, itemName, (LPARAM)p, iconIndex);
			}
			OptTree_Translate(hwndTree);

			// Bold Title
			for (auto &it : titleIds) {
				LOGFONT lf;
				GetObject((HFONT)SendDlgItemMessage(hwnd, it, WM_GETFONT, 0, 0), sizeof(lf), &lf);
				lf.lfWeight = FW_BOLD;
				SendDlgItemMessage(hwnd, it, WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), TRUE);
			}

			// spindown for Timeout
			SendDlgItemMessage(hwnd, IDC_TIMEOUT_SPIN, UDM_SETRANGE, 0, (LPARAM)MAKELONG(SETTING_LIFETIME_MAX, SETTING_LIFETIME_INFINITE));
			SetDlgItemInt(hwnd, IDC_TIMEOUT, (UINT)0, TRUE);
			// status buttons
			for (auto &it : statusButtons) {
				SendDlgItemMessage(hwnd, it.idCtrl, BUTTONSETASFLATBTN, TRUE, 0);
				SendDlgItemMessage(hwnd, it.idCtrl, BUTTONSETASPUSHBTN, TRUE, 0);
				SendDlgItemMessage(hwnd, it.idCtrl, BM_SETIMAGE,
					IMAGE_ICON, (LPARAM)Skin_LoadIcon(it.iconId));
				SendDlgItemMessage(hwnd, it.idCtrl, BUTTONADDTOOLTIP,
					(WPARAM)Translate(it.title), 0);
			}
			// info icon
			SendDlgItemMessage(hwnd, IDC_ICO_INFO, STM_SETICON, (WPARAM)g_plugin.getIcon(IDI_MB_INFO), 0);
			// more button
			EnableWindow(GetDlgItem(hwnd, IDC_MORE), FALSE);
			// preview button
			ShowWindow(GetDlgItem(hwnd, IDC_CHECKWINDOW), SW_HIDE);

			SendMessage(hwnd, WM_USER, 0, 0);
		}
		return TRUE;

	case WM_USER:
		{
			HWND hTV = GetDlgItem(hwnd, IDC_TREE1);
			// get TreeView selection
			TVITEM tvi = { 0 };
			tvi.hItem = TreeView_GetSelection(hTV);
			tvi.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_TEXT;
			if (tvi.hItem) TreeView_GetItem(hTV, &tvi);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, tvi.lParam);

			if (tvi.lParam) {
				POPUPTREEDATA* ptd = (POPUPTREEDATA *)tvi.lParam;
				// combo left action (default)

				HWND hCtrl = GetDlgItem(hwnd, IDC_LACTION);
				ComboBox_ResetContent(hCtrl);
				ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT(POPUP_ACTION_DISMISS)), POPUP_ACTION_DISMISS);
				ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT(POPUP_ACTION_NOTHING)), POPUP_ACTION_NOTHING);

				// combo right action (default)
				hCtrl = GetDlgItem(hwnd, IDC_RACTION);
				ComboBox_ResetContent(hCtrl);
				ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT(POPUP_ACTION_DISMISS)), POPUP_ACTION_DISMISS);
				ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT(POPUP_ACTION_NOTHING)), POPUP_ACTION_NOTHING);

				// element typ1 (Notification)
				if (ptd->typ == 1) {
					LPTSTR psztAction = nullptr;
					// Timeout
					SetDlgItemInt(hwnd, IDC_TIMEOUT, (UINT)ptd->timeoutValue, TRUE);
					SendDlgItemMessage(hwnd, IDC_TIMEOUT_SPIN, UDM_SETRANGE, 0, (LPARAM)MAKELONG(250, -1));
					// combo left action (EXTRA)
					hCtrl = GetDlgItem(hwnd, IDC_LACTION);
					for (int i = 0; i < ptd->notification.actionCount; ++i) {
						psztAction = mir_a2u(ptd->notification.lpActions[i].lpzTitle);
						ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateW(psztAction)), ptd->notification.lpActions[i].lpzTitle);
						mir_free(psztAction); psztAction = nullptr;
					}
					// combo right action (EXTRA)
					hCtrl = GetDlgItem(hwnd, IDC_RACTION);
					psztAction = nullptr;
					for (int i = 0; i < ptd->notification.actionCount; ++i) {
						psztAction = mir_a2u(ptd->notification.lpActions[i].lpzTitle);
						ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateW(psztAction)), ptd->notification.lpActions[i].lpzTitle);
						mir_free(psztAction); psztAction = nullptr;
					}
					// enable all controls
					for (auto &it : ctrlsAll) {
						ShowWindow(GetDlgItem(hwnd, it.idCtrl), it.onTyp1 ? SW_SHOW : SW_HIDE);
						EnableWindow(GetDlgItem(hwnd, it.idCtrl), it.onTyp1);
					}
					// enable or disable controls ctrlsEnable
					for (auto &it : ctrlsEnable)
						EnableWindow(GetDlgItem(hwnd, it), ptd->enabled ? TRUE : FALSE);
					
					// show or hide controls ctrlsContact
					for (auto &it : ctrlsContact)
						ShowWindow(GetDlgItem(hwnd, it), ptd->notification.dwFlags&PNF_CONTACT ? SW_SHOW : SW_HIDE);
					
					// statusButtons state
					for (auto &it : statusButtons)
						CheckDlgButton(hwnd, it.idCtrl, ptd->disableWhen & it.disableWhenFlag ? BST_CHECKED : BST_UNCHECKED);
				}
				// element typ2 (CLASS Plugins)
				else if (ptd->typ == 2) {
					// Timeout
					SetDlgItemInt(hwnd, IDC_TIMEOUT, (UINT)ptd->timeoutValue, TRUE);
					SendDlgItemMessage(hwnd, IDC_TIMEOUT_SPIN, UDM_SETRANGE, 0, (LPARAM)MAKELONG(250, -1));
					// enable ctrls
					for (auto &it : ctrlsAll) {
						ShowWindow(GetDlgItem(hwnd, it.idCtrl), it.onTyp2 ? SW_SHOW : SW_HIDE);
						EnableWindow(GetDlgItem(hwnd, it.idCtrl), it.onTyp2);
					}
				}
				// checkbox enable notify
				CheckDlgButton(hwnd, IDC_ENABLE, ptd->enabled ? BST_CHECKED : BST_UNCHECKED);
				// combo left action (SELECT)
				hCtrl = GetDlgItem(hwnd, IDC_LACTION);
				ComboBox_SelectItem(hCtrl, ptd->leftAction);	// use Workaround for MS bug ComboBox_SelectItemData
				// combo right action (SELECT)
				hCtrl = GetDlgItem(hwnd, IDC_RACTION);
				ComboBox_SelectItem(hCtrl, ptd->rightAction);	// use Workaround for MS bug ComboBox_SelectItemData
			} // end if (tvi.lParam)
			else {
				// enable / disable controls
				for (auto &it : ctrlsAll) {
					ShowWindow(GetDlgItem(hwnd, it.idCtrl), it.onTyp0 ? SW_SHOW : SW_HIDE);
					EnableWindow(GetDlgItem(hwnd, it.idCtrl), it.onTyp0);
				}
			}
		}
		break;

	case WM_COMMAND:
		{
			UINT idCtrl = LOWORD(wParam);
			POPUPTREEDATA *ptd = (POPUPTREEDATA *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (!ptd) break;
			switch (HIWORD(wParam)) {
			case BN_CLICKED:		// Button controls
				switch (idCtrl) {
				case IDC_ENABLE:
					ptd->enabled = (uint8_t)Button_GetCheck((HWND)lParam);
					for (auto &it : ctrlsEnable)
						EnableWindow(GetDlgItem(hwnd, it), ptd->enabled);
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
					break;

				case IDC_PREVIEW:
					{
						POPUPDATA2 ppd = { 0 };
						ppd.cbSize = sizeof(ppd);
						ppd.flags = PU2_UNICODE;
						ppd.szTitle.w = ptd->pszDescription;
						ppd.szText.w = TranslateT("Ph'nglui mglw'nafh Cthulhu R'lyeh wgah'nagl fhtagn!");
						ppd.iSeconds = ptd->timeoutValue;
						ppd.colorBack = ptd->pupClass.colorBack;
						ppd.colorText = ptd->pupClass.colorText;
						POPUPTREEDATA *ptdPrev = nullptr;
						if (ptd->typ == 1) {
							// we work with a copy for preview
							ptdPrev = (POPUPTREEDATA *)mir_alloc(sizeof(POPUPTREEDATA));
							memcpy(ptdPrev, ptd, sizeof(POPUPTREEDATA));
							ptdPrev->enabled = ptd->enabled;
							ptdPrev->timeoutValue = ptd->timeoutValue;
							mir_strcpy(ptdPrev->leftAction, ptd->leftAction);		// geht noch nicht??
							mir_strcpy(ptdPrev->rightAction, ptd->rightAction);	// geht noch nicht??
							ptdPrev->disableWhen = ptd->disableWhen;

							ppd.lchNotification = (HANDLE)ptdPrev;
						}
						else if (ptd->typ == 2)
							ppd.lchIcon = ptd->pupClass.hIcon;

						CallService(MS_POPUP_ADDPOPUP2, (WPARAM)&ppd, APF_NO_HISTORY);
						mir_free(ptdPrev); ptdPrev = nullptr;
					}
					break;

				case IDC_MORE:
					g_plugin.openOptions(L"Customize", L"Fonts and colors");
					break;

				case IDC_SOFFLINE:  case IDC_SONLINE:     case IDC_SAWAY:       case IDC_SNA:     case IDC_SOCCUPIED:
				case IDC_SDND:	     case IDC_SFREE4CHAT:  case IDC_SINVISIBLE:
				case IDC_SOFFLINE2: case IDC_SONLINE2:    case IDC_SAWAY2:      case IDC_SNA2:    case IDC_SOCCUPIED2:
				case IDC_SDND2:     case IDC_SFREE4CHAT2: case IDC_SINVISIBLE2:
					ptd->disableWhen = 0;
					for (auto &it : statusButtons) {
						if (IsDlgButtonChecked(hwnd, it.idCtrl))
							ptd->disableWhen |= it.disableWhenFlag;
						if (idCtrl == (UINT)it.idCtrl)
							SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
					}
				}
				break;

			case CBN_SELCHANGE:		// ComboBox controls
				switch (idCtrl) {
				case IDC_LACTION:
					mir_strncpy(ptd->leftAction,
						(char*)ComboBox_GetItemData((HWND)lParam, ComboBox_GetCurSel((HWND)lParam)),
						_countof(ptd->leftAction));
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
					break;
				case IDC_RACTION:
					mir_strncpy(ptd->rightAction,
						(char*)ComboBox_GetItemData((HWND)lParam, ComboBox_GetCurSel((HWND)lParam)),
						_countof(ptd->rightAction));
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
					break;
				}
				break;

			case EN_CHANGE:			// Edit controls
				switch (idCtrl) {
				case IDC_TIMEOUT:
					int seconds = GetDlgItemInt(hwnd, idCtrl, nullptr, TRUE);
					if (seconds >= SETTING_LIFETIME_INFINITE && seconds <= SETTING_LIFETIME_MAX && seconds != ptd->timeoutValue) {
						ptd->timeoutValue = seconds;
						SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
					}
				}
				break;

			case EN_KILLFOCUS:		// Edit controls lost fokus
				switch (idCtrl) {
				case IDC_TIMEOUT:
					int seconds = GetDlgItemInt(hwnd, idCtrl, nullptr, TRUE);
					if (seconds > SETTING_LIFETIME_MAX)
						ptd->timeoutValue = SETTING_LIFETIME_MAX;
					else if (seconds < SETTING_LIFETIME_INFINITE)
						ptd->timeoutValue = SETTING_LIFETIME_INFINITE;
					if (seconds != ptd->timeoutValue) {
						SetDlgItemInt(hwnd, idCtrl, ptd->timeoutValue, TRUE);
						ErrorMSG(SETTING_LIFETIME_INFINITE, SETTING_LIFETIME_MAX);
						SetFocus((HWND)lParam);
					}
				}
			}
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_RESET:
				for (auto &p : gTreeData) {
					switch (p->typ) {
					case 1:
						LoadNotificationSettings(p, "PopupNotifications");
						break;
					case 2:			// not finish
						LoadClassSettings(p, PU_MODULCLASS);
						p->timeoutValue = p->pupClass.iSeconds;
						break;
					}
				}
				return TRUE;

			case PSN_APPLY:
				for (auto &p : gTreeData) {
					switch (p->typ) {
					case 1:
						p->notification.iSeconds = p->timeoutValue;
						SaveNotificationSettings(p, "PopupNotifications");
						break;
					case 2:			// not finish
						p->pupClass.iSeconds = p->timeoutValue;
						SaveClassSettings(p, "PopupCLASS");
						break;
					}
				}
				return TRUE;
			}
			break;

		case IDC_TREE1:
			switch (((LPNMHDR)lParam)->code) {
			case TVN_SELCHANGED:
				PostMessage(hwnd, WM_USER, 0, 0);
				break;
			}
		}
		break;
	}
	return FALSE;
}

void LoadClassSettings(POPUPTREEDATA *ptd, char* szModul)
{
	char setting[2 * MAXMODULELABELLENGTH];
	mir_snprintf(setting, "%s/enabled", ptd->pupClass.pszName);
	ptd->enabled =
		(signed char)db_get_b(0, szModul, setting, TRUE);

	mir_snprintf(setting, "%s/Timeout", ptd->pupClass.pszName);
	ptd->pupClass.iSeconds =
		(signed char)db_get_w(0, szModul, setting, 0);

	mir_snprintf(setting, "%s/TimeoutVal", ptd->pupClass.pszName);
	ptd->timeoutValue =
		(signed char)db_get_w(0, szModul, setting,
		ptd->pupClass.iSeconds ? ptd->pupClass.iSeconds : PopupOptions.Seconds);

	mir_snprintf(setting, "%s/leftAction", ptd->pupClass.pszName);
	char *szTmp = db_get_sa(0, szModul, setting, POPUP_ACTION_NOTHING);	// standart ??
	mir_strncpy(ptd->leftAction, szTmp, _countof(ptd->leftAction));
	mir_free(szTmp);

	mir_snprintf(setting, "%s/rightAction", ptd->pupClass.pszName);
	szTmp = db_get_sa(0, szModul, setting, POPUP_ACTION_DISMISS);	// standart ??
	mir_strncpy(ptd->rightAction, szTmp, _countof(ptd->rightAction));
	mir_free(szTmp);
}

void SaveClassSettings(POPUPTREEDATA *ptd, char* szModul)
{
	char setting[2 * MAXMODULELABELLENGTH];

	mir_snprintf(setting, "%s/enabled", ptd->pupClass.pszName);
	db_set_b(0, szModul, setting, ptd->enabled);

	mir_snprintf(setting, "%s/Timeout", ptd->pupClass.pszName);
	db_set_w(0, szModul, setting, ptd->pupClass.iSeconds);

	mir_snprintf(setting, "%s/TimeoutVal", ptd->pupClass.pszName);
	db_set_w(0, szModul, setting, ptd->timeoutValue);

	mir_snprintf(setting, "%s/leftAction", ptd->pupClass.pszName);
	db_set_s(0, szModul, setting, ptd->leftAction);

	mir_snprintf(setting, "%s/rightAction", ptd->pupClass.pszName);
	db_set_s(0, szModul, setting, ptd->rightAction);
}
