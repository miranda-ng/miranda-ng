/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

#include "stdafx.h"
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
		mir_snprintf(szSetting, "ColOrder%d", i);
		db_set_b(0, "FindAdd", szSetting, (uint8_t)columnOrder[i]);
		mir_snprintf(szSetting, "ColWidth%d", i);
		db_set_w(0, "FindAdd", szSetting, (uint16_t)ListView_GetColumnWidth(hwndResults, i));
	}
	db_set_b(0, "FindAdd", "SortColumn", (uint8_t)dat->iLastColumnSortIndex);
	db_set_b(0, "FindAdd", "SortAscending", (uint8_t)dat->bSortAscending);
}

static const wchar_t *szColumnNames[] = { nullptr, nullptr, L"Nick", L"First Name", L"Last Name", L"E-mail" };
static int defaultColumnSizes[] = { 0, 90, 100, 100, 100, 2000 };
void LoadColumnSizes(HWND hwndResults, const char *szProto)
{
	defaultColumnSizes[COLUMNID_PROTO] = g_iIconSX + 4;
	FindAddDlgData *dat = (FindAddDlgData*)GetWindowLongPtr(GetParent(hwndResults), GWLP_USERDATA);

	LVCOLUMN lvc;

	int columnOrder[NUM_COLUMNID];
	bool colOrdersValid = true;
	for (int i = 0; i < NUM_COLUMNID; i++) {
		lvc.mask = LVCF_TEXT | LVCF_WIDTH;
		if (szColumnNames[i] != nullptr)
			lvc.pszText = TranslateW(szColumnNames[i]);
		else {
			if (i == COLUMNID_HANDLE) {
				lvc.pszText = L"ID";
				if (szProto) {
					INT_PTR ret = CallProtoServiceInt(0, szProto, PS_GETCAPS, PFLAG_UNIQUEIDTEXT, 0);
					if (ret != CALLSERVICE_NOTFOUND)
						lvc.pszText = (wchar_t*)ret;
				}
			}
			else lvc.mask &= ~LVCF_TEXT;
		}
			
		char szSetting[32];
		mir_snprintf(szSetting, "ColWidth%d", i);
		lvc.cx = db_get_w(0, "FindAdd", szSetting, defaultColumnSizes[i]);
		ListView_InsertColumn(hwndResults, i, (LPARAM)&lvc);

		mir_snprintf(szSetting, "ColOrder%d", i);
		columnOrder[i] = db_get_b(0, "FindAdd", szSetting, -1);
		if (columnOrder[i] == -1 || columnOrder[i] >= NUM_COLUMNID)
			colOrdersValid = false;
	}

	if (colOrdersValid)
		ListView_SetColumnOrderArray(hwndResults, NUM_COLUMNID, columnOrder);

	dat->iLastColumnSortIndex = db_get_b(0, "FindAdd", "SortColumn", COLUMNID_NICK);
	if (dat->iLastColumnSortIndex >= NUM_COLUMNID)
		dat->iLastColumnSortIndex = COLUMNID_NICK;
	dat->bSortAscending = db_get_b(0, "FindAdd", "SortAscending", TRUE);

	HDITEM hdi;
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
	FindAddDlgData *dat = (FindAddDlgData*)GetWindowLongPtr((HWND)lParamSort, GWLP_USERDATA);
	HWND hList = GetDlgItem((HWND)lParamSort, IDC_RESULTS);

	int sortMultiplier = dat->bSortAscending ? 1 : -1;
	int sortCol = dat->iLastColumnSortIndex;
	if (!dat->bFlexSearchResult) {
		ListSearchResult *lsr1 = (ListSearchResult*)ListView_GetItemLParam(hList, (int)lParam1);
		ListSearchResult *lsr2 = (ListSearchResult*)ListView_GetItemLParam(hList, (int)lParam2);
		if (lsr1 == nullptr || lsr2 == nullptr)
			return 0;
		
		switch (sortCol) {
		case COLUMNID_PROTO:
			return mir_strcmp(lsr1->szProto, lsr2->szProto)*sortMultiplier;
		case COLUMNID_HANDLE:
			return mir_wstrcmpi(lsr1->psr.id.w, lsr2->psr.id.w)*sortMultiplier;
		case COLUMNID_NICK:
			return mir_wstrcmpi(lsr1->psr.nick.w, lsr2->psr.nick.w)*sortMultiplier;
		case COLUMNID_FIRST:
			return mir_wstrcmpi(lsr1->psr.firstName.w, lsr2->psr.firstName.w)*sortMultiplier;
		case COLUMNID_LAST:
			return mir_wstrcmpi(lsr1->psr.lastName.w, lsr2->psr.lastName.w)*sortMultiplier;
		case COLUMNID_EMAIL:
			return mir_wstrcmpi(lsr1->psr.email.w, lsr2->psr.email.w)*sortMultiplier;
		}
	}
	else {
		wchar_t szText1[100], szText2[100];
		ListView_GetItemText(hList, (int)lParam1, sortCol, szText1, _countof(szText1));
		ListView_GetItemText(hList, (int)lParam2, sortCol, szText2, _countof(szText2));
		return mir_wstrcmpi(szText1, szText2)*sortMultiplier;
	}
	return 0;
}

void FreeSearchResults(HWND hwndResults)
{
	LV_ITEM lvi;
	for (lvi.iItem = ListView_GetItemCount(hwndResults) - 1; lvi.iItem >= 0; lvi.iItem--) {
		lvi.mask = LVIF_PARAM;
		ListView_GetItem(hwndResults, &lvi);
		ListSearchResult *lsr = (ListSearchResult*)lvi.lParam;
		if (lsr == nullptr) continue;
		mir_free(lsr->psr.id.w);
		mir_free(lsr->psr.email.w);
		mir_free(lsr->psr.nick.w);
		mir_free(lsr->psr.firstName.w);
		mir_free(lsr->psr.lastName.w);
		mir_free(lsr);
	}
	ListView_DeleteAllItems(hwndResults);
	EnableResultButtons(GetParent(hwndResults), 0);
}

// on its own thread
static void __cdecl BeginSearchFailed(wchar_t *protoName)
{
	CMStringW buf;
	if (protoName != nullptr) {
		buf.Format(
			TranslateT("Could not start a search on '%s', there was a problem - is %s connected?"),
			protoName, protoName);
		mir_free(protoName);
	}
	else buf = TranslateT("Could not search on any of the protocols, are you online?");
	MessageBoxW(nullptr, buf, TranslateT("Problem with search"), MB_OK | MB_ICONERROR);
}

int BeginSearch(HWND, struct FindAddDlgData *dat, const char *szProto, const char *szSearchService, uint32_t requiredCapability, void *pvSearchParams)
{
	if (szProto == nullptr) {
		int failures = 0;
		dat->searchCount = 0;
		dat->search = (struct ProtoSearchInfo*)mir_calloc(sizeof(struct ProtoSearchInfo) * g_arAccounts.getCount());
		for (auto &pa : g_arAccounts) {
			if (!pa->IsEnabled())
				continue;
			
			uint32_t caps = (uint32_t)CallProtoServiceInt(0, pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0);
			if (!(caps & requiredCapability))
				continue;
			
			dat->search[dat->searchCount].hProcess = (HANDLE)CallProtoServiceInt(0, pa->szModuleName, szSearchService, 0, (LPARAM)pvSearchParams);
			dat->search[dat->searchCount].szProto = pa->szModuleName;
			if (dat->search[dat->searchCount].hProcess == nullptr) failures++;
			else dat->searchCount++;
		}
		if (failures) {
			// infuriatingly vague error message. fixme.
			if (dat->searchCount == 0) {
				mir_forkThread<wchar_t>(BeginSearchFailed, nullptr);
				mir_free(dat->search);
				dat->search = nullptr;
				return 1;
			}
		}
	}
	else {
		dat->search = (struct ProtoSearchInfo*)mir_alloc(sizeof(struct ProtoSearchInfo));
		dat->searchCount = 1;
		dat->search[0].hProcess = (HANDLE)CallProtoServiceInt(0, szProto, szSearchService, 0, (LPARAM)pvSearchParams);
		dat->search[0].szProto = szProto;
		if (dat->search[0].hProcess == nullptr) {
			// infuriatingly vague error message. fixme.
			PROTOACCOUNT *pa = Proto_GetAccount(szProto);
			mir_forkThread<wchar_t>(BeginSearchFailed, mir_wstrdup(pa->tszAccountName));
			mir_free(dat->search);
			dat->search = nullptr;
			dat->searchCount = 0;
			return 1;
		}
	}
	return 0;
}

void SetStatusBarSearchInfo(HWND hwndStatus, struct FindAddDlgData *dat)
{
	CMStringW str;

	if (dat->searchCount != 0) {
		str = TranslateT("Searching");
		for (int i = 0; i < dat->searchCount; i++) {
			PROTOACCOUNT *pa = Proto_GetAccount(dat->search[i].szProto);
			if (!pa)
				continue;

			str.Append(i ? L", " : L" ");
			str.Append(pa->tszAccountName);
		}
	}
	else str = TranslateT("Idle");

	SendMessage(hwndStatus, SB_SETTEXT, 0, (LPARAM)str.c_str());
}

struct ProtoResultsSummary
{
	const char *szProto;
	int count;
};

void SetStatusBarResultInfo(HWND hwndDlg)
{
	HWND hwndResults = GetDlgItem(hwndDlg, IDC_RESULTS);
	CMStringW str;

	int total = ListView_GetItemCount(hwndResults);
	if (total != 0) {
		LV_ITEM lvi;
		struct ProtoResultsSummary *subtotal = nullptr;
		int subtotalCount = 0;
		for (lvi.iItem = total - 1; lvi.iItem >= 0; lvi.iItem--) {
			lvi.mask = LVIF_PARAM;
			ListView_GetItem(hwndResults, &lvi);
			ListSearchResult *lsr = (ListSearchResult*)lvi.lParam;
			if (lsr == nullptr)
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
			if (pa == nullptr) {
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
				if (pa == nullptr)
					continue;

				if (i)
					str.Append(L", ");

				str.AppendFormat(L"%d %s", subtotal[i].count, pa->tszAccountName);
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
	LVITEM lvi;
	if (ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_RESULTS)) != 1) return;
	lvi.mask = LVIF_PARAM;
	lvi.iItem = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_RESULTS), -1, LVNI_ALL | LVNI_SELECTED);
	ListView_GetItem(GetDlgItem(hwndDlg, IDC_RESULTS), &lvi);
	ListSearchResult *lsr = (ListSearchResult*)lvi.lParam;

	HMENU hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_CONTEXT));
	HMENU hPopupMenu = GetSubMenu(hMenu, 4);
	TranslateMenu(hPopupMenu);
	int commandId = TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, x, y, 0, hwndDlg, nullptr);
	switch (commandId) {
	case IDC_ADD:
		Contact::AddBySearch(lsr->szProto, &lsr->psr, hwndDlg);
		break;

	case IDC_DETAILS:
		{
			MCONTACT hContact = (MCONTACT)CallProtoServiceInt(0, lsr->szProto, PS_ADDTOLIST, PALF_TEMPORARY, (LPARAM)&lsr->psr);
			CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);
		}
		break;
	case IDM_SENDMESSAGE:
		{
			MCONTACT hContact = (MCONTACT)CallProtoServiceInt(0, lsr->szProto, PS_ADDTOLIST, PALF_TEMPORARY, (LPARAM)&lsr->psr);
			CallService(MS_MSG_SENDMESSAGE, hContact, 0);
		}
		break;
	}
	DestroyMenu(hPopupMenu);
	DestroyMenu(hMenu);
}
