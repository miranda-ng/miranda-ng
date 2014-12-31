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

#define PF1_ANYSEARCH (PF1_BASICSEARCH | PF1_EXTSEARCHUI | PF1_SEARCHBYEMAIL | PF1_SEARCHBYNAME | PF1_EXTSEARCH)

struct ListSearchResult
{
	const char *szProto;
	PROTOSEARCHRESULT psr;
};

struct ProtoSearchInfo
{
	const char *szProto;
	HANDLE hProcess;
};

struct FindAddDlgData
{
	HANDLE hResultHook;
	int bSortAscending;
	int iLastColumnSortIndex;
	HIMAGELIST himlComboIcons;
	int showProtoId, showEmail, showName, showAdvanced, showTiny;
	int minDlgHeight;
	int notSearchedYet;
	struct ProtoSearchInfo *search;
	int searchCount;
	int throbbing;
	int pivot;
	HWND hwndAdvSearch;
	HWND hwndTinySearch;
	BOOL bFlexSearchResult;
};

int CALLBACK SearchResultsCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
void FreeSearchResults(HWND hwndResults);
int BeginSearch(HWND hwndDlg, struct FindAddDlgData *dat, const char *szProto, const char *szSearchService, DWORD requiredCapability, void *pvSearchParams);
void SetStatusBarSearchInfo(HWND hwndStatus, struct FindAddDlgData *dat);
void SetStatusBarResultInfo(HWND hwndDlg);
void CreateResultsColumns(HWND hwndResults, struct FindAddDlgData *dat, char *szProto);
void EnableResultButtons(HWND hwndDlg, int enable);
void ShowMoreOptionsMenu(HWND hwndDlg, int x, int y);
void SaveColumnSizes(HWND hwndResults);
