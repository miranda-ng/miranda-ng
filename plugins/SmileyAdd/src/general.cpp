/*
Miranda SmileyAdd Plugin
Copyright (C) 2005 - 2012 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2003 - 2004 Rein-Peter de Boer

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

static ULONG_PTR g_gdiplusToken = 0;
static bool gdiPlusFail = false;

//
// General functions
//
int CalculateTextHeight(HDC hdc, CHARFORMAT2 *chf)
{
	HDC hcdc = CreateCompatibleDC(hdc);

	int logPixelsY = GetDeviceCaps(hdc, LOGPIXELSY);
	HFONT hFont = CreateFont(-(chf->yHeight * logPixelsY / 1440), 0, 0, 0,
		chf->wWeight, chf->dwEffects & CFE_ITALIC ? 1 : 0, 0, 0,
		chf->bCharSet, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		chf->szFaceName);
	SelectObject(hcdc, hFont);

	SIZE fontSize;
	GetTextExtentPoint32(hcdc, L")", 1, &fontSize);

	DeleteObject(hFont);
	DeleteDC(hcdc);

	return fontSize.cy;
}

HICON GetDefaultIcon(bool copy)
{
	HICON resIco = IcoLib_GetIcon("SmileyAdd_ButtonSmiley");
	if (resIco == nullptr)
		resIco = (HICON)LoadImage(g_plugin.getInst(), MAKEINTRESOURCE(IDI_SMILINGICON), IMAGE_ICON, 0, 0, copy ? 0 : LR_SHARED);
	else if (copy) {
		resIco = (HICON)CopyImage(resIco, IMAGE_ICON, 0, 0, 0);
		IcoLib_Release("SmileyAdd_ButtonSmiley");
	}

	return resIco;
}

HICON ImageList_GetIconFixed(HIMAGELIST himl, INT i, UINT fStyle)
{
	ICONINFO ii;
	HICON hIcon;
	HBITMAP hOldDstBitmap;
	HDC hdcDst;

	int cx, cy;
	ImageList_GetIconSize(himl, &cx, &cy);

	hdcDst = CreateCompatibleDC(nullptr);

	ii.fIcon = TRUE;
	ii.xHotspot = 0;
	ii.yHotspot = 0;

	// draw mask
	ii.hbmMask = CreateBitmap(cx, cy, 1, 1, nullptr);
	hOldDstBitmap = (HBITMAP)SelectObject(hdcDst, ii.hbmMask);
	PatBlt(hdcDst, 0, 0, cx, cy, WHITENESS);
	ImageList_Draw(himl, i, hdcDst, 0, 0, fStyle | ILD_MASK);

	// draw image
	ii.hbmColor = CreateBitmap(cx, cy, 1, 32, nullptr);
	SelectObject(hdcDst, ii.hbmColor);
	PatBlt(hdcDst, 0, 0, cx, cy, BLACKNESS);
	ImageList_Draw(himl, i, hdcDst, 0, 0, fStyle | ILD_TRANSPARENT);

	// CreateIconIndirect requires us to deselect the bitmaps from
	// the DCs before calling
	SelectObject(hdcDst, hOldDstBitmap);

	hIcon = CreateIconIndirect(&ii);

	DeleteObject(ii.hbmMask);
	DeleteObject(ii.hbmColor);
	DeleteDC(hdcDst);

	return hIcon;
}

bool InitGdiPlus(void)
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;

	static const wchar_t errmsg[] = L"GDI+ not installed.\n"
		L"GDI+ can be downloaded here: http://www.microsoft.com/downloads";

	__try {
		if (g_gdiplusToken == 0 && !gdiPlusFail)
			Gdiplus::GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, nullptr);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		gdiPlusFail = true;
		ReportError(errmsg);
	}

	return !gdiPlusFail;
}

void DestroyGdiPlus(void)
{
	if (g_gdiplusToken != 0) {
		Gdiplus::GdiplusShutdown(g_gdiplusToken);
		g_gdiplusToken = 0;
	}
}

MCONTACT DecodeMetaContact(MCONTACT hContact)
{
	if (hContact == 0)
		return 0;

	MCONTACT hReal = db_mc_getMostOnline(hContact);
	if (hReal == 0 || (INT_PTR)hReal == CALLSERVICE_NOTFOUND)
		hReal = hContact;

	return hReal;
}

bool IsSmileyProto(char *proto)
{
	return proto && mir_strcmp(proto, META_PROTO) && (CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0) & (PF1_IM | PF1_CHAT));
}

void ReportError(const wchar_t *errmsg)
{
	POPUPDATAW ppd = {};
	mir_wstrcpy(ppd.lpwzContactName, L"Miranda SmileyAdd");
	mir_wstrcpy(ppd.lpwzText, errmsg);
	PUAddPopupW(&ppd);
}
