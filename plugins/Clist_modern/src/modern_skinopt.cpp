/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"
#include "modern_sync.h"

/*******************************/
// Main skin selection routine //
/*******************************/
#define MAX_NAME 100

struct SkinListData
{
	wchar_t Name[MAX_NAME];
	wchar_t File[MAX_PATH];
};

HBITMAP hPreviewBitmap = nullptr;
HTREEITEM AddItemToTree(HWND hTree, wchar_t *itemName, void *data);
HTREEITEM AddSkinToListFullName(HWND hwndDlg, wchar_t *fullName);
HTREEITEM AddSkinToList(HWND hwndDlg, wchar_t *path, wchar_t *file);
HTREEITEM FillAvailableSkinList(HWND hwndDlg);

INT_PTR CALLBACK DlgSkinOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

int SkinOptInit(WPARAM wParam, LPARAM)
{
	if (!g_CluiData.fDisableSkinEngine) {
		//Tabbed settings
		OPTIONSDIALOGPAGE odp = {};
		odp.position = -200000000;
		odp.pfnDlgProc = DlgSkinOpts;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SKIN);
		odp.szGroup.w = LPGENW("Skins");
		odp.szTitle.w = LPGENW("Contact list");
		odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
		g_plugin.addOptions(wParam, &odp);
	}
	return 0;
}

INT_PTR CALLBACK DlgSkinOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_DESTROY:
		if (hPreviewBitmap)
			ske_UnloadGlyphImage(hPreviewBitmap);
		break;

	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetDlgItemText(hwndDlg, IDC_SKINFOLDERLABEL, SkinsFolder);
		{
			HTREEITEM it = FillAvailableSkinList(hwndDlg);
			TreeView_SelectItem(GetDlgItem(hwndDlg, IDC_TREE1), it);
		}
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_COLOUR_MENUNORMAL:
		case IDC_COLOUR_MENUSELECTED:
		case IDC_COLOUR_FRAMES:
		case IDC_COLOUR_STATUSBAR:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_BUTTON_INFO:
			{
				HTREEITEM hti = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_TREE1));
				if (hti == nullptr)
					return 0;

				TVITEM tvi = { 0 };
				tvi.hItem = hti;
				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE1), &tvi);
				SkinListData *sd = (SkinListData*)(tvi.lParam);
				if (!sd)
					return 0;

				wchar_t Author[255], URL[MAX_PATH], Contact[255], Description[400], text[2000];
				if (!wcschr(sd->File, '%')) {
					GetPrivateProfileString(L"Skin_Description_Section", L"Author", TranslateT("( unknown )"), Author, _countof(Author), sd->File);
					GetPrivateProfileString(L"Skin_Description_Section", L"URL", L"", URL, _countof(URL), sd->File);
					GetPrivateProfileString(L"Skin_Description_Section", L"Contact", L"", Contact, _countof(Contact), sd->File);
					GetPrivateProfileString(L"Skin_Description_Section", L"Description", L"", Description, _countof(Description), sd->File);
					mir_snwprintf(text, TranslateT("%s\n\n%s\n\nAuthor(s):\t %s\nContact:\t %s\nWeb:\t %s\n\nFile:\t %s"),
						sd->Name, Description, Author, Contact, URL, sd->File);
				}
				else {
					mir_snwprintf(text, TranslateT("%s\n\n%s\n\nAuthor(s): %s\nContact:\t %s\nWeb:\t %s\n\nFile:\t %s"),
						TranslateT("reVista for Modern v0.5"),
						TranslateT("This is second default Modern Contact list skin in Vista Aero style"),
						TranslateT("Angeli-Ka (graphics), FYR (template)"),
						L"JID: fyr@jabber.ru",
						L"fyr.mirandaim.ru",
						TranslateT("Inside library"));
				}
				MessageBox(hwndDlg, text, TranslateT("Skin information"), MB_OK | MB_ICONINFORMATION);
			}
			break;

		case IDC_BUTTON_APPLY_SKIN:
			if (HIWORD(wParam) == BN_CLICKED) {
				HTREEITEM hti = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_TREE1));
				if (hti == nullptr)
					return 0;

				TVITEM tvi = { 0 };
				tvi.hItem = hti;
				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE1), &tvi);
				SkinListData *sd = (SkinListData*)(tvi.lParam);
				if (!sd)
					return 0;

				ske_LoadSkinFromIniFile(sd->File, FALSE);
				ske_LoadSkinFromDB();
				Clist_Broadcast(INTM_RELOADOPTIONS, 0, 0);
				Sync(CLUIFrames_OnClistResize_mod, 0, 0);
				ske_RedrawCompleteWindow();
				Sync(CLUIFrames_OnClistResize_mod, 0, 0);

				RECT rc = {};
				GetWindowRect(g_clistApi.hwndContactList, &rc);
				Sync(CLUIFrames_OnMoving, g_clistApi.hwndContactList, &rc);

				if (g_hCLUIOptionsWnd) {
					SendDlgItemMessage(g_hCLUIOptionsWnd, IDC_LEFTMARGINSPIN, UDM_SETPOS, 0, db_get_b(0, "CLUI", "LeftClientMargin", SETTING_LEFTCLIENTMARIGN_DEFAULT));
					SendDlgItemMessage(g_hCLUIOptionsWnd, IDC_RIGHTMARGINSPIN, UDM_SETPOS, 0, db_get_b(0, "CLUI", "RightClientMargin", SETTING_RIGHTCLIENTMARIGN_DEFAULT));
					SendDlgItemMessage(g_hCLUIOptionsWnd, IDC_TOPMARGINSPIN, UDM_SETPOS, 0, db_get_b(0, "CLUI", "TopClientMargin", SETTING_TOPCLIENTMARIGN_DEFAULT));
					SendDlgItemMessage(g_hCLUIOptionsWnd, IDC_BOTTOMMARGINSPIN, UDM_SETPOS, 0, db_get_b(0, "CLUI", "BottomClientMargin", SETTING_BOTTOMCLIENTMARIGN_DEFAULT));
				}
			}
			break;

		case IDC_GETSKINS:
			if (HIWORD(wParam) == BN_CLICKED)
				Utils_OpenUrl("https://miranda-ng.org/tags/modern-contact-list/");
			break;


		case IDC_BUTTON_RESCAN:
			if (HIWORD(wParam) == BN_CLICKED) {
				HTREEITEM it = FillAvailableSkinList(hwndDlg);
				HWND wnd = GetDlgItem(hwndDlg, IDC_TREE1);
				TreeView_SelectItem(wnd, it);
			}
		}
		break;

	case WM_DRAWITEM:
		if (wParam == IDC_PREVIEW) {
			// TODO:Draw hPreviewBitmap here
			HBRUSH hbr = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT*)lParam;
			int mWidth = dis->rcItem.right - dis->rcItem.left;
			int mHeight = dis->rcItem.bottom - dis->rcItem.top;
			HDC memDC = CreateCompatibleDC(dis->hDC);
			HBITMAP hbmp = ske_CreateDIB32(mWidth, mHeight);
			HBITMAP holdbmp = (HBITMAP)SelectObject(memDC, hbmp);
			RECT workRect = dis->rcItem;
			OffsetRect(&workRect, -workRect.left, -workRect.top);
			FillRect(memDC, &workRect, hbr);
			DeleteObject(hbr);
			if (hPreviewBitmap) {
				// variables
				BITMAP bmp = {};
				GetObject(hPreviewBitmap, sizeof(bmp), &bmp);

				// GetSize
				float xScale = 1, yScale = 1;
				int wWidth = workRect.right - workRect.left;
				int wHeight = workRect.bottom - workRect.top;
				if (wWidth < bmp.bmWidth)
					xScale = (float)wWidth / bmp.bmWidth;
				if (wHeight < bmp.bmHeight)
					yScale = (float)wHeight / bmp.bmHeight;
				xScale = min(xScale, yScale);
				yScale = xScale;
				int dWidth = (int)(xScale*bmp.bmWidth);
				int dHeight = (int)(yScale*bmp.bmHeight);
				
				// CalcPosition
				POINT imgPos = { 0 };
				imgPos.x = workRect.left + ((wWidth - dWidth) >> 1);
				imgPos.y = workRect.top + ((wHeight - dHeight) >> 1);
				
				// DrawImage
				DrawAvatarImageWithGDIp(memDC, imgPos.x, imgPos.y, dWidth, dHeight, hPreviewBitmap, 0, 0, bmp.bmWidth, bmp.bmHeight, 8, 255);
			}
			BitBlt(dis->hDC, dis->rcItem.left, dis->rcItem.top, mWidth, mHeight, memDC, 0, 0, SRCCOPY);
			SelectObject(memDC, holdbmp);
			DeleteObject(hbmp);
			DeleteDC(memDC);
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			if (((LPNMHDR)lParam)->code == PSN_APPLY) {
				Clist_Broadcast(INTM_RELOADOPTIONS, 0, 0);
				NotifyEventHooks(g_CluiData.hEventBkgrChanged, 0, 0);
				Clist_Broadcast(INTM_INVALIDATE, 0, 0);
				RedrawWindow(GetParent(g_clistApi.hwndContactTree), nullptr, nullptr, RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
			}
			break;

		case IDC_TREE1:
			NMTREEVIEW *nmtv = (NMTREEVIEW*)lParam;
			if (nmtv == nullptr)
				return 0;

			if (nmtv->hdr.code == TVN_SELCHANGED) {
				if (hPreviewBitmap) {
					ske_UnloadGlyphImage(hPreviewBitmap);
					hPreviewBitmap = nullptr;
				}

				if (nmtv->itemNew.lParam) {
					SkinListData *sd = (SkinListData*)nmtv->itemNew.lParam;

					wchar_t buf[MAX_PATH];
					PathToRelativeW(sd->File, buf);
					SetDlgItemText(hwndDlg, IDC_EDIT_SKIN_FILENAME, buf);

					wchar_t prfn[MAX_PATH] = { 0 }, imfn[MAX_PATH] = { 0 }, skinfolder[MAX_PATH] = { 0 };
					GetPrivateProfileString(L"Skin_Description_Section", L"Preview", L"", imfn, _countof(imfn), sd->File);
					IniParser::GetSkinFolder(sd->File, skinfolder);
					mir_snwprintf(prfn, L"%s\\%s", skinfolder, imfn);
					PathToAbsoluteW(prfn, imfn);
					hPreviewBitmap = ske_LoadGlyphImage(imfn);

					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_APPLY_SKIN), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_INFO), TRUE);
					if (hPreviewBitmap)
						InvalidateRect(GetDlgItem(hwndDlg, IDC_PREVIEW), nullptr, TRUE);
					else { // prepare text
						HTREEITEM hti = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_TREE1));
						if (hti == nullptr)
							return 0;

						TVITEM tvi = { 0 };
						tvi.hItem = hti;
						tvi.mask = TVIF_HANDLE | TVIF_PARAM;
						TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TREE1), &tvi);
						SkinListData *sd2 = (SkinListData*)(tvi.lParam);
						if (!sd2)
							return 0;

						wchar_t Author[255], URL[MAX_PATH], Contact[255], Description[400], text[2000];
						if (!wcschr(sd2->File, '%')) {
							GetPrivateProfileString(L"Skin_Description_Section", L"Author", TranslateT("( unknown )"), Author, _countof(Author), sd2->File);
							GetPrivateProfileString(L"Skin_Description_Section", L"URL", L"", URL, _countof(URL), sd2->File);
							GetPrivateProfileString(L"Skin_Description_Section", L"Contact", L"", Contact, _countof(Contact), sd2->File);
							GetPrivateProfileString(L"Skin_Description_Section", L"Description", L"", Description, _countof(Description), sd2->File);
							mir_snwprintf(text, TranslateT("Preview is not available\n\n%s\n----------------------\n\n%s\n\nAUTHOR(S):\n%s\n\nCONTACT:\n%s\n\nHOMEPAGE:\n%s"),
								sd2->Name, Description, Author, Contact, URL);
						}
						else {
							mir_snwprintf(text, TranslateT("%s\n\n%s\n\nAUTHORS:\n%s\n\nCONTACT:\n%s\n\nWEB:\n%s\n\n\n"),
								TranslateT("reVista for Modern v0.5"),
								TranslateT("This is second default Modern Contact list skin in Vista Aero style"),
								TranslateT("graphics by Angeli-Ka\ntemplate by FYR"),
								L"JID: fyr@jabber.ru",
								L"fyr.mirandaim.ru");
						}
						ShowWindow(GetDlgItem(hwndDlg, IDC_PREVIEW), SW_HIDE);
						ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC_INFO), SW_SHOW);
						SetDlgItemText(hwndDlg, IDC_STATIC_INFO, text);
					}
				}
				else {
					// no selected
					SetDlgItemText(hwndDlg, IDC_EDIT_SKIN_FILENAME, TranslateT("Select skin from list"));
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_APPLY_SKIN), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_INFO), FALSE);
					SetDlgItemText(hwndDlg, IDC_STATIC_INFO, TranslateT("Please select skin to apply"));
					ShowWindow(GetDlgItem(hwndDlg, IDC_PREVIEW), SW_HIDE);
				}
				ShowWindow(GetDlgItem(hwndDlg, IDC_PREVIEW), hPreviewBitmap ? SW_SHOW : SW_HIDE);
				return 0;
			}
			else if (nmtv->hdr.code == TVN_DELETEITEM) {
				mir_free_and_nil(nmtv->itemOld.lParam);
				return 0;
			}
			break;
		}
	}
	return 0;
}

int SearchSkinFiles(HWND hwndDlg, wchar_t *Folder)
{
	wchar_t mask[MAX_PATH];
	mir_snwprintf(mask, L"%s\\*.msf", Folder);

	struct _wfinddata_t fd = {};
	intptr_t hFile = _wfindfirst(mask, &fd);
	if (hFile != -1) {
		do {
			AddSkinToList(hwndDlg, Folder, fd.name);
		} while (!_wfindnext(hFile, &fd));
		_findclose(hFile);
	}

	mir_snwprintf(mask, L"%s\\*", Folder);
	hFile = _wfindfirst(mask, &fd);

	do {
		if (fd.attrib & _A_SUBDIR && !(mir_wstrcmpi(fd.name, L".") == 0 || mir_wstrcmpi(fd.name, L"..") == 0)) { //Next level of subfolders
			wchar_t path[MAX_PATH];
			mir_snwprintf(path, L"%s\\%s", Folder, fd.name);
			SearchSkinFiles(hwndDlg, path);
		}
	} while (!_wfindnext(hFile, &fd));
	
	_findclose(hFile);
	return 0;
}

HTREEITEM FillAvailableSkinList(HWND hwndDlg)
{
	TreeView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_TREE1));
	AddSkinToList(hwndDlg, TranslateT("Default Skin"), L"%Default Skin%");
	int attrib = GetFileAttributes(SkinsFolder);
	if (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY))
		SearchSkinFiles(hwndDlg, SkinsFolder);

	HTREEITEM res = (HTREEITEM)-1;
	wchar_t skinfull[MAX_PATH];
	ptrW skinfile(db_get_wsa(0, SKIN, "SkinFile"));
	if (skinfile) {
		PathToAbsoluteW(skinfile, skinfull);
		res = AddSkinToListFullName(hwndDlg, skinfull);
	}

	return res;
}

HTREEITEM AddSkinToListFullName(HWND hwndDlg, wchar_t *fullName)
{
	wchar_t path[MAX_PATH] = {}, file[MAX_PATH] = {};
	mir_wstrncpy(path, fullName, _countof(path));
	
	wchar_t *buf = path + mir_wstrlen(path);
	while (buf > path) {
		if (*buf == '\\') {
			*buf = '\0';
			break;
		}
		buf--;
	}
	buf++;
	mir_wstrncpy(file, buf, _countof(file));
	return AddSkinToList(hwndDlg, path, file);
}

HTREEITEM AddSkinToList(HWND hwndDlg, wchar_t *path, wchar_t *file)
{
	wchar_t fullName[MAX_PATH], defskinname[MAX_PATH];
	SkinListData *sd = (SkinListData*)mir_alloc(sizeof(SkinListData));
	if (!sd)
		return nullptr;

	if (!file || wcschr(file, '%')) {
		mir_snwprintf(sd->File, L"%%Default Skin%%");
		mir_snwprintf(sd->Name, TranslateT("%Default Skin%"));
		wcsncpy_s(fullName, TranslateT("Default Skin"), _TRUNCATE);
	}
	else {
		mir_snwprintf(fullName, L"%s\\%s", path, file);
		wcsncpy_s(defskinname, file, _TRUNCATE);
		wchar_t *p = wcsrchr(defskinname, '.'); if (p) *p = 0;
		GetPrivateProfileString(L"Skin_Description_Section", L"Name", defskinname, sd->Name, _countof(sd->Name), fullName);
		wcsncpy_s(sd->File, fullName, _TRUNCATE);
	}
	return AddItemToTree(GetDlgItem(hwndDlg, IDC_TREE1), sd->Name, sd);
}

HTREEITEM FindChild(HWND hTree, HTREEITEM Parent, wchar_t *Caption, void *data)
{
	HTREEITEM tmp = nullptr;
	if (Parent)
		tmp = TreeView_GetChild(hTree, Parent);
	else
		tmp = TreeView_GetRoot(hTree);

	while (tmp) {
		TVITEM tvi;
		wchar_t buf[255];
		tvi.hItem = tmp;
		tvi.mask = TVIF_TEXT | TVIF_HANDLE;
		tvi.pszText = buf;
		tvi.cchTextMax = _countof(buf);
		TreeView_GetItem(hTree, &tvi);
		if (mir_wstrcmpi(Caption, tvi.pszText) == 0) {
			if (!data)
				return tmp;

			TVITEM tvi2 = { 0 };
			tvi2.hItem = tmp;
			tvi2.mask = TVIF_HANDLE | TVIF_PARAM;
			TreeView_GetItem(hTree, &tvi2);
			SkinListData *sd = (SkinListData*)tvi2.lParam;
			if (sd)
				if (!mir_wstrcmpi(sd->File, ((SkinListData*)data)->File))
					return tmp;
		}
		tmp = TreeView_GetNextSibling(hTree, tmp);
	}
	return tmp;
}

HTREEITEM AddItemToTree(HWND hTree, wchar_t *itemName, void *data)
{
	HTREEITEM cItem = nullptr;
	// Insert item node
	cItem = FindChild(hTree, nullptr, itemName, data);
	if (!cItem) {
		TVINSERTSTRUCT tvis = {};
		tvis.hInsertAfter = TVI_SORT;
		tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_PARAM;
		tvis.item.pszText = itemName;
		tvis.item.lParam = (LPARAM)data;
		return TreeView_InsertItem(hTree, &tvis);
	}

	mir_free(data); // need to free otherwise memory leak
	return cItem;
}

INT_PTR SvcActiveSkin(WPARAM, LPARAM)
{
	ptrW skinfile(db_get_wsa(0, SKIN, "SkinFile"));
	if (skinfile) {
		wchar_t skinfull[MAX_PATH];
		PathToAbsoluteW(skinfile, skinfull);
		return (INT_PTR)mir_wstrdup(skinfull);
	}

	return 0;
}

INT_PTR SvcApplySkin(WPARAM, LPARAM lParam)
{
	ske_LoadSkinFromIniFile((wchar_t*)lParam, FALSE);
	ske_LoadSkinFromDB();
	Clist_Broadcast(INTM_RELOADOPTIONS, 0, 0);
	Sync(CLUIFrames_OnClistResize_mod, 0, 0);
	ske_RedrawCompleteWindow();
	Sync(CLUIFrames_OnClistResize_mod, 0, 0);

	HWND hwnd = g_clistApi.hwndContactList;
	RECT rc = { 0 };
	GetWindowRect(hwnd, &rc);
	Sync(CLUIFrames_OnMoving, hwnd, &rc);

	g_bChangingMode = TRUE;
	CLUI_UpdateLayeredMode();
	CLUI_ChangeWindowMode();
	SendMessage(g_clistApi.hwndContactTree, WM_SIZE, 0, 0);	//forces it to send a cln_listsizechanged
	CLUI_ReloadCLUIOptions();
	cliShowHide(true);
	g_bChangingMode = FALSE;

	if (g_hCLUIOptionsWnd) {
		SendDlgItemMessage(g_hCLUIOptionsWnd, IDC_LEFTMARGINSPIN, UDM_SETPOS, 0, db_get_b(0, "CLUI", "LeftClientMargin", SETTING_LEFTCLIENTMARIGN_DEFAULT));
		SendDlgItemMessage(g_hCLUIOptionsWnd, IDC_RIGHTMARGINSPIN, UDM_SETPOS, 0, db_get_b(0, "CLUI", "RightClientMargin", SETTING_RIGHTCLIENTMARIGN_DEFAULT));
		SendDlgItemMessage(g_hCLUIOptionsWnd, IDC_TOPMARGINSPIN, UDM_SETPOS, 0, db_get_b(0, "CLUI", "TopClientMargin", SETTING_TOPCLIENTMARIGN_DEFAULT));
		SendDlgItemMessage(g_hCLUIOptionsWnd, IDC_BOTTOMMARGINSPIN, UDM_SETPOS, 0, db_get_b(0, "CLUI", "BottomClientMargin", SETTING_BOTTOMCLIENTMARIGN_DEFAULT));
	}
	return 0;
}

INT_PTR SvcPreviewSkin(WPARAM wParam, LPARAM lParam)
{
	DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT*)wParam;
	RECT workRect = dis->rcItem;
	OffsetRect(&workRect, -workRect.left, -workRect.top);

	if (lParam) {
		wchar_t prfn[MAX_PATH] = { 0 };
		wchar_t imfn[MAX_PATH] = { 0 };
		wchar_t skinfolder[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"Skin_Description_Section", L"Preview", L"", imfn, _countof(imfn), (LPCTSTR)lParam);
		IniParser::GetSkinFolder((LPCTSTR)lParam, skinfolder);
		mir_snwprintf(prfn, L"%s\\%s", skinfolder, imfn);
		PathToAbsoluteW(prfn, imfn);

		hPreviewBitmap = ske_LoadGlyphImage(imfn);
		if (hPreviewBitmap) {
			// variables
			BITMAP bmp = { 0 };
			GetObject(hPreviewBitmap, sizeof(BITMAP), &bmp);

			// GetSize
			float xScale = 1, yScale = 1;
			int wWidth = workRect.right - workRect.left;
			int wHeight = workRect.bottom - workRect.top;
			if (wWidth < bmp.bmWidth)
				xScale = (float)wWidth / bmp.bmWidth;
			if (wHeight < bmp.bmHeight)
				yScale = (float)wHeight / bmp.bmHeight;
			xScale = min(xScale, yScale);
			yScale = xScale;
			int dWidth = (int)(xScale*bmp.bmWidth);
			int dHeight = (int)(yScale*bmp.bmHeight);
			
			// CalcPosition
			POINT imgPos = { 0 };
			imgPos.x = workRect.left + ((wWidth - dWidth) >> 1);
			imgPos.y = workRect.top + ((wHeight - dHeight) >> 1);
			
			// DrawImage
			DrawAvatarImageWithGDIp(dis->hDC, imgPos.x, imgPos.y, dWidth, dHeight, hPreviewBitmap, 0, 0, bmp.bmWidth, bmp.bmHeight, 8, 255);
			ske_UnloadGlyphImage(hPreviewBitmap);
		}
	}

	return 0;
}
