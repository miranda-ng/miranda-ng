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
	int columnCount;
	char szSetting[32];
	int i;
	struct FindAddDlgData *dat;

	dat = (struct FindAddDlgData*)GetWindowLongPtr(GetParent(hwndResults), GWLP_USERDATA);
	columnCount = Header_GetItemCount(ListView_GetHeader(hwndResults));
	if (columnCount != NUM_COLUMNID) return;
	ListView_GetColumnOrderArray(hwndResults, columnCount, columnOrder);
	for (i=0; i < NUM_COLUMNID; i++) {
		mir_snprintf(szSetting, SIZEOF(szSetting), "ColOrder%d", i);
		db_set_b(NULL, "FindAdd", szSetting, (BYTE)columnOrder[i]);
		if (i>=columnCount) continue;
		mir_snprintf(szSetting, SIZEOF(szSetting), "ColWidth%d", i);
		db_set_w(NULL, "FindAdd", szSetting, (WORD)ListView_GetColumnWidth(hwndResults, i));
	}
	db_set_b(NULL, "FindAdd", "SortColumn", (BYTE)dat->iLastColumnSortIndex);
	db_set_b(NULL, "FindAdd", "SortAscending", (BYTE)dat->bSortAscending);
}

static const TCHAR *szColumnNames[] = { NULL, NULL, _T("Nick"), _T("First Name"), _T("Last Name"), _T("E-mail") };
static int defaultColumnSizes[] = {0, 90, 100, 100, 100, 2000};
void LoadColumnSizes(HWND hwndResults, const char *szProto)
{
	HDITEM hdi;
	int columnOrder[NUM_COLUMNID];
	int columnCount;
	char szSetting[32];
	int i;
	FindAddDlgData *dat;
	bool colOrdersValid;

	defaultColumnSizes[COLUMNID_PROTO] = GetSystemMetrics(SM_CXSMICON) + 4;
	dat = (FindAddDlgData*)GetWindowLongPtr(GetParent(hwndResults), GWLP_USERDATA);

	columnCount = NUM_COLUMNID;
	colOrdersValid = true;
	for (i=0; i < NUM_COLUMNID; i++)
	{
		LVCOLUMN lvc;
		if (i < columnCount)
		{
			int bNeedsFree = FALSE;
			lvc.mask = LVCF_TEXT | LVCF_WIDTH;
			if (szColumnNames[i] != NULL)
				lvc.pszText = TranslateTS(szColumnNames[i]);
			else if (i == COLUMNID_HANDLE)
			{
				if (szProto)
				{
					bNeedsFree = TRUE;
					lvc.pszText = mir_a2t((char*)CallProtoServiceInt(NULL,szProto, PS_GETCAPS, PFLAG_UNIQUEIDTEXT, 0));
				}
				else
					lvc.pszText = _T("ID");
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
		if (columnOrder[i] == -1 || columnOrder[i] >= NUM_COLUMNID) colOrdersValid = false;
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
	struct FindAddDlgData *dat = (struct FindAddDlgData*)GetWindowLongPtr((HWND) lParamSort, GWLP_USERDATA);
	int sortMultiplier;
	int sortCol;
	struct ListSearchResult *lsr1, *lsr2;
	HWND hList = GetDlgItem((HWND) lParamSort, IDC_RESULTS);

	sortMultiplier = dat->bSortAscending?1:-1;
	sortCol = dat->iLastColumnSortIndex;
	if (!dat->bFlexSearchResult)
	{
		lsr1 = (struct ListSearchResult*)ListView_GetItemLParam(hList, (int)lParam1);
		lsr2 = (struct ListSearchResult*)ListView_GetItemLParam(hList, (int)lParam2);

		if (lsr1 == NULL || lsr2 == NULL) return 0;
		switch(sortCol)
		{
		case COLUMNID_PROTO:
			return mir_strcmp(lsr1->szProto, lsr2->szProto)*sortMultiplier;
		case COLUMNID_HANDLE:
			return mir_tstrcmpi(lsr1->psr.id, lsr2->psr.id)*sortMultiplier;
		case COLUMNID_NICK:
			return mir_tstrcmpi(lsr1->psr.nick, lsr2->psr.nick)*sortMultiplier;
		case COLUMNID_FIRST:
			return mir_tstrcmpi(lsr1->psr.firstName, lsr2->psr.firstName)*sortMultiplier;
		case COLUMNID_LAST:
			return mir_tstrcmpi(lsr1->psr.lastName, lsr2->psr.lastName)*sortMultiplier;
		case COLUMNID_EMAIL:
			return mir_tstrcmpi(lsr1->psr.email, lsr2->psr.email)*sortMultiplier;
		}
	}
	else
	{
		TCHAR szText1[100];
		TCHAR szText2[100];
		ListView_GetItemText(hList, (int)lParam1, sortCol, szText1, SIZEOF(szText1));
		ListView_GetItemText(hList, (int)lParam2, sortCol, szText2, SIZEOF(szText2));
		return _tcsicmp(szText1, szText2)*sortMultiplier;
	}
	return 0;
}

void FreeSearchResults(HWND hwndResults)
{
	LV_ITEM lvi;
	struct ListSearchResult *lsr;
	for (lvi.iItem = ListView_GetItemCount(hwndResults)-1;lvi.iItem>=0;lvi.iItem--) {
		lvi.mask = LVIF_PARAM;
		ListView_GetItem(hwndResults, &lvi);
		lsr = (struct ListSearchResult*)lvi.lParam;
		if (lsr == NULL) continue;
		mir_free(lsr->psr.id);
		mir_free(lsr->psr.email);
		mir_free(lsr->psr.nick);
		mir_free(lsr->psr.firstName);
		mir_free(lsr->psr.lastName);
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
		for (int i=0; i < accounts.getCount();i++) {
			PROTOACCOUNT *pa = accounts[i];
			if (!Proto_IsAccountEnabled(pa)) continue;
			DWORD caps = (DWORD)CallProtoServiceInt(NULL,pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0);
			if (!(caps&requiredCapability)) continue;
			dat->search[dat->searchCount].hProcess = (HANDLE)CallProtoServiceInt(NULL,pa->szModuleName, szSearchService, 0, (LPARAM)pvSearchParams);
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
		dat->search[0].hProcess = (HANDLE)CallProtoServiceInt(NULL,szProto, szSearchService, 0, (LPARAM)pvSearchParams);
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

// !!!!!!!! this code is dangerous like a hell
void SetStatusBarSearchInfo(HWND hwndStatus, struct FindAddDlgData *dat)
{
	TCHAR str[256];

	if (dat->searchCount != 0) {
		int i;

		mir_tstrcpy(str, TranslateT("Searching"));
		for (i=0; i < dat->searchCount; i++) {
			PROTOACCOUNT *pa = Proto_GetAccount(dat->search[i].szProto);
			if (!pa)
				continue;

			mir_tstrcat(str, i ? _T(", ") : _T(" "));
			mir_tstrcat(str, pa->tszAccountName);
		}
	}
	else mir_tstrcpy(str, TranslateT("Idle"));

	SendMessage(hwndStatus, SB_SETTEXT, 0, (LPARAM)str);
}

struct ProtoResultsSummary {
	const char *szProto;
	int count;
};
void SetStatusBarResultInfo(HWND hwndDlg)
{
	HWND hwndStatus = GetDlgItem(hwndDlg, IDC_STATUSBAR);
	HWND hwndResults = GetDlgItem(hwndDlg, IDC_RESULTS);
	LV_ITEM lvi;
	struct ListSearchResult *lsr;
	struct ProtoResultsSummary *subtotal = NULL;
	int subtotalCount = 0;
	int i, total;
	TCHAR str[256];

	total = ListView_GetItemCount(hwndResults);
	for (lvi.iItem = total-1;lvi.iItem>=0;lvi.iItem--) {
		lvi.mask = LVIF_PARAM;
		ListView_GetItem(hwndResults, &lvi);
		lsr = (struct ListSearchResult*)lvi.lParam;
		if (lsr == NULL) continue;
		for (i=0;i<subtotalCount;i++) {
			if (subtotal[i].szProto == lsr->szProto) {
				subtotal[i].count++;
				break;
			}
		}
		if (i == subtotalCount) {
			subtotal = (struct ProtoResultsSummary*)mir_realloc(subtotal, sizeof(struct ProtoResultsSummary)*(subtotalCount+1));
			subtotal[subtotalCount].szProto = lsr->szProto;
			subtotal[subtotalCount++].count = 1;
		}
	}
	if (total != 0) {
		TCHAR substr[64];
		PROTOACCOUNT *pa = Proto_GetAccount(subtotal[0].szProto);
		if (pa == NULL)
			return;

		if (subtotalCount == 1) {
			if (total == 1) mir_sntprintf(str, SIZEOF(str), TranslateT("1 %s user found"), pa->tszAccountName);
			else mir_sntprintf(str, SIZEOF(str), TranslateT("%d %s users found"), total, pa->tszAccountName);
		}
		else {
			mir_sntprintf(str, SIZEOF(str), TranslateT("%d users found ("), total);
			for (i=0; i < subtotalCount; i++) {
				if (i) {
					if ((pa = Proto_GetAccount(subtotal[i].szProto)) == NULL)
						return;
					mir_tstrcat(str, _T(", "));
				}
				mir_sntprintf(substr, SIZEOF(substr), _T("%d %s"), subtotal[i].count, pa->tszAccountName);
				mir_tstrcat(str, substr);
			}
			mir_tstrcat(str, _T(")"));
		}
		mir_free(subtotal);
	}
	else mir_tstrcpy(str, TranslateT("No users found"));
	SendMessage(hwndStatus, SB_SETTEXT, 2, (LPARAM)str);
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
	struct FindAddDlgData *dat;
	HMENU hPopupMenu, hMenu;
	int commandId;
	struct ListSearchResult *lsr;

	dat = (struct FindAddDlgData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	{
		LVITEM lvi;
		if (ListView_GetSelectedCount( GetDlgItem(hwndDlg, IDC_RESULTS)) != 1) return;
		lvi.mask = LVIF_PARAM;
		lvi.iItem = ListView_GetNextItem( GetDlgItem(hwndDlg, IDC_RESULTS), -1, LVNI_ALL|LVNI_SELECTED);
		ListView_GetItem( GetDlgItem(hwndDlg, IDC_RESULTS), &lvi);
		lsr = (struct ListSearchResult*)lvi.lParam;
	}

	hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_CONTEXT));
	hPopupMenu = GetSubMenu(hMenu, 4);
	TranslateMenu(hPopupMenu);
	commandId = TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON|TPM_RETURNCMD, x, y, 0, hwndDlg, NULL);
	switch(commandId) {
		case IDC_ADD:
		{
			ADDCONTACTSTRUCT acs = { 0 };
			acs.handleType = HANDLE_SEARCHRESULT;
			acs.szProto = lsr->szProto;
			acs.psr = &lsr->psr;
			CallService(MS_ADDCONTACT_SHOW, (WPARAM)hwndDlg, (LPARAM)&acs);
			break;
		}
		case IDC_DETAILS:
		{
			MCONTACT hContact = (MCONTACT)CallProtoServiceInt(NULL, lsr->szProto, PS_ADDTOLIST, PALF_TEMPORARY, (LPARAM)&lsr->psr);
			CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);
			break;
		}
		case IDC_SENDMESSAGE:
		{
			MCONTACT hContact = (MCONTACT)CallProtoServiceInt(NULL, lsr->szProto, PS_ADDTOLIST, PALF_TEMPORARY, (LPARAM)&lsr->psr);
			CallService(MS_MSG_SENDMESSAGE, hContact, (LPARAM)(const char*)NULL);
			break;
		}
	}
	DestroyMenu(hPopupMenu);
	DestroyMenu(hMenu);
}
