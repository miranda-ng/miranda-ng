/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-04 Miranda ICQ/IM project,
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

extern OBJLIST<protoPicCacheEntry> g_ProtoPictures;
extern HANDLE hEventChanged;

static BOOL dialoginit = TRUE;

static void RemoveProtoPic(protoPicCacheEntry *pce)
{
	if (pce == nullptr)
		return;

	db_unset(0, PPICT_MODULE, pce->szProtoname);

	// common for all accounts
	if (pce->cacheType == PCE_TYPE_GLOBAL) {
		for (auto &p : g_ProtoPictures) {
			if (p->szProtoname == nullptr)
				continue;

			p->clear();
			CreateAvatarInCache(0, p, p->szProtoname);
			NotifyEventHooks(hEventChanged, 0, (LPARAM)p);
		}
		return;
	}

	// common for all accounts of this proto
	if (pce->cacheType == PCE_TYPE_PROTO) {
		for (auto &p : g_ProtoPictures) {
			if (p->szProtoname == nullptr)
				continue;

			PROTOACCOUNT *pdescr = Proto_GetAccount(p->szProtoname);
			if (pdescr == nullptr && mir_strcmp(p->szProtoname, pce->szProtoname))
				continue;

			if (!mir_strcmp(p->szProtoname, pce->szProtoname) || !mir_strcmp(pdescr->szProtoName, pce->pd->szName)) {
				p->clear();
				CreateAvatarInCache(0, p, p->szProtoname);
				NotifyEventHooks(hEventChanged, 0, (LPARAM)p);
			}
		}
		return;
	}

	for (auto &p : g_ProtoPictures) {
		if (!mir_strcmp(p->szProtoname, pce->szProtoname)) {
			p->clear();
			NotifyEventHooks(hEventChanged, 0, (LPARAM)p);
		}
	}
}

static void SetProtoPic(protoPicCacheEntry *pce)
{
	wchar_t FileName[MAX_PATH], filter[256];
	Bitmap_GetFilter(filter, _countof(filter));

	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.lpstrFilter = filter;
	ofn.hwndOwner = nullptr;
	ofn.lpstrFile = FileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.Flags = OFN_HIDEREADONLY;
	ofn.lpstrInitialDir = L".";
	*FileName = '\0';
	ofn.lpstrDefExt = L"";
	if (!GetOpenFileName(&ofn))
		return;

	if (_waccess(FileName, 4) == -1)
		return;

	wchar_t szNewPath[MAX_PATH];
	PathToRelativeW(FileName, szNewPath, g_szDataPath);
	db_set_ws(0, PPICT_MODULE, pce->szProtoname, szNewPath);

	switch(pce->cacheType) {
	case PCE_TYPE_GLOBAL:
		for (auto &p : g_ProtoPictures) {
			if (mir_strlen(p->szProtoname) == 0)
				continue;

			if (p->hbmPic == nullptr || !mir_strcmp(p->szProtoname, AVS_DEFAULT)) {
				CreateAvatarInCache(0, p, pce->szProtoname);
				NotifyEventHooks(hEventChanged, 0, (LPARAM)p);
			}
		}
		break;
	
	case PCE_TYPE_PROTO:
		for (auto &p : g_ProtoPictures) {
			PROTOACCOUNT* pdescr = Proto_GetAccount(p->szProtoname);
			if (pdescr == nullptr && mir_strcmp(p->szProtoname, pce->szProtoname))
				continue;

			if (!mir_strcmp(p->szProtoname, pce->szProtoname) || !mir_strcmp(pdescr->szProtoName, pce->pd->szName)) {
				if (mir_strlen(p->szProtoname) != 0) {
					if (p->hbmPic == nullptr) {
						CreateAvatarInCache(0, p, pce->szProtoname);
						NotifyEventHooks(hEventChanged, 0, (LPARAM)p);
					}
				}
			}
		}
		break;

	default:
		for (auto &p : g_ProtoPictures) {
			if (mir_strlen(p->szProtoname) == 0)
				break;

			if (!mir_strcmp(p->szProtoname, pce->szProtoname) && mir_strlen(p->szProtoname) == mir_strlen(pce->szProtoname)) {
				if (p->hbmPic != nullptr)
					DeleteObject(p->hbmPic);
				memset(p, 0, sizeof(AVATARCACHEENTRY));
				CreateAvatarInCache(0, p, pce->szProtoname);
				NotifyEventHooks(hEventChanged, 0, (LPARAM)p);
				break;
			}
		}
	}
}

static protoPicCacheEntry *g_selectedProto;

static INT_PTR CALLBACK DlgProcOptionsAvatars(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		CheckDlgButton(hwndDlg, IDC_SHOWWARNINGS, g_plugin.getByte("warnings", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MAKE_GRAYSCALE, g_plugin.getByte("MakeGrayscale", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MAKE_TRANSPARENT_BKG, g_plugin.getByte("MakeTransparentBkg", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MAKE_TRANSP_PROPORTIONAL, g_plugin.getByte("MakeTransparencyProportionalToColorDiff", 0) ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg, IDC_BKG_NUM_POINTS_SPIN, UDM_SETRANGE, 0, MAKELONG(8, 2));
		SendDlgItemMessage(hwndDlg, IDC_BKG_NUM_POINTS_SPIN, UDM_SETPOS, 0, g_plugin.getWord("TranspBkgNumPoints", 5));

		SendDlgItemMessage(hwndDlg, IDC_BKG_COLOR_DIFFERENCE_SPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
		SendDlgItemMessage(hwndDlg, IDC_BKG_COLOR_DIFFERENCE_SPIN, UDM_SETPOS, 0, g_plugin.getWord("TranspBkgColorDiff", 10));
		{
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_MAKE_TRANSPARENT_BKG);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BKG_NUM_POINTS_L), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BKG_NUM_POINTS_SPIN), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BKG_NUM_POINTS), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BKG_COLOR_DIFFERENCE_L), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BKG_COLOR_DIFFERENCE_SPIN), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BKG_COLOR_DIFFERENCE), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_MAKE_TRANSP_PROPORTIONAL), enabled);
		}

		return TRUE;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDC_BKG_NUM_POINTS || LOWORD(wParam) == IDC_BKG_COLOR_DIFFERENCE) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return FALSE;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_MAKE_TRANSPARENT_BKG:
			{
				BOOL transp_enabled = IsDlgButtonChecked(hwndDlg, IDC_MAKE_TRANSPARENT_BKG);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BKG_NUM_POINTS_L), transp_enabled);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BKG_NUM_POINTS_SPIN), transp_enabled);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BKG_NUM_POINTS), transp_enabled);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BKG_COLOR_DIFFERENCE_L), transp_enabled);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BKG_COLOR_DIFFERENCE_SPIN), transp_enabled);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BKG_COLOR_DIFFERENCE), transp_enabled);
				EnableWindow(GetDlgItem(hwndDlg, IDC_MAKE_TRANSP_PROPORTIONAL), transp_enabled);
				break;
			}
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				g_plugin.setByte("warnings", IsDlgButtonChecked(hwndDlg, IDC_SHOWWARNINGS) ? 1 : 0);
				g_plugin.setByte("MakeGrayscale", IsDlgButtonChecked(hwndDlg, IDC_MAKE_GRAYSCALE) ? 1 : 0);
				g_plugin.setByte("MakeTransparentBkg", IsDlgButtonChecked(hwndDlg, IDC_MAKE_TRANSPARENT_BKG) ? 1 : 0);
				g_plugin.setByte("MakeTransparencyProportionalToColorDiff", IsDlgButtonChecked(hwndDlg, IDC_MAKE_TRANSP_PROPORTIONAL) ? 1 : 0);
				g_plugin.setWord("TranspBkgNumPoints", (uint16_t)SendDlgItemMessage(hwndDlg, IDC_BKG_NUM_POINTS_SPIN, UDM_GETPOS, 0, 0));
				g_plugin.setWord("TranspBkgColorDiff", (uint16_t)SendDlgItemMessage(hwndDlg, IDC_BKG_COLOR_DIFFERENCE_SPIN, UDM_GETPOS, 0, 0));
			}
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK DlgProcOptionsOwn(HWND hwndDlg, UINT msg, WPARAM, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		CheckDlgButton(hwndDlg, IDC_MAKE_MY_AVATARS_TRANSP, g_plugin.getByte("MakeMyAvatarsTransparent", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SET_MAKE_SQUARE, g_plugin.getByte("SetAllwaysMakeSquare", 0) ? BST_CHECKED : BST_UNCHECKED);

		return TRUE;

	case WM_COMMAND:
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				g_plugin.setByte("MakeMyAvatarsTransparent", IsDlgButtonChecked(hwndDlg, IDC_MAKE_MY_AVATARS_TRANSP) ? 1 : 0);
				g_plugin.setByte("SetAllwaysMakeSquare", IsDlgButtonChecked(hwndDlg, IDC_SET_MAKE_SQUARE) ? 1 : 0);
			}
		}
		break;
	}
	return FALSE;
}

static protoPicCacheEntry* GetProtoFromList(HWND hwndDlg, int iItem)
{
	LVITEM item;
	item.mask = LVIF_PARAM;
	item.iItem = iItem;
	if (!ListView_GetItem(GetDlgItem(hwndDlg, IDC_PROTOCOLS), &item))
		return nullptr;

	return (protoPicCacheEntry*)item.lParam;
}

static INT_PTR CALLBACK DlgProcOptionsProtos(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList = GetDlgItem(hwndDlg, IDC_PROTOCOLS);
	HWND hwndChoosePic = GetDlgItem(hwndDlg, IDC_SETPROTOPIC);
	HWND hwndRemovePic = GetDlgItem(hwndDlg, IDC_REMOVEPROTOPIC);

	switch (msg) {
	case WM_INITDIALOG:
		dialoginit = TRUE;
		TranslateDialogDefault(hwndDlg);

		ListView_SetExtendedListViewStyle(hwndList, LVS_EX_CHECKBOXES);
		{
			LVCOLUMN lvc = { 0 };
			lvc.mask = LVCF_FMT;
			lvc.fmt = LVCFMT_IMAGE | LVCFMT_LEFT;
			ListView_InsertColumn(hwndList, 0, &lvc);

			CMStringW tmp;

			LVITEM item = { 0 };
			item.mask = LVIF_TEXT | LVIF_PARAM;
			item.iItem = 1000;
			for (auto &p : g_ProtoPictures) {
				switch (p->cacheType) {
				case PCE_TYPE_ACCOUNT:
					item.pszText = p->pa->tszAccountName;
					break;
				case PCE_TYPE_PROTO:
					tmp.Format(TranslateT("Global avatar for %s accounts"), _A2T(p->pd->szName).get());
					item.pszText = tmp.GetBuffer();
					break;
				default:
					item.pszText = TranslateT("Global avatar");
					break;
				}
				item.lParam = (LPARAM)p;
				int newItem = ListView_InsertItem(hwndList, &item);
				if (newItem >= 0)
					ListView_SetCheckState(hwndList, newItem, g_plugin.getByte(p->szProtoname, 1) ? TRUE : FALSE);
			}
			ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);
			ListView_Arrange(hwndList, LVA_ALIGNLEFT | LVA_ALIGNTOP);
			EnableWindow(hwndChoosePic, FALSE);
			EnableWindow(hwndRemovePic, FALSE);
		}
		dialoginit = FALSE;
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SETPROTOPIC:
		case IDC_REMOVEPROTOPIC:
			int iItem = ListView_GetSelectionMark(hwndList);
			auto *pce = GetProtoFromList(hwndDlg, iItem);
			if (pce) {
				if (LOWORD(wParam) == IDC_SETPROTOPIC)
					SetProtoPic(pce);
				else
					RemoveProtoPic(pce);

				NMHDR nm = { hwndList, IDC_PROTOCOLS, NM_CLICK };
				SendMessage(hwndDlg, WM_NOTIFY, 0, (LPARAM)&nm);
			}
		}
		break;

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			if (dis->CtlType == ODT_BUTTON && dis->CtlID == IDC_PROTOPIC) {
				AVATARDRAWREQUEST avdrq = { 0 };
				avdrq.hTargetDC = dis->hDC;
				avdrq.dwFlags |= AVDRQ_PROTOPICT;
				avdrq.szProto = (g_selectedProto) ? g_selectedProto->szProtoname : nullptr;
				GetClientRect(GetDlgItem(hwndDlg, IDC_PROTOPIC), &avdrq.rcDraw);
				CallService(MS_AV_DRAWAVATAR, 0, (LPARAM)&avdrq);
			}
		}
		return TRUE;

	case WM_NOTIFY:
		if (dialoginit)
			break;

		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_PROTOCOLS:
			switch (((LPNMHDR)lParam)->code) {
			case LVN_KEYDOWN:
				{
					NMLVKEYDOWN* ptkd = (NMLVKEYDOWN*)lParam;
					if (ptkd&&ptkd->wVKey == VK_SPACE && ListView_GetSelectedCount(ptkd->hdr.hwndFrom) == 1)
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				break;

			case LVN_ITEMCHANGED:
				{
					NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;
					if (IsWindowVisible(GetDlgItem(hwndDlg, IDC_PROTOCOLS)) && ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK))
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				break;

			case NM_CLICK:
				EnableWindow(hwndChoosePic, TRUE);
				EnableWindow(hwndRemovePic, TRUE);

				int iItem = ListView_GetSelectionMark(hwndList);
				g_selectedProto = GetProtoFromList(hwndDlg, iItem);
				if (g_selectedProto) {
					DBVARIANT dbv;
					if (!db_get_ws(0, PPICT_MODULE, g_selectedProto->szProtoname, &dbv)) {
						if (!PathIsAbsoluteW(VARSW(dbv.pwszVal))) {
							wchar_t szFinalPath[MAX_PATH];
							mir_snwprintf(szFinalPath, L"%%miranda_path%%\\%s", dbv.pwszVal);
							SetDlgItemText(hwndDlg, IDC_PROTOAVATARNAME, szFinalPath);
						}
						else SetDlgItemText(hwndDlg, IDC_PROTOAVATARNAME, dbv.pwszVal);

						InvalidateRect(GetDlgItem(hwndDlg, IDC_PROTOPIC), nullptr, TRUE);
						db_free(&dbv);
					}
					else {
						SetDlgItemText(hwndDlg, IDC_PROTOAVATARNAME, L"");
						InvalidateRect(GetDlgItem(hwndDlg, IDC_PROTOPIC), nullptr, TRUE);
					}
				}
			}
			break;

		case 0:
			if (((LPNMHDR)lParam)->code == PSN_APPLY) {
				for (int i = 0; i < ListView_GetItemCount(hwndList); i++) {
					auto *pce = GetProtoFromList(hwndDlg, i);

					BOOL oldVal = g_plugin.getByte(pce->szProtoname, 1);
					BOOL newVal = ListView_GetCheckState(hwndList, i);

					if (oldVal && !newVal)
						for (auto &hContact : Contacts(pce->szProtoname))
							DeleteAvatarFromCache(hContact, TRUE);

					if (newVal)
						g_plugin.setByte(pce->szProtoname, 1);
					else
						g_plugin.setByte(pce->szProtoname, 0);
				}
			}
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

int OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.szGroup.a = LPGEN("Contacts");
	odp.szTitle.a = LPGEN("Avatars");

	odp.szTab.a = LPGEN("Protocols");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_PICTS);
	odp.pfnDlgProc = DlgProcOptionsProtos;
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Contact avatars");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_AVATARS);
	odp.pfnDlgProc = DlgProcOptionsAvatars;
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Own avatars");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_OWN);
	odp.pfnDlgProc = DlgProcOptionsOwn;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
