/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-07 Miranda ICQ/IM project,
Copyright (c) 2007 Artem Shpynov

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

static void sttApplySkin(MODERNOPTOBJECT *obj, TCHAR *fn)
{
	ProtoCallService(obj->lpzThemeModuleName, TS_SKIN_APPLY, NULL, (LPARAM)fn);
}

static TCHAR *sttGetActiveSkin(MODERNOPTOBJECT *obj)
{
	return ProtoServiceExists(obj->lpzThemeModuleName, TS_SKIN_ACTIVE) ?
		(TCHAR*)ProtoCallService(obj->lpzThemeModuleName, TS_SKIN_ACTIVE, 0, 0) : 0;
}

static void sttPreviewSkin(MODERNOPTOBJECT *obj, TCHAR *fn, LPDRAWITEMSTRUCT lps)
{
	if (!fn) return;

	if ( ProtoServiceExists(obj->lpzThemeModuleName, TS_SKIN_PREVIEW)) {
		ProtoCallService(obj->lpzThemeModuleName, TS_SKIN_PREVIEW, (WPARAM)lps, (LPARAM)fn);
		return;
	}

	char *afn = mir_t2a(fn);
	char *fnpreview = (char *)mir_alloc(mir_strlen(afn) + 10);
	mir_strcpy(fnpreview, afn);
	mir_strcat(fnpreview, ".png");
	HBITMAP hbmPreview = (HBITMAP)CallService(MS_UTILS_LOADBITMAP, 0, (LPARAM)fnpreview);
	mir_free(afn);
	mir_free(fnpreview);

	if (!hbmPreview) return;

	BITMAP bmp;
	GetObject(hbmPreview, sizeof(bmp), &bmp);

	SIZE szDst = { abs(bmp.bmWidth), abs(bmp.bmHeight) };
	if ((szDst.cx > lps->rcItem.right-lps->rcItem.left) || (szDst.cy > lps->rcItem.bottom-lps->rcItem.top)) {
		float q = min(
			float(lps->rcItem.right-lps->rcItem.left) / szDst.cx,
			float(lps->rcItem.bottom-lps->rcItem.top) / szDst.cy);
		szDst.cx *= q;
		szDst.cy *= q;
	}
	POINT ptDst = {
		(lps->rcItem.left+lps->rcItem.right-szDst.cx) / 2,
		(lps->rcItem.top+lps->rcItem.bottom-szDst.cy) / 2 };

	HDC hdc = CreateCompatibleDC(lps->hDC);
	SelectObject(hdc, hbmPreview);
	SetStretchBltMode(hdc, HALFTONE);
	StretchBlt(lps->hDC, ptDst.x, ptDst.y, szDst.cx, szDst.cy, hdc, 0, 0, abs(bmp.bmWidth), abs(bmp.bmHeight), SRCCOPY);
	DeleteDC(hdc);
	DeleteObject(hbmPreview);
}

struct TSkinListItem
{
	TCHAR *path;
	TCHAR *title;
	TCHAR *filename;

	TSkinListItem(TCHAR *fn)
	{
		title = mir_tstrdup(fn);
		if (TCHAR *p = _tcsrchr(title, _T('.'))) *p = 0;

		TCHAR curPath[MAX_PATH];
		GetCurrentDirectory(SIZEOF(curPath), curPath);

		path = (TCHAR *)mir_alloc(MAX_PATH * sizeof(TCHAR));
		PathToRelativeT(curPath, path);

		int length = mir_tstrlen(curPath)+mir_tstrlen(fn)+2;
		filename = (TCHAR *)mir_alloc(length * sizeof(TCHAR));
		mir_sntprintf(filename, length, _T("%s\\%s"), curPath, fn);
	}

	~TSkinListItem()
	{
		mir_free(path);
		mir_free(title);
		mir_free(filename);
	}
};

struct TSelectorData
{
	MODERNOPTOBJECT *obj;
	TCHAR *active;
	HBITMAP hbmpPreview;

	TSelectorData()
	{
		ZeroMemory(this, sizeof(*this));
	}
	~TSelectorData()
	{
		mir_free(active);
		DeleteObject(hbmpPreview);
	}
};

static bool CheckExt(TCHAR *fn, TCHAR *ext, int n)
{
	int l = mir_tstrlen(fn);
	return (l > n) && !lstrcmp(fn + l - n, ext);
}

static void BuildSkinList(HWND hwndList, TCHAR *szExt, int nExtLength = -1, bool start = true)
{
	if (start) {
		static TCHAR mirPath[MAX_PATH];
		GetModuleFileName(NULL, mirPath, SIZEOF(mirPath));
		if (TCHAR *p = _tcsrchr(mirPath, _T('\\'))) *p = 0;
		SetCurrentDirectory(mirPath);
		SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
		nExtLength = mir_tstrlen(szExt);
		SendMessage(hwndList, WM_SETREDRAW, FALSE, 0);
	}

	WIN32_FIND_DATA ffd = {0};
	HANDLE h = FindFirstFile(_T("*.*"), &ffd);
	if (h != INVALID_HANDLE_VALUE) {
		do {
			if (!lstrcmp(ffd.cFileName, _T("")) || !lstrcmp(ffd.cFileName, _T(".")) || !lstrcmp(ffd.cFileName, _T("..")))
				continue;

			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				SetCurrentDirectory(ffd.cFileName);
				BuildSkinList(hwndList, szExt, nExtLength, false);
				SetCurrentDirectory(_T(".."));
			}
			else {
				if (CheckExt(ffd.cFileName, szExt, nExtLength)) {
					TSkinListItem *dat = new TSkinListItem(ffd.cFileName);
					DWORD dwItem = SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)ffd.cFileName);
					SendMessage(hwndList, LB_SETITEMDATA, dwItem, (LPARAM)dat);
			}	}
		}
			while (FindNextFile(h, &ffd));
		FindClose(h);
	}

	if (start) {
		SendMessage(hwndList, WM_SETREDRAW, TRUE, 0);
		RedrawWindow(hwndList, NULL, NULL, RDW_INVALIDATE);
	}
}

static void CreatePreview(TSelectorData *sd, TCHAR *fn, LPDRAWITEMSTRUCT lps)
{
	HDC hdc = CreateCompatibleDC(lps->hDC);
	sd->hbmpPreview = CreateCompatibleBitmap(lps->hDC, lps->rcItem.right - lps->rcItem.left, lps->rcItem.bottom - lps->rcItem.top);
	SelectObject(hdc, sd->hbmpPreview);

	BITMAPINFO bi = {0};
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = 8;
	bi.bmiHeader.biHeight = -8;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;

	HBITMAP hBmpBrush = (HBITMAP)CreateDIBSection(0, &bi, DIB_RGB_COLORS, 0, 0, 0);
	HDC dcBmp = CreateCompatibleDC(0);
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

	rc = lps->rcItem;
	OffsetRect(&rc, -rc.left, -rc.top);
	hbr = CreatePatternBrush(hBmpBrush);
	SetBrushOrgEx(hdc, 1, 1, 0);
	FillRect(hdc, &rc, hbr);
	DeleteObject(hbr);
	DeleteObject(hBmpBrush);

	HDC hdcSave = lps->hDC;
	lps->hDC = hdc;
	sttPreviewSkin(sd->obj, fn, lps);
	lps->hDC = hdcSave;

	FrameRect(hdc, &rc, GetStockBrush(LTGRAY_BRUSH));
	DeleteDC(hdc);
}

INT_PTR CALLBACK ModernOptSelector_DlgProc(HWND hwndDlg, UINT  msg, WPARAM wParam, LPARAM lParam)
{
	TSelectorData *sd = (TSelectorData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	MODERNOPTOBJECT *&obj = sd->obj;

	switch (msg) {
	case WM_INITDIALOG:
		{
			sd = new TSelectorData;
			MODERNOPTOBJECT *&obj = sd->obj;

			sd->obj = (MODERNOPTOBJECT *)lParam;
			sd->active = sttGetActiveSkin(obj);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)sd);

			TCHAR *s = mir_a2t(obj->lpzThemeExtension);
			BuildSkinList(GetDlgItem(hwndDlg, IDC_SKINLIST), s);
			mir_free(s);
		}
		return FALSE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_SKINLIST) {
			switch (HIWORD(wParam)) {
			case LBN_SELCHANGE:
				DeleteObject(sd->hbmpPreview);
				sd->hbmpPreview = 0;
				RedrawWindow(GetDlgItem(hwndDlg, IDC_PREVIEW1), NULL, NULL, RDW_INVALIDATE);
				break;

			case LBN_DBLCLK:
				{
					int idx = SendDlgItemMessage(hwndDlg, IDC_SKINLIST, LB_GETCURSEL, 0, 0);
					if (idx >= 0)
					{
						TSkinListItem *dat = (TSkinListItem *)SendDlgItemMessage(hwndDlg, IDC_SKINLIST, LB_GETITEMDATA, idx, 0);
						sttApplySkin(obj, dat->filename);
						mir_free(sd->active);
						sd->active = sttGetActiveSkin(obj);
						RedrawWindow(GetDlgItem(hwndDlg, IDC_SKINLIST), NULL, NULL, RDW_INVALIDATE);
					}
					break;
			}	}
			break;
		}
		return FALSE;

	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lps = (LPMEASUREITEMSTRUCT)lParam;
			if (lps->CtlID != IDC_SKINLIST)
				break;
			TSkinListItem *dat = (TSkinListItem *)lps->itemData;
			if (!dat) break;

			lps->itemWidth = 10;
			lps->itemHeight = 30;

			return FALSE;
		}

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lps = (LPDRAWITEMSTRUCT)lParam;
			if (lps->CtlID == IDC_SKINLIST) {
				TSkinListItem *dat = (TSkinListItem *)lps->itemData;
				if (!dat) break;

				SetBkMode(lps->hDC, TRANSPARENT);
				COLORREF clLine1, clLine2, clBack;
				if (lps->itemState & ODS_SELECTED) {
					FillRect(lps->hDC, &lps->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
					clBack = GetSysColor(COLOR_HIGHLIGHT);
					clLine1 = GetSysColor(COLOR_HIGHLIGHTTEXT);
				}
				else {
					FillRect(lps->hDC, &lps->rcItem, GetSysColorBrush(COLOR_WINDOW));
					clBack = GetSysColor(COLOR_WINDOW);
					clLine1 = GetSysColor(COLOR_WINDOWTEXT);
				}
				clLine2 = RGB(
						GetRValue(clLine1) * 0.66 + GetRValue(clBack) * 0.34,
						GetGValue(clLine1) * 0.66 + GetGValue(clBack) * 0.34,
						GetBValue(clLine1) * 0.66 + GetBValue(clBack) * 0.34
					);

				lps->rcItem.left += 2;
				lps->rcItem.top += 2;
				lps->rcItem.bottom -= 2;
				lps->rcItem.right -= 5;

				int cxIcon = GetSystemMetrics(SM_CXSMICON);
				int cyIcon = GetSystemMetrics(SM_CYSMICON);

				if (sd->active && !lstrcmp(sd->active, dat->filename)) {
					DrawIconEx(lps->hDC, lps->rcItem.left, (lps->rcItem.top+lps->rcItem.bottom-cyIcon)/2,
						LoadSkinnedIcon(SKINICON_OTHER_EMPTYBLOB),
						cxIcon, cyIcon, 0, NULL, DI_NORMAL);
				}
				else {
					DrawIconEx(lps->hDC, lps->rcItem.left, (lps->rcItem.top+lps->rcItem.bottom-cyIcon)/2,
						LoadSkinnedIcon(SKINICON_OTHER_SMALLDOT),
						cxIcon, cyIcon, 0, NULL, DI_NORMAL);
				}
				lps->rcItem.left += cxIcon;
				lps->rcItem.left += 5;

//				SelectObject(lps->hDC, dat->hfntTitle);
				SetTextColor(lps->hDC, clLine1);
				DrawText(lps->hDC, dat->title, -1, &lps->rcItem, DT_LEFT|DT_NOPREFIX|DT_SINGLELINE|DT_END_ELLIPSIS|DT_TOP);
				lps->rcItem.left += cxIcon;

				SetTextColor(lps->hDC, clLine2);
				DrawText(lps->hDC, dat->path, -1, &lps->rcItem, DT_LEFT|DT_NOPREFIX|DT_SINGLELINE|DT_PATH_ELLIPSIS|DT_BOTTOM);
			}
			else if (lps->CtlID == IDC_PREVIEW1) {
				int idx = SendDlgItemMessage(hwndDlg, IDC_SKINLIST, LB_GETCURSEL, 0, 0);

				if (!sd->hbmpPreview) {
					if (idx >= 0) {
						TSkinListItem *dat = (TSkinListItem *)SendDlgItemMessage(hwndDlg, IDC_SKINLIST, LB_GETITEMDATA, idx, 0);
						CreatePreview(sd, dat->filename, lps);
						//sttPreviewSkin(obj, dat->filename, lps);
					}
					else CreatePreview(sd, NULL, lps);
				}

				if (sd->hbmpPreview) {
					HDC hdc = CreateCompatibleDC(lps->hDC);
					SelectObject(hdc, sd->hbmpPreview);
					BitBlt(lps->hDC,
						lps->rcItem.left, lps->rcItem.top,
						lps->rcItem.right - lps->rcItem.left, lps->rcItem.bottom - lps->rcItem.top,
						hdc, 0, 0, SRCCOPY);
					DeleteDC(hdc);
				}
			}

			return TRUE;
		}

	case WM_DELETEITEM:
		{
			LPDELETEITEMSTRUCT lps = (LPDELETEITEMSTRUCT)lParam;
			if (lps->CtlID != IDC_SKINLIST) break;
			TSkinListItem *dat = (TSkinListItem *)lps->itemData;
			if (dat) delete dat;
			return FALSE;
		}

	case WM_DESTROY:
		delete sd;
		return FALSE;
	}
	return FALSE;
}
