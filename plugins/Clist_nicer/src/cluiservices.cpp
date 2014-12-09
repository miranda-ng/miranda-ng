/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#include <commonheaders.h>
#include "../cluiframes/cluiframes.h"

extern HIMAGELIST hCListImages;
extern ButtonItem *g_ButtonItems;
extern PLUGININFOEX pluginInfo;

static INT_PTR GetClistVersion(WPARAM wParam, LPARAM lParam)
{
	static char g_szVersionString[256];

	mir_snprintf(g_szVersionString, SIZEOF(g_szVersionString), "%s, %d.%d.%d.%d", pluginInfo.shortName, HIBYTE(HIWORD(pluginInfo.version)), LOBYTE(HIWORD(pluginInfo.version)), HIBYTE(LOWORD(pluginInfo.version)), LOBYTE(LOBYTE(pluginInfo.version)));
	if (!IsBadWritePtr((LPVOID)lParam, 4))
		*((DWORD *)lParam) = pluginInfo.version;

	return (INT_PTR)g_szVersionString;
}


void FreeProtocolData(void)
{
	// free protocol data
	int nParts = SendMessage(pcli->hwndStatus, SB_GETPARTS, 0, 0);
	for (int nPanel = 0; nPanel < nParts; nPanel++) {
		ProtocolData *PD = (ProtocolData *)SendMessage(pcli->hwndStatus, SB_GETTEXT, nPanel, 0);
		if (PD != NULL && !IsBadCodePtr((FARPROC)PD)) {
			SendMessage(pcli->hwndStatus, SB_SETTEXT, (WPARAM)nPanel | SBT_OWNERDRAW, 0);
			if (PD->RealName) mir_free(PD->RealName);
			if (PD) mir_free(PD);
		}
	}
}

int g_maxStatus = ID_STATUS_OFFLINE;
char g_maxProto[100] = "";

void CluiProtocolStatusChanged(int parStatus, const char* szProto)
{
	int maxOnline = 0, onlineness = 0;
	WORD maxStatus = ID_STATUS_OFFLINE;
	DBVARIANT dbv = { 0 };
	int iIcon = 0;
	HICON hIcon = 0;
	int rdelta = cfg::dat.bCLeft + cfg::dat.bCRight;
	BYTE windowStyle;

	if (pcli->hwndStatus == 0 || cfg::shutDown)
		return;

	int protoCount;
	PROTOACCOUNT **accs;
	ProtoEnumAccounts(&protoCount, &accs);
	if (protoCount == 0)
		return;

	FreeProtocolData();
	g_maxStatus = ID_STATUS_OFFLINE;
	g_maxProto[0] = 0;

	int borders[3];
	SendMessage(pcli->hwndStatus, SB_GETBORDERS, 0, (LPARAM)&borders);

	int *partWidths = (int*)_alloca((protoCount + 1)*sizeof(int));

	int partCount;
	if (cfg::dat.bEqualSections) {
		RECT rc;
		GetClientRect(pcli->hwndStatus, &rc);
		rc.right -= borders[0] * 2;
		int toshow = 0;
		for (int i = 0; i < protoCount; i++)
			if (pcli->pfnGetProtocolVisibility(accs[i]->szModuleName))
				toshow++;

		if (toshow > 0) {
			for (int part = 0, i = 0; i < protoCount; i++) {
				if (!pcli->pfnGetProtocolVisibility(accs[i]->szModuleName))
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
		SIZE textSize;
		BYTE showOpts = cfg::getByte("CLUI", "SBarShow", 1);
		TCHAR szName[32];

		HDC hdc = GetDC(NULL);
		HFONT hofont = reinterpret_cast<HFONT>(SelectObject(hdc, (HFONT)SendMessage(pcli->hwndStatus, WM_GETFONT, 0, 0)));

		// count down since built in ones tend to go at the end
		for (int i = 0, partCount = 0; i < protoCount; i++) {
			int idx = pcli->pfnGetAccountIndexByPos(i);
			if (idx == -1)
				continue;

			PROTOACCOUNT *pa = accs[idx];
			if (!pcli->pfnGetProtocolVisibility(pa->szModuleName))
				continue;

			int x = 2;
			if (showOpts & 1)
				x += 16;
			if (showOpts & 2) {
				mir_tstrncpy(szName, pa->tszAccountName, SIZEOF(szName));
				szName[SIZEOF(szName) - 1] = 0;
				if ((showOpts & 4) && mir_tstrlen(szName) < sizeof(szName) - 1)
					mir_tstrcat(szName, _T(" "));
				GetTextExtentPoint32(hdc, szName, (int)mir_tstrlen(szName), &textSize);
				x += textSize.cx + GetSystemMetrics(SM_CXBORDER) * 4; // The SB panel doesnt allocate enough room
			}
			if (showOpts & 4) {
				TCHAR* modeDescr = pcli->pfnGetStatusModeDescription(CallProtoService(accs[i]->szModuleName, PS_GETSTATUS, 0, 0), 0);
				GetTextExtentPoint32(hdc, modeDescr, (int)mir_tstrlen(modeDescr), &textSize);
				x += textSize.cx + GetSystemMetrics(SM_CXBORDER) * 4; // The SB panel doesnt allocate enough room
			}
			partWidths[partCount] = (partCount ? partWidths[partCount - 1] : cfg::dat.bCLeft) + x + 2;
			partCount++;
		}
		SelectObject(hdc, hofont);
		ReleaseDC(NULL, hdc);
	}
	if (partCount == 0) {
		SendMessage(pcli->hwndStatus, SB_SIMPLE, TRUE, 0);
		return;
	}
	SendMessage(pcli->hwndStatus, SB_SIMPLE, FALSE, 0);

	partWidths[partCount - 1] = -1;
	windowStyle = cfg::getByte("CLUI", "WindowStyle", 0);
	SendMessage(pcli->hwndStatus, SB_SETMINHEIGHT, 18 + cfg::dat.bClipBorder + ((windowStyle == SETTING_WINDOWSTYLE_THINBORDER || windowStyle == SETTING_WINDOWSTYLE_NOBORDER) ? 3 : 0), 0);
	SendMessage(pcli->hwndStatus, SB_SETPARTS, partCount, (LPARAM)partWidths);

	// count down since built in ones tend to go at the end
	char *szMaxProto = NULL;
	for (int i = 0, partCount = 0; i < protoCount; i++) {
		int idx = pcli->pfnGetAccountIndexByPos(i);
		if (idx == -1)
			continue;

		PROTOACCOUNT *pa = accs[idx];
		if (!pcli->pfnGetProtocolVisibility(pa->szModuleName))
			continue;

		int status = CallProtoService(pa->szModuleName, PS_GETSTATUS, 0, 0);
		ProtocolData *PD = (ProtocolData*)mir_alloc(sizeof(ProtocolData));
		PD->RealName = mir_strdup(pa->szModuleName);
		PD->protopos = partCount;
		{
			int flags;
			flags = SBT_OWNERDRAW;
			if (cfg::getByte("CLUI", "SBarBevel", 1) == 0)
				flags |= SBT_NOBORDERS;
			SendMessageA(pcli->hwndStatus, SB_SETTEXTA, partCount | flags, (LPARAM)PD);
		}
		int caps2 = CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0);
		int caps1 = CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0);
		if ((caps1 & PF1_IM) && (caps2 & (PF2_LONGAWAY | PF2_SHORTAWAY))) {
			onlineness = GetStatusOnlineness(status);
			if (onlineness > maxOnline) {
				maxStatus = status;
				maxOnline = onlineness;
				szMaxProto = pa->szModuleName;
			}
		}
		partCount++;
	}
	// update the clui button

	WORD wStatus;
	if (!db_get(NULL, "CList", "PrimaryStatus", &dbv)) {
		if (dbv.type == DBVT_ASCIIZ && mir_strlen(dbv.pszVal) > 1) {
			wStatus = (WORD)CallProtoService(dbv.pszVal, PS_GETSTATUS, 0, 0);
			iIcon = IconFromStatusMode(dbv.pszVal, (int)wStatus, 0, &hIcon);
		}
		mir_free(dbv.pszVal);
	}
	else {
		wStatus = maxStatus;
		iIcon = IconFromStatusMode((wStatus >= ID_STATUS_CONNECTING && wStatus < ID_STATUS_OFFLINE) ? szMaxProto : NULL, (int)wStatus, 0, &hIcon);
		g_maxStatus = (int)wStatus;
		if (szMaxProto)
			strncpy_s(g_maxProto, SIZEOF(g_maxProto), szMaxProto, _TRUNCATE);
	}

	/*
	* this is used globally (actually, by the clist control only) to determine if
	* any protocol is "in connection" state. If true, then the clist discards redraws
	* and uses timer based sort and redraw handling. This can improve performance
	* when connecting multiple protocols significantly.
	*/
	TCHAR *szStatus = pcli->pfnGetStatusModeDescription(wStatus, 0);

	/*
	* set the global status icon and display the global (most online) status mode on the
	* status mode button
	*/
	if (szStatus && pcli->hwndContactList) {
		HWND hwndClistBtn = GetDlgItem(pcli->hwndContactList, IDC_TBGLOBALSTATUS);
		if (IsWindow(hwndClistBtn)) {
			SendMessage(hwndClistBtn, WM_SETTEXT, 0, (LPARAM)szStatus);
			if (!hIcon)
				SendMessage(hwndClistBtn, BUTTONSETIMLICON, (WPARAM)hCListImages, (LPARAM)iIcon);
			else
				SendMessage(hwndClistBtn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
			InvalidateRect(hwndClistBtn, NULL, TRUE);
		}

		HWND hwndTtbStatus = ClcGetButtonWindow(IDC_TBTOPSTATUS);
		if (IsWindow(hwndTtbStatus)) {
			if (g_ButtonItems == NULL) {
				if (!hIcon)
					SendMessage(hwndTtbStatus, BUTTONSETIMLICON, (WPARAM)hCListImages, (LPARAM)iIcon);
				else
					SendMessage(hwndTtbStatus, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
			}
			InvalidateRect(hwndTtbStatus, NULL, TRUE);
		}
	}
}
