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

===============================================================================

File name      : $HeadURL: http://svn.miranda.im/mainrepo/popup/trunk/src/opt_skins.cpp $
Revision       : $Revision: 1651 $
Last change on : $Date: 2010-07-15 20:31:06 +0300 (Ð§Ñ‚, 15 Ð¸ÑŽÐ» 2010) $
Last change by : $Author: Merlin_de $

===============================================================================
*/

#include "headers.h"

static volatile bool gPreviewOk = false;
static PopupWnd2 *wndPreview = NULL;

INT_PTR CALLBACK BoxPreviewWndProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

void RegisterOptPrevBox()
{
	DWORD err;
	WNDCLASSEX wcl;
	wcl.cbSize = sizeof(wcl);
	wcl.lpfnWndProc = (WNDPROC)BoxPreviewWndProc;
	wcl.style = IsWinVerXPPlus() ? CS_DROPSHADOW : 0;
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 0;
	wcl.hInstance = hInst;
	wcl.hIcon = NULL;
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.hbrBackground = NULL; //(HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wcl.lpszMenuName = NULL;
	wcl.lpszClassName = _T(BOXPREVIEW_WNDCLASS);
	wcl.hIconSm = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_POPUP), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	g_wndClass.cPopupPreviewBoxWndclass = RegisterClassEx(&wcl);
	err = GetLastError();
	if (!g_wndClass.cPopupPreviewBoxWndclass) {
		TCHAR msg[1024];
		wsprintf(msg, TranslateT("Failed to register %s class."),wcl.lpszClassName);
		MSGERROR(msg);
	}

	// register custom class for dialog box with drop-shadow attribute
	// "#32770" stays for class name of default system dialog box
	GetClassInfoEx(hInst, _T("#32770"), &wcl);
	wcl.hInstance = hInst;
	wcl.lpszClassName = _T("PopupPlusDlgBox");
	wcl.style |= IsWinVerXPPlus() ? CS_DROPSHADOW : 0;
	g_wndClass.cPopupPlusDlgBox = RegisterClassEx(&wcl);
	err = GetLastError();
	if (!g_wndClass.cPopupPlusDlgBox) {
		TCHAR msg[1024];
		wsprintf(msg, TranslateT("Failed to register %s class."),wcl.lpszClassName);
		MSGERROR(msg);
	}
}

static void updatePreviewImage(HWND hwndBox)
{
	gPreviewOk = false;

	POPUPDATA2 ppd;
	ZeroMemory(&ppd, sizeof(ppd));
	ppd.cbSize		= sizeof(ppd);
	ppd.flags		= PU2_TCHAR;
	ppd.lchIcon		= LoadSkinnedIcon(SKINICON_STATUS_ONLINE);
	ppd.lptzTitle	= TranslateT("Skin preview");
	ppd.lptzText	= TranslateT("Just take a look at this skin... ;)");

	POPUPOPTIONS customOptions = PopUpOptions;
	customOptions.DynamicResize = FALSE;
	customOptions.MinimumWidth = customOptions.MaximumWidth = 250;

	if (wndPreview) delete wndPreview;
	wndPreview = new PopupWnd2(&ppd, &customOptions, true);
	wndPreview->buildMText();
	wndPreview->update();
	gPreviewOk = true;

	InvalidateRect(hwndBox, NULL, TRUE);
}

static void DrawPreview(HWND hwnd, HDC hdc)
{
	RECT rc;
	HBRUSH hbr;

	BITMAPINFO bi;
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = 8;
	bi.bmiHeader.biHeight = -8;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	HBITMAP hBmpBrush = (HBITMAP)CreateDIBSection(0, &bi, DIB_RGB_COLORS, 0, 0, 0);
	HDC dcBmp = CreateCompatibleDC(0);
	HBITMAP hBmpSave = (HBITMAP)SelectObject(dcBmp, hBmpBrush);
	hbr = CreateSolidBrush(RGB(0xcc, 0xcc, 0xcc));
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
	SetBrushOrgEx(hdc, 1, 1, 0);
	FillRect(hdc, &rc, hbr);
	DeleteObject(hbr);
	DeleteObject(hBmpBrush);

	if (gPreviewOk)
	{
		int width = min(rc.right, wndPreview->getContent()->getWidth());
		int height = min(rc.bottom, wndPreview->getContent()->getHeight());
		int left = (rc.right - width) / 2;
		int top = (rc.bottom - height) / 2;

#if defined(_UNICODE)
		BLENDFUNCTION bf;
		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.SourceConstantAlpha = 255;
		bf.AlphaFormat = AC_SRC_ALPHA;
		AlphaBlend(hdc, left, top, width, height,
			wndPreview->getContent()->getDC(),
			0, 0, width, height, bf);
#else
		if (MyAlphaBlend) {
			BLENDFUNCTION bf;
			bf.BlendOp = AC_SRC_OVER;
			bf.BlendFlags = 0;
			bf.SourceConstantAlpha = 255;
			bf.AlphaFormat = AC_SRC_ALPHA;
			MyAlphaBlend(hdc, left, top, width, height,
				wndPreview->getContent()->getDC(),
				0, 0, width, height, bf);
		}
		else {
			BitBlt(hdc,
				left, top, left+width, top+height,
				wndPreview->getContent()->getDC(),
				0, 0, SRCCOPY);
		}
#endif
	}

	FrameRect(hdc, &rc, GetStockBrush(LTGRAY_BRUSH));
}

static WNDPROC WndProcPreviewBoxSave;
LRESULT CALLBACK WndProcPreviewBox(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (!wndPreview)
		return CallWindowProc(WndProcPreviewBoxSave, hwnd, msg, wParam, lParam);

	switch (msg)
	{
		case WM_PAINT:
		{
			if (GetUpdateRect(hwnd, 0, FALSE))
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hwnd, &ps);
				DrawPreview(hwnd, hdc);
				EndPaint(hwnd, &ps);
				return 0;
			}
		}

		case WM_PRINT:
		case WM_PRINTCLIENT:
		{
			HDC hdc = (HDC)wParam;
			DrawPreview(hwnd, hdc);
			return 0;
		}
	}
	return CallWindowProc(WndProcPreviewBoxSave, hwnd, msg, wParam, lParam);
}

int  SkinOptionList_AddSkin(OPTTREE_OPTION* &options, int *OptionsCount, int pos, DWORD *dwGlobalOptions) {
	const PopupSkin *skin = 0;
	LPTSTR pszName = NULL;
	if (skin = skins.getSkin(PopUpOptions.SkinPack)) {
		for (int i = 1; i <= 10; i++) {
			if(!skin->getFlagName(i))
				continue;
			*OptionsCount += 1;
			options = (OPTTREE_OPTION*)mir_realloc(options,sizeof(OPTTREE_OPTION)*(*OptionsCount));
			options[pos].dwFlag			= (DWORD)(1 << (i-1));
			options[pos].groupId		= OPTTREE_CHECK;
			options[pos].iconIndex		= 0;
			options[pos].pszSettingName	= mir_tstrdup(_T("Skin options"));
			options[pos].pszOptionName	= (LPTSTR)mir_alloc(sizeof(TCHAR)*(
				lstrlen(options[pos].pszSettingName)+
				lstrlenA(skin->getFlagName(i)) +10 ));
			wsprintf(options[pos].pszOptionName,_T("%s/%hs"), options[pos].pszSettingName, skin->getFlagName(i));
			options[pos].bState			= skin->getFlag(i) ? TRUE : FALSE;
			options[pos].Data			= i;	//skin flag index
			*dwGlobalOptions |= skin->getFlag(i) ? (1 << (i-1)) : 0;
			pos++;
		}
	}
	return pos;
}

int  SkinOptionList_AddMain(OPTTREE_OPTION* &options, int *OptionsCount, int pos, DWORD *dwGlobalOptions) {
	BOOL bCheck;
	LPTSTR mainOption [] = {
		LPGENT("Show clock"),
		LPGENT("Drop shadow effect (Windows XP+)"),
		LPGENT("Drop shadow effect (Windows XP+)/non rectangular"),
		LPGENT("Enable Aero Glass (Vista+)"),
		LPGENT("Use Windows colours"),
		LPGENT("Use advanced text render")};
	for (int i = 0; i < SIZEOF(mainOption); i++) {
		bCheck = 0;
		switch (i) {
			case 0:
				*dwGlobalOptions |= PopUpOptions.DisplayTime ? (1 << i) : 0;
				bCheck = PopUpOptions.DisplayTime;
				break;
			case 1:
				if(!IsWinVerXPPlus()) continue;
				*dwGlobalOptions |= PopUpOptions.DropShadow ? (1 << i) : 0;
				bCheck = PopUpOptions.DropShadow;
				break;
			case 2:
				if(!IsWinVerXPPlus()) continue;
				*dwGlobalOptions |= PopUpOptions.EnableFreeformShadows ? (1 << i) : 0;
				bCheck = PopUpOptions.EnableFreeformShadows;
				break;
			case 3:
				if(!MyDwmEnableBlurBehindWindow) continue;
				*dwGlobalOptions |= PopUpOptions.EnableAeroGlass ? (1 << i) : 0;
				bCheck = PopUpOptions.EnableAeroGlass;
				break;
			case 4:
				*dwGlobalOptions |= PopUpOptions.UseWinColors ? (1 << i) : 0;
				bCheck = PopUpOptions.UseWinColors;
				break;
			case 5:
				if(!(htuText&&htuTitle)) continue;
				*dwGlobalOptions |= PopUpOptions.UseMText ? (1 << i) : 0;
				bCheck = PopUpOptions.UseMText;
				break;
			default:
				break;
		}
		*OptionsCount += 1;
		options = (OPTTREE_OPTION*)mir_realloc(options,sizeof(OPTTREE_OPTION)*(*OptionsCount));
		options[pos].dwFlag		= (1 << i);
		options[pos].groupId	= OPTTREE_CHECK;
		options[pos].iconIndex	= 0;
		options[pos].pszSettingName = mir_tstrdup(_T("Global settings"));
		options[pos].pszOptionName	= (LPTSTR)mir_alloc(sizeof(TCHAR)*(
			lstrlen(options[pos].pszSettingName)+
			lstrlen(mainOption[i]) + 10));
		wsprintf(options[pos].pszOptionName,_T("%s/%s"), options[pos].pszSettingName, mainOption[i]);
		options[pos].bState			= bCheck;
		pos++;
	}
	return pos;
}

bool SkinOptionList_Update (OPTTREE_OPTION* &options, int *OptionsCount, HWND hwndDlg) {
	if (options) {
		int index = -1;
		OptTree_ProcessMessage(hwndDlg, WM_DESTROY, 0, 0, &index, IDC_SKIN_LIST_OPT, options, *OptionsCount);
		for (int i = 0; i < *OptionsCount; ++i) {
			mir_free(options[i].pszOptionName);
			mir_free(options[i].pszSettingName);
		}
		mir_free(options);
		options = NULL;
		*OptionsCount = 0;
	}
	int pos = 0;
	//add "Global options"
	DWORD dwGlobalOptions = 0;
	pos = SkinOptionList_AddMain(options, OptionsCount, pos, &dwGlobalOptions);
	//add "Skin options"
	DWORD dwSkinOptions = 0;
	pos = SkinOptionList_AddSkin(options, OptionsCount, pos, &dwSkinOptions);
	//generate treeview
	int index = -1;
	OptTree_ProcessMessage(hwndDlg, WM_INITDIALOG, 0, 0, &index, IDC_SKIN_LIST_OPT, options, *OptionsCount);

	//check "Skin options" state 
	char prefix[128];
	mir_snprintf(prefix, sizeof(prefix),"skin."TCHAR_STR_PARAM, PopUpOptions.SkinPack);
	OptTree_SetOptions(hwndDlg, IDC_SKIN_LIST_OPT, options, *OptionsCount,
		DBGetContactSettingDword(NULL, MODULNAME, prefix, dwSkinOptions), _T("Skin options"));

	//check "Global Settings"
	OptTree_SetOptions(hwndDlg, IDC_SKIN_LIST_OPT, options, *OptionsCount,
		dwGlobalOptions, _T("Global settings"));

	return true;
}

void LoadOption_Skins() {
	//skin pack
	PopUpOptions.SkinPack				= (LPTSTR)DBGetContactSettingStringX(NULL,MODULNAME, "SkinPack", "* Popup Classic",DBVT_TCHAR);
	//more Skin options
	PopUpOptions.DisplayTime			= DBGetContactSettingByte(NULL,MODULNAME, "DisplayTime", TRUE);
	PopUpOptions.DropShadow				= DBGetContactSettingByte(NULL,MODULNAME, "DropShadow", TRUE);
	PopUpOptions.EnableFreeformShadows	= DBGetContactSettingByte(NULL,MODULNAME, "EnableShadowRegion", 1);
	PopUpOptions.EnableAeroGlass		= DBGetContactSettingByte(NULL,MODULNAME, "EnableAeroGlass", 1);
	PopUpOptions.UseWinColors			= DBGetContactSettingByte(NULL,MODULNAME, "UseWinColors", FALSE);
	PopUpOptions.UseMText				= DBGetContactSettingByte(NULL,MODULNAME, "UseMText", TRUE);
}

INT_PTR CALLBACK DlgProcPopSkinsOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool bDlgInit = false;	//some controls send WM_COMMAND before or during WM_INITDIALOG
	static HANDLE hhkFontsReload = 0;
	static OPTTREE_OPTION *skinOptions = NULL;
	static int skinOptionsCount = 0;

	if (skinOptions) {
		int index = -1;
		OptTree_ProcessMessage(hwndDlg, msg, wParam, lParam, &index, IDC_SKIN_LIST_OPT, skinOptions, skinOptionsCount);
		if (index != -1) {
			if(lstrcmp(skinOptions[index].pszSettingName, _T("Skin options")) == 0) {
				const PopupSkin *skin = 0;
				if (skin = skins.getSkin(PopUpOptions.SkinPack)) {
					skin->setFlag(skinOptions[index].Data, skinOptions[index].bState ? true : false);
				}
			}
			else if (lstrcmp(skinOptions[index].pszSettingName, _T("Global settings")) == 0) {
				switch (skinOptions[index].dwFlag) {
					case (1 << 0):
						PopUpOptions.DisplayTime = skinOptions[index].bState;
						break;
					case (1 << 1):
						PopUpOptions.DropShadow = skinOptions[index].bState;
						break;
					case (1 << 2):
						PopUpOptions.EnableFreeformShadows = skinOptions[index].bState;
						break;
					case (1 << 3):
						PopUpOptions.EnableAeroGlass = skinOptions[index].bState;
						break;
					case (1 << 4):
						PopUpOptions.UseWinColors = skinOptions[index].bState;
						break;
					case (1 << 5):
						PopUpOptions.UseMText = skinOptions[index].bState;
						break;
					default:
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
			HWND		hCtrl	= NULL;
			DWORD		dwIndex	= 0;

			//Skin List
			hCtrl = GetDlgItem(hwndDlg, IDC_SKINLIST);
			ListBox_ResetContent(hCtrl);
			LPTSTR Temp = NULL;
			for (const Skins::SKINLIST *sl = skins.getSkinList(); sl; sl = sl->next)
			{
				dwIndex = ListBox_AddString(hCtrl, sl->name);
				ListBox_SetItemData(hCtrl, dwIndex, sl->name);
			}
			ListBox_SetCurSel(hCtrl, ListBox_FindString(hCtrl, 0, PopUpOptions.SkinPack));

			//Skin List reload button
			SendMessage(GetDlgItem(hwndDlg, IDC_BTN_RELOAD), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_BTN_RELOAD), BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIcon(ICO_OPT_RELOAD,0));
			SendMessage(GetDlgItem(hwndDlg, IDC_BTN_RELOAD), BUTTONADDTOOLTIP, (WPARAM)Translate("Refresh List"), 0);

			//Skin Option List
			SkinOptionList_Update (skinOptions, &skinOptionsCount, hwndDlg);

			//PreviewBox
			WndProcPreviewBoxSave = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_PREVIEWBOX), GWLP_WNDPROC, (LONG_PTR)WndProcPreviewBox);
			updatePreviewImage(GetDlgItem(hwndDlg, IDC_PREVIEWBOX));

			//hooks
			hhkFontsReload = HookEventMessage(ME_FONT_RELOAD, hwndDlg, WM_USER);

			TranslateDialogDefault(hwndDlg);
			bDlgInit = true;
			}
			return TRUE;

		case WM_USER:
			{
			updatePreviewImage(GetDlgItem(hwndDlg, IDC_PREVIEWBOX));
			}
			return TRUE;

		case WM_COMMAND: {
			HWND hCtrl	= NULL;
			UINT idCtrl	= LOWORD(wParam);
			switch (HIWORD(wParam)) {
				case BN_KILLFOCUS:		//Button controls
				case BN_SETFOCUS:		//Button controls
					return TRUE;
					break;
				case BN_CLICKED:		//Button controls
					switch(idCtrl)
					{
						case IDC_PREVIEW:
							{
							PopUpPreview();
							}
							break;
						case IDC_BTN_RELOAD:
							{
							LPTSTR Temp		= NULL;
							DWORD  dwIndex	= 0;
							TCHAR  szNewSkin[128];
							LPTSTR pszOldSkin = mir_tstrdup(PopUpOptions.SkinPack);
							skins.load(_T(""));
							hCtrl = GetDlgItem(hwndDlg, IDC_SKINLIST);
							ListBox_ResetContent(hCtrl);
							for (const Skins::SKINLIST *sl = skins.getSkinList(); sl; sl = sl->next)
							{
								dwIndex = ListBox_AddString(hCtrl, sl->name);
								ListBox_SetItemData(hCtrl, dwIndex, sl->name);
							}
							ListBox_SetCurSel(hCtrl, ListBox_FindString(hCtrl, 0, PopUpOptions.SkinPack));
							//make shure we have select skin (ListBox_SetCurSel may be fail)
							ListBox_GetText(hCtrl, ListBox_GetCurSel(hCtrl), &szNewSkin);
							if(lstrcmp(pszOldSkin, szNewSkin) != 0) {
								mir_free(PopUpOptions.SkinPack);
								PopUpOptions.SkinPack = mir_tstrdup(szNewSkin);
							}
							mir_free(pszOldSkin);

							const PopupSkin *skin = 0;
							if (skin = skins.getSkin(PopUpOptions.SkinPack)) {
								//update Skin Option List from reload SkinPack
								bDlgInit = false;
								bDlgInit = SkinOptionList_Update (skinOptions, &skinOptionsCount, hwndDlg);
							}

							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
							}//end IDC_BTN_RELOAD:
							break;
						case IDC_GETSKINS:
							CallService(MS_UTILS_OPENURL,0,(LPARAM)"http://addons.miranda-im.org/index.php?action=display&id=72");
							break;
						default:
							break;
					}//end switch(idCtrl)
					updatePreviewImage(GetDlgItem(hwndDlg, IDC_PREVIEWBOX));
					break;
				case CBN_SELCHANGE:		//combo box controls
					switch(idCtrl) {
						case IDC_SKINLIST: {
							//Skin list change
							mir_free(PopUpOptions.SkinPack);
							PopUpOptions.SkinPack = mir_tstrdup((TCHAR *)SendDlgItemMessage(
								hwndDlg,
								IDC_SKINLIST,
								LB_GETITEMDATA,
								(WPARAM)SendDlgItemMessage(hwndDlg, IDC_SKINLIST, LB_GETCURSEL,(WPARAM)0,(LPARAM)0),
								(LPARAM)0) );
							const PopupSkin *skin = 0;
							if (skin = skins.getSkin(PopUpOptions.SkinPack)) {
								mir_free(PopUpOptions.SkinPack);
								PopUpOptions.SkinPack = mir_tstrdup(skin->getName());

								//update Skin Option List
								bDlgInit = false;
								bDlgInit = SkinOptionList_Update (skinOptions, &skinOptionsCount, hwndDlg);
							}
							updatePreviewImage(GetDlgItem(hwndDlg, IDC_PREVIEWBOX));
							}
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
							break;
						default:
							break;
					}//end switch(idCtrl)
					break;
				default:
					break;
			}//end switch (HIWORD(wParam))
			break;
			}// end WM_COMMAND
			return FALSE;

		case WM_NOTIFY: {
			if(!bDlgInit) return FALSE;
			switch (((LPNMHDR)lParam)->idFrom) {
			case 0: {
				switch (((LPNMHDR)lParam)->code) {
				case PSN_RESET:
					LoadOption_Skins();
					return TRUE;
				case PSN_APPLY: 
					{	//skin pack
						DBWriteContactSettingTString(NULL, MODULNAME, "SkinPack", PopUpOptions.SkinPack);
						//skin options
						const PopupSkin *skin = 0;
						if (skin = skins.getSkin(PopUpOptions.SkinPack))
							skin->saveOpts();
						skins.freeAllButActive();
						//more Skin options
						DBWriteContactSettingByte(NULL, MODULNAME, "DisplayTime", PopUpOptions.DisplayTime);
						DBWriteContactSettingByte(NULL, MODULNAME, "DropShadow", PopUpOptions.DropShadow);
						DBWriteContactSettingByte(NULL, MODULNAME, "EnableShadowRegion", PopUpOptions.EnableFreeformShadows);
						DBWriteContactSettingByte(NULL, MODULNAME, "EnableAeroGlass", PopUpOptions.EnableAeroGlass);
						DBWriteContactSettingByte(NULL, MODULNAME, "UseMText", PopUpOptions.UseMText);
					}//end PSN_APPLY:
					return TRUE;
				default:
					break;
				}//switch (((LPNMHDR)lParam)->code)
				}// end case 0:
				break;
			default:
				break;
			}//end switch (((LPNMHDR)lParam)->idFrom)
			}//end WM_NOTIFY:
			return FALSE;

		case WM_DESTROY:
			{
				if (wndPreview) {
					delete wndPreview;
					wndPreview = NULL;
					gPreviewOk = false;
				}
				if (hhkFontsReload) UnhookEvent(hhkFontsReload);
				if (skinOptions) {
					for (int i = 0; i < skinOptionsCount; ++i) {
						mir_free(skinOptions[i].pszOptionName);
						mir_free(skinOptions[i].pszSettingName);
					}
					mir_free(skinOptions);
					skinOptions = NULL;
					skinOptionsCount = 0;
				}
			}
			return TRUE;

		default:
			break;
	}//end switch (msg)
	return FALSE;
}

static void BoxPreview_OnPaint(HWND hwnd, HDC mydc, int mode)
{
	switch (mode)
	{
		case 0:
		{ // Avatar
			HDC hdcAvatar = CreateCompatibleDC(mydc);
			HBITMAP hbmSave = (HBITMAP)SelectObject(hdcAvatar, hbmNoAvatar);
			RECT rc;
			GetClientRect(hwnd, &rc);
			BITMAP bmp;
			GetObject(hbmNoAvatar, sizeof(bmp), &bmp);
			StretchBlt(mydc, 0, 0, rc.right, rc.bottom, hdcAvatar, 0, 0, abs(bmp.bmWidth), abs(bmp.bmHeight), SRCCOPY);
			SelectObject(hdcAvatar, hbmSave);
			DeleteDC(hdcAvatar);
			HRGN rgn = CreateRoundRectRgn(0, 0, rc.right, rc.bottom, 2 * PopUpOptions.avatarRadius, 2 * PopUpOptions.avatarRadius);
			FrameRgn(mydc, rgn, (HBRUSH)GetStockObject(BLACK_BRUSH), 1, 1);
			DeleteObject(rgn);
			break;
		}
		case 1:
		{ // Opacity
			RECT rc;
			HBRUSH hbr = CreateSolidBrush(fonts.clBack);
			HFONT hfnt = (HFONT)SelectObject(mydc, fonts.title);
			GetClientRect(hwnd, &rc);
			FillRect(mydc, &rc, hbr);
			DrawIconEx(mydc, 10, (rc.bottom-rc.top-16)/2, IcoLib_GetIcon(ICO_POPUP_ON,0), 16, 16, 0, hbr, DI_NORMAL);
			SetBkMode(mydc, TRANSPARENT);
			GetClientRect(hwnd, &rc);
			rc.left += 30; // 10+16+4 -- icon
			rc.right -= (rc.right-rc.left)/3;
			rc.bottom -= (rc.bottom-rc.top)/3;
			DrawText(mydc, _T(MODULNAME_LONG), lstrlen(_T(MODULNAME_LONG)), &rc, DT_CENTER|DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER);
			GetClientRect(hwnd, &rc);
			rc.left += 30; // 10+16+4 -- icon
			rc.left += (rc.right-rc.left)/3;
			rc.top += (rc.bottom-rc.top)/3;
			DrawText(mydc, _T(MODULNAME_LONG), lstrlen(_T(MODULNAME_LONG)), &rc, DT_CENTER|DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER);
			GetClientRect(hwnd, &rc);
			FrameRect(mydc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
			SelectObject(mydc, hfnt);
			DeleteObject(hbr);
			break;
		}
		case 2:
		{ // Position
			RECT rc;
			HBRUSH hbr;
			hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
			GetClientRect(hwnd, &rc);
			FillRect(mydc, &rc, hbr);
			DeleteObject(hbr);

			hbr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
			GetClientRect(hwnd, &rc);
			rc.right -= 100;
			rc.top += 100;
			FillRect(mydc, &rc, hbr);
			DeleteObject(hbr);

			HPEN hpen = (HPEN)SelectObject(mydc, CreatePen(PS_DOT, 1, RGB(0,0,0)));
			MoveToEx(mydc,   0, 100, NULL);
			LineTo  (mydc, 201, 100);
			MoveToEx(mydc, 100,   0, NULL);
			LineTo  (mydc, 100, 201);
			DeleteObject(SelectObject(mydc, hpen));

			HRGN hrgn = CreateRectRgn(0,0,0,0);
			GetWindowRgn(hwnd, hrgn);
			FrameRgn(mydc, hrgn, (HBRUSH)GetStockObject(BLACK_BRUSH), 1, 1);
			DeleteObject(hrgn);

			break;
		}
	}
}

INT_PTR CALLBACK BoxPreviewWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_PAINT:
		{
			if (GetUpdateRect(hwnd, 0, FALSE))
			{
				PAINTSTRUCT ps;
				HDC mydc = BeginPaint(hwnd, &ps);
				BoxPreview_OnPaint(hwnd, mydc, GetWindowLongPtr(hwnd, GWLP_USERDATA));
				EndPaint(hwnd, &ps);
				return TRUE;
			}
			break;
		}

		case WM_PRINT:
		case WM_PRINTCLIENT:
		{
			HDC mydc = (HDC)wParam;
			BoxPreview_OnPaint(hwnd, mydc, GetWindowLongPtr(hwnd, GWLP_USERDATA));
			return TRUE;
		}

		case WM_LBUTTONDOWN:
		{
			ReleaseCapture();
			SendMessage(hwnd, WM_SYSCOMMAND, 0xF012 /*SC_DRAGMOVE*/, 0);
			return TRUE;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
