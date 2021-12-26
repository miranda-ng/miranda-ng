/*

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

#define IDI_BLANK 200

void LoadSettings(void);

extern uint8_t 
	bFlashOnMsg, bFlashOnFile, bFlashOnGC, bFlashOnOther, bFullScreenMode, bScreenSaverRunning, bWorkstationLocked, bProcessesAreRunning,
	bWorkstationActive, bFlashIfMsgOpen, bFlashIfMsgOlder, bFlashUntil, bMirandaOrWindows, bFlashLed[3], bFlashEffect, bSequenceOrder, bFlashSpeed,
	bEmulateKeypresses, bOverride, bFlashIfMsgWinNotTop, bTrillianLedsMsg, bTrillianLedsURL, bTrillianLedsFile, bTrillianLedsOther;

extern uint16_t wSecondsOlder, wBlinksNumber, wStatusMap, wReminderCheck, wCustomTheme, wStartDelay;

extern PROTOCOL_LIST ProtoList;
extern PROCESS_LIST ProcessList;

HWND hwndProto, hwndBasic, hwndEffect, hwndTheme, hwndIgnore, hwndCurrentTab;

wchar_t *AttendedName[] = { L"Miranda", L"Windows" };
wchar_t *OrderName[] = { LPGENW("left -> right"), LPGENW("right -> left"), LPGENW("left <-> right") };

PROCESS_LIST ProcessListAux;
XSTATUS_INFO *XstatusListAux;
uint8_t trillianLedsMsg, trillianLedsURL, trillianLedsFile, trillianLedsOther;

static void writeThemeToCombo(const wchar_t *theme, const wchar_t *custom, BOOL overrideExisting)
{
	int item = SendDlgItemMessage(hwndTheme, IDC_THEME, CB_FINDSTRINGEXACT, -1, (LPARAM)theme);
	if (item == CB_ERR) {
		item = SendDlgItemMessage(hwndTheme, IDC_THEME, CB_ADDSTRING, 0, (LPARAM)theme);
		wchar_t *str = mir_wstrdup(custom);
		SendDlgItemMessage(hwndTheme, IDC_THEME, CB_SETITEMDATA, (WPARAM)item, (LPARAM)str);
	}
	else if (overrideExisting) {
		wchar_t *str = (wchar_t *)SendDlgItemMessage(hwndTheme, IDC_THEME, CB_GETITEMDATA, (WPARAM)item, 0);
		if (str)
			mir_wstrcpy(str, custom);
	}
}

void exportThemes(const wchar_t *filename)
{
	FILE *fExport = _wfopen(filename, L"wt");
	if (!fExport)
		return;

	fwprintf(fExport, TranslateT("\n; Automatically generated Keyboard Notify Theme file\n\n\n"));

	wchar_t *szTheme;
	for (int i = 0; szTheme = g_plugin.getWStringA(fmtDBSettingName("theme%d", i)); i++) {
		fwprintf(fExport, L"[%s]\n", szTheme);
		mir_free(szTheme);
		if (szTheme = g_plugin.getWStringA(fmtDBSettingName("custom%d", i))) {
			fwprintf(fExport, L"%s\n\n", szTheme);
			mir_free(szTheme);
		}
		else fwprintf(fExport, L"0\n\n");
	}

	fwprintf(fExport, TranslateT("\n; End of automatically generated Keyboard Notify Theme file\n"));

	fclose(fExport);
}

void importThemes(const wchar_t *filename, BOOL overrideExisting)
{
	FILE *fImport = _wfopen(filename, L"rt");
	if (!fImport)
		return;

	int status = 0;
	size_t i;
	wchar_t buffer[MAX_PATH + 1], theme[MAX_PATH + 1], *str;

	while (fgetws(buffer, MAX_PATH, fImport) != nullptr) {
		for (str = buffer; *str && isspace(*str); str++); //ltrim
		if (!*str || *str == ';') //empty line or comment
			continue;
		for (i = mir_wstrlen(str) - 1; isspace(str[i]); str[i--] = '\0'); //rtrim
		switch (status) {
		case 0:
			if (i > 1 && str[0] == '[' && str[i] == ']') {
				status = 1;
				mir_wstrcpy(theme, str + 1);
				theme[i - 1] = '\0';
			}
			break;
		case 1:
			status = 0;
			writeThemeToCombo(theme, normalizeCustomString(str), overrideExisting);
			break;
		}
	}

	fclose(fImport);
}

static void createProcessListAux(void)
{
	ProcessListAux.count = ProcessList.count;
	ProcessListAux.szFileName = (wchar_t **)mir_alloc(ProcessListAux.count * sizeof(wchar_t *));
	if (ProcessListAux.szFileName) {
		for (int i = 0; i < ProcessListAux.count; i++) {
			if (!ProcessList.szFileName[i])
				ProcessListAux.szFileName[i] = nullptr;
			else
				ProcessListAux.szFileName[i] = mir_wstrdup(ProcessList.szFileName[i]);
		}
	}
	else ProcessListAux.count = 0;
}

static void destroyProcessListAux(void)
{
	if (ProcessListAux.szFileName == nullptr)
		return;

	for (int i = 0; i < ProcessListAux.count; i++)
		if (ProcessListAux.szFileName[i])
			mir_free(ProcessListAux.szFileName[i]);

	mir_free(ProcessListAux.szFileName);
	ProcessListAux.count = 0;
	ProcessListAux.szFileName = nullptr;
}

static void createXstatusListAux(void)
{
	XstatusListAux = (XSTATUS_INFO *)mir_alloc(ProtoList.protoCount * sizeof(XSTATUS_INFO));
	if (XstatusListAux)
		for (int i = 0; i < ProtoList.protoCount; i++) {
			XstatusListAux[i].count = ProtoList.protoInfo[i].xstatus.count;
			if (!XstatusListAux[i].count)
				XstatusListAux[i].enabled = nullptr;
			else {
				XstatusListAux[i].enabled = (BOOL *)mir_alloc(XstatusListAux[i].count * sizeof(BOOL));
				if (!XstatusListAux[i].enabled)
					XstatusListAux[i].count = 0;
				else
					for (int j = 0; j < (int)XstatusListAux[i].count; j++)
						XstatusListAux[i].enabled[j] = ProtoList.protoInfo[i].xstatus.enabled[j];
			}
		}

}

static void destroyXstatusListAux(void)
{
	if (XstatusListAux) {
		for (int i = 0; i < ProtoList.protoCount; i++)
			if (XstatusListAux[i].enabled)
				mir_free(XstatusListAux[i].enabled);
		mir_free(XstatusListAux);
		XstatusListAux = nullptr;
	}
}

static INT_PTR CALLBACK DlgProcProcesses(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, EM_LIMITTEXT, MAX_PATH, 0);

		for (int i = 0; i < ProcessListAux.count; i++)
			if (ProcessListAux.szFileName[i]) {
				int index = SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)ProcessListAux.szFileName[i]);
				if (index != CB_ERR && index != CB_ERRSPACE)
					SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, CB_SETITEMDATA, (WPARAM)index, (LPARAM)i);
			}

		EnableWindow(GetDlgItem(hwndDlg, IDC_ADDPGM), FALSE);
		if (SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, CB_GETCOUNT, 0, 0) == 0)
			EnableWindow(GetDlgItem(hwndDlg, IDC_DELETEPGM), FALSE);
		else
			SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, CB_SETCURSEL, 0, 0);

		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_PROGRAMS:
			switch (HIWORD(wParam)) {

			case CBN_SELENDOK:
			case CBN_SELCHANGE:
				EnableWindow(GetDlgItem(hwndDlg, IDC_ADDPGM), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_DELETEPGM), TRUE);
				break;
			case CBN_EDITCHANGE:
				wchar_t szFileName[MAX_PATH + 1];
				GetDlgItemText(hwndDlg, IDC_PROGRAMS, szFileName, _countof(szFileName));
				int item = SendMessage((HWND)lParam, CB_FINDSTRINGEXACT, -1, (LPARAM)szFileName);
				if (item == CB_ERR) {	//new program
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADDPGM), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_DELETEPGM), FALSE);
				}
				else {
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADDPGM), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_DELETEPGM), TRUE);
				}
				break;
			}
			break;
		case IDC_ADDPGM:
			{
				wchar_t szFileName[MAX_PATH + 1];
				GetDlgItemText(hwndDlg, IDC_PROGRAMS, szFileName, _countof(szFileName));
				if (!szFileName[0])
					break;
				SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, CB_ADDSTRING, 0, (LPARAM)szFileName);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ADDPGM), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_DELETEPGM), TRUE);
			}
			break;
		case IDC_DELETEPGM:
			{
				wchar_t szFileName[MAX_PATH + 1];
				GetDlgItemText(hwndDlg, IDC_PROGRAMS, szFileName, _countof(szFileName));
				int item = SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, CB_FINDSTRINGEXACT, -1, (LPARAM)szFileName);
				SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, CB_DELETESTRING, (WPARAM)item, 0);
				if (SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, CB_GETCOUNT, 0, 0) == 0) {
					SetDlgItemText(hwndDlg, IDC_PROGRAMS, L"");
					EnableWindow(GetDlgItem(hwndDlg, IDC_DELETEPGM), FALSE);
				}
				else
					SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, CB_SETCURSEL, 0, 0);
			}
			break;
		case IDC_OKPGM:
			destroyProcessListAux();

			ProcessListAux.count = SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, CB_GETCOUNT, 0, 0);
			ProcessListAux.szFileName = (wchar_t **)mir_alloc(ProcessListAux.count * sizeof(wchar_t *));
			if (!ProcessListAux.szFileName)
				ProcessListAux.count = 0;
			else
				for (int i = 0; i < ProcessListAux.count; i++) {
					wchar_t szFileNameAux[MAX_PATH + 1];

					SendDlgItemMessage(hwndDlg, IDC_PROGRAMS, CB_GETLBTEXT, (WPARAM)i, (LPARAM)szFileNameAux);
					ProcessListAux.szFileName[i] = mir_wstrdup(szFileNameAux);
				}
			// fallthrough

		case IDC_CANCELPGM:
			EndDialog(hwndDlg, LOWORD(wParam));
			break;
		}
		break;
	}

	return FALSE;
}

static INT_PTR CALLBACK DlgProcXstatusList(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			WPARAM j;
			TVINSERTSTRUCT tvis = {};
			TVITEM tvi = { 0 };
			HTREEITEM hSectionItem, hItem;
			HWND hwndTree = GetDlgItem(hwndDlg, IDC_TREE_XSTATUS);

			SetWindowLongPtr(hwndTree, GWL_STYLE, GetWindowLongPtr(hwndTree, GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);

			if (!XstatusListAux) return TRUE;

			// Calculate hImageList size
			int imageCount = 1;
			for (int i = 0; i < ProtoList.protoCount; i++)
				if (ProtoList.protoInfo[i].enabled && XstatusListAux[i].count)
					imageCount += XstatusListAux[i].count;

			HIMAGELIST hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, imageCount, imageCount);
			TreeView_SetImageList(hwndTree, hImageList, TVSIL_NORMAL);

			HICON hIconAux = (HICON)LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_BLANK), IMAGE_ICON, 0, 0, 0);
			if (hIconAux) {
				ImageList_AddIcon(hImageList, hIconAux);
				DestroyIcon(hIconAux);
			}

			TreeView_SelectItem(hwndTree, NULL);
			ShowWindow(hwndTree, SW_HIDE);
			TreeView_DeleteAllItems(hwndTree);

			for (int i = 0; i < ProtoList.protoCount; i++)
				if (ProtoList.protoInfo[i].enabled && XstatusListAux[i].count) {
					int count;
					PROTOACCOUNT **protos;
					Proto_EnumAccounts(&count, &protos);

					tvis.hParent = nullptr;
					tvis.hInsertAfter = TVI_LAST;
					tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
					tvis.item.pszText = protos[i]->tszAccountName;
					tvis.item.lParam = (LPARAM)i;
					tvis.item.stateMask = TVIS_BOLD | TVIS_EXPANDED;
					tvis.item.state = TVIS_BOLD | TVIS_EXPANDED;
					tvis.item.iImage = tvis.item.iSelectedImage = ImageList_AddIcon(hImageList, hIconAux = (HICON)CallProtoService(ProtoList.protoInfo[i].szProto, PS_LOADICON, PLI_PROTOCOL, 0));
					if (hIconAux) DestroyIcon(hIconAux);
					HTREEITEM hParent = TreeView_InsertItem(hwndTree, &tvis);
					for (j = 0; j < XstatusListAux[i].count; j++) {
						tvis.hParent = hParent;
						tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
						if (!j) {
							tvis.item.pszText = TranslateT("None");
						}
						else {
							wchar_t szDefaultName[1024];
							CUSTOM_STATUS xstatus = { 0 };
							xstatus.cbSize = sizeof(CUSTOM_STATUS);
							xstatus.flags = CSSF_MASK_NAME | CSSF_DEFAULT_NAME | CSSF_UNICODE;
							xstatus.ptszName = szDefaultName;
							xstatus.wParam = &j;
							CallProtoService(ProtoList.protoInfo[i].szProto, PS_GETCUSTOMSTATUSEX, 0, (LPARAM)&xstatus);
							tvis.item.pszText = TranslateW(szDefaultName);
						}
						tvis.item.lParam = (LPARAM)j;
						tvis.item.iImage = tvis.item.iSelectedImage = j ? ImageList_AddIcon(hImageList, hIconAux = (HICON)CallProtoService(ProtoList.protoInfo[i].szProto, PS_GETCUSTOMSTATUSICON, (WPARAM)j, 0)) : 0;
						if (hIconAux) DestroyIcon(hIconAux);
						TreeView_InsertItem(hwndTree, &tvis);
					}
				}

			tvi.mask = TVIF_HANDLE | TVIF_PARAM;
			for (hSectionItem = TreeView_GetRoot(hwndTree); hSectionItem; hSectionItem = TreeView_GetNextSibling(hwndTree, hSectionItem)) {
				tvi.hItem = hSectionItem;
				TreeView_GetItem(hwndTree, &tvi);
				unsigned int i = (unsigned int)tvi.lParam;
				TreeView_SetItemState(hwndTree, hSectionItem, INDEXTOSTATEIMAGEMASK(0), TVIS_STATEIMAGEMASK);
				for (hItem = TreeView_GetChild(hwndTree, hSectionItem); hItem; hItem = TreeView_GetNextSibling(hwndTree, hItem)) {
					tvi.hItem = hItem;
					TreeView_GetItem(hwndTree, &tvi);
					j = (unsigned int)tvi.lParam;
					TreeView_SetItemState(hwndTree, hItem, INDEXTOSTATEIMAGEMASK(XstatusListAux[i].enabled[j] ? 2 : 1), TVIS_STATEIMAGEMASK);
				}
			}

			ShowWindow(hwndTree, SW_SHOW);
			TreeView_SetItemState(hwndTree, 0, TVIS_SELECTED, TVIS_SELECTED);
		}
		return TRUE;

	case WM_DESTROY:
		{
			// Destroy tree view imagelist since it does not get destroyed automatically (see msdn docs)
			HIMAGELIST hImageList = TreeView_GetImageList(GetDlgItem(hwndDlg, IDC_TREE_XSTATUS), TVSIL_STATE);
			if (hImageList) {
				TreeView_SetImageList(GetDlgItem(hwndDlg, IDC_TREE_XSTATUS), NULL, TVSIL_STATE);
				ImageList_Destroy(hImageList);
			}
			hImageList = TreeView_GetImageList(GetDlgItem(hwndDlg, IDC_TREE_XSTATUS), TVSIL_NORMAL);
			if (hImageList) {
				TreeView_SetImageList(GetDlgItem(hwndDlg, IDC_TREE_XSTATUS), NULL, TVSIL_NORMAL);
				ImageList_Destroy(hImageList);
			}
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_OKXST:
			{
				unsigned int i, j;
				HTREEITEM hSectionItem, hItem;
				TVITEM tvi = { 0 };
				HWND hwndTree = GetDlgItem(hwndDlg, IDC_TREE_XSTATUS);

				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				for (hSectionItem = TreeView_GetRoot(hwndTree); hSectionItem; hSectionItem = TreeView_GetNextSibling(hwndTree, hSectionItem)) {
					tvi.hItem = hSectionItem;
					TreeView_GetItem(hwndTree, &tvi);
					i = (unsigned int)tvi.lParam;
					for (hItem = TreeView_GetChild(hwndTree, hSectionItem); hItem; hItem = TreeView_GetNextSibling(hwndTree, hItem)) {
						tvi.hItem = hItem;
						TreeView_GetItem(hwndTree, &tvi);
						j = (unsigned int)tvi.lParam;
						XstatusListAux[i].enabled[j] = (TreeView_GetItemState(hwndTree, hItem, TVIS_STATEIMAGEMASK) & INDEXTOSTATEIMAGEMASK(2));
					}
				}
			} // fallthrough

		case IDC_CANCELXST:
			EndDialog(hwndDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}

	return FALSE;
}

static INT_PTR CALLBACK DlgProcEventLeds(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		CheckDlgButton(hwndDlg, IDC_MSGLEDNUM, trillianLedsMsg & 2 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MSGLEDCAPS, trillianLedsMsg & 4 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MSGLEDSCROLL, trillianLedsMsg & 1 ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_FILELEDNUM, trillianLedsFile & 2 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_FILELEDCAPS, trillianLedsFile & 4 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_FILELEDSCROLL, trillianLedsFile & 1 ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_URLLEDNUM, trillianLedsURL & 2 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_URLLEDCAPS, trillianLedsURL & 4 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_URLLEDSCROLL, trillianLedsURL & 1 ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_OTHERLEDNUM, trillianLedsOther & 2 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_OTHERLEDCAPS, trillianLedsOther & 4 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_OTHERLEDSCROLL, trillianLedsOther & 1 ? BST_CHECKED : BST_UNCHECKED);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_OK:
			trillianLedsMsg = 0;
			if (IsDlgButtonChecked(hwndDlg, IDC_MSGLEDNUM) == BST_CHECKED)
				trillianLedsMsg |= 2;
			if (IsDlgButtonChecked(hwndDlg, IDC_MSGLEDCAPS) == BST_CHECKED)
				trillianLedsMsg |= 4;
			if (IsDlgButtonChecked(hwndDlg, IDC_MSGLEDSCROLL) == BST_CHECKED)
				trillianLedsMsg |= 1;

			trillianLedsFile = 0;
			if (IsDlgButtonChecked(hwndDlg, IDC_FILELEDNUM) == BST_CHECKED)
				trillianLedsFile |= 2;
			if (IsDlgButtonChecked(hwndDlg, IDC_FILELEDCAPS) == BST_CHECKED)
				trillianLedsFile |= 4;
			if (IsDlgButtonChecked(hwndDlg, IDC_FILELEDSCROLL) == BST_CHECKED)
				trillianLedsFile |= 1;

			trillianLedsURL = 0;
			if (IsDlgButtonChecked(hwndDlg, IDC_URLLEDNUM) == BST_CHECKED)
				trillianLedsURL |= 2;
			if (IsDlgButtonChecked(hwndDlg, IDC_URLLEDCAPS) == BST_CHECKED)
				trillianLedsURL |= 4;
			if (IsDlgButtonChecked(hwndDlg, IDC_URLLEDSCROLL) == BST_CHECKED)
				trillianLedsURL |= 1;

			trillianLedsOther = 0;
			if (IsDlgButtonChecked(hwndDlg, IDC_OTHERLEDNUM) == BST_CHECKED)
				trillianLedsOther |= 2;
			if (IsDlgButtonChecked(hwndDlg, IDC_OTHERLEDCAPS) == BST_CHECKED)
				trillianLedsOther |= 4;
			if (IsDlgButtonChecked(hwndDlg, IDC_OTHERLEDSCROLL) == BST_CHECKED)
				trillianLedsOther |= 1;
			// fall through
		case IDC_CANCEL:
			EndDialog(hwndDlg, LOWORD(wParam));
			break;
		}
		break;
	}

	return FALSE;
}

static INT_PTR CALLBACK DlgProcProtoOptions(HWND hwndDlg, UINT msg, WPARAM, LPARAM lParam)
{
	static BOOL initDlg = FALSE;

	switch (msg) {
	case WM_INITDIALOG:
		initDlg = TRUE;
		TranslateDialogDefault(hwndDlg);

		// proto list
		{
			LVCOLUMN lvCol;
			LVITEM lvItem;
			HWND hList = GetDlgItem(hwndDlg, IDC_PROTOCOLLIST);

			// create columns
			ListView_SetExtendedListViewStyleEx(hList, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);
			memset(&lvCol, 0, sizeof(lvCol));
			lvCol.mask = LVCF_WIDTH | LVCF_TEXT;
			lvCol.pszText = TranslateT("Protocol");
			lvCol.cx = 118;
			ListView_InsertColumn(hList, 0, &lvCol);
			// fill
			memset(&lvItem, 0, sizeof(lvItem));
			lvItem.mask = LVIF_TEXT | LVIF_PARAM;
			lvItem.iItem = 0;
			lvItem.iSubItem = 0;
			for (int i = 0; i < ProtoList.protoCount; i++) {
				int count; PROTOACCOUNT **protos;
				Proto_EnumAccounts(&count, &protos);
				if (ProtoList.protoInfo[i].visible) {
					lvItem.lParam = (LPARAM)ProtoList.protoInfo[i].szProto;
					lvItem.pszText = protos[i]->tszAccountName;
					ListView_InsertItem(hList, &lvItem);
					ListView_SetCheckState(hList, lvItem.iItem, ProtoList.protoInfo[i].enabled);
					lvItem.iItem++;
				}
			}
		}

		initDlg = FALSE;
		return TRUE;

	case WM_NOTIFY:
		{
			//Here we have pressed either the OK or the APPLY button.
			switch (((LPNMHDR)lParam)->idFrom) {
			case 0:
				switch (((LPNMHDR)lParam)->code) {
				case PSN_APPLY:
					// enabled protos
					{
						LVITEM lvItem;
						HWND hList = GetDlgItem(hwndDlg, IDC_PROTOCOLLIST);

						memset(&lvItem, 0, sizeof(lvItem));
						lvItem.mask = LVIF_PARAM;
						lvItem.iSubItem = 0;
						for (int i = 0; i < ListView_GetItemCount(hList); i++) {
							lvItem.iItem = i;
							ListView_GetItem(hList, &lvItem);
							g_plugin.setByte((char *)lvItem.lParam, (uint8_t)ListView_GetCheckState(hList, lvItem.iItem));
						}
					}

					LoadSettings();

					return TRUE;
				} // switch code - 0
				break;
			case IDC_PROTOCOLLIST:
				switch (((NMHDR *)lParam)->code) {
				case LVN_ITEMCHANGED:
					{
						NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;

						if (!initDlg && ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK))
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
					break;
				} // switch code - IDC_PROTOCOLLIST
				break;
			} //switch idFrom
		}
		break; //End WM_NOTIFY

	default:
		break;
	}

	return FALSE;
}

static INT_PTR CALLBACK DlgProcBasicOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL initDlg = FALSE;

	switch (msg) {

	case WM_INITDIALOG:
		initDlg = TRUE;
		TranslateDialogDefault(hwndDlg);

		createProcessListAux();
		createXstatusListAux();

		CheckDlgButton(hwndDlg, IDC_ONMESSAGE, bFlashOnMsg ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ONFILE, bFlashOnFile ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_GCMSG, bFlashOnGC ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ONOTHER, bFlashOnOther ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_FSCREEN, bFullScreenMode ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SSAVER, bScreenSaverRunning ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOCKED, bWorkstationLocked ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_PGMS, bProcessesAreRunning ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ACTIVE, bWorkstationActive ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_IFOPEN, bFlashIfMsgOpen ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_IFNOTTOP, bFlashIfMsgWinNotTop ? BST_CHECKED : BST_UNCHECKED);
		if (!bFlashIfMsgOpen)
			EnableWindow(GetDlgItem(hwndDlg, IDC_IFNOTTOP), FALSE);
		CheckDlgButton(hwndDlg, IDC_IFOLDER, bFlashIfMsgOlder ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_OLDERSPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_SOLDER), 0);
		SendDlgItemMessage(hwndDlg, IDC_OLDERSPIN, UDM_SETRANGE32, 1, MAKELONG(UD_MAXVAL, 0));
		SendDlgItemMessage(hwndDlg, IDC_OLDERSPIN, UDM_SETPOS, 0, MAKELONG(wSecondsOlder, 0));
		if (!bFlashIfMsgOlder) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_SOLDER), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OLDERSPIN), FALSE);
		}

		CheckDlgButton(hwndDlg, IDC_UNTILBLK, bFlashUntil & UNTIL_NBLINKS ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_SBLINK), 0);
		SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_SETRANGE32, 1, MAKELONG(UD_MAXVAL, 0));
		SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_SETPOS, 0, MAKELONG(wBlinksNumber, 0));
		if (!(bFlashUntil & UNTIL_NBLINKS)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_SBLINK), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BLINKSPIN), FALSE);
		}
		CheckDlgButton(hwndDlg, IDC_UNTILATTENDED, bFlashUntil & UNTIL_REATTENDED ? BST_CHECKED : BST_UNCHECKED);
		for (int i = 0; i < 2; i++) {
			int index = SendDlgItemMessage(hwndDlg, IDC_MIRORWIN, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)AttendedName[i]);
			if (index != CB_ERR && index != CB_ERRSPACE)
				SendDlgItemMessage(hwndDlg, IDC_MIRORWIN, CB_SETITEMDATA, (WPARAM)index, (LPARAM)i);
		}
		SendDlgItemMessage(hwndDlg, IDC_MIRORWIN, CB_SETCURSEL, (WPARAM)bMirandaOrWindows, 0);
		if (!(bFlashUntil & UNTIL_REATTENDED))
			EnableWindow(GetDlgItem(hwndDlg, IDC_MIRORWIN), FALSE);
		CheckDlgButton(hwndDlg, IDC_UNTILOPEN, bFlashUntil & UNTIL_EVENTSOPEN ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_UNTILCOND, bFlashUntil & UNTIL_CONDITIONS ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_ONLINE, wStatusMap & MAP_ONLINE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AWAY, wStatusMap & MAP_AWAY ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_NA, wStatusMap & MAP_NA ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_OCCUPIED, wStatusMap & MAP_OCCUPIED ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DND, wStatusMap & MAP_DND ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_FREECHAT, wStatusMap & MAP_FREECHAT ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_INVISIBLE, wStatusMap & MAP_INVISIBLE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_OFFLINE, wStatusMap & MAP_OFFLINE ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg, IDC_REMCHECK, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_SREMCHECK), 0);
		SendDlgItemMessage(hwndDlg, IDC_REMCHECK, UDM_SETRANGE32, 0, MAKELONG(UD_MAXVAL, 0));
		SendDlgItemMessage(hwndDlg, IDC_REMCHECK, UDM_SETPOS, 0, MAKELONG(wReminderCheck, 0));

		initDlg = FALSE;
		return TRUE;

	case WM_VSCROLL:
	case WM_HSCROLL:
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_DESTROY:
		destroyProcessListAux();
		destroyXstatusListAux();
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ONMESSAGE:
		case IDC_GCMSG:
		case IDC_ONFILE:
		case IDC_ONOTHER:
		case IDC_IFOPEN:
		case IDC_IFNOTTOP:
		case IDC_IFOLDER:
		case IDC_UNTILBLK:
		case IDC_UNTILATTENDED:
		case IDC_UNTILOPEN:
		case IDC_UNTILCOND:
		case IDC_FSCREEN:
		case IDC_SSAVER:
		case IDC_LOCKED:
		case IDC_PGMS:
		case IDC_ACTIVE:
		case IDC_ONLINE:
		case IDC_AWAY:
		case IDC_NA:
		case IDC_OCCUPIED:
		case IDC_DND:
		case IDC_FREECHAT:
		case IDC_INVISIBLE:
		case IDC_OFFLINE:
			EnableWindow(GetDlgItem(hwndDlg, IDC_IFNOTTOP), IsDlgButtonChecked(hwndDlg, IDC_IFOPEN) == BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SOLDER), IsDlgButtonChecked(hwndDlg, IDC_IFOLDER) == BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OLDERSPIN), IsDlgButtonChecked(hwndDlg, IDC_IFOLDER) == BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SBLINK), IsDlgButtonChecked(hwndDlg, IDC_UNTILBLK) == BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BLINKSPIN), IsDlgButtonChecked(hwndDlg, IDC_UNTILBLK) == BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_MIRORWIN), IsDlgButtonChecked(hwndDlg, IDC_UNTILATTENDED) == BST_CHECKED);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		case IDC_SOLDER:
			if (HIWORD(wParam) == EN_CHANGE && !initDlg) {
				BOOL translated;
				int val = GetDlgItemInt(hwndDlg, IDC_SOLDER, &translated, FALSE);
				if (translated && val < 1)
					SendDlgItemMessage(hwndDlg, IDC_OLDERSPIN, UDM_SETPOS, 0, MAKELONG(val, 0));
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			return TRUE;
		case IDC_SBLINK:
			if (HIWORD(wParam) == EN_CHANGE && !initDlg) {
				BOOL translated;
				int val = GetDlgItemInt(hwndDlg, IDC_SBLINK, &translated, FALSE);
				if (translated && val < 1)
					SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_SETPOS, 0, MAKELONG(val, 0));
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			return TRUE;
		case IDC_ASSIGNPGMS:
			if (DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_PROCESSES), hwndDlg, DlgProcProcesses, 0) == IDC_OKPGM)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		case IDC_SELECTXSTATUS:
			if (DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_XSTATUSES), hwndDlg, DlgProcXstatusList, 0) == IDC_OKXST)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		case IDC_SREMCHECK:
			if (HIWORD(wParam) == EN_CHANGE && !initDlg)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		case IDC_MIRORWIN:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		}
		break;

	case WM_NOTIFY:
		{
			uint8_t untilMap = 0;
			uint16_t statusMap = 0;
			//Here we have pressed either the OK or the APPLY button.
			switch (((LPNMHDR)lParam)->idFrom) {
			case 0:
				switch (((LPNMHDR)lParam)->code) {
				case PSN_APPLY:
					g_plugin.setByte("onmsg", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_ONMESSAGE) == BST_CHECKED ? 1 : 0));
					g_plugin.setByte("ongcmsg", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_GCMSG) == BST_CHECKED ? 1 : 0));
					g_plugin.setByte("onfile", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_ONFILE) == BST_CHECKED ? 1 : 0));
					g_plugin.setByte("onother", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_ONOTHER) == BST_CHECKED ? 1 : 0));

					g_plugin.setByte("fscreenmode", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_FSCREEN) == BST_CHECKED ? 1 : 0));
					g_plugin.setByte("ssaverrunning", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_SSAVER) == BST_CHECKED ? 1 : 0));
					g_plugin.setByte("wstationlocked", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_LOCKED) == BST_CHECKED ? 1 : 0));
					g_plugin.setByte("procsrunning", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_PGMS) == BST_CHECKED ? 1 : 0));
					g_plugin.setByte("wstationactive", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_ACTIVE) == BST_CHECKED ? 1 : 0));

					g_plugin.setByte("ifmsgopen", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_IFOPEN) == BST_CHECKED ? 1 : 0));
					g_plugin.setByte("ifmsgnottop", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_IFNOTTOP) == BST_CHECKED ? 1 : 0));
					g_plugin.setByte("ifmsgolder", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_IFOLDER) == BST_CHECKED ? 1 : 0));
					g_plugin.setWord("secsolder", (uint16_t)SendDlgItemMessage(hwndDlg, IDC_OLDERSPIN, UDM_GETPOS, 0, 0));

					if (IsDlgButtonChecked(hwndDlg, IDC_UNTILBLK) == BST_CHECKED)
						untilMap |= UNTIL_NBLINKS;
					if (IsDlgButtonChecked(hwndDlg, IDC_UNTILATTENDED) == BST_CHECKED)
						untilMap |= UNTIL_REATTENDED;
					if (IsDlgButtonChecked(hwndDlg, IDC_UNTILOPEN) == BST_CHECKED)
						untilMap |= UNTIL_EVENTSOPEN;
					if (IsDlgButtonChecked(hwndDlg, IDC_UNTILCOND) == BST_CHECKED)
						untilMap |= UNTIL_CONDITIONS;
					g_plugin.setByte("funtil", untilMap);
					g_plugin.setWord("nblinks", (uint16_t)SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_GETPOS, 0, 0));
					g_plugin.setByte("mirorwin", (uint8_t)SendDlgItemMessage(hwndDlg, IDC_MIRORWIN, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_MIRORWIN, CB_GETCURSEL, 0, 0), 0));

					if (IsDlgButtonChecked(hwndDlg, IDC_ONLINE) == BST_CHECKED)
						statusMap |= MAP_ONLINE;
					if (IsDlgButtonChecked(hwndDlg, IDC_AWAY) == BST_CHECKED)
						statusMap |= MAP_AWAY;
					if (IsDlgButtonChecked(hwndDlg, IDC_NA) == BST_CHECKED)
						statusMap |= MAP_NA;
					if (IsDlgButtonChecked(hwndDlg, IDC_OCCUPIED) == BST_CHECKED)
						statusMap |= MAP_OCCUPIED;
					if (IsDlgButtonChecked(hwndDlg, IDC_DND) == BST_CHECKED)
						statusMap |= MAP_DND;
					if (IsDlgButtonChecked(hwndDlg, IDC_FREECHAT) == BST_CHECKED)
						statusMap |= MAP_FREECHAT;
					if (IsDlgButtonChecked(hwndDlg, IDC_INVISIBLE) == BST_CHECKED)
						statusMap |= MAP_INVISIBLE;
					if (IsDlgButtonChecked(hwndDlg, IDC_OFFLINE) == BST_CHECKED)
						statusMap |= MAP_OFFLINE;
					g_plugin.setWord("status", statusMap);

					g_plugin.setWord("remcheck", (uint16_t)SendDlgItemMessage(hwndDlg, IDC_REMCHECK, UDM_GETPOS, 0, 0));

					int i = 0;
					for (int j = 0; j < ProcessListAux.count; j++)
						if (ProcessListAux.szFileName[j])
							g_plugin.setWString(fmtDBSettingName("process%d", i++), ProcessListAux.szFileName[j]);
					g_plugin.setWord("processcount", (uint16_t)i);
					while (!g_plugin.delSetting(fmtDBSettingName("process%d", i++)));

					if (XstatusListAux)
						for (i = 0; i < ProtoList.protoCount; i++)
							for (int j = 0; j < (int)XstatusListAux[i].count; j++)
								g_plugin.setByte(fmtDBSettingName("%sxstatus%d", ProtoList.protoInfo[i].szProto, j), (uint8_t)XstatusListAux[i].enabled[j]);

					LoadSettings();
					return TRUE;
				} // switch code
				break;
			} //switch idFrom
		}
		break; //End WM_NOTIFY

	default:
		break;
	}

	return FALSE;
}

static INT_PTR CALLBACK DlgProcEffectOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL initDlg = FALSE;

	switch (msg) {

	case WM_INITDIALOG:
		initDlg = TRUE;
		TranslateDialogDefault(hwndDlg);

		trillianLedsMsg = bTrillianLedsMsg;
		trillianLedsURL = bTrillianLedsURL;
		trillianLedsFile = bTrillianLedsFile;
		trillianLedsOther = bTrillianLedsOther;

		CheckDlgButton(hwndDlg, IDC_NUM, bFlashLed[0] ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CAPS, bFlashLed[1] ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SCROLL, bFlashLed[2] ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_SAMETIME, bFlashEffect == FLASH_SAMETIME ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_INTURN, bFlashEffect == FLASH_INTURN ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_INSEQUENCE, bFlashEffect == FLASH_INSEQUENCE ? BST_CHECKED : BST_UNCHECKED);
		for (int i = 0; i < 3; i++) {
			int index = SendDlgItemMessage(hwndDlg, IDC_SEQORDER, CB_INSERTSTRING, -1, (LPARAM)TranslateW(OrderName[i]));
			if (index != CB_ERR && index != CB_ERRSPACE)
				SendDlgItemMessage(hwndDlg, IDC_SEQORDER, CB_SETITEMDATA, (WPARAM)index, (LPARAM)i);
		}
		SendDlgItemMessage(hwndDlg, IDC_SEQORDER, CB_SETCURSEL, (WPARAM)bSequenceOrder, 0);
		if (bFlashEffect != FLASH_INSEQUENCE)
			EnableWindow(GetDlgItem(hwndDlg, IDC_SEQORDER), FALSE);
		CheckDlgButton(hwndDlg, IDC_CUSTOM, bFlashEffect == FLASH_CUSTOM ? BST_CHECKED : BST_UNCHECKED);

		wchar_t *szTheme;
		for (int i = 0; szTheme = g_plugin.getWStringA(fmtDBSettingName("theme%d", i)); i++) {
			int index = SendDlgItemMessage(hwndDlg, IDC_SCUSTOM, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)szTheme);
			mir_free(szTheme);
			if (index != CB_ERR && index != CB_ERRSPACE)
				SendDlgItemMessage(hwndDlg, IDC_SCUSTOM, CB_SETITEMDATA, (WPARAM)index, (LPARAM)i);
		}

		SendDlgItemMessage(hwndDlg, IDC_SCUSTOM, CB_SETCURSEL, (WPARAM)wCustomTheme, 0);
		if (bFlashEffect != FLASH_CUSTOM)
			EnableWindow(GetDlgItem(hwndDlg, IDC_SCUSTOM), FALSE);
		CheckDlgButton(hwndDlg, IDC_TRILLIAN, bFlashEffect == FLASH_TRILLIAN ? BST_CHECKED : BST_UNCHECKED);
		if (bFlashEffect != FLASH_TRILLIAN)
			EnableWindow(GetDlgItem(hwndDlg, IDC_ASSIGNLEDS), FALSE);

		SendDlgItemMessage(hwndDlg, IDC_DELAYSPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_SDELAY), 0);
		SendDlgItemMessage(hwndDlg, IDC_DELAYSPIN, UDM_SETRANGE32, 0, MAKELONG(UD_MAXVAL, 0));
		SendDlgItemMessage(hwndDlg, IDC_DELAYSPIN, UDM_SETPOS, 0, MAKELONG(wStartDelay, 0));

		SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_SETRANGE, FALSE, MAKELONG(0, 5));
		SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_SETPOS, TRUE, bFlashSpeed);

		CheckDlgButton(hwndDlg, IDC_KEYPRESSES, bEmulateKeypresses ? BST_CHECKED : BST_UNCHECKED);

		initDlg = FALSE;
		return TRUE;

	case WM_VSCROLL:
	case WM_HSCROLL:
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_DESTROY:
		previewFlashing(FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_NUM:
		case IDC_CAPS:
		case IDC_SCROLL:
		case IDC_SAMETIME:
		case IDC_INTURN:
		case IDC_INSEQUENCE:
		case IDC_CUSTOM:
		case IDC_TRILLIAN:
		case IDC_SPEED:
		case IDC_KEYPRESSES:
			EnableWindow(GetDlgItem(hwndDlg, IDC_SEQORDER), IsDlgButtonChecked(hwndDlg, IDC_INSEQUENCE) == BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SCUSTOM), IsDlgButtonChecked(hwndDlg, IDC_CUSTOM) == BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ASSIGNLEDS), IsDlgButtonChecked(hwndDlg, IDC_TRILLIAN) == BST_CHECKED);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		case IDC_ASSIGNLEDS:
			if (DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_EVENTLEDS), hwndDlg, DlgProcEventLeds, 0) == IDC_OK)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		case IDC_SDELAY:
			if (HIWORD(wParam) == EN_CHANGE && !initDlg)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		case IDC_PREVIEW:
			previewFlashing(IsDlgButtonChecked(hwndDlg, IDC_PREVIEW) == BST_CHECKED);
			return TRUE;
		case IDC_SEQORDER:
		case IDC_SCUSTOM:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		}
		break;

	case WM_NOTIFY:
		{
			//Here we have pressed either the OK or the APPLY button.
			switch (((LPNMHDR)lParam)->idFrom) {
			case 0:
				switch (((LPNMHDR)lParam)->code) {
				case PSN_APPLY:
					g_plugin.setByte("fnum", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_NUM) == BST_CHECKED ? 1 : 0));
					g_plugin.setByte("fcaps", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_CAPS) == BST_CHECKED ? 1 : 0));
					g_plugin.setByte("fscroll", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_SCROLL) == BST_CHECKED ? 1 : 0));

					if (IsDlgButtonChecked(hwndDlg, IDC_INTURN) == BST_CHECKED)
						g_plugin.setByte("feffect", FLASH_INTURN);
					else if (IsDlgButtonChecked(hwndDlg, IDC_INSEQUENCE) == BST_CHECKED)
						g_plugin.setByte("feffect", FLASH_INSEQUENCE);
					else if (IsDlgButtonChecked(hwndDlg, IDC_CUSTOM) == BST_CHECKED)
						g_plugin.setByte("feffect", FLASH_CUSTOM);
					else if (IsDlgButtonChecked(hwndDlg, IDC_TRILLIAN) == BST_CHECKED)
						g_plugin.setByte("feffect", FLASH_TRILLIAN);
					else
						g_plugin.setByte("feffect", FLASH_SAMETIME);
					g_plugin.setByte("order", (uint8_t)SendDlgItemMessage(hwndDlg, IDC_SEQORDER, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_SEQORDER, CB_GETCURSEL, 0, 0), 0));
					g_plugin.setWord("custom", (uint16_t)SendDlgItemMessage(hwndDlg, IDC_SCUSTOM, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_SCUSTOM, CB_GETCURSEL, 0, 0), 0));

					g_plugin.setByte("ledsmsg", trillianLedsMsg);
					g_plugin.setByte("ledsfile", trillianLedsFile);
					g_plugin.setByte("ledsurl", trillianLedsURL);
					g_plugin.setByte("ledsother", trillianLedsOther);

					g_plugin.setWord("sdelay", (uint16_t)SendDlgItemMessage(hwndDlg, IDC_DELAYSPIN, UDM_GETPOS, 0, 0));

					g_plugin.setByte("speed", (uint8_t)SendDlgItemMessage(hwndDlg, IDC_SPEED, TBM_GETPOS, 0, 0));

					g_plugin.setByte("keypresses", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_KEYPRESSES) == BST_CHECKED ? 1 : 0));

					LoadSettings();

					return TRUE;
				} // switch code
				break;
			} //switch idFrom
		}
		break; //End WM_NOTIFY

	default:
		break;
	}

	return FALSE;
}

static INT_PTR CALLBACK DlgProcThemeOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL initDlg = FALSE;

	switch (msg) {

	case WM_INITDIALOG:
		initDlg = TRUE;
		TranslateDialogDefault(hwndDlg);

		SendDlgItemMessage(hwndDlg, IDC_THEME, EM_LIMITTEXT, MAX_PATH, 0);
		SendDlgItemMessage(hwndDlg, IDC_CUSTOMSTRING, EM_LIMITTEXT, MAX_PATH, 0);

		wchar_t *szTheme;
		for (int i = 0; szTheme = g_plugin.getWStringA(fmtDBSettingName("theme%d", i)); i++) {
			int index = SendDlgItemMessage(hwndDlg, IDC_THEME, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)szTheme);
			mir_free(szTheme);
			if (index != CB_ERR && index != CB_ERRSPACE) {
				wchar_t *str = g_plugin.getWStringA(fmtDBSettingName("custom%d", i));
				if (str)
					SendDlgItemMessage(hwndDlg, IDC_THEME, CB_SETITEMDATA, (WPARAM)index, (LPARAM)str);
			}
		}

		EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE), FALSE);
		if (SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETCOUNT, 0, 0) == 0)
			EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), FALSE);
		else {
			SendDlgItemMessage(hwndDlg, IDC_THEME, CB_SETCURSEL, (WPARAM)wCustomTheme, 0);
			wchar_t *str = (wchar_t *)SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETITEMDATA, (WPARAM)wCustomTheme, 0);
			if (str)
				SetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, str);
		}

		CheckDlgButton(hwndDlg, IDC_OVERRIDE, bOverride ? BST_CHECKED : BST_UNCHECKED);

		initDlg = FALSE;
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_THEME:
			switch (HIWORD(wParam)) {
			case CBN_SELENDOK:
			case CBN_SELCHANGE:
				{
					wchar_t *str = (wchar_t *)SendMessage((HWND)lParam, CB_GETITEMDATA, (WPARAM)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0), 0);
					if (str)
						SetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, str);
					else
						SetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, L"");
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), TRUE);
				}
				break;
			case CBN_EDITCHANGE:
				wchar_t theme[MAX_PATH + 1];
				GetDlgItemText(hwndDlg, IDC_THEME, theme, _countof(theme));
				int item = SendMessage((HWND)lParam, CB_FINDSTRINGEXACT, -1, (LPARAM)theme);
				if (item == CB_ERR) {
					//new theme
					SetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, L"");
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), FALSE);
				}
				else {
					wchar_t *str = (wchar_t *)SendMessage((HWND)lParam, CB_GETITEMDATA, (WPARAM)item, 0);
					if (str)
						SetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, str);
					else
						SetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, L"");
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), TRUE);
				}
				EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE), FALSE);
				break;
			}
			return TRUE;
		case IDC_CUSTOMSTRING:
			if (HIWORD(wParam) == EN_CHANGE) {
				wchar_t theme[MAX_PATH + 1], customAux[MAX_PATH + 1];

				GetDlgItemText(hwndDlg, IDC_THEME, theme, _countof(theme));
				int item = SendDlgItemMessage(hwndDlg, IDC_THEME, CB_FINDSTRINGEXACT, -1, (LPARAM)theme);
				if (item == CB_ERR)
					return TRUE;
				wchar_t *str = (wchar_t *)SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETITEMDATA, (WPARAM)item, 0);
				if (str) {
					GetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, customAux, _countof(customAux));
					if (mir_wstrcmp(str, customAux))
						EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE), TRUE);
					else
						EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE), FALSE);
				}
			}
			return TRUE;
		case IDC_TEST:
			{
				wchar_t custom[MAX_PATH + 1];

				GetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, custom, _countof(custom));
				SetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, normalizeCustomString(custom));
				testSequence(custom);
			}
			return TRUE;
		case IDC_ADD:
			{
				wchar_t theme[MAX_PATH + 1];

				GetDlgItemText(hwndDlg, IDC_THEME, theme, _countof(theme));
				if (!theme[0])
					return TRUE;
				int item = SendDlgItemMessage(hwndDlg, IDC_THEME, CB_ADDSTRING, 0, (LPARAM)theme);
				wchar_t *str = (wchar_t *)mir_alloc((MAX_PATH + 1) * sizeof(wchar_t));
				if (str) {
					GetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, str, MAX_PATH);
					SetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, normalizeCustomString(str));
				}
				SendDlgItemMessage(hwndDlg, IDC_THEME, CB_SETITEMDATA, (WPARAM)item, (LPARAM)str);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), TRUE);
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		case IDC_UPDATE:
			{
				wchar_t theme[MAX_PATH + 1];

				GetDlgItemText(hwndDlg, IDC_THEME, theme, _countof(theme));
				int item = SendDlgItemMessage(hwndDlg, IDC_THEME, CB_FINDSTRINGEXACT, -1, (LPARAM)theme);
				wchar_t *str = (wchar_t *)SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETITEMDATA, (WPARAM)item, 0);
				if (str) {
					GetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, str, MAX_PATH);
					SetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, normalizeCustomString(str));
				}
				EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE), FALSE);
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		case IDC_DELETE:
			{
				wchar_t theme[MAX_PATH + 1];

				GetDlgItemText(hwndDlg, IDC_THEME, theme, _countof(theme));
				int item = SendDlgItemMessage(hwndDlg, IDC_THEME, CB_FINDSTRINGEXACT, -1, (LPARAM)theme);
				wchar_t *str = (wchar_t *)SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETITEMDATA, (WPARAM)item, 0);
				if (str)
					mir_free(str);
				SendDlgItemMessage(hwndDlg, IDC_THEME, CB_DELETESTRING, (WPARAM)item, 0);
				if (SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETCOUNT, 0, 0) == 0) {
					SetDlgItemText(hwndDlg, IDC_THEME, L"");
					SetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, L"");
					EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), FALSE);
				}
				else {
					SendDlgItemMessage(hwndDlg, IDC_THEME, CB_SETCURSEL, 0, 0);
					str = (wchar_t *)SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETITEMDATA, 0, 0);
					if (str)
						SetDlgItemText(hwndDlg, IDC_CUSTOMSTRING, str);
				}
				EnableWindow(GetDlgItem(hwndDlg, IDC_UPDATE), FALSE);
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		case IDC_EXPORT:
			{
				wchar_t path[MAX_PATH + 1], filter[MAX_PATH + 1], *pfilter;
				OPENFILENAME ofn = { 0 };

				path[0] = '\0';
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = hwndDlg;
				ofn.hInstance = nullptr;
				mir_wstrcpy(filter, TranslateT("Keyboard Notify Theme"));
				mir_wstrcat(filter, L" (*.knt)");
				pfilter = filter + mir_wstrlen(filter) + 1;
				mir_wstrcpy(pfilter, L"*.knt");
				pfilter = pfilter + mir_wstrlen(pfilter) + 1;
				mir_wstrcpy(pfilter, TranslateT("All Files"));
				pfilter = pfilter + mir_wstrlen(pfilter) + 1;
				mir_wstrcpy(pfilter, L"*.*");
				pfilter = pfilter + mir_wstrlen(pfilter) + 1;
				*pfilter = '\0';
				ofn.lpstrFilter = filter;
				ofn.lpstrFile = path;
				ofn.Flags = OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST;
				ofn.nMaxFile = _countof(path);
				ofn.lpstrDefExt = L"knt";
				if (GetSaveFileName(&ofn))
					exportThemes(path);
			}
			return TRUE;
		case IDC_IMPORT:
			{
				wchar_t path[MAX_PATH + 1], filter[MAX_PATH + 1], *pfilter;
				OPENFILENAME ofn = { 0 };

				path[0] = '\0';
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = hwndDlg;
				ofn.hInstance = nullptr;
				mir_wstrcpy(filter, TranslateT("Keyboard Notify Theme"));
				mir_wstrcat(filter, L" (*.knt)");
				pfilter = filter + mir_wstrlen(filter) + 1;
				mir_wstrcpy(pfilter, L"*.knt");
				pfilter = pfilter + mir_wstrlen(pfilter) + 1;
				mir_wstrcpy(pfilter, TranslateT("All Files"));
				pfilter = pfilter + mir_wstrlen(pfilter) + 1;
				mir_wstrcpy(pfilter, L"*.*");
				pfilter = pfilter + mir_wstrlen(pfilter) + 1;
				*pfilter = '\0';
				ofn.lpstrFilter = filter;
				ofn.lpstrFile = path;
				ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
				ofn.nMaxFile = _countof(path);
				ofn.lpstrDefExt = L"knt";
				if (GetOpenFileName(&ofn)) {
					importThemes(path, IsDlgButtonChecked(hwndDlg, IDC_OVERRIDE) == BST_CHECKED);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
			}
			return TRUE;
		case IDC_OVERRIDE:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		}
		break;

	case WM_NOTIFY:
		{
			int count;
			wchar_t theme[MAX_PATH + 1], themeAux[MAX_PATH + 1];
			// Here we have pressed either the OK or the APPLY button.
			switch (((LPNMHDR)lParam)->idFrom) {
			case 0:
				switch (((LPNMHDR)lParam)->code) {
				case PSN_APPLY:
					if (szTheme = g_plugin.getWStringA(fmtDBSettingName("theme%d", wCustomTheme))) {
						mir_wstrcpy(theme, szTheme);
						mir_free(szTheme);
					}
					else
						theme[0] = '\0';

					// Here we will delete all the items in the theme combo on the Flashing tab: we will load them again later
					for (int i = 0; SendDlgItemMessage(hwndEffect, IDC_SCUSTOM, CB_DELETESTRING, 0, (LPARAM)i) != CB_ERR; i++);

					count = SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETCOUNT, 0, 0);
					for (int i = 0; i < count; i++) {
						SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETLBTEXT, (WPARAM)i, (LPARAM)themeAux);
						g_plugin.setWString(fmtDBSettingName("theme%d", i), themeAux);
						wchar_t *str = (wchar_t *)SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETITEMDATA, (WPARAM)i, 0);
						if (str)
							g_plugin.setWString(fmtDBSettingName("custom%d", i), str);
						else
							g_plugin.setWString(fmtDBSettingName("custom%d", i), L"");

						if (!mir_wstrcmp(theme, themeAux))
							wCustomTheme = i;

						// Here we will update the theme combo on the Flashing tab: horrible but can't imagine a better way right now
						SendDlgItemMessage(hwndEffect, IDC_SCUSTOM, CB_INSERTSTRING, (WPARAM)i, (LPARAM)themeAux);
						SendDlgItemMessage(hwndEffect, IDC_SCUSTOM, CB_SETITEMDATA, (WPARAM)i, (LPARAM)i);
					}
					for (int i = count; !g_plugin.delSetting(fmtDBSettingName("theme%d", i)); i++)
						g_plugin.delSetting(fmtDBSettingName("custom%d", i));

					g_plugin.setWord("custom", wCustomTheme);
					// Still updating here the the Flashing tab's controls
					SendDlgItemMessage(hwndEffect, IDC_SCUSTOM, CB_SETCURSEL, (WPARAM)wCustomTheme, 0);

					g_plugin.setByte("override", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_OVERRIDE) == BST_CHECKED ? 1 : 0));

					return TRUE;
				} // switch code
				break;
			} //switch idFrom
		}
		break; //End WM_NOTIFY

	case WM_DESTROY:
		int count = SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETCOUNT, 0, 0);

		for (int item = 0; item < count; item++) {
			wchar_t *str = (wchar_t *)SendDlgItemMessage(hwndDlg, IDC_THEME, CB_GETITEMDATA, (WPARAM)item, 0);
			if (str)
				mir_free(str);
		}
		break;
	}

	return FALSE;
}

// **
// ** Initialize the Miranda options page
// **
int InitializeOptions(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szTitle.a = LPGEN("Keyboard Flash");
	odp.szGroup.a = LPGEN("Events");
	odp.flags = ODPF_BOLDGROUPS;

	odp.szTab.a = LPGEN("Accounts");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_PROTO_OPTIONS);
	odp.pfnDlgProc = DlgProcProtoOptions;
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Rules");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_BASIC_OPTIONS);
	odp.pfnDlgProc = DlgProcBasicOptions;
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Flashing");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_EFFECT_OPTIONS);
	odp.pfnDlgProc = DlgProcEffectOptions;
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Themes");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_THEME_OPTIONS);
	odp.pfnDlgProc = DlgProcThemeOptions;
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Ignore");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_IGNORE_OPTIONS);
	odp.pfnDlgProc = DlgProcIgnoreOptions;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
