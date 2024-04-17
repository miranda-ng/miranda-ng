/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org),
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

#pragma once

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
	int minDlgHeight;
	int notSearchedYet;
	struct ProtoSearchInfo *search;
	int searchCount;
	int throbbing;
	int pivot;
	HWND hwndAdvSearch;
	HWND hwndTinySearch;
	bool bFlexSearchResult, bShowProtoId, bShowEmail, bShowName, bShowAdvanced, bShowTiny;

	uint32_t proto2show(const char *szProto)
	{
		uint32_t protoCaps = (uint32_t)CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1);
		if (protoCaps & PF1_BASICSEARCH)   bShowProtoId = 1;
		if (protoCaps & PF1_SEARCHBYEMAIL) bShowEmail = 1;
		if (protoCaps & PF1_SEARCHBYNAME)  bShowName = 1;

		if (protoCaps & PF1_EXTSEARCHUI)    bShowAdvanced = 1;
		else if (protoCaps & PF1_EXTSEARCH) bShowTiny = 1;

		if (protoCaps & PF1_USERIDISEMAIL && bShowProtoId) {
			bShowProtoId = 0;
			bShowEmail = 1;
		}
		return protoCaps;
	}
};

int CALLBACK SearchResultsCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
void FreeSearchResults(HWND hwndResults);
int BeginSearch(HWND hwndDlg, struct FindAddDlgData *dat, const char *szProto, const char *szSearchService, uint32_t requiredCapability, void *pvSearchParams);
void SetStatusBarSearchInfo(HWND hwndStatus, struct FindAddDlgData *dat);
void SetStatusBarResultInfo(HWND hwndDlg);
void CreateResultsColumns(HWND hwndResults, struct FindAddDlgData *dat, char *szProto);
void EnableResultButtons(HWND hwndDlg, int enable);
void ShowMoreOptionsMenu(HWND hwndDlg, int x, int y);
void SaveColumnSizes(HWND hwndResults);
