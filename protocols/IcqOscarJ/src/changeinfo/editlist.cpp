// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2001-2004 Richard Hughes, Martin Öberg
// Copyright © 2004-2009 Joe Kucera, Bio
// Copyright © 2012-2014 Miranda NG Team
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  ChangeInfo Plugin stuff
// -----------------------------------------------------------------------------

#include "icqoscar.h"

static ChangeInfoData *dataListEdit = NULL;
static HWND hwndListEdit = NULL;

static LRESULT CALLBACK ListEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_LBUTTONUP:
		mir_callNextSubclass(hwnd, ListEditSubclassProc, msg, wParam, lParam);
		{
			POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
			ClientToScreen(hwnd, &pt);
			if (SendMessage(hwnd, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y)) == HTVSCROLL)
				break;

			int i = SendMessage(hwnd, LB_GETCURSEL, 0, 0);
			if (dataListEdit)
				dataListEdit->EndListEdit(i != LB_ERR);
		}
		return 0;

	case WM_CHAR:
		if (wParam != '\r') break;
		{
			int i = SendMessage(hwnd, LB_GETCURSEL, 0, 0);
			if (dataListEdit)
				dataListEdit->EndListEdit(i != LB_ERR);
		}
		return 0;

	case WM_KILLFOCUS:
		if (dataListEdit)
			dataListEdit->EndListEdit(1);
		return 0;
	}
	return mir_callNextSubclass(hwnd, ListEditSubclassProc, msg, wParam, lParam);
}

void ChangeInfoData::BeginListEdit(int iItem, RECT *rc, int iSetting, WORD wVKey)
{
	if (dataListEdit)
		dataListEdit->EndListEdit(0);

	POINT pt = { 0, 0 };
	ClientToScreen(hwndList, &pt);
	OffsetRect(rc, pt.x, pt.y);
	InflateRect(rc, -2, -2);
	rc->left -= 2;
	iEditItem = iItem;
	ListView_RedrawItems(hwndList, iEditItem, iEditItem);
	UpdateWindow(hwndList);

	dataListEdit = this;
	hwndListEdit = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, _T("LISTBOX"), _T(""), WS_POPUP | WS_BORDER | WS_VSCROLL,
											rc->left, rc->bottom, rc->right - rc->left, 150, NULL, NULL, hInst, NULL);
	SendMessage(hwndListEdit, WM_SETFONT, (WPARAM)hListFont, 0);
	int itemHeight = SendMessage(hwndListEdit, LB_GETITEMHEIGHT, 0, 0);

	const SettingItem &si = setting[iSetting];
	SettingItemData &sid = settingData[iSetting];
	FieldNamesItem *list = (FieldNamesItem*)si.pList;

	// some country codes were changed leaving old details uknown, convert it for the user
	if (list == countryField) {
		if (sid.value == 420)
			sid.value = 42; // conversion of obsolete codes (OMG!)
		else if (sid.value == 421)
			sid.value = 4201;
		else if (sid.value == 102)
			sid.value = 1201;
	}

	if (list == timezonesField) {
		tmi.prepareList(NULL, ppro->m_szModuleName, hwndListEdit, TZF_PLF_LB);
	}
	else {
		int j, n = ListBoxAddStringUtf(hwndListEdit, "Unspecified");
		for (j = 0;; j++)
			if (!list[j].text) {
				SendMessage(hwndListEdit, LB_SETITEMDATA, n, j);
				if ((sid.value == 0 && list[j].code == 0) || (si.dbType != DBVT_ASCIIZ && sid.value == list[j].code))
					SendMessage(hwndListEdit, LB_SETCURSEL, n, 0);
				break;
			}

		for (j = 0; list[j].text; j++) {
			char str[MAX_PATH];
			n = ListBoxAddStringUtf(hwndListEdit, list[j].text);
			SendMessage(hwndListEdit, LB_SETITEMDATA, n, j);
			if ((si.dbType == DBVT_ASCIIZ && (!strcmpnull((char*)sid.value, list[j].text))
				|| (si.dbType == DBVT_ASCIIZ && (!strcmpnull((char*)sid.value, ICQTranslateUtfStatic(list[j].text, str, MAX_PATH))))
				|| ((char*)sid.value == NULL && list[j].code == 0))
				|| (si.dbType != DBVT_ASCIIZ && sid.value == list[j].code))
				SendMessage(hwndListEdit, LB_SETCURSEL, n, 0);
		}
		SendMessage(hwndListEdit, LB_SETTOPINDEX, SendMessage(hwndListEdit, LB_GETCURSEL, 0, 0) - 3, 0);
	}

	int listCount = SendMessage(hwndListEdit, LB_GETCOUNT, 0, 0);
	if (itemHeight * listCount < 150)
		SetWindowPos(hwndListEdit, 0, 0, 0, rc->right - rc->left, itemHeight * listCount + GetSystemMetrics(SM_CYBORDER) * 2, SWP_NOZORDER | SWP_NOMOVE);
	mir_subclassWindow(hwndListEdit, ListEditSubclassProc);
	AnimateWindow(hwndListEdit, 200, AW_SLIDE | AW_ACTIVATE | AW_VER_POSITIVE);
	ShowWindow(hwndListEdit, SW_SHOW);
	SetFocus(hwndListEdit);
	if (wVKey)
		PostMessage(hwndListEdit, WM_KEYDOWN, wVKey, 0);
}

void ChangeInfoData::EndListEdit(int save)
{
	if (hwndListEdit == NULL || iEditItem == -1 || this != dataListEdit)
		return;

	if (save) {
		const SettingItem &si = setting[iEditItem];
		SettingItemData &sid = settingData[iEditItem];

		int iItem = SendMessage(hwndListEdit, LB_GETCURSEL, 0, 0);
		int i = SendMessage(hwndListEdit, LB_GETITEMDATA, iItem, 0);
		if (iItem != -1 && i != -1) {
			FieldNamesItem *list = (FieldNamesItem*)si.pList;
			if (list == timezonesField) {
				tmi.storeListResults(NULL, ppro->m_szModuleName, hwndListEdit, TZF_PLF_LB);
				list[i = 0].code = ppro->getDword("Timezone", 0);
			}

			FieldNamesItem &pItem = list[i];

			if (si.dbType == DBVT_ASCIIZ) {
				char *szNewValue = pItem.text;
				if (pItem.code || (si.displayType & LIF_ZEROISVALID)) {
					sid.changed = strcmpnull(szNewValue, (char*)sid.value);
					SAFE_FREE((void**)&sid.value);
					sid.value = (LPARAM)null_strdup(szNewValue);
				}
				else {
					sid.changed = (char*)sid.value != NULL;
					SAFE_FREE((void**)&sid.value);
				}
			}
			else {
				sid.changed = pItem.code != sid.value;
				sid.value = pItem.code;
			}

			if (sid.changed) {
				char buf[MAX_PATH];
				TCHAR tbuf[MAX_PATH];
				if (utf8_to_tchar_static(ICQTranslateUtfStatic(pItem.text, buf, SIZEOF(buf)), tbuf, SIZEOF(buf)))
					ListView_SetItemText(hwndList, iEditItem, 1, tbuf);

				EnableDlgItem(GetParent(hwndList), IDC_SAVE, TRUE);
			}
		}
	}
	ListView_RedrawItems(hwndList, iEditItem, iEditItem);
	iEditItem = -1;
	dataListEdit = NULL;
	DestroyWindow(hwndListEdit);
	hwndListEdit = NULL;
}

int IsListEditWindow(HWND hwnd)
{
	if (hwnd == hwndListEdit) return 1;
	return 0;
}
