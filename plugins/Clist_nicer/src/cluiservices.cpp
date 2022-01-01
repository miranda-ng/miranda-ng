/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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
#include "cluiframes.h"

extern ButtonItem *g_ButtonItems;

void FreeProtocolData(void)
{
	// free protocol data
	int nParts = SendMessage(g_clistApi.hwndStatus, SB_GETPARTS, 0, 0);
	for (int nPanel = 0; nPanel < nParts; nPanel++) {
		ProtocolData *PD = (ProtocolData *)SendMessage(g_clistApi.hwndStatus, SB_GETTEXT, nPanel, 0);
		if (PD != nullptr && !IsBadCodePtr((FARPROC)PD)) {
			SendMessage(g_clistApi.hwndStatus, SB_SETTEXT, (WPARAM)nPanel | SBT_OWNERDRAW, 0);
			if (PD->RealName) mir_free(PD->RealName);
			if (PD) mir_free(PD);
		}
	}
}

int g_maxStatus = ID_STATUS_OFFLINE;

void CluiProtocolStatusChanged(int, const char*)
{
	if (g_clistApi.hwndStatus == nullptr || cfg::shutDown)
		return;

	auto &accs = Accounts();
	if (accs.getCount() == 0)
		return;

	FreeProtocolData();
	g_maxStatus = ID_STATUS_OFFLINE;

	int borders[3];
	SendMessage(g_clistApi.hwndStatus, SB_GETBORDERS, 0, (LPARAM)&borders);

	int *partWidths = (int*)_alloca((accs.getCount() + 1)*sizeof(int));

	int rdelta = cfg::dat.bCLeft + cfg::dat.bCRight;
	int partCount;
	if (cfg::dat.bEqualSections) {
		RECT rc;
		GetClientRect(g_clistApi.hwndStatus, &rc);
		rc.right -= borders[0] * 2;
		int toshow = 0;
		for (auto &pa : accs)
			if (pa->IsVisible())
				toshow++;

		if (toshow > 0) {
			int part = 0;
			for (auto &pa : accs) {
				if (!pa->IsVisible())
					continue;

				partWidths[part] = ((rc.right - rc.left - rdelta) / toshow)*(part + 1) + cfg::dat.bCLeft;
				if (part == toshow - 1)
					partWidths[part] += cfg::dat.bCRight;
				part++;
			}
		}

		partCount = toshow;
	}
	else {
		uint8_t showOpts = db_get_b(0, "CLUI", "SBarShow", 1);
		wchar_t szName[32];

		HDC hdc = GetDC(nullptr);
		HFONT hofont = reinterpret_cast<HFONT>(SelectObject(hdc, (HFONT)SendMessage(g_clistApi.hwndStatus, WM_GETFONT, 0, 0)));

		// count down since built in ones tend to go at the end
		partCount = 0;
		for (int i = 0; i < accs.getCount(); i++) {
			int idx = Clist_GetAccountIndex(i);
			if (idx == -1)
				continue;

			PROTOACCOUNT *pa = accs[idx];
			if (!pa->IsVisible())
				continue;

			SIZE textSize;
			int x = 2;
			if (showOpts & 1)
				x += 16;
			if (showOpts & 2) {
				mir_wstrncpy(szName, pa->tszAccountName, _countof(szName));
				szName[_countof(szName) - 1] = 0;
				if ((showOpts & 4) && mir_wstrlen(szName) < sizeof(szName) - 1)
					mir_wstrcat(szName, L" ");
				GetTextExtentPoint32(hdc, szName, (int)mir_wstrlen(szName), &textSize);
				x += textSize.cx + GetSystemMetrics(SM_CXBORDER) * 4; // The SB panel doesnt allocate enough room
			}
			if (showOpts & 4) {
				wchar_t* modeDescr = Clist_GetStatusModeDescription(accs[i]->iRealStatus, 0);
				GetTextExtentPoint32(hdc, modeDescr, (int)mir_wstrlen(modeDescr), &textSize);
				x += textSize.cx + GetSystemMetrics(SM_CXBORDER) * 4; // The SB panel doesnt allocate enough room
			}
			partWidths[partCount] = (partCount ? partWidths[partCount - 1] : cfg::dat.bCLeft) + x + 2;
			partCount++;
		}
		SelectObject(hdc, hofont);
		ReleaseDC(nullptr, hdc);
	}
	if (partCount == 0) {
		SendMessage(g_clistApi.hwndStatus, SB_SIMPLE, TRUE, 0);
		return;
	}
	SendMessage(g_clistApi.hwndStatus, SB_SIMPLE, FALSE, 0);

	partWidths[partCount - 1] = -1;
	uint8_t windowStyle = db_get_b(0, "CLUI", "WindowStyle", 0);
	SendMessage(g_clistApi.hwndStatus, SB_SETMINHEIGHT, 18 + cfg::dat.bClipBorder + ((windowStyle == SETTING_WINDOWSTYLE_THINBORDER || windowStyle == SETTING_WINDOWSTYLE_NOBORDER) ? 3 : 0), 0);
	SendMessage(g_clistApi.hwndStatus, SB_SETPARTS, partCount, (LPARAM)partWidths);

	// count down since built in ones tend to go at the end
	partCount = 0;
	for (int i = 0; i < accs.getCount(); i++) {
		int idx = Clist_GetAccountIndex(i);
		if (idx == -1)
			continue;

		PROTOACCOUNT *pa = accs[idx];
		if (!pa->IsVisible())
			continue;

		ProtocolData *PD = (ProtocolData*)mir_alloc(sizeof(ProtocolData));
		PD->RealName = mir_strdup(pa->szModuleName);
		PD->protopos = partCount;

		int flags = SBT_OWNERDRAW;
		if (db_get_b(0, "CLUI", "SBarBevel", 1) == 0)
			flags |= SBT_NOBORDERS;
		SendMessageA(g_clistApi.hwndStatus, SB_SETTEXTA, partCount | flags, (LPARAM)PD);
		
		partCount++;
	}
	
	// update the clui button
	int iIcon = 0;
	int wStatus = 0;
	ptrA szPrimaryStatus(g_plugin.getStringA("PrimaryStatus"));
	if (szPrimaryStatus != nullptr) {
		wStatus = Proto_GetStatus(szPrimaryStatus);
		iIcon = IconFromStatusMode(szPrimaryStatus, (int)wStatus, 0);
	}
	else {
		char *szMaxProto = nullptr;
		wStatus = Clist_GetGeneralizedStatus(&szMaxProto);
		iIcon = IconFromStatusMode(szMaxProto, wStatus, 0);
		g_maxStatus = wStatus;
	}

	// this is used globally (actually, by the clist control only) to determine if
	// any protocol is "in connection" state. If true, then the clist discards redraws
	// and uses timer based sort and redraw handling. This can improve performance
	// when connecting multiple protocols significantly.

	wchar_t *szStatus = Clist_GetStatusModeDescription(wStatus, 0);

	// set the global status icon and display the global (most online) status mode on the
	// status mode button
	if (szStatus && g_clistApi.hwndContactList) {
		HWND hwndClistBtn = GetDlgItem(g_clistApi.hwndContactList, IDC_TBGLOBALSTATUS);
		if (IsWindow(hwndClistBtn)) {
			SetWindowText(hwndClistBtn, szStatus);
			SendMessage(hwndClistBtn, BUTTONSETIMLICON, (WPARAM)hCListImages, (LPARAM)iIcon);
			InvalidateRect(hwndClistBtn, nullptr, TRUE);
		}

		HWND hwndTtbStatus = ClcGetButtonWindow(IDC_TBTOPSTATUS);
		if (IsWindow(hwndTtbStatus)) {
			if (g_ButtonItems == nullptr)
				SendMessage(hwndTtbStatus, BUTTONSETIMLICON, (WPARAM)hCListImages, (LPARAM)iIcon);

			InvalidateRect(hwndTtbStatus, nullptr, TRUE);
		}
	}
}
