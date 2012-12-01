/*
Fingerprint Mod+ (client version) icons module for Miranda IM

Copyright © 2006-2007 Artem Shpynov aka FYR, Bio, Faith Healer. 2009-2010 HierOS

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

//Start of header
#include "global.h"

static UINT g_LPCodePage;
static TCHAR g_szSkinLib[MAX_PATH];
static HANDLE hExtraIcon = NULL;
static HANDLE hFolderChanged = NULL, hIconFolder = NULL;

static FOUNDINFO* fiList = NULL;
static int nFICount = 0;

/*
*	Prepare
*	prepares upperstring masks and registers them in IcoLib
*/

static TCHAR* getSectionName(int flag)
{
	switch(flag)
	{
		#include "finger_groups.h"
	}
	return NULL;
}

void FASTCALL Prepare(KN_FP_MASK* mask, bool bEnable)
{
	mask->szMaskUpper = NULL;

	if (mask->hIcolibItem)
		Skin_RemoveIcon(mask->szIconName);
	mask->hIcolibItem = NULL;

	if (!mask->szMask || !mask->szIconFileName || !bEnable)
		return;

	size_t iMaskLen = _tcslen(mask->szMask) + 1;
	LPTSTR pszNewMask = (LPTSTR)HeapAlloc(hHeap, HEAP_NO_SERIALIZE, iMaskLen * sizeof(TCHAR));
	_tcscpy_s(pszNewMask, iMaskLen, mask->szMask);
	_tcsupr_s(pszNewMask, iMaskLen);
	mask->szMaskUpper = pszNewMask;

	TCHAR destfile[MAX_PATH];
	if (*mask->szIconFileName == 0)
		GetModuleFileName(g_hInst, destfile, MAX_PATH);
	else {
		mir_sntprintf(destfile, SIZEOF(destfile), _T("%s\\%s.dll"), g_szSkinLib, mask->szIconFileName);

		struct _stat64i32 stFileInfo;
		if ( _tstat(destfile, &stFileInfo) == -1)
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

	for (i=0; i < DEFAULT_KN_FP_MASK_COUNT; i++)
		Prepare(&def_kn_fp_mask[i], true);

	bool bEnable = db_get_b(NULL, "Finger", "Overlay1", 1) != 0;
	for (i=0; i < DEFAULT_KN_FP_OVERLAYS_COUNT; i++)
		Prepare(&def_kn_fp_overlays_mask[i], bEnable);

	bEnable = db_get_b(NULL, "Finger", "Overlay2", 1) != 0;
	if ( db_get_b(NULL, "Finger", "ShowVersion", 0)) {
		for (i = 0; i < DEFAULT_KN_FP_OVERLAYS2_COUNT; i++)
			Prepare(&def_kn_fp_overlays2_mask[i], bEnable);
	}
	else {
		for (i=0; i < DEFAULT_KN_FP_OVERLAYS2_NO_VER_COUNT; i++)
			Prepare(&def_kn_fp_overlays2_mask[i], bEnable);
		for (; i < DEFAULT_KN_FP_OVERLAYS2_COUNT; i++)
			Prepare(&def_kn_fp_overlays2_mask[i], false);
	}

	bEnable = db_get_b(NULL, "Finger", "Overlay3", 1) != 0;
	for (i=0; i < DEFAULT_KN_FP_OVERLAYS3_COUNT; i++)
		Prepare(&def_kn_fp_overlays3_mask[i], bEnable);
}

/*
*	OnModulesLoaded
*	Hook necessary events here
*/
int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	g_LPCodePage = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);

	//Hook necessary events
	HookEvent(ME_SKIN2_ICONSCHANGED, OnIconsChanged);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnContactSettingChanged);
	HookEvent(ME_OPT_INITIALISE, OnOptInitialise);

	hExtraIcon = ExtraIcon_Register("Client","Fingerprint","client_Miranda_Unknown",
		OnExtraIconListRebuild,OnExtraImageApply,OnExtraIconClick);

	if (ServiceExists(MS_FOLDERS_REGISTER_PATH)) {
		hIconFolder = FoldersRegisterCustomPathT("Fingerprint", "Icons", _T(MIRANDA_PATH) _T("\\") DEFAULT_SKIN_FOLDER);
		FoldersGetCustomPathT(hIconFolder, g_szSkinLib, SIZEOF(g_szSkinLib), _T(""));
	}
	else CallService(MS_UTILS_PATHTOABSOLUTET, (WPARAM)DEFAULT_SKIN_FOLDER, (LPARAM)g_szSkinLib);

	RegisterIcons();
	return 0;
}

/*	ApplyFingerprintImage
*	 1)Try to find appropriate mask
*	 2)Register icon in extraimage list if not yet registered (EMPTY_EXTRA_ICON)
*	 3)Set ExtraImage for contact
*/

int FASTCALL ApplyFingerprintImage(HANDLE hContact, LPTSTR szMirVer)
{
	if (hContact == NULL)
		return 0;

	HANDLE hImage = INVALID_HANDLE_VALUE;
	if (szMirVer)
		hImage = GetIconIndexFromFI(szMirVer);

	ExtraIcon_SetIcon(hExtraIcon, hContact, hImage);
	return 0;
}

int OnExtraIconClick(WPARAM wParam, LPARAM lParam, LPARAM)
{
	CallService(MS_USERINFO_SHOWDIALOG, wParam, NULL);
	return 0;
}

/*
*	OnExtraIconListRebuild
*	Set all registered indexes in array to EMPTY_EXTRA_ICON (unregistered icon)
*/
int OnExtraIconListRebuild(WPARAM wParam, LPARAM lParam)
{
	ClearFI();
	return 0;
}

/*
*	OnIconsChanged
*/
int OnIconsChanged(WPARAM wParam, LPARAM lParam)
{
	ClearFI();
	return 0;
}

/*
*	 OnExtraImageApply
*	 Try to get MirVer value from db for contact and if success calls ApplyFingerprintImage
*/

int OnExtraImageApply(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if (hContact == NULL)
		return 0;

	char *szProto = GetContactProto((HANDLE)wParam);
	if (szProto != NULL) {
		DBVARIANT dbvMirVer = { 0 };

		if (!DBGetContactSettingTString(hContact, szProto, "MirVer", &dbvMirVer)) {
			ApplyFingerprintImage(hContact, dbvMirVer.ptszVal);
			DBFreeVariant(&dbvMirVer);
		}
		else ApplyFingerprintImage(hContact, NULL);
	}
	else ApplyFingerprintImage(hContact, NULL);
	return 0;
}

/*
*	 OnContactSettingChanged
*	 if contact settings changed apply new image or remove it
*/
int OnContactSettingChanged(WPARAM wParam, LPARAM lParam)
{
	if ((HANDLE)wParam == NULL)
		return 0;

	DBCONTACTWRITESETTING* cws = (DBCONTACTWRITESETTING*)lParam;
	if (cws && cws->szSetting && !strcmp(cws->szSetting, "MirVer")) {
		if (cws->value.type == DBVT_UTF8) {
			LPWSTR wszVal = NULL;
			int iValLen = MultiByteToWideChar(CP_UTF8, 0, cws->value.pszVal, -1, NULL, 0);
			if (iValLen > 0) {
				wszVal = (LPWSTR)mir_alloc(iValLen * sizeof(WCHAR));
				MultiByteToWideChar(CP_UTF8, 0, cws->value.pszVal, -1, wszVal, iValLen);
			}
			ApplyFingerprintImage((HANDLE)wParam, wszVal);
			mir_free(wszVal);
		}
		else if (cws->value.type == DBVT_ASCIIZ) {
			LPWSTR wszVal = NULL;
			int iValLen = MultiByteToWideChar(g_LPCodePage, 0, cws->value.pszVal, -1, NULL, 0);
			if (iValLen > 0) {
				wszVal = (LPWSTR)mir_alloc(iValLen * sizeof(WCHAR));
				MultiByteToWideChar(g_LPCodePage, 0, cws->value.pszVal, -1, wszVal, iValLen);
			}
			ApplyFingerprintImage((HANDLE)wParam, wszVal);
			mir_free(wszVal);
		}
		else if (cws->value.type == DBVT_WCHAR) {
			ApplyFingerprintImage((HANDLE)wParam, cws->value.pwszVal);
		}
		else ApplyFingerprintImage((HANDLE)wParam, NULL);
	}
	return 0;
}

/*
*	WildCompareA
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
BOOL FASTCALL WildCompareA(LPSTR szName, LPSTR szMask)
{
	if (*szMask != '|')
		return WildCompareProcA(szName, szMask);

	size_t s = 1, e = 1;
	LPSTR szTemp = (LPSTR)_alloca(strlen(szMask) * sizeof(CHAR) + sizeof(CHAR));
	BOOL bExcept;

	while(szMask[e] != '\0') {
		s = e;
		while(szMask[e] != '\0' && szMask[e] != '|') e++;

		// exception mask
		bExcept = (*(szMask + s) == '^');
		if (bExcept) s++;

		memcpy(szTemp, szMask + s, (e - s) * sizeof(CHAR));
		szTemp[e - s] = '\0';

		if (WildCompareProcA(szName, szTemp))
			return !bExcept;

		if (szMask[e] != '\0')
			e++;
		else
			return FALSE;
	}
	return FALSE;
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
BOOL FASTCALL WildCompareW(LPWSTR wszName, LPWSTR wszMask)
{
	if (wszMask == NULL)
		return NULL;

	if (*wszMask != L'|')
		return WildCompareProcW(wszName, wszMask);

	size_t s = 1, e = 1;
	LPWSTR wszTemp = (LPWSTR)_alloca(wcslen(wszMask) * sizeof(WCHAR) + sizeof(WCHAR));
	BOOL bExcept;

	while(wszMask[e] != L'\0')
	{
		s = e;
		while(wszMask[e] != L'\0' && wszMask[e] != L'|') e++;

		// exception mask
		bExcept = (*(wszMask + s) == L'^');
		if (bExcept) s++;

		memcpy(wszTemp, wszMask + s, (e - s) * sizeof(WCHAR));
		wszTemp[e - s] = L'\0';

		if (WildCompareProcW(wszName, wszTemp))
			return !bExcept;

		if (wszMask[e] != L'\0')
			e++;
		else
			return FALSE;
	}
	return FALSE;
}

BOOL __inline WildCompareProcA(LPSTR szName, LPSTR szMask)
{
	LPSTR szLast = NULL;
	for (;; szMask++, szName++)
	{
		if (*szMask != '?' && *szMask != *szName) break;
		if (*szName == '\0') return ((BOOL)!*szMask);
	}
	if (*szMask != '*') return FALSE;
	for (;; szMask++, szName++)
	{
		while(*szMask == '*')
		{
			szLast = szMask++;
			if (*szMask == '\0') return ((BOOL)!*szMask);	/* true */
		}
		if (*szName == '\0') return ((BOOL)!*szMask);		/* *mask == EOS */
		if (*szMask != '?' && *szMask != *szName && szLast != NULL)
		{
			szName -= (size_t)(szMask - szLast) - 1;
			szMask = szLast;
		}
	}
}

BOOL __inline WildCompareProcW(LPWSTR wszName, LPWSTR wszMask)
{
	LPWSTR wszLast = NULL;
	for (;; wszMask++, wszName++)
	{
		if (*wszMask != L'?' && *wszMask != *wszName) break;
		if (*wszName == L'\0') return ((BOOL)!*wszMask);
	}
	if (*wszMask != L'*') return FALSE;
	for (;; wszMask++, wszName++)
	{
		while(*wszMask == L'*')
		{
			wszLast = wszMask++;
			if (*wszMask == L'\0') return ((BOOL)!*wszMask);	/* true */
		}
		if (*wszName == L'\0') return ((BOOL)!*wszMask);		/* *mask == EOS */
		if (*wszMask != L'?' && *wszMask != *wszName && wszLast != NULL)
		{
			wszName -= (size_t)(wszMask - wszLast) - 1;
			wszMask = wszLast;
		}
	}
}

static void MatchMasks(TCHAR* szMirVer, short *base, short *overlay,short *overlay2,short *overlay3)
{
	short i = 0, j = -1, k = -1, n = -1;

	for (i=0; i < DEFAULT_KN_FP_MASK_COUNT; i++) {
		KN_FP_MASK& p = def_kn_fp_mask[i];
		if (p.hIcolibItem == NULL)
			continue;

		if ( !WildCompareW(szMirVer, p.szMaskUpper))
			continue;

		if (p.szIconFileName != _T("")) {
			TCHAR destfile[MAX_PATH];
			mir_sntprintf(destfile, SIZEOF(destfile), _T("%s\\%s.dll"), g_szSkinLib, p.szIconFileName);
			struct _stat64i32 stFileInfo;

			if (_tstat(destfile, &stFileInfo) == -1)
				i = NOTFOUND_MASK_NUMBER;
		}
		break;
	}

	if (!def_kn_fp_mask[i].fNotUseOverlay && i < DEFAULT_KN_FP_MASK_COUNT) {
		for (j = 0; j < DEFAULT_KN_FP_OVERLAYS_COUNT; j++) {
			KN_FP_MASK& p = def_kn_fp_overlays_mask[j];
			if (p.hIcolibItem == NULL)
				continue;

			if ( !WildCompare(szMirVer, p.szMaskUpper))
				continue;

			if (p.szIconFileName != _T("ClientIcons_Packs"))
				break;

			TCHAR destfile[MAX_PATH];
			mir_sntprintf(destfile, SIZEOF(destfile), _T("%s\\%s.dll"), g_szSkinLib, p.szIconFileName);

			struct _stat64i32 stFileInfo;
			if ( _tstat(destfile, &stFileInfo) != -1)
				break;
		}

		for (k = 0; k < DEFAULT_KN_FP_OVERLAYS2_COUNT; k++) {
			KN_FP_MASK& p = def_kn_fp_overlays2_mask[k];
			if (p.hIcolibItem == NULL)
				continue;

			if ( WildCompareW(szMirVer, p.szMaskUpper))
				break;
		}

		for (n = 0; n < DEFAULT_KN_FP_OVERLAYS3_COUNT; n++) {
			KN_FP_MASK& p = def_kn_fp_overlays3_mask[n];
			if (p.hIcolibItem == NULL)
				continue;

			if ( WildCompareW(szMirVer, p.szMaskUpper))
				break;
		}
	}

	*base = (i < DEFAULT_KN_FP_MASK_COUNT) ? i : -1;
	*overlay = (j < DEFAULT_KN_FP_OVERLAYS_COUNT) ? j : -1;
	*overlay2 = (k < DEFAULT_KN_FP_OVERLAYS2_COUNT) ? k : -1;
	*overlay3 = (n < DEFAULT_KN_FP_OVERLAYS3_COUNT) ? n : -1;
}

/*	GetIconsIndexesA
*	Retrieves Icons indexes by Mirver
*/

void FASTCALL GetIconsIndexesA(LPSTR szMirVer, short *base, short *overlay,short *overlay2,short *overlay3)
{
	if (strcmp(szMirVer, "?") == 0) {
		*base = UNKNOWN_MASK_NUMBER;
		*overlay = -1;
		*overlay2 = -1;
		*overlay3 = -1;
		return;
	}

	LPTSTR tszMirVerUp = mir_a2t(szMirVer);
	_tcsupr(tszMirVerUp);
	MatchMasks(tszMirVerUp, base, overlay, overlay2, overlay3);
	mir_free(tszMirVerUp);
}

/*	GetIconsIndexesW
*	Retrieves Icons indexes by Mirver
*/

void FASTCALL GetIconsIndexesW(LPWSTR wszMirVer, short *base, short *overlay,short *overlay2,short *overlay3)
{
	if (wcscmp(wszMirVer, L"?") == 0)
	{
		*base = UNKNOWN_MASK_NUMBER;
		*overlay = -1;
		*overlay2 = -1;
		*overlay3 = -1;
		return;
	}

	LPWSTR wszMirVerUp = NEWWSTR_ALLOCA(wszMirVer);
	_wcsupr(wszMirVerUp);
	MatchMasks(wszMirVerUp, base, overlay, overlay2, overlay3);
}

/*
* CreateIconFromIndexes
* returns hIcon of joined icon by given indexes
*/

HICON FASTCALL CreateIconFromIndexes(short base, short overlay, short overlay2, short overlay3)
{
	HICON hIcon = NULL;	// returned HICON
	HICON hTmp = NULL;
	HICON icMain = NULL;
	HICON icOverlay = NULL;
	HICON icOverlay2 = NULL;
	HICON icOverlay3 = NULL;

	KN_FP_MASK* mainMask = &(def_kn_fp_mask[base]);
	icMain = Skin_GetIconByHandle(mainMask->hIcolibItem);

	if (icMain) {
		KN_FP_MASK* overlayMask = (overlay != -1) ? &(def_kn_fp_overlays_mask[overlay]) : NULL;
		KN_FP_MASK* overlay2Mask = (overlay2 != -1) ? &(def_kn_fp_overlays2_mask[overlay2]) : NULL;
		KN_FP_MASK* overlay3Mask = (overlay3 != -1) ? &(def_kn_fp_overlays3_mask[overlay3]) : NULL;
		icOverlay = (overlayMask == NULL) ? NULL : Skin_GetIconByHandle(overlayMask->hIcolibItem);
		icOverlay2 = (overlay2Mask == NULL) ? NULL : Skin_GetIconByHandle(overlay2Mask->hIcolibItem);
		icOverlay3 = (overlay3Mask == NULL) ? NULL : Skin_GetIconByHandle(overlay3Mask->hIcolibItem);

		hIcon = icMain;

		if (overlayMask) {
			hIcon = CreateJoinedIcon(hIcon, icOverlay);
			hTmp = hIcon;
		}

		if (overlay2Mask) {
			hIcon = CreateJoinedIcon(hIcon, icOverlay2);
			if (hTmp) DestroyIcon(hTmp);
			hTmp = hIcon;
		}

		if (overlay3Mask) {
			hIcon = CreateJoinedIcon(hIcon, icOverlay3);
			if (hTmp) DestroyIcon(hTmp);
		}
	}

	if (hIcon == icMain)
		hIcon = CopyIcon(icMain);

	Skin_ReleaseIcon(icMain);
	Skin_ReleaseIcon(icOverlay);
	Skin_ReleaseIcon(icOverlay2);
	Skin_ReleaseIcon(icOverlay3);
	return hIcon;
}

/*
*	ServiceGetClientIconA
*	MS_FP_GETCLIENTICON service implementation.
*	wParam - char * MirVer value to get client for.
*	lParam - int noCopy - if wParam is equal to "1"	will return icon handler without copiing icon.
*	ICON IS ALWAYS COPIED!!!
*/

INT_PTR ServiceGetClientIconA(WPARAM wParam, LPARAM lParam)
{
	LPSTR szMirVer = (LPSTR)wParam;			// MirVer value to get client for.
	if (szMirVer == NULL)
		return 0;

	HICON hIcon = NULL;			// returned HICON
	int NoCopy = (int)lParam;	// noCopy
	short base, overlay, overlay2, overlay3;

	GetIconsIndexesA(szMirVer, &base, &overlay, &overlay2, &overlay3);
	if (base != -1)
		hIcon = CreateIconFromIndexes(base, overlay, overlay2, overlay3);
	return (INT_PTR)hIcon;
}

/*
 *	 ServiceSameClientA
 *	 MS_FP_SAMECLIENTS service implementation.
 *	 wParam - char * first MirVer value
 *	 lParam - char * second MirVer value
 *	 return pointer to char string - client desription (do not destroy) if clients are same
 */

INT_PTR ServiceSameClientsA(WPARAM wParam, LPARAM lParam)
{
	LPSTR szMirVerFirst = (LPSTR)wParam;	// MirVer value to get client for.
	LPSTR szMirVerSecond = (LPSTR)lParam;	// MirVer value to get client for.
	int firstIndex, secondIndex;
	BOOL Result = FALSE;

	firstIndex = secondIndex = 0;
	if (!szMirVerFirst || !szMirVerSecond)
		return (INT_PTR)NULL;	//one of its is not null

	{
		LPTSTR tszMirVerFirstUp, tszMirVerSecondUp;
		int iMirVerFirstUpLen, iMirVerSecondUpLen;

		iMirVerFirstUpLen = MultiByteToWideChar(g_LPCodePage, 0, szMirVerFirst, -1, NULL, 0);
		iMirVerSecondUpLen = MultiByteToWideChar(g_LPCodePage, 0, szMirVerSecond, -1, NULL, 0);

		tszMirVerFirstUp = (LPTSTR)mir_alloc(iMirVerFirstUpLen * sizeof(TCHAR));
		tszMirVerSecondUp = (LPTSTR)mir_alloc(iMirVerSecondUpLen * sizeof(TCHAR));

		MultiByteToWideChar(g_LPCodePage, 0, szMirVerFirst, -1, tszMirVerFirstUp, iMirVerFirstUpLen);
		MultiByteToWideChar(g_LPCodePage, 0, szMirVerSecond, -1, tszMirVerSecondUp, iMirVerSecondUpLen);

		_tcsupr_s(tszMirVerFirstUp, iMirVerFirstUpLen);
		_tcsupr_s(tszMirVerSecondUp, iMirVerSecondUpLen);

		if (_tcscmp(tszMirVerFirstUp, _T("?")) == 0)
			firstIndex = UNKNOWN_MASK_NUMBER;
		else
			while(firstIndex < DEFAULT_KN_FP_MASK_COUNT) {
				if (WildCompare(tszMirVerFirstUp, def_kn_fp_mask[firstIndex].szMaskUpper))
					break;
				firstIndex++;
			}

		if (_tcscmp(tszMirVerSecondUp, _T("?")) == 0)
			secondIndex = UNKNOWN_MASK_NUMBER;
		else
			while(secondIndex < DEFAULT_KN_FP_MASK_COUNT) {
				if (WildCompare(tszMirVerSecondUp, def_kn_fp_mask[secondIndex].szMaskUpper))
					break;
	 			secondIndex++;
			}

		mir_free(tszMirVerFirstUp);
		mir_free(tszMirVerSecondUp);

		if (firstIndex == secondIndex && firstIndex < DEFAULT_KN_FP_MASK_COUNT)
		{
			int iClientDescriptionLen = WideCharToMultiByte(g_LPCodePage, 0, def_kn_fp_mask[firstIndex].szClientDescription, -1, NULL, 0, NULL, NULL);
			if (iClientDescriptionLen > 0)
				g_szClientDescription = (LPSTR)mir_realloc(g_szClientDescription, iClientDescriptionLen * sizeof(CHAR));
			else
				return (INT_PTR)NULL;

			WideCharToMultiByte(g_LPCodePage, 0, def_kn_fp_mask[firstIndex].szClientDescription, -1, g_szClientDescription, iClientDescriptionLen, NULL, NULL);
			return (INT_PTR)g_szClientDescription;

		}
	}
	return (INT_PTR)NULL;
}

/*
*	ServiceGetClientIconW
*	MS_FP_GETCLIENTICONW service implementation.
*	wParam - LPWSTR MirVer value to get client for.
*	lParam - int noCopy - if wParam is equal to "1"	will return icon handler without copiing icon.
*	ICON IS ALWAYS COPIED!!!
*/

INT_PTR ServiceGetClientIconW(WPARAM wParam, LPARAM lParam)
{
	LPWSTR wszMirVer = (LPWSTR)wParam;			// MirVer value to get client for.
	if (wszMirVer == NULL)
		return 0;

	short base, overlay, overlay2, overlay3;
	GetIconsIndexesW(wszMirVer, &base, &overlay, &overlay2, &overlay3);

	HICON hIcon = NULL;			// returned HICON
	if (base != -1)
		hIcon = CreateIconFromIndexes(base, overlay, overlay2, overlay3);

	return (INT_PTR)hIcon;
}

/*
 *	 ServiceSameClientW
 *	 MS_FP_SAMECLIENTSW service implementation.
 *	 wParam - LPWSTR first MirVer value
 *	 lParam - LPWSTR second MirVer value
 *	 return pointer to char string - client desription (do not destroy) if clients are same
 */
INT_PTR ServiceSameClientsW(WPARAM wParam, LPARAM lParam)
{
	LPWSTR wszMirVerFirst = (LPWSTR)wParam;	// MirVer value to get client for.
	LPWSTR wszMirVerSecond = (LPWSTR)lParam;	// MirVer value to get client for.
	int firstIndex, secondIndex;
	BOOL Result = FALSE;

	firstIndex = secondIndex = 0;
	if (!wszMirVerFirst || !wszMirVerSecond) return (INT_PTR)NULL;	//one of its is not null

	{
		LPWSTR wszMirVerFirstUp, wszMirVerSecondUp;
		size_t iMirVerFirstUpLen, iMirVerSecondUpLen;

		iMirVerFirstUpLen = wcslen(wszMirVerFirst) + 1;
		iMirVerSecondUpLen = wcslen(wszMirVerSecond) + 1;

		wszMirVerFirstUp = (LPWSTR)mir_alloc(iMirVerFirstUpLen * sizeof(WCHAR));
		wszMirVerSecondUp = (LPWSTR)mir_alloc(iMirVerSecondUpLen * sizeof(WCHAR));

		wcscpy_s(wszMirVerFirstUp, iMirVerFirstUpLen, wszMirVerFirst);
		wcscpy_s(wszMirVerSecondUp, iMirVerSecondUpLen, wszMirVerSecond);

		_wcsupr_s(wszMirVerFirstUp, iMirVerFirstUpLen);
		_wcsupr_s(wszMirVerSecondUp, iMirVerSecondUpLen);

		if (wcscmp(wszMirVerFirstUp, L"?") == 0)
			firstIndex = UNKNOWN_MASK_NUMBER;
		else
			while(firstIndex < DEFAULT_KN_FP_MASK_COUNT) {
				if (WildCompareW(wszMirVerFirstUp, def_kn_fp_mask[firstIndex].szMaskUpper))
					break;
				firstIndex++;
			}

		if (wcscmp(wszMirVerSecondUp, L"?") == 0)
			secondIndex = UNKNOWN_MASK_NUMBER;
		else
			while(secondIndex < DEFAULT_KN_FP_MASK_COUNT) {
				if (WildCompareW(wszMirVerSecondUp, def_kn_fp_mask[secondIndex].szMaskUpper))
					break;
	 			secondIndex++;
			}

		mir_free(wszMirVerFirstUp);
		mir_free(wszMirVerSecondUp);

		if (firstIndex == secondIndex && firstIndex < DEFAULT_KN_FP_MASK_COUNT)
		{
			return (INT_PTR)def_kn_fp_mask[firstIndex].szClientDescription;
		}
	}
	return (INT_PTR)NULL;
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
HBITMAP FASTCALL CreateBitmap32Point(int cx, int cy, LPVOID* bits)
{
	BITMAPINFO bmpi = { 0 };
	LPVOID ptPixels = NULL;
	HBITMAP DirectBitmap;

	if (cx < 0 || cy < 0) return NULL;

	bmpi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpi.bmiHeader.biWidth = cx;
	bmpi.bmiHeader.biHeight = cy;
	bmpi.bmiHeader.biPlanes = 1;
//	bmpi.bmiHeader.biCompression = BI_RGB;
	bmpi.bmiHeader.biBitCount = 32;

	DirectBitmap = CreateDIBSection(NULL,
					&bmpi,
					DIB_RGB_COLORS,
					&ptPixels,
					NULL, 0);
 	GdiFlush();
	if (ptPixels) memset(ptPixels, 0, cx * cy * 4);
	if (bits != NULL) *bits = ptPixels;

	return DirectBitmap;
}

/*
*	 checkHasAlfa - checks if image has at least one BYTE in alpha channel
*				 that is not a 0. (is image real 32 bit or just 24 bit)
*/
BOOL FASTCALL checkHasAlfa(LPBYTE from, int width, int height)
{
	LPDWORD pt = (LPDWORD)from;
	LPDWORD lim = pt + width * height;
	while(pt < lim)
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
BOOL FASTCALL checkMaskUsed(LPBYTE from)
{
	int i;
	for (i=0; i < 16 * 16 / 8; i++)
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
DWORD FASTCALL blend(DWORD X1,DWORD X2)
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
HICON FASTCALL CreateJoinedIcon(HICON hBottom, HICON hTop)
{
	BOOL drawn = FALSE;
	HDC tempDC, tempDC2, tempDC3;
	HICON res = NULL;
	HBITMAP oImage,nImage;
	HBITMAP nMask, hbm, obmp, obmp2;
	LPBYTE ptPixels = NULL;
	ICONINFO iNew = { 0 };
	BYTE p[32] = { 0 };

	tempDC = CreateCompatibleDC(NULL);
	nImage = CreateBitmap32Point(16, 16, (LPVOID*)&ptPixels);
	oImage = (HBITMAP)SelectObject(tempDC, nImage);

//	if (ptPixels) memset(ptPixels, 0, 16 * 16 * 4);

	if (IsWinVerXPPlus())
	{
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

				for(y = 0; y < 16; y++)
				{
					for(x = 0; x < 16; x++)
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
	}

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

HANDLE FASTCALL GetIconIndexFromFI(LPTSTR szMirVer)
{
	short base, overlay, overlay2, overlay3;
	GetIconsIndexes(szMirVer, &base, &overlay, &overlay2, &overlay3);
	if (base == -1 || nFICount == 0xFFFF)
		return INVALID_HANDLE_VALUE;

	// MAX: 1024 + 256 + 128 + 128
	DWORD val = (base << 22) | ((overlay & 0xFF) << 14) | ((overlay2 & 0x7F) << 7) | (overlay3 & 0x7F);

	int i;
	HANDLE hFoundImage = INVALID_HANDLE_VALUE;
	for (i=0; i < nFICount; i++) {
		if (fiList[i].dwArray == val) {
			hFoundImage = fiList[i].hRegisteredImage;
			break;
		}
	}

	if (hFoundImage == INVALID_HANDLE_VALUE && i == nFICount) { //not found - then add
		HICON hIcon = CreateIconFromIndexes(base, overlay, overlay2, overlay3);

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
