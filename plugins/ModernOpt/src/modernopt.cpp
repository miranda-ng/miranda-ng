/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-07 Miranda ICQ/IM project,
Copyright 2007 Artem Shpynov

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

#include "commonheaders.h"

//#define MOPT_SAMPLE_PAGES

extern HMODULE hInst;

static HANDLE hevtModernOpt_Initialize = 0;

static HWND hwndModernOpt = NULL;
static INT_PTR CALLBACK ModernOptDlgProc(HWND hwndDlg, UINT  msg, WPARAM wParam, LPARAM lParam);

struct ModernOptionsObject
{
	MODERNOPTOBJECT	optObject;

	HWND			hwnd;
	BOOL			bChanged;
	DWORD			dwFlags;
	int				iTreeImageIndex;
	DWORD			dwIdx;
};

static DWORD g_dwIdx = 0;
static int g_iSectionRestore = 0;

static int ModernOptionsObject_Comparator(const ModernOptionsObject *ptr1, const ModernOptionsObject *ptr2);

struct ModernOptionsData : public MZeroedObject
{
	ModernOptionsData(): pObjectList(1, ModernOptionsObject_Comparator) {}

	LIST<ModernOptionsObject> pObjectList;
	HFONT			hfntBold;
	int				iPage, iSection;
};

////////////////////////////////////////////////////////////////////////////////
// Forwards
static void ModernOptUI_ShowPage(HWND hwndDlg, struct ModernOptionsData *dat, int iPage);
static void ModernOptUI_SelectSection(HWND hwndDlg, struct ModernOptionsData *dat, int iSection);

////////////////////////////////////////////////////////////////////////////////
// Main option pages
struct ModernOptMainPageInfo g_ModernOptPages[MODERNOPT_PAGE_COUNT] =
{
	{0,					FALSE,	IDI_MIRANDA,		LPGENT("Home"),			LPGENT("Miranda NG configuration center")},
	{IDC_BTN_ACCOUNTS,	TRUE,	IDI_BIG_NETWORK,	LPGENT("Accounts"),		LPGENT("Setup your account information to start messaging.")},
	{IDC_BTN_SKINS,		TRUE,	IDI_BIG_SKINS,		LPGENT("Style"),		LPGENT("Change the look of Miranda NG according to your taste.")},
	{IDC_BTN_CLIST,		TRUE,	IDI_BIG_BUDDYLIST,	LPGENT("Contacts"),		LPGENT("Configure behavior of your contact list.")},
	{IDC_BTN_MESSAGING,	TRUE,	IDI_BIG_MESSAGE,	LPGENT("Chats"),		LPGENT("Customize look&&feel of your chat windows here.")},
	{IDC_BTN_IGNORE,	TRUE,	IDI_BIG_IGNORE,		LPGENT("Ignore"),		LPGENT("Ban those users and events, you are annoyed with.")},
	{IDC_BTN_STATUS,	TRUE,	IDI_BIG_STATUS,		LPGENT("Status"),		LPGENT("Set your status messages and idle reporting.")},
	{IDC_BTN_ADVANCED,	TRUE,	IDI_BIG_ADVANCED,	LPGENT("Advanced"),		LPGENT("")},
	{IDC_BTN_PLUGINS,	TRUE,	IDI_BIG_MODULES,	LPGENT("Addons"),		LPGENT("Miranda NG is all about plugins. Manage all the plugins you have here.")},
};

////////////////////////////////////////////////////////////////////////////////
// Utilities
typedef void (*ItemDestuctor)(void *);

static void ModernOptionsObject_Dtor(void *ptr)
{
	ModernOptionsObject *obj = (struct ModernOptionsObject *)ptr;

	mir_free(obj->optObject.lptzSubsection);
	mir_free(obj->optObject.iBoldControls);
	mir_free(obj->optObject.lpzClassicGroup);
	mir_free(obj->optObject.lpzClassicPage);
	mir_free(obj->optObject.lpzClassicTab);
	mir_free(obj->optObject.lpzHelpUrl);
	mir_free(obj->optObject.lpzThemeExtension);
	mir_free(obj->optObject.lpzThemeModuleName);

	switch (obj->optObject.iType) {
		case MODERNOPT_TYPE_IGNOREOBJECT:
			mir_free(obj->optObject.lpzIgnoreModule);
			mir_free(obj->optObject.lpzIgnoreSetting);
			break;
	}

	if (obj->hwnd) DestroyWindow(obj->hwnd);
	mir_free(obj);
}

static int ModernOptionsObject_Comparator(const ModernOptionsObject *ptr1, const ModernOptionsObject *ptr2)
{
	struct ModernOptionsObject *obj1 = ((struct ModernOptionsObject *)ptr1);
	struct ModernOptionsObject *obj2 = ((struct ModernOptionsObject *)ptr2);

	if (obj1->optObject.iSection < obj2->optObject.iSection) return -1;
	if (obj1->optObject.iSection > obj2->optObject.iSection) return +1;

	if (obj1->optObject.iType < obj2->optObject.iType) return -1;
	if (obj1->optObject.iType > obj2->optObject.iType) return +1;

	if (obj1->optObject.lptzSubsection && obj2->optObject.lptzSubsection)
	{
		int c = lstrcmp(obj1->optObject.lptzSubsection, obj2->optObject.lptzSubsection);
		if (c) return c;
	}

	if (obj1->dwIdx < obj2->dwIdx) return -1;
	if (obj1->dwIdx > obj2->dwIdx) return +1;
	return 0;
}

void li_List_Destruct(LIST<ModernOptionsObject> &pList, ItemDestuctor pItemDestructor)
{
	for (int i=0; i<pList.getCount(); i++)
		pItemDestructor(pList[i]);
	pList.destroy();
}
////////////////////////////////////////////////////////////////////////////////
// Main dlgproc
static void sttNotifyPages(struct ModernOptionsData *dat, int code)
{
	PSHNOTIFY pshn = {0};
	pshn.hdr.code = code;
	for (int i = 0; i < dat->pObjectList.getCount(); ++i)
	{
		struct ModernOptionsObject *obj = (struct ModernOptionsObject *)dat->pObjectList[i];
		if (obj->hwnd && obj->bChanged)
		{
			obj->bChanged = FALSE;
			pshn.hdr.hwndFrom = obj->hwnd;
			SendMessage(obj->hwnd, WM_NOTIFY, 0, (LPARAM)&pshn);
		}
	}
}

static INT_PTR CALLBACK ModernOptDlgProc(HWND hwndDlg, UINT  msg, WPARAM wParam, LPARAM lParam)
{
	struct ModernOptionsData *dat = (struct ModernOptionsData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	int i;
	HWND hwndCtrl;

	switch (msg) {
	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwndDlg);

		dat = (struct ModernOptionsData *)lParam;
		dat->iPage = -1;

		LOGFONT lf;
		dat->hfntBold = (HFONT)SendMessage(hwndDlg, WM_GETFONT, 0, 0);
		GetObject(dat->hfntBold, sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		dat->hfntBold = CreateFontIndirect(&lf);

		hwndCtrl = GetDlgItem(hwndDlg, IDC_TV_SUBSECTIONS);
		HIMAGELIST himl = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 2, 1);
		TreeView_SetImageList(hwndCtrl, himl, TVSIL_NORMAL);

		for (i = 0; i < dat->pObjectList.getCount(); ++i) {
			struct ModernOptionsObject *obj = (struct ModernOptionsObject *)dat->pObjectList[i];
			if ((obj->optObject.iSection >= 0) && (obj->optObject.iSection < MODERNOPT_PAGE_COUNT))
				g_ModernOptPages[obj->optObject.iSection].bShow = TRUE;
			if (obj->optObject.hIcon)
				obj->iTreeImageIndex = ImageList_AddIcon(himl, obj->optObject.hIcon);
		}

		hwndCtrl = GetDlgItem(hwndDlg, IDC_ICOTABS);
		for (i = 0; i < SIZEOF(g_ModernOptPages); ++i)
			if (g_ModernOptPages[i].bShow && g_ModernOptPages[i].bShowTab)
			{
				HICON hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(g_ModernOptPages[i].iIcon), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
				MIcoTab_AddItem(hwndCtrl, TranslateTS(g_ModernOptPages[i].lpzTitle), hIcon, i, FALSE);
				DestroyIcon(hIcon);
			}

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);

		if (g_iSectionRestore)
			CallService(MS_MODERNOPT_SELECTPAGE, g_iSectionRestore, 0);
		else
			ModernOptUI_SelectSection(hwndDlg, dat, g_iSectionRestore);

		Utils_RestoreWindowPositionNoSize(hwndDlg, NULL, "Options", "");

		return TRUE;
	}

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ICOTABS:
			{
				int iPage = MIcoTab_GetItemData(GetDlgItem(hwndDlg, IDC_ICOTABS), lParam);
				ModernOptUI_SelectSection(hwndDlg, dat, iPage);
				SetFocus(GetDlgItem(hwndDlg, IDC_ICOTABS));
			}
			break;

		case IDC_BTN_CLASSICOPT:
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			db_set_b(NULL, "Options", "Expert", 1);
			{
				OPENOPTIONSDIALOG ood = { sizeof(ood) };
				Options_Open(&ood);
			}
			break;

		case IDC_BTN_HELP:
			{
				struct ModernOptionsObject *obj = (struct ModernOptionsObject *)dat->pObjectList[dat->iPage];
				if (obj->optObject.lpzHelpUrl)
					CallService(MS_UTILS_OPENURL,0,(LPARAM)obj->optObject.lpzHelpUrl);
			}
			break;

		case IDC_BTN_EXPERT:
			if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BTN_APPLY)))
			{
				int idResult = MessageBox(hwndDlg,
					TranslateT("You have some unsaved changes here.\n Do you wish to apply settings before switching?"),
					_T("Miranda NG"), MB_ICONQUESTION | MB_YESNOCANCEL);
				if (idResult == IDCANCEL) break;
				sttNotifyPages(dat, (idResult == IDYES) ? PSN_APPLY : PSN_RESET);
			}

			{	Utils_SaveWindowPosition(hwndDlg, NULL, "Options", "");
				g_iSectionRestore = dat->iSection;
				struct ModernOptionsObject *obj = (struct ModernOptionsObject *)dat->pObjectList[dat->iPage];
				if (obj->optObject.lpzClassicGroup || obj->optObject.lpzClassicPage)
				{
					OPENOPTIONSDIALOG ood = {0};
					ood.cbSize = sizeof(ood);
					ood.pszGroup = obj->optObject.lpzClassicGroup;
					ood.pszPage = obj->optObject.lpzClassicPage;
					ood.pszTab = obj->optObject.lpzClassicTab;
					HWND hwndOpt = Options_OpenPage(&ood);
					PostMessage(hwndDlg, WM_CLOSE, 0, 0);
				}
				else {
					CallService("Options/OptionsCommand", 0, 0);
					PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			}	}
			break;

		case IDOK:
			sttNotifyPages(dat, PSN_APPLY);
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			break;

		case IDCANCEL:
			sttNotifyPages(dat, PSN_RESET);
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			break;

		case IDC_BTN_APPLY:
			EnableWindow(GetDlgItem(hwndDlg,IDC_BTN_APPLY), FALSE);
			sttNotifyPages(dat, PSN_APPLY);
			break;
		}

		return TRUE;

	case PSM_CHANGED:
		EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_APPLY), TRUE);
		for (i = 0; i < dat->pObjectList.getCount(); ++i) {
			struct ModernOptionsObject *obj = (struct ModernOptionsObject *)dat->pObjectList[i];
			if (obj->hwnd && IsWindowVisible(obj->hwnd))
				obj->bChanged = TRUE;
		}
		break;

	case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;
 			if (lpnmhdr->idFrom == IDC_TV_SUBSECTIONS ) {
				LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)lParam;
				if (lpnmhdr->code == TVN_SELCHANGED ) {
					ModernOptUI_ShowPage(hwndDlg, dat, lpnmtv->itemNew.lParam);
					break;
				}
				break;
			}
			return TRUE;
		}

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		hwndModernOpt = 0;
		return TRUE;

	case WM_DESTROY:
		if (!dat) return TRUE;
		li_List_Destruct(dat->pObjectList, ModernOptionsObject_Dtor);
		DeleteObject(dat->hfntBold);
		ImageList_Destroy(TreeView_SetImageList(GetDlgItem(hwndDlg, IDC_TV_SUBSECTIONS), NULL, TVSIL_NORMAL));
		Utils_SaveWindowPosition(hwndDlg, NULL, "Options", "");
		delete dat;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// UI utilities
static HWND ModernOptUI_ShowPage_Impl(HWND hwndDlg, struct ModernOptionsData *dat, int iPage, int dx, HWND hwndInsertAfter)
{
	if ((iPage < 0) || (iPage >= dat->pObjectList.getCount()))
		return NULL;

	dat->iPage = iPage;
	struct ModernOptionsObject *obj = (struct ModernOptionsObject *)dat->pObjectList[dat->iPage];
	if (!obj->hwnd) {
		RECT rc1, rc2;
		GetWindowRect(GetDlgItem(hwndDlg, IDC_TV_SUBSECTIONS), &rc1);
		GetWindowRect(GetDlgItem(hwndDlg, IDC_TXT_DUMMY), &rc2);
		MapWindowPoints(NULL, hwndDlg, (LPPOINT)&rc1, 2);
		MapWindowPoints(NULL, hwndDlg, (LPPOINT)&rc2, 2);
		rc1.top += dx; rc2.top += dx;

		obj->hwnd = CreateDialogParamA(obj->optObject.hInstance, obj->optObject.lpzTemplate, hwndDlg, obj->optObject.pfnDlgProc, (LPARAM)&obj->optObject);
		if (obj->hwnd) {
			DWORD dwShowFlags =
				SWP_SHOWWINDOW |
				((obj->optObject.dwFlags & MODEROPT_FLG_NORESIZE) ? SWP_NOSIZE : 0);

			int i = 0;

			if (obj->optObject.iBoldControls)
				for (i = 0; obj->optObject.iBoldControls[i]; ++i) {
					HWND hwndChild = GetDlgItem(obj->hwnd, obj->optObject.iBoldControls[i]);
					if (!hwndChild) continue;
					SendMessage(hwndChild, WM_SETFONT, (WPARAM)dat->hfntBold, TRUE);
				}

			if (obj->optObject.iType == MODERNOPT_TYPE_SECTIONPAGE)
				SetWindowPos(obj->hwnd, hwndInsertAfter, rc1.left, rc1.top, rc2.right-rc1.left, rc2.bottom-rc1.top, dwShowFlags);
			else
				SetWindowPos(obj->hwnd, hwndInsertAfter, rc2.left, rc2.top, rc2.right-rc2.left, rc2.bottom-rc2.top, dwShowFlags);

			if (obj->optObject.iSection == MODERNOPT_PAGE_IGNORE) {
				for (i = 0; i < dat->pObjectList.getCount(); ++i) {
					struct ModernOptionsObject *ignoreObj = (struct ModernOptionsObject *)dat->pObjectList[i];
					if (ignoreObj->optObject.iType == MODERNOPT_TYPE_IGNOREOBJECT)
						ModernOptIgnore_AddItem(&ignoreObj->optObject);
		}	}	}
	}
	else ShowWindow(obj->hwnd, SW_SHOW);

	ShowWindow(GetDlgItem(hwndDlg, IDC_BTN_EXPERT), (obj->optObject.lpzClassicGroup || obj->optObject.lpzClassicPage) ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_BTN_HELP), obj->optObject.lpzHelpUrl ? SW_SHOW : SW_HIDE);

	return obj->hwnd;
}

static int lstrcmp_null(TCHAR *p1, TCHAR *p2)
{
	if (!p1 && !p2) return 0;
	if (!p1) return -1;
	if (!p2) return 1;
	return lstrcmp(p1, p2);
}

static void ModernOptUI_ShowPage(HWND hwndDlg, struct ModernOptionsData *dat, int iPage)
{
	HWND hwndInsertAfter = GetDlgItem(hwndDlg, IDC_TV_SUBSECTIONS);

	int dx = 0;
	for (int i = 0; i < dat->pObjectList.getCount(); ++i)
		if ((dat->pObjectList[i]->optObject.iType == dat->pObjectList[iPage]->optObject.iType) &&
			(dat->pObjectList[i]->optObject.iSection == dat->pObjectList[iPage]->optObject.iSection) &&
			!lstrcmp_null(dat->pObjectList[i]->optObject.lptzSubsection, dat->pObjectList[iPage]->optObject.lptzSubsection))
		{
			hwndInsertAfter = ModernOptUI_ShowPage_Impl(hwndDlg, dat, i, dx, hwndInsertAfter);

			if (dat->pObjectList[i]->hwnd) {
				RECT rcWnd; GetWindowRect(dat->pObjectList[i]->hwnd, &rcWnd);
				dx += rcWnd.bottom - rcWnd.top;// + 30;
			}
		}
		else if (dat->pObjectList[i]->hwnd)
		{
			ShowWindow(dat->pObjectList[i]->hwnd, SW_HIDE);
		}
}

static void ModernOptUI_SelectSection(HWND hwndDlg, struct ModernOptionsData *dat, int iSection)
{
	int iPageType = -1;

	HWND hwndTree = GetDlgItem(hwndDlg, IDC_TV_SUBSECTIONS);
	dat->iSection = iSection;

	SendMessage(hwndTree, WM_SETREDRAW, FALSE, 0);
	TreeView_DeleteAllItems(hwndTree);
	for (int i = 0; i < dat->pObjectList.getCount(); ++i) {
		struct ModernOptionsObject *obj = (struct ModernOptionsObject *)dat->pObjectList[i];
		if (obj->optObject.iSection != iSection) continue;

		if ((iPageType < -1) && (iPageType != obj->optObject.iType)) continue;
		iPageType = obj->optObject.iType;

		if (obj->optObject.iType == MODERNOPT_TYPE_SECTIONPAGE) {
			ModernOptUI_ShowPage(hwndDlg, dat, i);
			break;
		}

		if (obj->optObject.iType == MODERNOPT_TYPE_SUBSECTIONPAGE) {
			TVINSERTSTRUCT tvis = {0};
			tvis.hParent = TVI_ROOT;
			tvis.hInsertAfter = TVI_LAST;
			tvis.item.mask = TVIF_IMAGE|TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM;
			tvis.item.pszText = TranslateTS(obj->optObject.lptzSubsection);
			tvis.item.iImage = tvis.item.iSelectedImage = obj->iTreeImageIndex;
			tvis.item.lParam = i;
			TreeView_InsertItem(hwndTree, &tvis);
	}	}

	SendMessage(hwndTree, WM_SETREDRAW, TRUE, 0);

	if (iPageType < 0)
		return;

	if (iPageType != MODERNOPT_TYPE_SECTIONPAGE) {
		ShowWindow(hwndTree, SW_SHOW);
		RedrawWindow(hwndTree, NULL, NULL, RDW_INVALIDATE);
		TreeView_Select(hwndTree, TreeView_GetRoot(hwndTree), TVGN_CARET);
	}
	else ShowWindow(hwndTree, SW_HIDE);
}

////////////////////////////////////////////////////////////////////////////////
// Services
static INT_PTR svcModernOpt_Impl(WPARAM wParam, LPARAM lParam)
{
	if (!hwndModernOpt) {
		ModernOptionsData *dat = new ModernOptionsData;
		g_dwIdx = 0;
		NotifyEventHooks(hevtModernOpt_Initialize, (WPARAM)dat, 0);
		hwndModernOpt = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_MODERNOPT), NULL, ModernOptDlgProc, (LPARAM)dat);
		ShowWindow(hwndModernOpt, SW_SHOW);
	}
	else SetForegroundWindow(hwndModernOpt);

	return 0;
}

static INT_PTR svcModernOpt_Show(WPARAM wParam, LPARAM lParam)
{
	if ( db_get_b(NULL, "Options", "Expert", 0)) {
		OPENOPTIONSDIALOG ood = { sizeof(ood) };
		return Options_Open(&ood);
	}

	g_iSectionRestore = 0;
	return svcModernOpt_Impl(wParam, lParam);
}

static INT_PTR svcModernOpt_Restore(WPARAM wParam, LPARAM lParam)
{
	if (g_iSectionRestore == 0) return 0;
	return svcModernOpt_Impl(wParam, lParam);
}

static INT_PTR svcModernOpt_SelectPage(WPARAM wParam, LPARAM)
{
	if (!hwndModernOpt)
		return 0;

	struct ModernOptionsData *dat = (struct ModernOptionsData *)GetWindowLongPtr(hwndModernOpt, GWLP_USERDATA);
	if (!dat)
		return 0;

	int nIdx = 0;
	for (size_t i = 0; i < MODERNOPT_PAGE_COUNT; ++i)
		if (g_ModernOptPages[i].bShow && g_ModernOptPages[i].bShowTab) {
			if (i == wParam) {
				MIcoTab_SetSel(GetDlgItem(hwndModernOpt, IDC_ICOTABS), nIdx);
				break;
			}

			++nIdx;
		}

//	ModernOptUI_SelectSection(hwndModernOpt, dat, wParam);
	return 0;
}

static INT_PTR svcModernOpt_AddObject(WPARAM wParam, LPARAM lParam)
{
	struct ModernOptionsData *dat = (struct ModernOptionsData *)wParam;
	MODERNOPTOBJECT *obj = (MODERNOPTOBJECT *)lParam;
	struct ModernOptionsObject *objCopy = (struct ModernOptionsObject *)mir_calloc(sizeof(struct ModernOptionsObject));

	objCopy->dwIdx = ++g_dwIdx;

	objCopy->optObject.cbSize		= sizeof(MODERNOPTOBJECT);
	objCopy->optObject.dwFlags		= obj->dwFlags;
	objCopy->optObject.hIcon		= obj->hIcon;
	objCopy->optObject.iSection		= obj->iSection;
	objCopy->optObject.iType		= obj->iType;
	objCopy->optObject.hInstance	= obj->hInstance;
	objCopy->optObject.lpzTemplate	= obj->lpzTemplate;
	objCopy->optObject.pfnDlgProc	= obj->pfnDlgProc;

	if (obj->lpzClassicGroup)	objCopy->optObject.lpzClassicGroup	= mir_strdup(obj->lpzClassicGroup);
	if (obj->lpzClassicPage)	objCopy->optObject.lpzClassicPage	= mir_strdup(obj->lpzClassicPage);
	if (obj->lpzClassicTab)		objCopy->optObject.lpzClassicTab	= mir_strdup(obj->lpzClassicTab);
	if (obj->lpzHelpUrl)		objCopy->optObject.lpzHelpUrl		= mir_strdup(obj->lpzHelpUrl);

	if (obj->iBoldControls) {
		int count = 0;
		while (obj->iBoldControls[count++]) ;
		objCopy->optObject.iBoldControls = (int *)mir_alloc(sizeof(int) * count);
		memcpy(objCopy->optObject.iBoldControls, obj->iBoldControls, sizeof(int) * count);
	}

	switch (obj->iType) {
	case MODERNOPT_TYPE_SECTIONPAGE:
		objCopy->optObject.lptzSubsection = NULL;
		break;

	case MODERNOPT_TYPE_SUBSECTIONPAGE:
		objCopy->optObject.lptzSubsection = (objCopy->optObject.dwFlags & MODEROPT_FLG_UNICODE) ?
			mir_u2t(obj->lpwzSubsection) :
			mir_a2t(obj->lpzSubsection);
		break;

	case MODERNOPT_TYPE_IGNOREOBJECT:
		objCopy->optObject.lptzSubsection = (objCopy->optObject.dwFlags & MODEROPT_FLG_UNICODE) ?
			mir_u2t(obj->lpwzSubsection) :
			mir_a2t(obj->lpzSubsection);
		objCopy->optObject.lpzIgnoreModule = mir_strdup(obj->lpzIgnoreModule);
		objCopy->optObject.lpzIgnoreSetting = mir_strdup(obj->lpzIgnoreSetting);
		objCopy->optObject.dwIgnoreBit = obj->dwIgnoreBit;
		break;

	case MODERNOPT_TYPE_SELECTORPAGE:
		objCopy->optObject.iType = MODERNOPT_TYPE_SUBSECTIONPAGE;
		objCopy->optObject.hInstance = hInst;
		objCopy->optObject.lpzTemplate = MAKEINTRESOURCEA(IDD_MODERNOPT_SKINS);
		objCopy->optObject.pfnDlgProc = ModernOptSelector_DlgProc;
		objCopy->optObject.lptzSubsection = (objCopy->optObject.dwFlags & MODEROPT_FLG_UNICODE) ?
			mir_u2t(obj->lpwzSubsection) :
			mir_a2t(obj->lpzSubsection);
		objCopy->optObject.lpzThemeExtension = mir_strdup(obj->lpzThemeExtension);
		objCopy->optObject.lpzThemeModuleName = mir_strdup(obj->lpzThemeModuleName);
		break;
	}

	dat->pObjectList.insert(objCopy);
	return 0;
}

static int hookModernOpt_Initialize(WPARAM wParam, LPARAM)
{
	static int iBoldControls[] =
	{
		IDC_TXT_TITLE1, IDC_TXT_TITLE2, IDC_TXT_TITLE3, IDC_TXT_TITLE4,
//		IDC_TXT_TITLE5, IDC_TXT_TITLE6, IDC_TXT_TITLE7, IDC_TXT_TITLE8,
		MODERNOPT_CTRL_LAST
	};

	MODERNOPTOBJECT obj = {0};

	obj.cbSize = sizeof(obj);
	obj.dwFlags = MODEROPT_FLG_TCHAR;

	obj.hIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	obj.hInstance = hInst;

	obj.iSection = MODERNOPT_PAGE_HOME;
	obj.iType = MODERNOPT_TYPE_SECTIONPAGE;
	obj.lpzTemplate = MAKEINTRESOURCEA(IDD_MODERNOPT_HOME);
	obj.pfnDlgProc = ModernOptHome_DlgProc;
	obj.lpzHelpUrl = "http://wiki.miranda-ng.org/";
	CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);

	#if defined(_DEBUG) && defined(MOPT_SAMPLE_PAGES)
		obj.iSection = MODERNOPT_PAGE_ADVANCED;
		obj.iType = MODERNOPT_TYPE_SUBSECTIONPAGE;
		obj.lptzSubsection = _T("Page 1");
		CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);
		obj.lptzSubsection = _T("Page 2");
		CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);
		obj.lptzSubsection = _T("Page 3");
		CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);

		obj.iType = MODERNOPT_TYPE_IGNOREOBJECT;
		obj.hIcon = LoadSkinnedIcon(SKINICON_EVENT_FILE);
		obj.lptzSubsection = _T("Files");
		CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);
		obj.hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
		obj.lptzSubsection = _T("Messages");
		CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);
		obj.hIcon = LoadSkinnedIcon(SKINICON_EVENT_URL);
		obj.lptzSubsection = _T("URL events");
		CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);
		obj.hIcon = LoadSkinnedIcon(SKINICON_OTHER_TYPING);
		obj.lptzSubsection = _T("Typing notifications");
		CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);
		obj.hIcon = LoadSkinnedIcon(SKINICON_OTHER_ADDCONTACT);
		obj.lptzSubsection = _T("Added notifications");
		CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);
		obj.hIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
		obj.lptzSubsection = _T("Auth requests");
		CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);

		obj.iSection = MODERNOPT_PAGE_SKINS;
		obj.iType = MODERNOPT_TYPE_SECTIONPAGE;
		obj.lpzTemplate = MAKEINTRESOURCEA(IDD_MODERNOPT_SKINS);
		obj.pfnDlgProc = NULL;
		obj.iBoldControls = iBoldControls;
		obj.lpzHelpUrl = NULL;
		CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);

		obj.lpzTemplate = MAKEINTRESOURCEA(IDD_MODERNOPT_EMPTY);
		obj.pfnDlgProc = NULL;
		obj.iBoldControls = 0;
		obj.lpzClassicGroup = 0;
		obj.lpzClassicPage = 0;
		obj.lpzHelpUrl = 0;

		obj.iSection = MODERNOPT_PAGE_CLIST;
		obj.iType = MODERNOPT_TYPE_SECTIONPAGE;
		CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);

		obj.iSection = MODERNOPT_PAGE_MSGS;
		obj.iType = MODERNOPT_TYPE_SECTIONPAGE;
		CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);

		obj.iSection = MODERNOPT_PAGE_ADVANCED;
		obj.iType = MODERNOPT_TYPE_SUBSECTIONPAGE;
		obj.lptzSubsection = _T("Page 1");
		CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);
		obj.lptzSubsection = _T("Page 2");
		CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);
		obj.lptzSubsection = _T("Page 3");
		CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);

		obj.iSection = MODERNOPT_PAGE_MODULES;
		obj.iType = MODERNOPT_TYPE_SUBSECTIONPAGE;
		obj.lptzSubsection = _T("Configure");
		CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);
		obj.lptzSubsection = _T("Update");
		CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);
		obj.lptzSubsection = _T("Download");
		CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);
	#endif

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Load module
void LoadModernOptsModule()
{
	CreateServiceFunction(MS_MODERNOPT_SHOW, svcModernOpt_Show);
	CreateServiceFunction(MS_MODERNOPT_RESTORE, svcModernOpt_Restore);
	CreateServiceFunction(MS_MODERNOPT_SELECTPAGE, svcModernOpt_SelectPage);
	CreateServiceFunction(MS_MODERNOPT_ADDOBJECT, svcModernOpt_AddObject);
	hevtModernOpt_Initialize = CreateHookableEvent(ME_MODERNOPT_INITIALIZE);
	HookEvent(ME_MODERNOPT_INITIALIZE, hookModernOpt_Initialize);
}

////////////////////////////////////////////////////////////////////////////////
// Unload module
void UnloadModernOptsModule()
{
	if (hwndModernOpt)
	{
		DestroyWindow(hwndModernOpt);
		hwndModernOpt = 0;
	}
}
