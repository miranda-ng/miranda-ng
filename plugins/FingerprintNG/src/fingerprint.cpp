fv/*
Fingerprint NG (client version) icons module for Miranda NG
Copyright © 2006-22 ghazan, mataes, HierOS, FYR, Bio, nullbie, faith_healer and all respective contributors.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

//Start of header
#include "stdafx.h"

static UINT g_LPCodePage;
static wchar_t g_szSkinLib[MAX_PATH];
static HANDLE hExtraIcon = nullptr;
static HANDLE hFolderChanged = nullptr, hIconFolder = nullptr;

static int CompareFI(const FOUNDINFO *p1, const FOUNDINFO *p2)
{
	if (p1->iBase != p2->iBase)
		return p1->iBase - p2->iBase;
	return p1->iOverlay - p2->iOverlay;
}

static OBJLIST<FOUNDINFO> arFI(50, CompareFI);

static LIST<void> arMonitoredWindows(3, PtrKeySortT);

static INT_PTR ServiceGetClientIconW(WPARAM wParam, LPARAM lParam);

/*
*	Prepare
*	prepares upperstring masks and registers them in IcoLib
*/

static wchar_t* getSectionName(int flag)
{
	switch (flag)
	{
#include "finger_groups.h"
	}
	return nullptr;
}

void __fastcall Prepare(KN_FP_MASK* mask, bool bEnable)
{
	mask->szMaskUpper = nullptr;

	if (mask->hIcolibItem)
		IcoLib_RemoveIcon(mask->szIconName);
	mask->hIcolibItem = nullptr;

	if (!mask->szMask || !bEnable)
		return;

	size_t iMaskLen = mir_wstrlen(mask->szMask) + 1;
	LPTSTR pszNewMask = (LPTSTR)HeapAlloc(hHeap, HEAP_NO_SERIALIZE, iMaskLen * sizeof(wchar_t));
	wcscpy_s(pszNewMask, iMaskLen, mask->szMask);
	_wcsupr_s(pszNewMask, iMaskLen);
	mask->szMaskUpper = pszNewMask;

	wchar_t destfile[MAX_PATH];
	if (mask->iIconIndex == IDI_NOTFOUND || mask->iIconIndex == IDI_UNKNOWN || mask->iIconIndex == IDI_UNDETECTED)
		GetModuleFileName(g_plugin.getInst(), destfile, MAX_PATH);
	else {
		wcsncpy_s(destfile, g_szSkinLib, _TRUNCATE);

		struct _stat64i32 stFileInfo;
		if (_wstat(destfile, &stFileInfo) == -1)
			return;
	}

	LPTSTR SectName = getSectionName(mask->iSectionFlag);
	if (SectName == nullptr)
		return;

	SKINICONDESC sid = {};
	sid.flags = SIDF_ALL_UNICODE;
	sid.section.w = SectName;
	sid.pszName = mask->szIconName;
	sid.description.w = mask->szClientDescription;
	sid.defaultFile.w = destfile;
	sid.iDefaultIndex = -mask->iIconIndex;
	mask->hIcolibItem = g_plugin.addIcon(&sid);
}

/*
*	Register icons
*/

void RegisterIcons()
{
	// prepare masks
	int i;

	if (hHeap)
		HeapDestroy(hHeap);
	hHeap = HeapCreate(HEAP_NO_SERIALIZE, 0, 0);

	for (i = 0; i < DEFAULT_KN_FP_MASK_COUNT; i++)
		Prepare(&def_kn_fp_mask[i], true);

	for (i = 0; i < DEFAULT_KN_FP_OVERLAYS_COUNT; i++)
		Prepare(&def_kn_fp_overlays_mask[i], true);

	if (g_plugin.getByte("GroupMirandaVersion", 0)) {
		for (i = 0; i < DEFAULT_KN_FP_OVERLAYS2_COUNT; i++)
			Prepare(&def_kn_fp_overlays2_mask[i], true);
	}
	else {
		for (i = 0; i < DEFAULT_KN_FP_OVERLAYS2_NO_VER_COUNT; i++)
			Prepare(&def_kn_fp_overlays2_mask[i], true);
		for (; i < DEFAULT_KN_FP_OVERLAYS2_COUNT; i++)
			Prepare(&def_kn_fp_overlays2_mask[i], false);
	}

	if (g_plugin.getByte("GroupOverlaysUnicode", 1)) {
		for (i = 0; i < DEFAULT_KN_FP_OVERLAYS3_COUNT; i++)
			Prepare(&def_kn_fp_overlays3_mask[i], true);
	}
	else {
		for (i = 0; i < DEFAULT_KN_FP_OVERLAYS3_NO_UNICODE_COUNT; i++)
			Prepare(&def_kn_fp_overlays3_mask[i], true);
		for (; i < DEFAULT_KN_FP_OVERLAYS3_COUNT; i++)
			Prepare(&def_kn_fp_overlays3_mask[i], false);
	}

	for (i = 0; i < DEFAULT_KN_FP_OVERLAYS4_COUNT; i++)
		Prepare(&def_kn_fp_overlays4_mask[i], true);
}

/*	ApplyFingerprintImage
*	 1)Try to find appropriate mask
*	 2)Register icon in extraimage list if not yet registered (EMPTY_EXTRA_ICON)
*	 3)Set ExtraImage for contact
*/

static void SetSrmmIcon(MCONTACT hContact, LPTSTR ptszMirver)
{
	if (mir_wstrlen(ptszMirver))
		Srmm_ModifyIcon(hContact, MODULENAME, 1, (HICON)ServiceGetClientIconW((WPARAM)ptszMirver, TRUE), ptszMirver);
	else
		Srmm_SetIconFlags(hContact, MODULENAME, 1, MBF_HIDDEN);
}

int __fastcall ApplyFingerprintImage(MCONTACT hContact, LPTSTR szMirVer)
{
	if (hContact == NULL)
		return 0;

	HANDLE hImage = INVALID_HANDLE_VALUE;
	if (szMirVer)
		hImage = GetIconIndexFromFI(szMirVer);

	ExtraIcon_SetIcon(hExtraIcon, hContact, hImage);

	if (arMonitoredWindows.getIndex((HANDLE)hContact) != -1)
		SetSrmmIcon(hContact, szMirVer);

	return 0;
}

int OnExtraIconClick(WPARAM wParam, LPARAM, LPARAM)
{
	CallService(MS_USERINFO_SHOWDIALOG, wParam, NULL);
	return 0;
}


/*
*	WildCompare
*	Compare 'name' string with 'mask' strings.
*	Masks can contain '*' or '?' wild symbols
*	Asterics '*' symbol covers 'empty' symbol too e.g WildCompare("Tst","T*st*"), returns TRUE
*	In order to handle situation 'at least one any sybol' use "?*" combination:
*	e.g WildCompare("Tst","T?*st*"), returns FALSE, but both WildCompare("Test","T?*st*") and
*	WildCompare("Teeest","T?*st*") return TRUE.
*
*	Function is case sensitive! so convert input or modify func to use _qtoupper()
*
*	Mask can contain several submasks. In this case each submask (including first)
*	should start from '|' e.g: "|first*submask|second*mask".
*
*	Dec 25, 2006 by FYR:
*	Added Exception to masks: the mask "|^mask3|mask2|mask1" means:
*	if NOT according to mask 3 AND (mask1 OR mask2)
*	EXCEPTION should be BEFORE main mask:
*		IF Exception match - the comparing stops as FALSE
*		IF Exception does not match - comparing continue
*		IF Mask match - comparing stops as TRUE
*		IF Mask does not not match comparing continue
*/
BOOL __fastcall WildCompare(LPWSTR wszName, LPWSTR wszMask)
{
	if (wszMask == nullptr)
		return NULL;

	if (*wszMask != L'|')
		return wildcmpw(wszName, wszMask);

	size_t s = 1, e = 1;
	LPWSTR wszTemp = (LPWSTR)_alloca(mir_wstrlen(wszMask) * sizeof(wchar_t) + sizeof(wchar_t));
	BOOL bExcept;

	while (wszMask[e] != L'\0')
	{
		s = e;
		while (wszMask[e] != L'\0' && wszMask[e] != L'|') e++;

		// exception mask
		bExcept = (*(wszMask + s) == L'^');
		if (bExcept) s++;

		memcpy(wszTemp, wszMask + s, (e - s) * sizeof(wchar_t));
		wszTemp[e - s] = L'\0';

		if (wildcmpw(wszName, wszTemp))
			return !bExcept;

		if (wszMask[e] != L'\0')
			e++;
		else
			return FALSE;
	}
	return FALSE;
}

static void MatchMasks(wchar_t* szMirVer, int *base, int *overlay, int *overlay2, int *overlay3, int *overlay4)
{
	int i = 0, j = -1, k = -1, n = -1, m = -1;

	for (i = 0; i < DEFAULT_KN_FP_MASK_COUNT; i++) {
		KN_FP_MASK& p = def_kn_fp_mask[i];
		if (p.hIcolibItem == nullptr)
			continue;

		if (!WildCompare(szMirVer, p.szMaskUpper))
			continue;

		if (p.iIconIndex != IDI_NOTFOUND && p.iIconIndex != IDI_UNKNOWN && p.iIconIndex != IDI_UNDETECTED) {
			wchar_t destfile[MAX_PATH];
			wcsncpy_s(destfile, g_szSkinLib, _TRUNCATE);

			struct _stat64i32 stFileInfo;
			if (_wstat(destfile, &stFileInfo) == -1)
				i = NOTFOUND_MASK_NUMBER;
		}
		break;
	}
	if (i == DEFAULT_KN_FP_MASK_COUNT - 1)
		i = -1;

	else if (!def_kn_fp_mask[i].fNotUseOverlay && i < DEFAULT_KN_FP_MASK_COUNT) {
		for (j = 0; j < DEFAULT_KN_FP_OVERLAYS_COUNT; j++) {
			KN_FP_MASK& p = def_kn_fp_overlays_mask[j];
			if (p.hIcolibItem == nullptr)
				continue;

			if (!WildCompare(szMirVer, p.szMaskUpper))
				continue;

			struct _stat64i32 stFileInfo;
			if (_wstat(g_szSkinLib, &stFileInfo) != -1)
				break;
		}

		for (k = 0; k < DEFAULT_KN_FP_OVERLAYS2_COUNT; k++) {
			KN_FP_MASK& p = def_kn_fp_overlays2_mask[k];
			if (p.hIcolibItem == nullptr)
				continue;

			if (WildCompare(szMirVer, p.szMaskUpper))
				break;
		}

		for (n = 0; n < DEFAULT_KN_FP_OVERLAYS3_COUNT; n++) {
			KN_FP_MASK& p = def_kn_fp_overlays3_mask[n];
			if (p.hIcolibItem == nullptr)
				continue;

			if (WildCompare(szMirVer, p.szMaskUpper))
				break;
		}

		for (m = 0; m < DEFAULT_KN_FP_OVERLAYS4_COUNT; m++) {
			KN_FP_MASK& p = def_kn_fp_overlays4_mask[m];
			if (p.hIcolibItem == nullptr)
				continue;

			if (WildCompare(szMirVer, p.szMaskUpper))
				break;
		}
	}

	*base = (i < DEFAULT_KN_FP_MASK_COUNT) ? i : -1;
	*overlay = (j < DEFAULT_KN_FP_OVERLAYS_COUNT) ? j : -1;
	*overlay2 = (k < DEFAULT_KN_FP_OVERLAYS2_COUNT) ? k : -1;
	*overlay3 = (n < DEFAULT_KN_FP_OVERLAYS3_COUNT) ? n : -1;
	*overlay4 = (m < DEFAULT_KN_FP_OVERLAYS4_COUNT) ? m : -1;
}

/*	GetIconsIndexes
*	Retrieves Icons indexes by Mirver
*/

void __fastcall GetIconsIndexes(LPWSTR wszMirVer, int *base, int *overlay, int *overlay2, int *overlay3, int *overlay4)
{
	if (mir_wstrcmp(wszMirVer, L"?") == 0) {
		*base = UNKNOWN_MASK_NUMBER;
		*overlay = -1;
		*overlay2 = -1;
		*overlay3 = -1;
		*overlay4 = -1;
		return;
	}

	LPWSTR wszMirVerUp = NEWWSTR_ALLOCA(wszMirVer);
	_wcsupr(wszMirVerUp);
	MatchMasks(wszMirVerUp, base, overlay, overlay2, overlay3, overlay4);
}

/*
* CreateIconFromIndexes
* returns hIcon of joined icon by given indexes
*/

HICON __fastcall CreateIconFromIndexes(int base, int overlay, int overlay2, int overlay3, int overlay4)
{
	HICON hIcon = nullptr;	// returned HICON
	HICON hTmp = nullptr;
	HICON icMain = nullptr;
	HICON icOverlay = nullptr;
	HICON icOverlay2 = nullptr;
	HICON icOverlay3 = nullptr;
	HICON icOverlay4 = nullptr;

	KN_FP_MASK* mainMask = &(def_kn_fp_mask[base]);
	icMain = IcoLib_GetIconByHandle(mainMask->hIcolibItem);

	if (icMain) {
		KN_FP_MASK* overlayMask = (overlay != -1) ? &(def_kn_fp_overlays_mask[overlay]) : nullptr;
		KN_FP_MASK* overlay2Mask = (overlay2 != -1) ? &(def_kn_fp_overlays2_mask[overlay2]) : nullptr;
		KN_FP_MASK* overlay3Mask = (overlay3 != -1) ? &(def_kn_fp_overlays3_mask[overlay3]) : nullptr;
		KN_FP_MASK* overlay4Mask = (overlay4 != -1) ? &(def_kn_fp_overlays4_mask[overlay4]) : nullptr;
		icOverlay = (overlayMask == nullptr) ? nullptr : IcoLib_GetIconByHandle(overlayMask->hIcolibItem);
		icOverlay2 = (overlay2Mask == nullptr) ? nullptr : IcoLib_GetIconByHandle(overlay2Mask->hIcolibItem);
		icOverlay3 = (overlay3Mask == nullptr) ? nullptr : IcoLib_GetIconByHandle(overlay3Mask->hIcolibItem);
		icOverlay4 = (overlay4Mask == nullptr) ? nullptr : IcoLib_GetIconByHandle(overlay4Mask->hIcolibItem);

		hIcon = icMain;

		if (overlayMask)
			hTmp = hIcon = CreateJoinedIcon(hIcon, icOverlay);

		if (overlay2Mask) {
			hIcon = CreateJoinedIcon(hIcon, icOverlay2);
			if (hTmp) DestroyIcon(hTmp);
			hTmp = hIcon;
		}

		if (overlay3Mask) {
			hIcon = CreateJoinedIcon(hIcon, icOverlay3);
			if (hTmp) DestroyIcon(hTmp);
			hTmp = hIcon;
		}

		if (overlay4Mask) {
			hIcon = CreateJoinedIcon(hIcon, icOverlay4);
			if (hTmp) DestroyIcon(hTmp);
		}
	}

	if (hIcon == icMain)
		hIcon = CopyIcon(icMain);

	IcoLib_ReleaseIcon(icMain);
	IcoLib_ReleaseIcon(icOverlay);
	IcoLib_ReleaseIcon(icOverlay2);
	IcoLib_ReleaseIcon(icOverlay3);
	IcoLib_ReleaseIcon(icOverlay4);
	return hIcon;
}

/******************************************************************************
 *	Futher routines is for creating joined 'overlay' icons.
 ******************************************************************************/

 /*
 *	CreateBitmap32 - Create DIB 32 bitmap with sizes cx*cy
 */

HBITMAP __inline CreateBitmap32(int cx, int cy)
{
	return CreateBitmap32Point(cx, cy, nullptr);
}

/*
*	CreateBitmap32 - Create DIB 32 bitmap with sizes cx*cy and put reference
*				to new bitmap pixel image memory area to void ** bits
*/
HBITMAP __fastcall CreateBitmap32Point(int cx, int cy, LPVOID* bits)
{
	LPVOID ptPixels = nullptr;

	if (cx < 0 || cy < 0) return nullptr;

	BITMAPINFO bmpi = { 0 };
	bmpi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpi.bmiHeader.biWidth = cx;
	bmpi.bmiHeader.biHeight = cy;
	bmpi.bmiHeader.biPlanes = 1;
	bmpi.bmiHeader.biBitCount = 32;
	HBITMAP DirectBitmap = CreateDIBSection(nullptr, &bmpi, DIB_RGB_COLORS, &ptPixels, nullptr, 0);

	GdiFlush();
	if (ptPixels) memset(ptPixels, 0, cx * cy * 4);
	if (bits != nullptr) *bits = ptPixels;

	return DirectBitmap;
}

/*
*	 checkHasAlfa - checks if image has at least one uint8_t in alpha channel
*				 that is not a 0. (is image real 32 bit or just 24 bit)
*/
BOOL __fastcall checkHasAlfa(LPBYTE from, int width, int height)
{
	LPDWORD pt = (LPDWORD)from;
	LPDWORD lim = pt + width * height;
	while (pt < lim)
	{
		if (*pt & 0xFF000000)
			return TRUE;
		pt++;
	}

	return FALSE;
}

/*
*	 checkMaskUsed - checks if mask image has at least one that is not a 0.
*	 Not sure is it required or not
*/
BOOL __fastcall checkMaskUsed(LPBYTE from)
{
	int i;
	for (i = 0; i < 16 * 16 / 8; i++)
	{
		if (from[i] != 0) return TRUE;
	}
	return FALSE;
}

/*
*	 GetMaskBit - return value of apropriate mask bit in line at x position
*/
BOOL __inline GetMaskBit(LPBYTE line, int x)
{
	return ((*(line + (x >> 3))) & (0x01 << (7 - (x & 0x07)))) != 0;
}

/*
*	blend	- alpha blend ARGB values of 2 pixels. X1 - underlaying,
*	 X2 - overlaying points.
*/
uint32_t __fastcall blend(uint32_t X1, uint32_t X2)
{
	RGBA* q1 = (RGBA*)&X1;
	RGBA* q2 = (RGBA*)&X2;
	uint8_t a_1 = ~q1->a;
	uint8_t a_2 = ~q2->a;
	uint16_t am = q1->a * a_2;

	uint16_t ar = q1->a + ((a_1 * q2->a) / 255);
	// if a2 more than 0 than result should be more
	// or equal (if a1==0) to a2, else in combination
	// with mask we can get here black points

	ar = (q2->a > ar) ? q2->a : ar;

	if (ar == 0) return 0;

	{
		uint16_t arm = ar * 255;
		uint16_t rr = ((q1->r * am + q2->r * q2->a * 255)) / arm;
		uint16_t gr = ((q1->g * am + q2->g * q2->a * 255)) / arm;
		uint16_t br = ((q1->b * am + q2->b * q2->a * 255)) / arm;
		return (ar << 24) | (rr << 16) | (gr << 8) | br;
	}
}

/*
*	CreateJoinedIcon	- creates new icon by drawing hTop over hBottom.
*/
HICON __fastcall CreateJoinedIcon(HICON hBottom, HICON hTop)
{
	BOOL drawn = FALSE;
	HDC tempDC, tempDC2, tempDC3;
	HICON res = nullptr;
	HBITMAP oImage, nImage;
	HBITMAP nMask, hbm, obmp, obmp2;
	LPBYTE ptPixels = nullptr;
	ICONINFO iNew = { 0 };
	uint8_t p[32] = { 0 };

	tempDC = CreateCompatibleDC(nullptr);
	nImage = CreateBitmap32Point(16, 16, (LPVOID*)&ptPixels);
	oImage = (HBITMAP)SelectObject(tempDC, nImage);

	ICONINFO iciBottom = { 0 };
	ICONINFO iciTop = { 0 };

	BITMAP bmp_top = { 0 };
	BITMAP bmp_top_mask = { 0 };

	BITMAP bmp_bottom = { 0 };
	BITMAP bmp_bottom_mask = { 0 };

	GetIconInfo(hBottom, &iciBottom);
	GetObject(iciBottom.hbmColor, sizeof(BITMAP), &bmp_bottom);
	GetObject(iciBottom.hbmMask, sizeof(BITMAP), &bmp_bottom_mask);

	GetIconInfo(hTop, &iciTop);
	GetObject(iciTop.hbmColor, sizeof(BITMAP), &bmp_top);
	GetObject(iciTop.hbmMask, sizeof(BITMAP), &bmp_top_mask);

	if (bmp_bottom.bmBitsPixel == 32 && bmp_top.bmBitsPixel == 32)
	{
		LPBYTE BottomBuffer, TopBuffer, BottomMaskBuffer, TopMaskBuffer;
		LPBYTE bb, tb, bmb, tmb;
		LPBYTE db = ptPixels;
		int vstep_d = 16 * 4;
		int vstep_b = bmp_bottom.bmWidthBytes;
		int vstep_t = bmp_top.bmWidthBytes;
		int vstep_bm = bmp_bottom_mask.bmWidthBytes;
		int vstep_tm = bmp_top_mask.bmWidthBytes;

		if (bmp_bottom.bmBits)
			bb = BottomBuffer = (LPBYTE)bmp_bottom.bmBits;
		else
		{
			BottomBuffer = (LPBYTE)_alloca(bmp_bottom.bmHeight * bmp_bottom.bmWidthBytes);
			GetBitmapBits(iciBottom.hbmColor, bmp_bottom.bmHeight * bmp_bottom.bmWidthBytes, BottomBuffer);
			bb = BottomBuffer + vstep_b * (bmp_bottom.bmHeight - 1);
			vstep_b = -vstep_b;
		}
		if (bmp_top.bmBits)
			tb = TopBuffer = (LPBYTE)bmp_top.bmBits;
		else
		{
			TopBuffer = (LPBYTE)_alloca(bmp_top.bmHeight * bmp_top.bmWidthBytes);
			GetBitmapBits(iciTop.hbmColor, bmp_top.bmHeight * bmp_top.bmWidthBytes, TopBuffer);
			tb = TopBuffer + vstep_t * (bmp_top.bmHeight - 1);
			vstep_t = -vstep_t;
		}
		if (bmp_bottom_mask.bmBits)
			bmb = BottomMaskBuffer = (LPBYTE)bmp_bottom_mask.bmBits;
		else
		{
			BottomMaskBuffer = (LPBYTE)_alloca(bmp_bottom_mask.bmHeight * bmp_bottom_mask.bmWidthBytes);
			GetBitmapBits(iciBottom.hbmMask, bmp_bottom_mask.bmHeight * bmp_bottom_mask.bmWidthBytes, BottomMaskBuffer);
			bmb = BottomMaskBuffer + vstep_bm * (bmp_bottom_mask.bmHeight - 1);
			vstep_bm = -vstep_bm;
		}
		if (bmp_top_mask.bmBits)
			tmb = TopMaskBuffer = (LPBYTE)bmp_top_mask.bmBits;
		else
		{
			TopMaskBuffer = (LPBYTE)_alloca(bmp_top_mask.bmHeight * bmp_top_mask.bmWidthBytes);
			GetBitmapBits(iciTop.hbmMask, bmp_top_mask.bmHeight * bmp_top_mask.bmWidthBytes, TopMaskBuffer);
			tmb = TopMaskBuffer + vstep_tm * (bmp_top_mask.bmHeight - 1);
			vstep_tm = -vstep_tm;
		}
		{
			int x; int y;
			BOOL topHasAlpha = checkHasAlfa(TopBuffer, bmp_top.bmWidth, bmp_top.bmHeight);
			BOOL bottomHasAlpha = checkHasAlfa(BottomBuffer, bmp_bottom.bmWidth, bmp_bottom.bmHeight);
			BOOL topMaskUsed = !topHasAlpha && checkMaskUsed(TopMaskBuffer);
			BOOL bottomMaskUsed = !bottomHasAlpha && checkMaskUsed(BottomMaskBuffer);

			for (y = 0; y < 16; y++)
			{
				for (x = 0; x < 16; x++)
				{
					uint32_t bottom_d = ((LPDWORD)bb)[x];
					uint32_t top_d = ((LPDWORD)tb)[x];

					if (topMaskUsed)
					{
						if (GetMaskBit(tmb, x))
							top_d &= 0x00FFFFFF;
						else
							top_d |= 0xFF000000;
					}
					else if (!topHasAlpha)
						top_d |= 0xFF000000;

					if (bottomMaskUsed)
					{
						if (GetMaskBit(bmb, x))
							bottom_d &= 0x00FFFFFF;
						else
							bottom_d |= 0xFF000000;
					}
					else if (!bottomHasAlpha)
						bottom_d |= 0xFF000000;

					((LPDWORD)db)[x] = blend(bottom_d, top_d);
				}
				bb += vstep_b;
				tb += vstep_t;
				bmb += vstep_bm;
				tmb += vstep_tm;
				db += vstep_d;
			}
		}

		drawn = TRUE;
	}

	DeleteObject(iciBottom.hbmColor);
	DeleteObject(iciTop.hbmColor);
	DeleteObject(iciBottom.hbmMask);
	DeleteObject(iciTop.hbmMask);

	if (!drawn) {
		DrawIconEx(tempDC, 0, 0, hBottom, 16, 16, 0, nullptr, DI_NORMAL);
		DrawIconEx(tempDC, 0, 0, hTop, 16, 16, 0, nullptr, DI_NORMAL);
	}

	nMask = CreateBitmap(16, 16, 1, 1, p);
	tempDC2 = CreateCompatibleDC(nullptr);
	tempDC3 = CreateCompatibleDC(nullptr);
	hbm = CreateCompatibleBitmap(tempDC3, 16, 16);
	obmp = (HBITMAP)SelectObject(tempDC2, nMask);
	obmp2 = (HBITMAP)SelectObject(tempDC3, hbm);
	DrawIconEx(tempDC2, 0, 0, hBottom, 16, 16, 0, nullptr, DI_MASK);
	DrawIconEx(tempDC3, 0, 0, hTop, 16, 16, 0, nullptr, DI_MASK);
	BitBlt(tempDC2, 0, 0, 16, 16, tempDC3, 0, 0, SRCAND);

	GdiFlush();

	SelectObject(tempDC2, obmp);
	DeleteDC(tempDC2);

	SelectObject(tempDC3, obmp2);
	DeleteDC(tempDC3);

	SelectObject(tempDC, oImage);
	DeleteDC(tempDC);

	DeleteObject(hbm);

	iNew.fIcon = TRUE;
	iNew.hbmColor = nImage;
	iNew.hbmMask = nMask;
	res = CreateIconIndirect(&iNew);

	DeleteObject(nImage);
	DeleteObject(nMask);

	return res;
}

HANDLE __fastcall GetIconIndexFromFI(LPTSTR szMirVer)
{
	int base, overlay, overlay2, overlay3, overlay4;
	GetIconsIndexes(szMirVer, &base, &overlay, &overlay2, &overlay3, &overlay4);
	if (base == -1)
		return INVALID_HANDLE_VALUE;

	// MAX: 256 + 64 + 64 + 64 + 64
	FOUNDINFO tmp = { base, ((overlay & 0xFF) << 18) | ((overlay2 & 0x3F) << 12) | ((overlay3 & 0x3F) << 6) | (overlay4 & 0x3F) };
	auto *F = arFI.find(&tmp);
	if (F != nullptr)
		return F->hRegisteredImage;

	// not found - then add
	F = new FOUNDINFO(tmp);
	HICON hIcon = CreateIconFromIndexes(base, overlay, overlay2, overlay3, overlay4);
	if (hIcon != nullptr) {
		F->hRegisteredImage = ExtraIcon_AddIcon(hIcon);
		DestroyIcon(hIcon);
	}
	else F->hRegisteredImage = INVALID_HANDLE_VALUE;
	
	arFI.insert(F);

	return F->hRegisteredImage;
}

VOID ClearFI()
{
	arFI.destroy();
}

/****************************************************************************************
*	ServiceGetClientIconW
*	MS_FP_GETCLIENTICONW service implementation.
*	wParam - LPWSTR MirVer value to get client for.
*	lParam - int noCopy - if wParam is equal to "1"	will return icon handler without copiing icon.
*	ICON IS ALWAYS COPIED!!!
*/

static INT_PTR ServiceGetClientIconW(WPARAM wParam, LPARAM)
{
	LPWSTR wszMirVer = (LPWSTR)wParam;			// MirVer value to get client for.
	if (wszMirVer == nullptr)
		return 0;

	int base, overlay, overlay2, overlay3, overlay4;
	GetIconsIndexes(wszMirVer, &base, &overlay, &overlay2, &overlay3, &overlay4);

	HICON hIcon = nullptr;			// returned HICON
	if (base != -1)
		hIcon = CreateIconFromIndexes(base, overlay, overlay2, overlay3, overlay4);

	return (INT_PTR)hIcon;
}

/****************************************************************************************
 *	 ServiceGetClientDescrW
 *	 MS_FP_GETCLIENTDESCRW service implementation.
 *	 wParam - LPCWSTR MirVer value
 *	 lParam - (NULL) unused
 *	 returns LPCWSTR: client description (do not destroy) or NULL
 */

static INT_PTR ServiceGetClientDescrW(WPARAM wParam, LPARAM)
{
	LPWSTR wszMirVer = (LPWSTR)wParam;  // MirVer value to get client for.
	if (wszMirVer == nullptr)
		return 0;

	LPWSTR wszMirVerUp = NEWWSTR_ALLOCA(wszMirVer); _wcsupr(wszMirVerUp);
	if (mir_wstrcmp(wszMirVerUp, L"?") == 0)
		return (INT_PTR)def_kn_fp_mask[UNKNOWN_MASK_NUMBER].szClientDescription;

	for (int index = 0; index < DEFAULT_KN_FP_MASK_COUNT; index++)
		if (WildCompare(wszMirVerUp, def_kn_fp_mask[index].szMaskUpper))
			return (INT_PTR)def_kn_fp_mask[index].szClientDescription;

	return NULL;
}

/****************************************************************************************
 *	 ServiceSameClientW
 *	 MS_FP_SAMECLIENTSW service implementation.
 *	 wParam - LPWSTR first MirVer value
 *	 lParam - LPWSTR second MirVer value
 *	 returns LPCWSTR: client description (do not destroy) if clients are same or NULL
 */

static INT_PTR ServiceSameClientsW(WPARAM wParam, LPARAM lParam)
{
	if (!wParam || !lParam)
		return NULL; //one of its is not null

	INT_PTR res1 = ServiceGetClientDescrW(wParam, 0);
	INT_PTR res2 = ServiceGetClientDescrW(lParam, 0);
	return (res1 == res2 && res1 != 0) ? res1 : NULL;
}

/****************************************************************************************
*	OnExtraIconListRebuild
*	Set all registered indexes in array to EMPTY_EXTRA_ICON (unregistered icon)
*/

static int OnExtraIconListRebuild(WPARAM, LPARAM)
{
	ClearFI();
	return 0;
}

/****************************************************************************************
*	OnIconsChanged
*/

static int OnIconsChanged(WPARAM, LPARAM)
{
	ClearFI();
	return 0;
}

/****************************************************************************************
*	 OnExtraImageApply
*	 Try to get MirVer value from db for contact and if success calls ApplyFingerprintImage
*/

int OnExtraImageApply(WPARAM hContact, LPARAM)
{
	if (hContact == NULL)
		return 0;

	ptrW tszMirver;
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto != nullptr)
		tszMirver = db_get_wsa(hContact, szProto, "MirVer");

	ApplyFingerprintImage(hContact, tszMirver);
	return 0;
}

/****************************************************************************************
*	 OnContactSettingChanged
*	 if contact settings changed apply new image or remove it
*/

static int OnContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	if (hContact == NULL)
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (cws && cws->szSetting && !strcmp(cws->szSetting, "MirVer")) {
		switch (cws->value.type) {
		case DBVT_UTF8:
			ApplyFingerprintImage(hContact, ptrW(mir_utf8decodeW(cws->value.pszVal)));
			break;
		case DBVT_ASCIIZ:
			ApplyFingerprintImage(hContact, _A2T(cws->value.pszVal));
			break;
		case DBVT_WCHAR:
			ApplyFingerprintImage(hContact, cws->value.pwszVal);
			break;
		default:
			ApplyFingerprintImage(hContact, nullptr);
		}
	}
	return 0;
}

/****************************************************************************************
*	OnSrmmWindowEvent
*	Monitors SRMM window's creation to draw a statusbar icon
*/

static int OnSrmmWindowEvent(WPARAM, LPARAM lParam)
{
	if (!g_plugin.getByte("StatusBarIcon", 1))
		return 0;

	MessageWindowEventData *event = (MessageWindowEventData *)lParam;
	if (event == nullptr)
		return 0;

	if (event->uType == MSG_WINDOW_EVT_OPEN) {
		ptrW ptszMirVer;
		char *szProto = Proto_GetBaseAccountName(event->hContact);
		if (szProto != nullptr)
			ptszMirVer = db_get_wsa(event->hContact, szProto, "MirVer");
		SetSrmmIcon(event->hContact, ptszMirVer);
		arMonitoredWindows.insert((HANDLE)event->hContact);
	}
	else if (event->uType == MSG_WINDOW_EVT_CLOSE)
		arMonitoredWindows.remove((HANDLE)event->hContact);

	return 0;
}

/****************************************************************************************
*	OnModulesLoaded
*	Hook necessary events here
*/

int OnModulesLoaded(WPARAM, LPARAM)
{
	g_LPCodePage = Langpack_GetDefaultCodePage();

	//Hook necessary events
	HookEvent(ME_SKIN_ICONSCHANGED, OnIconsChanged);
	HookEvent(ME_MSG_WINDOWEVENT, OnSrmmWindowEvent);

	HookEvent(ME_MC_DEFAULTTCHANGED, OnExtraImageApply);

	PathToAbsoluteW(DEFAULT_SKIN_FOLDER, g_szSkinLib);

	RegisterIcons();

	hExtraIcon = ExtraIcon_RegisterCallback("Client", LPGEN("Fingerprint"), "client_Miranda_unknown",
		OnExtraIconListRebuild, OnExtraImageApply, OnExtraIconClick);

	if (g_plugin.getByte("StatusBarIcon", 1)) {
		StatusIconData sid = {};
		sid.szModule = MODULENAME;
		sid.flags = MBF_HIDDEN;
		sid.dwId = 1;
		Srmm_AddIcon(&sid, &g_plugin);
	}

	return 0;
}

void InitFingerModule()
{
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_OPT_INITIALISE, OnOptInitialise);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnContactSettingChanged);

	CreateServiceFunction(MS_FP_SAMECLIENTSW, ServiceSameClientsW);
	CreateServiceFunction(MS_FP_GETCLIENTDESCRW, ServiceGetClientDescrW);
	CreateServiceFunction(MS_FP_GETCLIENTICONW, ServiceGetClientIconW);
}
