/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include "..\..\core\commonheaders.h"
#include "findadd.h"

enum {
	COLUMNID_PROTO,
	COLUMNID_HANDLE,
	COLUMNID_NICK,
	COLUMNID_FIRST,
	COLUMNID_LAST,
	COLUMNID_EMAIL,
	NUM_COLUMNID
};

void SaveColumnSizes(HWND hwndResults)
{
	int columnOrder[NUM_COLUMNID];

	struct FindAddDlgData *dat = (struct FindAddDlgData*)GetWindowLongPtr(GetParent(hwndResults), GWLP_USERDATA);
	int columnCount = Header_GetItemCount(ListView_GetHeader(hwndResults));
	if (columnCount != NUM_COLUMNID) return;
	ListView_GetColumnOrderArray(hwndResults, columnCount, columnOrder);
	for (int i = 0; i < NUM_COLUMNID; i++) {
		char szSetting[32];
		mir_snprintf(szSetting, SIZEOF(szSetting), "ColOrder%d", i);
		db_set_b(NULL, "FindAdd", szSetting, (BYTE)columnOrder[i]);
		mir_snprintf(szSetting, SIZEOF(szSetting), "ColWidth%d", i);
		db_set_w(NULL, "FindAdd", szSetting, (WORD)ListView_GetColumnWidth(hwndResults, i));
	}
	db_set_b(NULL, "FindAdd", "SortColumn", (BYTE)dat->iLastColumnSortIndex);
	db_set_b(NULL, "FindAdd", "SortAscending", (BYTE)dat->bSortAscending);
}

static const TCHAR *szColumnNames[] = { NULL, NULL, _T("Nick"), _T("First Name"), _T("Last Name"), _T("E-mail") };
static int defaultColumnSizes[] = { 0, 90, 100, 100, 100, 2000 };
void LoadColumnSizes(HWND hwndResults, const char *szProto)
{
	HDITEM hdi;
	int columnOrder[NUM_COLUMNID];
	int columnCount;
	char szSetting[32];
	bool colOrdersValid;

	defaultColumnSizes[COLUMNID_PROTO] = GetSystemMetrics(SM_CXSMICON) + 4;
	FindAddDlgData *dat = (FindAddDlgData*)GetWindowLongPtr(GetParent(hwndResults), GWLP_USERDATA);

	columnCount = NUM_COLUMNID;
	colOrdersValid = true;
	for (int i = 0; i < NUM_COLUMNID; i++) {
		LVCOLUMN lvc;
		if (i < columnCount) {
			int bNeedsFree = FALSE;
			lvc.mask = LVCF_TEXT | LVCF_WIDTH;
			if (szColumnNames[i] != NULL)
				lvc.pszText = TranslateTS(szColumnNames[i]);
			else if (i == COLUMNID_HANDLE) {
				if (szProto) {
					bNeedsFree = TRUE;
					lvc.pszText = mir_a2t((char*)CallProtoServiceInt(NULL, szProto, PS_GETCAPS, PFLAG_UNIQUEIDTEXT, 0));
				}
				else lvc.pszText = _T("ID");
			}
			else lvc.mask &= ~LVCF_TEXT;
			mir_snprintf(szSetting, SIZEOF(szSetting), "ColWidth%d", i);
			lvc.cx = db_get_w(NULL, "FindAdd", szSetting, defaultColumnSizes[i]);
			ListView_InsertColumn(hwndResults, i, (LPARAM)&lvc);

			if (bNeedsFree)
				mir_free(lvc.pszText);
		}
		mir_snprintf(szSetting, SIZEOF(szSetting), "ColOrder%d", i);
		columnOrder[i] = db_get_b(NULL, "FindAdd", szSetting, -1);
		if (columnOrder[i] == -1 || columnOrder[i] >= NUM_COLUMNID)
			colOrdersValid = false;
	}

	if (colOrdersValid)
		ListView_SetColumnOrderArray(hwndResults, columnCount, columnOrder);

	dat->iLastColumnSortIndex = db_get_b(NULL, "FindAdd", "SortColumn", COLUMNID_NICK);
	if (dat->iLastColumnSortIndex >= columnCount) dat->iLastColumnSortIndex = COLUMNID_NICK;
	dat->bSortAscending = db_get_b(NULL, "FindAdd", "SortAscending", TRUE);

	hdi.mask = HDI_FORMAT;
	hdi.fmt = HDF_LEFT | HDF_STRING | (dat->bSortAscending ? HDF_SORTDOWN : HDF_SORTUP);
	Header_SetItem(ListView_GetHeader(hwndResults), dat->iLastColumnSortIndex, &hdi);
}

static LPARAM ListView_GetItemLParam(HWND hwndList, int idx)
{
	LVITEM lv;
	lv.iItem = idx;
	lv.mask = LVIF_PARAM;
	ListView_GetItem(hwndList, &lv);
	return lv.lParam;
}

int CALLBACK SearchResultsCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	struct FindAddDlgData *dat = (struct FindAddDlgData*)GetWindowLongPtr((HWND)lParamSort, GWLP_USERDATA);
	struct ListSearchResult *lsr1, *lsr2;
	HWND hList = GetDlgItem((HWND)lParamSort, IDC_RESULTS);

	int sortMultiplier = dat->bSortAscending ? 1 : -1;
	int sortCol = dat->iLastColumnSortIndex;
	if (!dat->bFlexSearchResult) {
		lsr1 = (struct ListSearchResult*)ListView_GetItemLParam(hList, (int)lParam1);
		lsr2 = (struct ListSearchResult*)ListView_GetItemLParam(hList, (int)lParam2);
		if (lsr1 == NULL || lsr2 == NULL)
			return 0;
		
		switch (sortCol) {
		case COLUMNID_PROTO:
			return mir_strcmp(lsr1->szProto, lsr2->szProto)*sortMultiplier;
		case COLUMNID_HANDLE:
			return mir_tstrcmpi(lsr1->psr.id.t, lsr2->psr.id.t)*sortMultiplier;
		case COLUMNID_NICK:
			return mir_tstrcmpi(lsr1->psr.nick.t, lsr2->psr.nick.t)*sortMultiplier;
		case COLUMNID_FIRST:
			return mir_tstrcmpi(lsr1->psr.firstName.t, lsr2->psr.firstName.t)*sortMultiplier;
		case COLUMNID_LAST:
			return mir_tstrcmpi(lsr1->psr.lastName.t, lsr2->psr.lastName.t)*sortMultiplier;
		case COLUMNID_EMAIL:
			return mir_tstrcmpi(lsr1->psr.email.t, lsr2->psr.email.t)*sortMultiplier;
		}
	}
	else {
		TCHAR szText1[100];
		TCHAR szText2[100];
		ListView_GetItemText(hList, (int)lParam1, sortCol, szText1, SIZEOF(szText1));
		ListView_GetItemText(hList, (int)lParam2, sortCol, szText2, SIZEOF(szText2));
		return mir_tstrcmpi(szText1, szText2)*sortMultiplier;
	}
	return 0;
}

void FreeSearchResults(HWND hwndResults)
{
	LV_ITEM lvi;
	for (lvi.iItem = ListView_GetItemCount(hwndResults) - 1; lvi.iItem >= 0; lvi.iItem--) {
		lvi.mask = LVIF_PARAM;
		ListView_GetItem(hwndResults, &lvi);
		struct ListSearchResult *lsr = (struct ListSearchResult*)lvi.lParam;
		if (lsr == NULL) continue;
		mir_free(lsr->psr.id.t);
		mir_free(lsr->psr.email.t);
		mir_free(lsr->psr.nick.t);
		mir_free(lsr->psr.firstName.t);
		mir_free(lsr->psr.lastName.t);
		mir_free(lsr);
	}
	ListView_DeleteAllItems(hwndResults);
	EnableResultButtons(GetParent(hwndResults), 0);
}

// on its own thread
static void BeginSearchFailed(void * arg)
{
	TCHAR buf[128];
	if (arg != NULL) {
		const TCHAR* protoName = (TCHAR*)arg;
		mir_sntprintf(buf, SIZEOF(buf),
			TranslateT("Could not start a search on '%s', there was a problem - is %s connected?"),
			protoName, protoName);
		mir_free((char*)arg);
	}
	else mir_tstrncpy(buf, TranslateT("Could not search on any of the protocols, are you online?"), SIZEOF(buf));
	MessageBox(0, buf, TranslateT("Problem with search"), MB_OK | MB_ICONERROR);
}

int BeginSearch(HWND, struct FindAddDlgData *dat, const char *szProto, const char *szSearchService, DWORD requiredCapability, void *pvSearchParams)
{
	if (szProto == NULL) {
		int failures = 0;
		dat->searchCount = 0;
		dat->search = (struct ProtoSearchInfo*)mir_calloc(sizeof(struct ProtoSearchInfo) * accounts.getCount());
		for (int i = 0; i < accounts.getCount(); i++) {
			PROTOACCOUNT *pa = accounts[i];
			if (!Proto_IsAccountEnabled(pa)) continue;
			DWORD caps = (DWORD)CallProtoServiceInt(NULL, pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0);
			if (!(caps&requiredCapability)) continue;
			dat->search[dat->searchCount].hProcess = (HANDLE)CallProtoServiceInt(NULL, pa->szModuleName, szSearchService, 0, (LPARAM)pvSearchParams);
			dat->search[dat->searchCount].szProto = pa->szModuleName;
			if (dat->search[dat->searchCount].hProcess == NULL) failures++;
			else dat->searchCount++;
		}
		if (failures) {
			//infuriatingly vague error message. fixme.
			if (dat->searchCount == 0) {
				forkthread(BeginSearchFailed, 0, NULL);
				mir_free(dat->search);
				dat->search = NULL;
				return 1;
			}
		}
	}
	else {
		dat->search = (struct ProtoSearchInfo*)mir_alloc(sizeof(struct ProtoSearchInfo));
		dat->searchCount = 1;
		dat->search[0].hProcess = (HANDLE)CallProtoServiceInt(NULL, szProto, szSearchService, 0, (LPARAM)pvSearchParams);
		dat->search[0].szProto = szProto;
		if (dat->search[0].hProcess == NULL) {
			//infuriatingly vague error message. fixme.
			PROTOACCOUNT *pa = Proto_GetAccount(szProto);
			forkthread(BeginSearchFailed, 0, mir_tstrdup(pa->tszAccountName));
			mir_free(dat->search);
			dat->search = NULL;
			dat->searchCount = 0;
			return 1;
		}
	}
	return 0;
}

void SetStatusBarSearchInfo(HWND hwndStatus, struct FindAddDlgData *dat)
{
	CMString str;

	if (dat->searchCount != 0) {
		str = TranslateT("Searching");
		for (int i = 0; i < dat->searchCount; i++) {
			PROTOACCOUNT *pa = Proto_GetAccount(dat->search[i].szProto);
			if (!pa)
				continue;

			str.Append(i ? _T(", ") : _T(" "));
			str.Append(pa->tszAccountName);
		}
	}
	else str = TranslateT("Idle");

	SendMessage(hwndStatus, SB_SETTEXT, 0, (LPARAM)str.c_str());
}

struct ProtoResultsSummary {
	const char *szProto;
	int count;
};

void SetStatusBarResultInfo(HWND hwndDlg)
{
	HWND hwndResults = GetDlgItem(hwndDlg, IDC_RESULTS);
	CMString str;

	int total = ListView_GetItemCount(hwndResults);
	if (total != 0) {
		LV_ITEM lvi;
		struct ProtoResultsSummary *subtotal = NULL;
		int subtotalCount = 0;
		for (lvi.iItem = total - 1; lvi.iItem >= 0; lvi.iItem--) {
			lvi.mask = LVIF_PARAM;
			ListView_GetItem(hwndResults, &lvi);
			struct ListSearchResult *lsr = (struct ListSearchResult*)lvi.lParam;
			if (lsr == NULL)
				continue;
			
			int i = 0;
			while (i < subtotalCount) {
				if (subtotal[i].szProto == lsr->szProto) {
					subtotal[i].count++;
					break;
				}
				i++;
			}
			if (i == subtotalCount) {
				subtotal = (struct ProtoResultsSummary*)mir_realloc(subtotal, sizeof(struct ProtoResultsSummary)*(subtotalCount + 1));
				subtotal[subtotalCount].szProto = lsr->szProto;
				subtotal[subtotalCount++].count = 1;
			}
		}
		if (subtotalCount == 1) {
			PROTOACCOUNT *pa = Proto_GetAccount(subtotal[0].szProto);
			if (pa == NULL) {
				mir_free(subtotal);
				return;
			}
			else if (total == 1)
				str.AppendFormat(TranslateT("1 %s user found"), pa->tszAccountName);
			else
				str.AppendFormat(TranslateT("%d %s users found"), total, pa->tszAccountName);
		}
		else {
			str.AppendFormat(TranslateT("%d users found ("), total);
			for (int i = 0; i < subtotalCount; i++) {
				PROTOACCOUNT *pa = Proto_GetAccount(subtotal[i].szProto);
				if (pa == NULL)
					continue;

				if (i)
					str.Append(_T(", "));

				str.AppendFormat(_T("%d %s"), subtotal[i].count, pa->tszAccountName);
			}
			str.AppendChar(')');
		}
		mir_free(subtotal);
	}
	else str = TranslateT("No users found");
	
	SendDlgItemMessage(hwndDlg, IDC_STATUSBAR, SB_SETTEXT, 2, (LPARAM)str.c_str());
}

void CreateResultsColumns(HWND hwndResults, struct FindAddDlgData *dat, char *szProto)
{
	SaveColumnSizes(hwndResults);
	while (ListView_DeleteColumn(hwndResults, 0));
	ListView_SetImageList(hwndResults, dat->himlComboIcons, LVSIL_SMALL);
	LoadColumnSizes(hwndResults, szProto);
}

void ShowMoreOptionsMenu(HWND hwndDlg, int x, int y)
{
	struct FindAddDlgData *dat = (struct FindAddDlgData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	LVITEM lvi;
	if (ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_RESULTS)) != 1) return;
	lvi.mask = LVIF_PARAM;
	lvi.iItem = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_RESULTS), -1, LVNI_ALL | LVNI_SELECTED);
	ListView_GetItem(GetDlgItem(hwndDlg, IDC_RESULTS), &lvi);
	struct ListSearchResult *lsr = (struct ListSearchResult*)lvi.lParam;

	HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_CONTEXT));
	HMENU hPopupMenu = GetSubMenu(hMenu, 4);
	TranslateMenu(hPopupMenu);
	int commandId = TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, x, y, 0, hwndDlg, NULL);
	switch (commandId) {
	case IDC_ADD:
		{
			ADDCONTACTSTRUCT acs = { 0 };
			acs.handleType = HANDLE_SEARCHRESULT;
			acs.szProto = lsr->szProto;
			acs.psr = &lsr->psr;
			CallService(MS_ADDCONTACT_SHOW, (WPARAM)hwndDlg, (LPARAM)&acs);
		}
		break;
	case IDC_DETAILS:
		{
			MCONTACT hContact = (MCONTACT)CallProtoServiceInt(NULL, lsr->szProto, PS_ADDTOLIST, PALF_TEMPORARY, (LPARAM)&lsr->psr);
			CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);
		}
		break;
	case IDC_SENDMESSAGE:
		{
			MCONTACT hContact = (MCONTACT)CallProtoServiceInt(NULL, lsr->szProto, PS_ADDTOLIST, PALF_TEMPORARY, (LPARAM)&lsr->psr);
			CallService(MS_MSG_SENDMESSAGE, hContact, 0);
		}
		break;
	}
	DestroyMenu(hPopupMenu);
	DestroyMenu(hMenu);
}
