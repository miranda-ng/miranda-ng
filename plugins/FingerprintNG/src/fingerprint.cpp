/*
Fingerprint NG (client version) icons module for Miranda NG
Copyright © 2006-12 ghazan, mataes, HierOS, FYR, Bio, nullbie, faith_healer and all respective contributors.

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
#include "global.h"

static UINT g_LPCodePage;
static TCHAR g_szSkinLib[MAX_PATH];
static HANDLE hExtraIcon = NULL;
static HANDLE hFolderChanged = NULL, hIconFolder = NULL;

static FOUNDINFO* fiList = NULL;
static int nFICount = 0;

static LIST<void> arMonitoredWindows(3, PtrKeySortT);

static INT_PTR ServiceGetClientIconW(WPARAM wParam, LPARAM lParam);

/*
*	Prepare
*	prepares upperstring masks and registers them in IcoLib
*/

static TCHAR* getSectionName(int flag)
{
	switch (flag)
	{
#include "finger_groups.h"
	}
	return NULL;
}

void __fastcall Prepare(KN_FP_MASK* mask, bool bEnable)
{
	mask->szMaskUpper = NULL;

	if (mask->hIcolibItem)
		Skin_RemoveIcon(mask->szIconName);
	mask->hIcolibItem = NULL;

	if (!mask->szMask || !bEnable)
		return;

	size_t iMaskLen = mir_tstrlen(mask->szMask) + 1;
	LPTSTR pszNewMask = (LPTSTR)HeapAlloc(hHeap, HEAP_NO_SERIALIZE, iMaskLen * sizeof(TCHAR));
	_tcscpy_s(pszNewMask, iMaskLen, mask->szMask);
	_tcsupr_s(pszNewMask, iMaskLen);
	mask->szMaskUpper = pszNewMask;

	TCHAR destfile[MAX_PATH];
	if (mask->iIconIndex == IDI_NOTFOUND || mask->iIconIndex == IDI_UNKNOWN || mask->iIconIndex == IDI_UNDETECTED)
		GetModuleFileName(g_hInst, destfile, MAX_PATH);
	else {
		_tcsncpy_s(destfile, g_szSkinLib, _TRUNCATE);

		struct _stat64i32 stFileInfo;
		if (_tstat(destfile, &stFileInfo) == -1)
			return;
	}

	LPTSTR SectName = getSectionName(mask->iSectionFlag);
	if (SectName == NULL)
		return;

	SKINICONDESC sid = { sizeof(sid) };
	sid.flags = SIDF_ALL_TCHAR;
	sid.ptszSection = SectName;
	sid.pszName = mask->szIconName;
	sid.ptszDescription = mask->szClientDescription;
	sid.ptszDefaultFile = destfile;
	sid.iDefaultIndex = -mask->iIconIndex;
	sid.cx = sid.cy = 16;
	mask->hIcolibItem = Skin_AddIcon(&sid);
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

	if (db_get_b(NULL, MODULENAME, "GroupMirandaVersion", 0)) {
		for (i = 0; i < DEFAULT_KN_FP_OVERLAYS2_COUNT; i++)
			Prepare(&def_kn_fp_overlays2_mask[i], true);
	}
	else {
		for (i = 0; i < DEFAULT_KN_FP_OVERLAYS2_NO_VER_COUNT; i++)
			Prepare(&def_kn_fp_overlays2_mask[i], true);
		for (; i < DEFAULT_KN_FP_OVERLAYS2_COUNT; i++)
			Prepare(&def_kn_fp_overlays2_mask[i], false);
	}

	if (db_get_b(NULL, MODULENAME, "GroupOverlaysUnicode", 1)) {
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
	StatusIconData sid = { sizeof(sid) };
	sid.szModule = MODULENAME;
	sid.dwId = 1;
	sid.flags = MBF_TCHAR;
	sid.tszTooltip = ptszMirver;

	if (mir_tstrlen(ptszMirver))
		sid.hIcon = (HICON)ServiceGetClientIconW((WPARAM)ptszMirver, TRUE);
	else
		sid.flags |= MBF_HIDDEN;

	Srmm_ModifyIcon(hContact, &sid);
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

	MCONTACT hMeta = db_mc_getMeta(hContact);
	if (hMeta && db_mc_getMostOnline(hMeta) == hContact)
		db_set_ts(hMeta, META_PROTO, "MirVer", szMirVer);

	return 0;
}

int OnExtraIconClick(WPARAM wParam, LPARAM, LPARAM)
{
	CallService(MS_USERINFO_SHOWDIALOG, wParam, NULL);
	return 0;
}


/*
*	WildCompareW
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
BOOL __fastcall WildCompareW(LPWSTR wszName, LPWSTR wszMask)
{
	if (wszMask == NULL)
		return NULL;

	if (*wszMask != L'|')
		return wildcmpw(wszName, wszMask);

	size_t s = 1, e = 1;
	LPWSTR wszTemp = (LPWSTR)_alloca(mir_wstrlen(wszMask) * sizeof(WCHAR) + sizeof(WCHAR));
	BOOL bExcept;

	while (wszMask[e] != L'\0')
	{
		s = e;
		while (wszMask[e] != L'\0' && wszMask[e] != L'|') e++;

		// exception mask
		bExcept = (*(wszMask + s) == L'^');
		if (bExcept) s++;

		memcpy(wszTemp, wszMask + s, (e - s) * sizeof(WCHAR));
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

static void MatchMasks(TCHAR* szMirVer, short *base, short *overlay, short *overlay2, short *overlay3, short *overlay4)
{
	int i = 0, j = -1, k = -1, n = -1, m = -1;

	for (i = 0; i < DEFAULT_KN_FP_MASK_COUNT; i++) {
		KN_FP_MASK& p = def_kn_fp_mask[i];
		if (p.hIcolibItem == NULL)
			continue;

		if (!WildCompareW(szMirVer, p.szMaskUpper))
			continue;

		if (p.iIconIndex != IDI_NOTFOUND && p.iIconIndex != IDI_UNKNOWN && p.iIconIndex != IDI_UNDETECTED) {
			TCHAR destfile[MAX_PATH];
			_tcsncpy_s(destfile, g_szSkinLib, _TRUNCATE);

			struct _stat64i32 stFileInfo;
			if (_tstat(destfile, &stFileInfo) == -1)
				i = NOTFOUND_MASK_NUMBER;
		}
		break;
	}
	if (i == DEFAULT_KN_FP_MASK_COUNT - 1)
		i = -1;

	else if (!def_kn_fp_mask[i].fNotUseOverlay && i < DEFAULT_KN_FP_MASK_COUNT) {
		for (j = 0; j < DEFAULT_KN_FP_OVERLAYS_COUNT; j++) {
			KN_FP_MASK& p = def_kn_fp_overlays_mask[j];
			if (p.hIcolibItem == NULL)
				continue;

			if (!WildCompare(szMirVer, p.szMaskUpper))
				continue;

			struct _stat64i32 stFileInfo;
			if (_tstat(g_szSkinLib, &stFileInfo) != -1)
				break;
		}

		for (k = 0; k < DEFAULT_KN_FP_OVERLAYS2_COUNT; k++) {
			KN_FP_MASK& p = def_kn_fp_overlays2_mask[k];
			if (p.hIcolibItem == NULL)
				continue;

			if (WildCompareW(szMirVer, p.szMaskUpper))
				break;
		}

		for (n = 0; n < DEFAULT_KN_FP_OVERLAYS3_COUNT; n++) {
			KN_FP_MASK& p = def_kn_fp_overlays3_mask[n];
			if (p.hIcolibItem == NULL)
				continue;

			if (WildCompareW(szMirVer, p.szMaskUpper))
				break;
		}

		for (m = 0; m < DEFAULT_KN_FP_OVERLAYS4_COUNT; m++) {
			KN_FP_MASK& p = def_kn_fp_overlays4_mask[m];
			if (p.hIcolibItem == NULL)
				continue;

			if (WildCompareW(szMirVer, p.szMaskUpper))
				break;
		}
	}

	*base = (i < DEFAULT_KN_FP_MASK_COUNT) ? i : -1;
	*overlay = (j < DEFAULT_KN_FP_OVERLAYS_COUNT) ? j : -1;
	*overlay2 = (k < DEFAULT_KN_FP_OVERLAYS2_COUNT) ? k : -1;
	*overlay3 = (n < DEFAULT_KN_FP_OVERLAYS3_COUNT) ? n : -1;
	*overlay4 = (m < DEFAULT_KN_FP_OVERLAYS4_COUNT) ? m : -1;
}

/*	GetIconsIndexesA
*	Retrieves Icons indexes by Mirver
*/

void __fastcall GetIconsIndexesA(LPSTR szMirVer, short *base, short *overlay, short *overlay2, short *overlay3, short *overlay4)
{
	if (mir_strcmp(szMirVer, "?") == 0) {
		*base = UNKNOWN_MASK_NUMBER;
		*overlay = -1;
		*overlay2 = -1;
		*overlay3 = -1;
		*overlay4 = -1;
		return;
	}

	LPTSTR tszMirVerUp = mir_a2t(szMirVer);
	_tcsupr(tszMirVerUp);
	MatchMasks(tszMirVerUp, base, overlay, overlay2, overlay3, overlay4);
	mir_free(tszMirVerUp);
}

/*	GetIconsIndexesW
*	Retrieves Icons indexes by Mirver
*/

void __fastcall GetIconsIndexesW(LPWSTR wszMirVer, short *base, short *overlay, short *overlay2, short *overlay3, short *overlay4)
{
	if (wcscmp(wszMirVer, L"?") == 0)
	{
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

HICON __fastcall CreateIconFromIndexes(short base, short overlay, short overlay2, short overlay3, short overlay4)
{
	HICON hIcon = NULL;	// returned HICON
	HICON hTmp = NULL;
	HICON icMain = NULL;
	HICON icOverlay = NULL;
	HICON icOverlay2 = NULL;
	HICON icOverlay3 = NULL;
	HICON icOverlay4 = NULL;

	KN_FP_MASK* mainMask = &(def_kn_fp_mask[base]);
	icMain = Skin_GetIconByHandle(mainMask->hIcolibItem);

	if (icMain) {
		KN_FP_MASK* overlayMask = (overlay != -1) ? &(def_kn_fp_overlays_mask[overlay]) : NULL;
		KN_FP_MASK* overlay2Mask = (overlay2 != -1) ? &(def_kn_fp_overlays2_mask[overlay2]) : NULL;
		KN_FP_MASK* overlay3Mask = (overlay3 != -1) ? &(def_kn_fp_overlays3_mask[overlay3]) : NULL;
		KN_FP_MASK* overlay4Mask = (overlay4 != -1) ? &(def_kn_fp_overlays4_mask[overlay4]) : NULL;
		icOverlay = (overlayMask == NULL) ? NULL : Skin_GetIconByHandle(overlayMask->hIcolibItem);
		icOverlay2 = (overlay2Mask == NULL) ? NULL : Skin_GetIconByHandle(overlay2Mask->hIcolibItem);
		icOverlay3 = (overlay3Mask == NULL) ? NULL : Skin_GetIconByHandle(overlay3Mask->hIcolibItem);
		icOverlay4 = (overlay4Mask == NULL) ? NULL : Skin_GetIconByHandle(overlay4Mask->hIcolibItem);

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

	Skin_ReleaseIcon(icMain);
	Skin_ReleaseIcon(icOverlay);
	Skin_ReleaseIcon(icOverlay2);
	Skin_ReleaseIcon(icOverlay3);
	Skin_ReleaseIcon(icOverlay4);
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
	return CreateBitmap32Point(cx, cy, NULL);
}

/*
*	CreateBitmap32 - Create DIB 32 bitmap with sizes cx*cy and put reference
*				to new bitmap pixel image memory area to void ** bits
*/
HBITMAP __fastcall CreateBitmap32Point(int cx, int cy, LPVOID* bits)
{
	BITMAPINFO bmpi = { 0 };
	LPVOID ptPixels = NULL;
	HBITMAP DirectBitmap;

	if (cx < 0 || cy < 0) return NULL;

	bmpi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpi.bmiHeader.biWidth = cx;
	bmpi.bmiHeader.biHeight = cy;
	bmpi.bmiHeader.biPlanes = 1;
	bmpi.bmiHeader.biBitCount = 32;
	DirectBitmap = CreateDIBSection(NULL, &bmpi, DIB_RGB_COLORS, &ptPixels, NULL, 0);

	GdiFlush();
	if (ptPixels) memset(ptPixels, 0, cx * cy * 4);
	if (bits != NULL) *bits = ptPixels;

	return DirectBitmap;
}

/*
*	 checkHasAlfa - checks if image has at least one BYTE in alpha channel
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
DWORD __fastcall blend(DWORD X1, DWORD X2)
{
	RGBA* q1 = (RGBA*)&X1;
	RGBA* q2 = (RGBA*)&X2;
	BYTE a_1 = ~q1->a;
	BYTE a_2 = ~q2->a;
	WORD am = q1->a * a_2;

	WORD ar = q1->a + ((a_1 * q2->a) / 255);
	// if a2 more than 0 than result should be more
	// or equal (if a1==0) to a2, else in combination
	// with mask we can get here black points

	ar = (q2->a > ar) ? q2->a : ar;

	if (ar == 0) return 0;

	{
		WORD arm = ar * 255;
		WORD rr = ((q1->r * am + q2->r * q2->a * 255)) / arm;
		WORD gr = ((q1->g * am + q2->g * q2->a * 255)) / arm;
		WORD br = ((q1->b * am + q2->b * q2->a * 255)) / arm;
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
	HICON res = NULL;
	HBITMAP oImage, nImage;
	HBITMAP nMask, hbm, obmp, obmp2;
	LPBYTE ptPixels = NULL;
	ICONINFO iNew = { 0 };
	BYTE p[32] = { 0 };

	tempDC = CreateCompatibleDC(NULL);
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
					DWORD bottom_d = ((LPDWORD)bb)[x];
					DWORD top_d = ((LPDWORD)tb)[x];

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
		DrawIconEx(tempDC, 0, 0, hBottom, 16, 16, 0, NULL, DI_NORMAL);
		DrawIconEx(tempDC, 0, 0, hTop, 16, 16, 0, NULL, DI_NORMAL);
	}

	nMask = CreateBitmap(16, 16, 1, 1, p);
	tempDC2 = CreateCompatibleDC(NULL);
	tempDC3 = CreateCompatibleDC(NULL);
	hbm = CreateCompatibleBitmap(tempDC3, 16, 16);
	obmp = (HBITMAP)SelectObject(tempDC2, nMask);
	obmp2 = (HBITMAP)SelectObject(tempDC3, hbm);
	DrawIconEx(tempDC2, 0, 0, hBottom, 16, 16, 0, NULL, DI_MASK);
	DrawIconEx(tempDC3, 0, 0, hTop, 16, 16, 0, NULL, DI_MASK);
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
	short base, overlay, overlay2, overlay3, overlay4;
	GetIconsIndexes(szMirVer, &base, &overlay, &overlay2, &overlay3, &overlay4);
	if (base == -1 || nFICount == 0xFFFF)
		return INVALID_HANDLE_VALUE;

	// MAX: 256 + 64 + 64 + 64 + 64
	DWORD val = (base << 24) | ((overlay & 0x3F) << 18) | ((overlay2 & 0x3F) << 12) | ((overlay3 & 0x3F) << 6) | (overlay4 & 0x3F);

	int i;
	HANDLE hFoundImage = INVALID_HANDLE_VALUE;
	for (i = 0; i < nFICount; i++) {
		if (fiList[i].dwArray == val) {
			hFoundImage = fiList[i].hRegisteredImage;
			break;
		}
	}

	if (hFoundImage == INVALID_HANDLE_VALUE && i == nFICount) { //not found - then add
		HICON hIcon = CreateIconFromIndexes(base, overlay, overlay2, overlay3, overlay4);

		fiList = (FOUNDINFO*)mir_realloc(fiList, sizeof(FOUNDINFO) * (nFICount + 1));
		fiList[nFICount].dwArray = val;

		if (hIcon != NULL) {
			hFoundImage = (HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM)hIcon, 0);
			fiList[nFICount].hRegisteredImage = hFoundImage;
			DestroyIcon(hIcon);
		}
		else fiList[nFICount].hRegisteredImage = INVALID_HANDLE_VALUE;

		nFICount++;
	}

	return hFoundImage;
}

VOID ClearFI()
{
	if (fiList != NULL)
		mir_free(fiList);
	fiList = NULL;
	nFICount = 0;
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
	if (wszMirVer == NULL)
		return 0;

	short base, overlay, overlay2, overlay3, overlay4;
	GetIconsIndexesW(wszMirVer, &base, &overlay, &overlay2, &overlay3, &overlay4);

	HICON hIcon = NULL;			// returned HICON
	if (base != -1)
		hIcon = CreateIconFromIndexes(base, overlay, overlay2, overlay3, overlay4);

	return (INT_PTR)hIcon;
}

/****************************************************************************************
 *	 ServiceGetClientDescrW
 *	 MS_FP_GETCLIENTDESCRW service implementation.
 *	 wParam - LPCWSTR MirVer value
 *	 lParam - (NULL) unused
 *	 returns LPCWSTR: client desription (do not destroy) or NULL
 */

static INT_PTR ServiceGetClientDescrW(WPARAM wParam, LPARAM)
{
	LPWSTR wszMirVer = (LPWSTR)wParam;  // MirVer value to get client for.
	if (wszMirVer == NULL)
		return 0;

	LPWSTR wszMirVerUp = NEWWSTR_ALLOCA(wszMirVer); _wcsupr(wszMirVerUp);
	if (wcscmp(wszMirVerUp, L"?") == 0)
		return (INT_PTR)def_kn_fp_mask[UNKNOWN_MASK_NUMBER].szClientDescription;

	for (int index = 0; index < DEFAULT_KN_FP_MASK_COUNT; index++)
		if (WildCompareW(wszMirVerUp, def_kn_fp_mask[index].szMaskUpper))
			return (INT_PTR)def_kn_fp_mask[index].szClientDescription;

	return NULL;
}

/****************************************************************************************
 *	 ServiceSameClientW
 *	 MS_FP_SAMECLIENTSW service implementation.
 *	 wParam - LPWSTR first MirVer value
 *	 lParam - LPWSTR second MirVer value
 *	 returns LPCWSTR: client desñription (do not destroy) if clients are same or NULL
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

	ptrT tszMirver;
	char *szProto = GetContactProto(hContact);
	if (szProto != NULL)
		tszMirver = db_get_tsa(hContact, szProto, "MirVer");

	ApplyFingerprintImage(hContact, tszMirver);
	return 0;
}

/****************************************************************************************
*	 OnMetaDefaultChanged
*	 update MC icon according to its default contact
*/

static int OnMetaDefaultChanged(WPARAM hMeta, LPARAM hSub)
{
	if (hSub != NULL) {
		char *szProto = GetContactProto(hSub);
		if (szProto != NULL) {
			ptrT tszMirver(db_get_tsa(hSub, szProto, "MirVer"));
			if (tszMirver)
				db_set_ts(hMeta, META_PROTO, "MirVer", tszMirver);
			else
				db_unset(hMeta, META_PROTO, "MirVer");
		}
	}

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
	if (cws && cws->szSetting && !mir_strcmp(cws->szSetting, "MirVer")) {
		switch (cws->value.type) {
		case DBVT_UTF8:
			ApplyFingerprintImage(hContact, ptrT(mir_utf8decodeT(cws->value.pszVal)));
			break;
		case DBVT_ASCIIZ:
			ApplyFingerprintImage(hContact, _A2T(cws->value.pszVal));
			break;
		case DBVT_WCHAR:
			ApplyFingerprintImage(hContact, cws->value.pwszVal);
			break;
		default:
			ApplyFingerprintImage(hContact, NULL);
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
	if (!db_get_b(NULL, MODULENAME, "StatusBarIcon", 1))
		return 0;

	MessageWindowEventData *event = (MessageWindowEventData *)lParam;
	if (event == NULL || event->cbSize < sizeof(MessageWindowEventData))
		return 0;

	if (event->uType == MSG_WINDOW_EVT_OPEN) {
		ptrT ptszMirVer;
		char *szProto = GetContactProto(event->hContact);
		if (szProto != NULL)
			ptszMirVer = db_get_tsa(event->hContact, szProto, "MirVer");
		SetSrmmIcon(event->hContact, ptszMirVer);
		arMonitoredWindows.insert((HANDLE)event->hContact);
	}
	else if (event->uType == MSG_WINDOW_EVT_CLOSE)
		arMonitoredWindows.remove(event->hContact);

	return 0;
}

/****************************************************************************************
*	OnModulesLoaded
*	Hook necessary events here
*/

int OnModulesLoaded(WPARAM, LPARAM)
{
	g_LPCodePage = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);

	//Hook necessary events
	HookEvent(ME_SKIN2_ICONSCHANGED, OnIconsChanged);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnContactSettingChanged);
	HookEvent(ME_OPT_INITIALISE, OnOptInitialise);
	HookEvent(ME_MSG_WINDOWEVENT, OnSrmmWindowEvent);

	PathToAbsoluteT(DEFAULT_SKIN_FOLDER, g_szSkinLib);

	RegisterIcons();

	if (db_get_b(NULL, MODULENAME, "StatusBarIcon", 1)) {
		StatusIconData sid = { sizeof(sid) };
		sid.szModule = MODULENAME;
		sid.flags = MBF_HIDDEN;
		sid.dwId = 1;
		Srmm_AddIcon(&sid);
	}

	return 0;
}

void InitFingerModule()
{
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_MC_DEFAULTTCHANGED, OnMetaDefaultChanged);

	CreateServiceFunction(MS_FP_SAMECLIENTSW, ServiceSameClientsW);
	CreateServiceFunction(MS_FP_GETCLIENTDESCRW, ServiceGetClientDescrW);
	CreateServiceFunction(MS_FP_GETCLIENTICONW, ServiceGetClientIconW);

	hExtraIcon = ExtraIcon_Register("Client", LPGEN("Fingerprint"), "client_Miranda_unknown",
		OnExtraIconListRebuild, OnExtraImageApply, OnExtraIconClick);
}
