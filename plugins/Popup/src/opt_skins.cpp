/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
© 2004-2007 Victor Pavlychko
© 2010 MPK
© 2010 Merlin_de

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

static volatile bool gPreviewOk = false;
static PopupWnd2 *wndPreview = nullptr;

INT_PTR CALLBACK BoxPreviewWndProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

void RegisterOptPrevBox()
{
	WNDCLASSEX wcl;
	wcl.cbSize = sizeof(wcl);
	wcl.lpfnWndProc = (WNDPROC)BoxPreviewWndProc;
	wcl.style = CS_DROPSHADOW;
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 0;
	wcl.hInstance = g_plugin.getInst();
	wcl.hIcon = nullptr;
	wcl.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcl.hbrBackground = nullptr; // (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wcl.lpszMenuName = nullptr;
	wcl.lpszClassName = BOXPREVIEW_WNDCLASS;
	wcl.hIconSm = Skin_LoadIcon(SKINICON_OTHER_POPUP);
	g_wndClass.cPopupPreviewBoxWndclass = RegisterClassEx(&wcl);

	//  register custom class for dialog box with drop-shadow attribute
	//  "#32770" stays for class name of default system dialog box
	GetClassInfoEx(g_plugin.getInst(), L"#32770", &wcl);
	wcl.hInstance = g_plugin.getInst();
	wcl.lpszClassName = L"PopupPlusDlgBox";
	wcl.style |= CS_DROPSHADOW;
	g_wndClass.cPopupPlusDlgBox = RegisterClassEx(&wcl);
}

static void updatePreviewImage(HWND hwndBox)
{
	gPreviewOk = false;

	POPUPDATA2 ppd;
	memset(&ppd, 0, sizeof(ppd));
	ppd.cbSize = sizeof(ppd);
	ppd.flags = PU2_UNICODE;
	ppd.lchIcon = Skin_LoadIcon(SKINICON_STATUS_ONLINE);
	ppd.szTitle.w = TranslateT("Skin preview");
	ppd.szText.w = TranslateT("Just take a look at this skin... ;)");

	POPUPOPTIONS customOptions = PopupOptions;
	customOptions.DynamicResize = FALSE;
	customOptions.MinimumWidth = customOptions.MaximumWidth = 250;

	if (wndPreview) delete wndPreview;
	wndPreview = new PopupWnd2(&ppd, &customOptions, true);
	wndPreview->buildMText();
	wndPreview->update();
	gPreviewOk = true;

	InvalidateRect(hwndBox, nullptr, TRUE);
}

static void DrawPreview(HWND hwnd, HDC hdc)
{
	BITMAPINFO bi = { 0 };
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = 8;
	bi.bmiHeader.biHeight = -8;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	HBITMAP hBmpBrush = CreateDIBSection(nullptr, &bi, DIB_RGB_COLORS, nullptr, nullptr, 0);
	HDC dcBmp = CreateCompatibleDC(nullptr);
	HBITMAP hBmpSave = (HBITMAP)SelectObject(dcBmp, hBmpBrush);
	HBRUSH hbr = CreateSolidBrush(RGB(0xcc, 0xcc, 0xcc));

	RECT rc;
	SetRect(&rc, 0, 0, 8, 8);
	FillRect(dcBmp, &rc, hbr);
	DeleteObject(hbr);
	hbr = CreateSolidBrush(RGB(0xff, 0xff, 0xff));
	SetRect(&rc, 4, 0, 8, 4);
	FillRect(dcBmp, &rc, hbr);
	SetRect(&rc, 0, 4, 4, 8);
	FillRect(dcBmp, &rc, hbr);
	DeleteObject(hbr);
	SelectObject(dcBmp, hBmpSave);
	DeleteDC(dcBmp);

	GetClientRect(hwnd, &rc);
	hbr = CreatePatternBrush(hBmpBrush);
	SetBrushOrgEx(hdc, 1, 1, nullptr);
	FillRect(hdc, &rc, hbr);
	DeleteObject(hbr);
	DeleteObject(hBmpBrush);

	if (gPreviewOk)
	{
		int width = min(rc.right, wndPreview->getContent()->getWidth());
		int height = min(rc.bottom, wndPreview->getContent()->getHeight());
		int left = (rc.right - width) / 2;
		int top = (rc.bottom - height) / 2;


		BLENDFUNCTION bf;
		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.SourceConstantAlpha = 255;
		bf.AlphaFormat = AC_SRC_ALPHA;
		AlphaBlend(hdc, left, top, width, height,
			wndPreview->getContent()->getDC(),
			0, 0, width, height, bf);

	}

	FrameRect(hdc, &rc, GetStockBrush(LTGRAY_BRUSH));
}

LRESULT CALLBACK WndProcPreviewBox(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (!wndPreview)
		return mir_callNextSubclass(hwnd, WndProcPreviewBox, msg, wParam, lParam);

	switch (msg) {
	case WM_PAINT:
		if (GetUpdateRect(hwnd, nullptr, FALSE))
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			DrawPreview(hwnd, hdc);
			EndPaint(hwnd, &ps);
			return 0;
		}

	case WM_PRINT:
	case WM_PRINTCLIENT:
		HDC hdc = (HDC)wParam;
		DrawPreview(hwnd, hdc);
		return 0;
	}
	return mir_callNextSubclass(hwnd, WndProcPreviewBox, msg, wParam, lParam);
}

int  SkinOptionList_AddSkin(OPTTREE_OPTION* &options, int *OptionsCount, int pos, uint32_t *dwGlobalOptions) {
	const PopupSkin *skin = nullptr;
	if (skin = skins.getSkin(PopupOptions.SkinPack)) {
		for (int i = 1; i <= 10; i++) {
			if (!skin->getFlagName(i))
				continue;
			*OptionsCount += 1;
			options = (OPTTREE_OPTION*)mir_realloc(options, sizeof(OPTTREE_OPTION)*(*OptionsCount));
			options[pos].dwFlag = (uint32_t)(1 << (i - 1));
			options[pos].groupId = OPTTREE_CHECK;
			options[pos].iconIndex = 0;
			options[pos].pszSettingName = mir_wstrdup(L"Skin options");
			options[pos].pszOptionName = (LPTSTR)mir_alloc(sizeof(wchar_t)*(
				mir_wstrlen(options[pos].pszSettingName) +
				mir_strlen(skin->getFlagName(i)) + 10));
			wsprintf(options[pos].pszOptionName, L"%s/%hs", options[pos].pszSettingName, skin->getFlagName(i)); // !!!!!!!!!!!!!
			options[pos].bState = skin->getFlag(i) ? TRUE : FALSE;
			options[pos].Data = i;	// skin flag index
			*dwGlobalOptions |= skin->getFlag(i) ? (1 << (i - 1)) : 0;
			pos++;
		}
	}
	return pos;
}

/////////////////////////////////////////////////////////////////////////////////////////

static LPTSTR mainOption[] = {
	LPGENW("Show clock"),
	LPGENW("Drop shadow effect"),
	LPGENW("Drop shadow effect") L"/" LPGENW("non rectangular"),
	LPGENW("Enable Aero Glass (Vista+)"),
	LPGENW("Use Windows colors"),
	LPGENW("Use advanced text render") };

int SkinOptionList_AddMain(OPTTREE_OPTION* &options, int *OptionsCount, int pos, uint32_t *dwGlobalOptions)
{
	for (int i = 0; i < _countof(mainOption); i++) {
		BOOL bCheck = 0;
		switch (i) {
		case 0:
			*dwGlobalOptions |= PopupOptions.DisplayTime ? (1 << i) : 0;
			bCheck = PopupOptions.DisplayTime;
			break;
		case 1:
			*dwGlobalOptions |= PopupOptions.DropShadow ? (1 << i) : 0;
			bCheck = PopupOptions.DropShadow;
			break;
		case 2:
			*dwGlobalOptions |= PopupOptions.EnableFreeformShadows ? (1 << i) : 0;
			bCheck = PopupOptions.EnableFreeformShadows;
			break;
		case 3:
			if (!MyDwmEnableBlurBehindWindow) continue;
			*dwGlobalOptions |= PopupOptions.EnableAeroGlass ? (1 << i) : 0;
			bCheck = PopupOptions.EnableAeroGlass;
			break;
		case 4:
			*dwGlobalOptions |= PopupOptions.UseWinColors ? (1 << i) : 0;
			bCheck = PopupOptions.UseWinColors;
			break;
		case 5:
			if (!(htuText&&htuTitle)) continue;
			*dwGlobalOptions |= PopupOptions.UseMText ? (1 << i) : 0;
			bCheck = PopupOptions.UseMText;
			break;
		}
		*OptionsCount += 1;
		options = (OPTTREE_OPTION*)mir_realloc(options, sizeof(OPTTREE_OPTION)*(*OptionsCount));
		options[pos].dwFlag = (1 << i);
		options[pos].groupId = OPTTREE_CHECK;
		options[pos].iconIndex = 0;
		options[pos].pszSettingName = mir_wstrdup(LPGENW("Global settings"));
		options[pos].pszOptionName = (LPTSTR)mir_alloc(sizeof(wchar_t)*(
			mir_wstrlen(options[pos].pszSettingName) +
			mir_wstrlen(mainOption[i]) + 10));
		wsprintf(options[pos].pszOptionName, L"%s/%s", options[pos].pszSettingName, mainOption[i]); // !!!!!!!!!!!!!
		options[pos].bState = bCheck;
		pos++;
	}
	return pos;
}

bool SkinOptionList_Update(OPTTREE_OPTION* &options, int *OptionsCount, HWND hwndDlg) {
	if (options) {
		int index = -1;
		OptTree_ProcessMessage(hwndDlg, WM_DESTROY, 0, 0, &index, IDC_SKIN_LIST_OPT, options, *OptionsCount);
		for (int i = 0; i < *OptionsCount; ++i) {
			mir_free(options[i].pszOptionName);
			mir_free(options[i].pszSettingName);
		}
		mir_free(options);
		options = nullptr;
		*OptionsCount = 0;
	}
	// add "Global options"
	uint32_t dwGlobalOptions = 0;
	int pos = SkinOptionList_AddMain(options, OptionsCount, 0, &dwGlobalOptions);
	// add "Skin options"
	uint32_t dwSkinOptions = 0;
	pos = SkinOptionList_AddSkin(options, OptionsCount, pos, &dwSkinOptions);
	// generate treeview
	int index = -1;
	OptTree_ProcessMessage(hwndDlg, WM_INITDIALOG, 0, 0, &index, IDC_SKIN_LIST_OPT, options, *OptionsCount);

	// check "Skin options" state 
	char prefix[128];
	mir_snprintf(prefix, "skin.%S", PopupOptions.SkinPack);
	OptTree_SetOptions(hwndDlg, IDC_SKIN_LIST_OPT, options, *OptionsCount,
		g_plugin.getDword(prefix, dwSkinOptions), L"Skin options");

	// check "Global Settings"
	OptTree_SetOptions(hwndDlg, IDC_SKIN_LIST_OPT, options, *OptionsCount,
		dwGlobalOptions, L"Global settings");

	return true;
}

void LoadOption_Skins() {
	// skin pack
	PopupOptions.SkinPack = (LPTSTR)DBGetContactSettingStringX(0, MODULENAME, "SkinPack", "* Popup Classic", DBVT_WCHAR);
	// more Skin options
	PopupOptions.DisplayTime = g_plugin.getByte("DisplayTime", TRUE);
	PopupOptions.DropShadow = g_plugin.getByte("DropShadow", TRUE);
	PopupOptions.EnableFreeformShadows = g_plugin.getByte("EnableShadowRegion", 1);
	PopupOptions.EnableAeroGlass = g_plugin.getByte("EnableAeroGlass", 1);
	PopupOptions.UseWinColors = g_plugin.getByte("UseWinColors", FALSE);
	PopupOptions.UseMText = g_plugin.getByte("UseMText", TRUE);
}

INT_PTR CALLBACK DlgProcPopSkinsOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool bDlgInit = false;	// some controls send WM_COMMAND before or during WM_INITDIALOG
	static HANDLE hhkFontsReload = nullptr;
	static OPTTREE_OPTION *skinOptions = nullptr;
	static int skinOptionsCount = 0;

	if (skinOptions) {
		int index = -1;
		OptTree_ProcessMessage(hwndDlg, msg, wParam, lParam, &index, IDC_SKIN_LIST_OPT, skinOptions, skinOptionsCount);
		if (index != -1) {
			if (mir_wstrcmp(skinOptions[index].pszSettingName, L"Skin options") == 0) {
				const PopupSkin *skin = nullptr;
				if (skin = skins.getSkin(PopupOptions.SkinPack)) {
					skin->setFlag(skinOptions[index].Data, skinOptions[index].bState ? true : false);
				}
			}
			else if (mir_wstrcmp(skinOptions[index].pszSettingName, L"Global settings") == 0) {
				switch (skinOptions[index].dwFlag) {
				case (1 << 0) :
					PopupOptions.DisplayTime = skinOptions[index].bState;
					break;
				case (1 << 1) :
					PopupOptions.DropShadow = skinOptions[index].bState;
					break;
				case (1 << 2) :
					PopupOptions.EnableFreeformShadows = skinOptions[index].bState;
					break;
				case (1 << 3) :
					PopupOptions.EnableAeroGlass = skinOptions[index].bState;
					break;
				case (1 << 4) :
					PopupOptions.UseWinColors = skinOptions[index].bState;
					break;
				case (1 << 5) :
					PopupOptions.UseMText = skinOptions[index].bState;
					break;
				}
			}
			updatePreviewImage(GetDlgItem(hwndDlg, IDC_PREVIEWBOX));
			return FALSE;
		}
	}

	switch (msg) {
	case WM_INITDIALOG:
		{
			// Skin List
			HWND hCtrl = GetDlgItem(hwndDlg, IDC_SKINLIST);
			ListBox_ResetContent(hCtrl);
			for (const Skins::SKINLIST *sl = skins.getSkinList(); sl; sl = sl->next) {
				uint32_t dwIndex = ListBox_AddString(hCtrl, sl->name);
				ListBox_SetItemData(hCtrl, dwIndex, sl->name);
			}
			ListBox_SetCurSel(hCtrl, ListBox_FindString(hCtrl, 0, PopupOptions.SkinPack));

			// Skin List reload button
			SendDlgItemMessage(hwndDlg, IDC_BTN_RELOAD, BUTTONSETASFLATBTN, TRUE, 0);
			SendDlgItemMessage(hwndDlg, IDC_BTN_RELOAD, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(IDI_RELOAD));
			SendDlgItemMessage(hwndDlg, IDC_BTN_RELOAD, BUTTONADDTOOLTIP, (WPARAM)Translate("Refresh List"), 0);

			// Skin Option List
			SkinOptionList_Update(skinOptions, &skinOptionsCount, hwndDlg);

			// PreviewBox
			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_PREVIEWBOX), WndProcPreviewBox);
			updatePreviewImage(GetDlgItem(hwndDlg, IDC_PREVIEWBOX));

			// hooks
			hhkFontsReload = HookEventMessage(ME_FONT_RELOAD, hwndDlg, WM_USER);

			TranslateDialogDefault(hwndDlg);
			bDlgInit = true;
		}
		return TRUE;

	case WM_USER:
		updatePreviewImage(GetDlgItem(hwndDlg, IDC_PREVIEWBOX));
		return TRUE;

	case WM_COMMAND:
		{
			HWND hCtrl = nullptr;
			UINT idCtrl = LOWORD(wParam);
			switch (HIWORD(wParam)) {
			case BN_KILLFOCUS:		// Button controls
			case BN_SETFOCUS:		// Button controls
				return TRUE;

			case BN_CLICKED:		// Button controls
				switch (idCtrl) {
				case IDC_PREVIEW:
					PopupPreview();
					break;

				case IDC_GETSKINS:
					Utils_OpenUrl("https://miranda-ng.org/tags/popup-plus/");
					break;

				case IDC_BTN_RELOAD:
					LPTSTR pszOldSkin = NEWWSTR_ALLOCA(PopupOptions.SkinPack);
					skins.load();
					hCtrl = GetDlgItem(hwndDlg, IDC_SKINLIST);
					ListBox_ResetContent(hCtrl);
					for (const Skins::SKINLIST *sl = skins.getSkinList(); sl; sl = sl->next) {
						uint32_t dwIndex = ListBox_AddString(hCtrl, sl->name);
						ListBox_SetItemData(hCtrl, dwIndex, sl->name);
					}
					ListBox_SetCurSel(hCtrl, ListBox_FindString(hCtrl, 0, PopupOptions.SkinPack));

					// make sure we have select skin (ListBox_SetCurSel may be fail)
					wchar_t szNewSkin[128];
					ListBox_GetText(hCtrl, ListBox_GetCurSel(hCtrl), &szNewSkin);
					if (mir_wstrcmp(pszOldSkin, szNewSkin) != 0) {
						mir_free(PopupOptions.SkinPack);
						PopupOptions.SkinPack = mir_wstrdup(szNewSkin);
					}

					const PopupSkin *skin = nullptr;
					if (skin = skins.getSkin(PopupOptions.SkinPack)) {
						// update Skin Option List from reload SkinPack
						bDlgInit = false;
						bDlgInit = SkinOptionList_Update(skinOptions, &skinOptionsCount, hwndDlg);
					}

					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				}
				updatePreviewImage(GetDlgItem(hwndDlg, IDC_PREVIEWBOX));
				break;

			case CBN_SELCHANGE:		// combo box controls
				switch (idCtrl) {
				case IDC_SKINLIST:
					{
						// Skin list change
						mir_free(PopupOptions.SkinPack);
						PopupOptions.SkinPack = mir_wstrdup((wchar_t *)SendDlgItemMessage(
							hwndDlg,
							IDC_SKINLIST,
							LB_GETITEMDATA,
							(WPARAM)SendDlgItemMessage(hwndDlg, IDC_SKINLIST, LB_GETCURSEL, 0, 0),
							0));
						const PopupSkin *skin = nullptr;
						if (skin = skins.getSkin(PopupOptions.SkinPack)) {
							mir_free(PopupOptions.SkinPack);
							PopupOptions.SkinPack = mir_wstrdup(skin->getName());

							// update Skin Option List
							bDlgInit = false;
							bDlgInit = SkinOptionList_Update(skinOptions, &skinOptionsCount, hwndDlg);
						}
						updatePreviewImage(GetDlgItem(hwndDlg, IDC_PREVIEWBOX));
					}
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				}// end switch(idCtrl)
				break;
			}// end switch (HIWORD(wParam))
			break;
		}//  end WM_COMMAND

	case WM_NOTIFY:
		if (!bDlgInit) return FALSE;
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_RESET:
				LoadOption_Skins();
				return TRUE;

			case PSN_APPLY:
				{
					// skin pack
					g_plugin.setWString("SkinPack", PopupOptions.SkinPack);
					// skin options
					const PopupSkin *skin = nullptr;
					if (skin = skins.getSkin(PopupOptions.SkinPack))
						skin->saveOpts();
					skins.freeAllButActive();
					// more Skin options
					g_plugin.setByte("DisplayTime", PopupOptions.DisplayTime);
					g_plugin.setByte("DropShadow", PopupOptions.DropShadow);
					g_plugin.setByte("EnableShadowRegion", PopupOptions.EnableFreeformShadows);
					g_plugin.setByte("EnableAeroGlass", PopupOptions.EnableAeroGlass);
					g_plugin.setByte("UseWinColors", PopupOptions.UseWinColors);
					g_plugin.setByte("UseMText", PopupOptions.UseMText);
				}// end PSN_APPLY:
				return TRUE;
			}// switch (((LPNMHDR)lParam)->code)
			break;
		}// end switch (((LPNMHDR)lParam)->idFrom)
		return FALSE;

	case WM_DESTROY:
		if (wndPreview) {
			delete wndPreview;
			wndPreview = nullptr;
			gPreviewOk = false;
		}
		if (hhkFontsReload) UnhookEvent(hhkFontsReload);
		if (skinOptions) {
			for (int i = 0; i < skinOptionsCount; ++i) {
				mir_free(skinOptions[i].pszOptionName);
				mir_free(skinOptions[i].pszSettingName);
			}
			mir_free(skinOptions);
			skinOptions = nullptr;
			skinOptionsCount = 0;
		}
		return TRUE;
	}// end switch (msg)
	return FALSE;
}

static void BoxPreview_OnPaint(HWND hwnd, HDC mydc, int mode)
{
	RECT rc;

	switch (mode) {
	case 0:
		{ //  Avatar
			HDC hdcAvatar = CreateCompatibleDC(mydc);
			HBITMAP hbmSave = (HBITMAP)SelectObject(hdcAvatar, hbmNoAvatar);
			GetClientRect(hwnd, &rc);
			BITMAP bmp;
			GetObject(hbmNoAvatar, sizeof(bmp), &bmp);
			StretchBlt(mydc, 0, 0, rc.right, rc.bottom, hdcAvatar, 0, 0, abs(bmp.bmWidth), abs(bmp.bmHeight), SRCCOPY);
			SelectObject(hdcAvatar, hbmSave);
			DeleteDC(hdcAvatar);
			HRGN rgn = CreateRoundRectRgn(0, 0, rc.right, rc.bottom, 2 * PopupOptions.avatarRadius, 2 * PopupOptions.avatarRadius);
			FrameRgn(mydc, rgn, (HBRUSH)GetStockObject(BLACK_BRUSH), 1, 1);
			DeleteObject(rgn);
		}
		break;

	case 1:
		{ //  Opacity
			HBRUSH hbr = CreateSolidBrush(fonts.clBack);
			HFONT hfnt = (HFONT)SelectObject(mydc, fonts.title);
			GetClientRect(hwnd, &rc);
			FillRect(mydc, &rc, hbr);
			DrawIconEx(mydc, 10, (rc.bottom - rc.top - 16) / 2, Skin_LoadIcon(SKINICON_OTHER_POPUP), 16, 16, 0, hbr, DI_NORMAL);
			SetBkMode(mydc, TRANSPARENT);
			GetClientRect(hwnd, &rc);
			rc.left += 30; //  10+16+4 -- icon
			rc.right -= (rc.right - rc.left) / 3;
			rc.bottom -= (rc.bottom - rc.top) / 3;
			DrawText(mydc, MODULNAME_LONG, -1, &rc, DT_CENTER | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
			GetClientRect(hwnd, &rc);
			rc.left += 30; //  10+16+4 -- icon
			rc.left += (rc.right - rc.left) / 3;
			rc.top += (rc.bottom - rc.top) / 3;
			DrawText(mydc, MODULNAME_LONG, -1, &rc, DT_CENTER | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
			GetClientRect(hwnd, &rc);
			FrameRect(mydc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
			SelectObject(mydc, hfnt);
			DeleteObject(hbr);
		}
		break;

	case 2:
		{ //  Position
			HBRUSH hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
			GetClientRect(hwnd, &rc);
			FillRect(mydc, &rc, hbr);
			DeleteObject(hbr);

			hbr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
			GetClientRect(hwnd, &rc);
			rc.right -= 100;
			rc.top += 100;
			FillRect(mydc, &rc, hbr);
			DeleteObject(hbr);

			HPEN hpen = (HPEN)SelectObject(mydc, CreatePen(PS_DOT, 1, RGB(0, 0, 0)));
			MoveToEx(mydc, 0, 100, nullptr);
			LineTo(mydc, 201, 100);
			MoveToEx(mydc, 100, 0, nullptr);
			LineTo(mydc, 100, 201);
			DeleteObject(SelectObject(mydc, hpen));

			HRGN hrgn = CreateRectRgn(0, 0, 0, 0);
			GetWindowRgn(hwnd, hrgn);
			FrameRgn(mydc, hrgn, (HBRUSH)GetStockObject(BLACK_BRUSH), 1, 1);
			DeleteObject(hrgn);
		}
		break;
	}
}

INT_PTR CALLBACK BoxPreviewWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_PAINT:
		if (GetUpdateRect(hwnd, nullptr, FALSE)) {
			PAINTSTRUCT ps;
			HDC mydc = BeginPaint(hwnd, &ps);
			BoxPreview_OnPaint(hwnd, mydc, GetWindowLongPtr(hwnd, GWLP_USERDATA));
			EndPaint(hwnd, &ps);
			return TRUE;
		}
		break;

	case WM_PRINT:
	case WM_PRINTCLIENT:
		BoxPreview_OnPaint(hwnd, (HDC)wParam, GetWindowLongPtr(hwnd, GWLP_USERDATA));
		return TRUE;

	case WM_LBUTTONDOWN:
		ReleaseCapture();
		SendMessage(hwnd, WM_SYSCOMMAND, 0xF012 /*SC_DRAGMOVE*/, 0);
		return TRUE;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
