/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#include "IcoLib.h"

static BOOL bModuleInitialized = FALSE;
HANDLE hIcons2ChangedEvent, hIconsChangedEvent;

HICON hIconBlank = NULL;

int iconEventActive = 0;

BOOL bNeedRebuild = FALSE;

mir_cs csIconList;

static int sttCompareSections(const SectionItem* p1, const SectionItem* p2)
{
	return mir_tstrcmp(p1->name, p2->name);
}

LIST<SectionItem> sectionList(20, sttCompareSections);

static int sttCompareIconSourceFiles(const IconSourceFile* p1, const IconSourceFile* p2)
{
	return mir_tstrcmpi(p1->file, p2->file);
}

static LIST<IconSourceFile> iconSourceFileList(10, sttCompareIconSourceFiles);

static int sttCompareIconSourceItems(const IconSourceItem* p1, const IconSourceItem* p2)
{
	if (p1->indx < p2->indx)
		return -1;

	if (p1->indx > p2->indx)
		return 1;

	if (p1->cx < p2->cx)
		return -1;

	if (p1->cx > p2->cx)
		return 1;

	if (p1->cy < p2->cy)
		return -1;

	if (p1->cy > p2->cy)
		return 1;

	if (p1->file == p2->file)
		return 0;

	return (p1->file > p2->file) ? 1 : -1;
}

static LIST<IconSourceItem> iconSourceList(20, sttCompareIconSourceItems);

static int sttCompareIcons(const IcolibItem* p1, const IcolibItem* p2)
{
	return mir_strcmp(p1->name, p2->name);
}

LIST<IcolibItem> iconList(20, sttCompareIcons);

/////////////////////////////////////////////////////////////////////////////////////////
// Utility functions

void __fastcall SAFE_FREE(void** p)
{
	if (*p) {
		mir_free(*p);
		*p = NULL;
	}
}

void __fastcall SafeDestroyIcon(HICON* icon)
{
	if (*icon) {
		DestroyIcon(*icon);
		*icon = NULL;
	}
}

// Helper functions to manage Icon resources

IconSourceFile* IconSourceFile_Get(const TCHAR* file, bool isPath)
{
	TCHAR fileFull[MAX_PATH];

	if (!file)
		return NULL;

	if (isPath)
		PathToAbsoluteT(file, fileFull); /// TODO: convert path to long - eliminate duplicate items
	else
		_tcsncpy_s(fileFull, file, _TRUNCATE);

	IconSourceFile key = { fileFull };
	int ix;
	if ((ix = iconSourceFileList.getIndex(&key)) != -1) {
		iconSourceFileList[ix]->ref_count++;
		return iconSourceFileList[ix];
	}

	IconSourceFile* newItem = (IconSourceFile*)mir_calloc(sizeof(IconSourceFile));
	newItem->file = mir_tstrdup(fileFull);
	newItem->ref_count = 1;
	iconSourceFileList.insert(newItem);

	return newItem;
}

int IconSourceFile_Release(IconSourceFile** pitem)
{
	if (pitem && *pitem && (*pitem)->ref_count) {
		IconSourceFile* item = *pitem;
		if (--item->ref_count <= 0) {
			int indx;
			if ((indx = iconSourceFileList.getIndex(item)) != -1) {
				SAFE_FREE((void**)&item->file);
				iconSourceFileList.remove(indx);
				SAFE_FREE((void**)&item);
			}
		}
		*pitem = NULL;
		return 0;
	}
	return 1;
}

static int BytesPerScanLine(int PixelsPerScanline, int BitsPerPixel, int Alignment)
{
	Alignment--;
	int bytes = ((PixelsPerScanline * BitsPerPixel) + Alignment) & ~Alignment;
	return bytes / 8;
}

static int InitializeBitmapInfoHeader(HBITMAP bitmap, BITMAPINFOHEADER* bi)
{
	DIBSECTION DS;
	int bytes;

	DS.dsBmih.biSize = 0;
	bytes = GetObject(bitmap, sizeof(DS), &DS);
	if (bytes == 0) return 1; // Failure
	else if ((bytes >= (sizeof(DS.dsBm) + sizeof(DS.dsBmih))) &&
		(DS.dsBmih.biSize >= DWORD(sizeof(DS.dsBmih))))
		*bi = DS.dsBmih;
	else {
		memset(bi, 0, sizeof(BITMAPINFOHEADER));
		bi->biSize = sizeof(BITMAPINFOHEADER);
		bi->biWidth = DS.dsBm.bmWidth;
		bi->biHeight = DS.dsBm.bmHeight;
	}
	bi->biBitCount = DS.dsBm.bmBitsPixel * DS.dsBm.bmPlanes;
	bi->biPlanes = 1;
	if (bi->biClrImportant > bi->biClrUsed)
		bi->biClrImportant = bi->biClrUsed;
	if (!bi->biSizeImage)
		bi->biSizeImage = BytesPerScanLine(bi->biWidth, bi->biBitCount, 32) * abs(bi->biHeight);
	return 0; // Success
}

static int InternalGetDIBSizes(HBITMAP bitmap, int* InfoHeaderSize, int* ImageSize)
{
	BITMAPINFOHEADER bi;

	if (InitializeBitmapInfoHeader(bitmap, &bi)) return 1; // Failure
	if (bi.biBitCount > 8) {
		*InfoHeaderSize = sizeof(BITMAPINFOHEADER);
		if ((bi.biCompression & BI_BITFIELDS) != 0)
			*InfoHeaderSize += 12;
	}
	else {
		if (bi.biClrUsed == 0)
			*InfoHeaderSize = sizeof(BITMAPINFOHEADER) +
			sizeof(RGBQUAD) * (int)(1 << bi.biBitCount);
		else
			*InfoHeaderSize = sizeof(BITMAPINFOHEADER) +
			sizeof(RGBQUAD) * bi.biClrUsed;
	}
	*ImageSize = bi.biSizeImage;
	return 0; // Success
}

static int InternalGetDIB(HBITMAP bitmap, HPALETTE palette, void* bitmapInfo, void* Bits)
{
	HPALETTE oldPal = 0;

	if (InitializeBitmapInfoHeader(bitmap, (BITMAPINFOHEADER*)bitmapInfo)) return 1; // Failure

	HDC DC = CreateCompatibleDC(0);
	if (palette) {
		oldPal = SelectPalette(DC, palette, FALSE);
		RealizePalette(DC);
	}
	int result = GetDIBits(DC, bitmap, 0, ((BITMAPINFOHEADER*)bitmapInfo)->biHeight, Bits, (BITMAPINFO*)bitmapInfo, DIB_RGB_COLORS) == 0;

	if (oldPal) SelectPalette(DC, oldPal, FALSE);
	DeleteDC(DC);
	return result;
}

static int GetIconData(HICON icon, BYTE** data, int* size)
{
	ICONINFO iconInfo;
	int MonoInfoSize, ColorInfoSize;
	int MonoBitsSize, ColorBitsSize;

	if (!data || !size) return 1; // Failure

	if (!GetIconInfo(icon, &iconInfo)) return 1; // Failure

	if (InternalGetDIBSizes(iconInfo.hbmMask, &MonoInfoSize, &MonoBitsSize) ||
		InternalGetDIBSizes(iconInfo.hbmColor, &ColorInfoSize, &ColorBitsSize)) {
		DeleteObject(iconInfo.hbmColor);
		DeleteObject(iconInfo.hbmMask);
		return 1; // Failure
	}
	void* MonoInfo = mir_alloc(MonoInfoSize);
	void* MonoBits = mir_alloc(MonoBitsSize);
	void* ColorInfo = mir_alloc(ColorInfoSize);
	void* ColorBits = mir_alloc(ColorBitsSize);

	if (InternalGetDIB(iconInfo.hbmMask, 0, MonoInfo, MonoBits) ||
		InternalGetDIB(iconInfo.hbmColor, 0, ColorInfo, ColorBits)) {
		SAFE_FREE(&MonoInfo);
		SAFE_FREE(&MonoBits);
		SAFE_FREE(&ColorInfo);
		SAFE_FREE(&ColorBits);
		DeleteObject(iconInfo.hbmColor);
		DeleteObject(iconInfo.hbmMask);
		return 1; // Failure
	}

	*size = ColorInfoSize + ColorBitsSize + MonoBitsSize;
	*data = (BYTE*)mir_alloc(*size);

	BYTE* buf = *data;
	((BITMAPINFOHEADER*)ColorInfo)->biHeight *= 2; // color height includes mono bits
	memcpy(buf, ColorInfo, ColorInfoSize);
	buf += ColorInfoSize;
	memcpy(buf, ColorBits, ColorBitsSize);
	buf += ColorBitsSize;
	memcpy(buf, MonoBits, MonoBitsSize);

	SAFE_FREE(&MonoInfo);
	SAFE_FREE(&MonoBits);
	SAFE_FREE(&ColorInfo);
	SAFE_FREE(&ColorBits);
	DeleteObject(iconInfo.hbmColor);
	DeleteObject(iconInfo.hbmMask);
	return 0; // Success
}

#define VER30           0x00030000

HICON IconSourceItem_GetIcon(IconSourceItem* item)
{
	if (item->icon) {
		item->icon_ref_count++;
		return item->icon;
	}

	if (item->icon_size) {
		item->icon = CreateIconFromResourceEx(item->icon_data, item->icon_size, TRUE, VER30, item->cx, item->cy, LR_COLOR);
		if (item->icon) {
			item->icon_ref_count++;
			return item->icon;
		}
	}
	//SHOULD BE REPLACED WITH GOOD ENOUGH FUNCTION
	_ExtractIconEx(item->file->file, item->indx, item->cx, item->cy, &item->icon, LR_COLOR);

	if (item->icon)
		item->icon_ref_count++;

	return item->icon;
}

int IconSourceItem_ReleaseIcon(IconSourceItem* item)
{
	if (item && item->icon_ref_count) {
		item->icon_ref_count--;
		if (!item->icon_ref_count) {
			if (!item->icon_size)
				if (GetIconData(item->icon, &item->icon_data, &item->icon_size))
					item->icon_size = 0; // Failure
			SafeDestroyIcon(&item->icon);
		}
		return 0; // Success
	}
	return 1; // Failure
}

IconSourceItem* GetIconSourceItem(const TCHAR* file, int indx, int cxIcon, int cyIcon)
{
	if (!file)
		return NULL;

	IconSourceFile* r_file = IconSourceFile_Get(file, true);
	IconSourceItem key = { r_file, indx, cxIcon, cyIcon };
	int ix;
	if ((ix = iconSourceList.getIndex(&key)) != -1) {
		IconSourceFile_Release(&r_file);
		iconSourceList[ix]->ref_count++;
		return iconSourceList[ix];
	}

	IconSourceItem* newItem = (IconSourceItem*)mir_calloc(sizeof(IconSourceItem));
	newItem->file = r_file;
	newItem->indx = indx;
	newItem->ref_count = 1;
	newItem->cx = cxIcon;
	newItem->cy = cyIcon;
	iconSourceList.insert(newItem);

	return newItem;
}

IconSourceItem* GetIconSourceItemFromPath(const TCHAR* path, int cxIcon, int cyIcon)
{
	if (!path)
		return NULL;

	TCHAR file[MAX_PATH];
	mir_tstrncpy(file, path, _countof(file));
	TCHAR *comma = _tcsrchr(file, ',');

	int n;
	if (!comma)
		n = 0;
	else {
		n = _ttoi(comma + 1);
		*comma = 0;
	}
	return GetIconSourceItem(file, n, cxIcon, cyIcon);
}

IconSourceItem* CreateStaticIconSourceItem(int cxIcon, int cyIcon)
{
	TCHAR sourceName[MAX_PATH];
	IconSourceFile key = { sourceName };

	int i = 0;
	do { // find new unique name
		mir_sntprintf(sourceName, _countof(sourceName), _T("*StaticIcon_%d"), i++);
	} while (iconSourceFileList.getIndex(&key) != -1);

	IconSourceItem* newItem = (IconSourceItem*)mir_calloc(sizeof(IconSourceItem));
	newItem->file = IconSourceFile_Get(sourceName, false);
	newItem->indx = 0;
	newItem->ref_count = 1;
	newItem->cx = cxIcon;
	newItem->cy = cyIcon;
	iconSourceList.insert(newItem);

	return newItem;
}

int IconSourceItem_Release(IconSourceItem* &pitem)
{
	if (pitem == NULL || pitem->ref_count == 0)
		return 1;

	pitem->ref_count--;
	if (!pitem->ref_count) {
		int indx;
		if ((indx = iconSourceList.getIndex(pitem)) != -1) {
			IconSourceFile_Release(&pitem->file);
			SafeDestroyIcon(&pitem->icon);
			SAFE_FREE((void**)&pitem->icon_data);
			iconSourceList.remove(indx);
			SAFE_FREE((void**)&pitem);
		}
	}
	pitem = NULL;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Service functions

static SectionItem* IcoLib_AddSection(TCHAR *sectionName, BOOL create_new)
{
	if (!sectionName)
		return NULL;

	int indx;
	SectionItem key = { sectionName, 0 };
	if ((indx = sectionList.getIndex(&key)) != -1)
		return sectionList[indx];

	if (create_new) {
		SectionItem* newItem = (SectionItem*)mir_calloc(sizeof(SectionItem));
		newItem->name = mir_tstrdup(sectionName);
		newItem->flags = 0;
		sectionList.insert(newItem);
		bNeedRebuild = TRUE;
		return newItem;
	}

	return NULL;
}

static void IcoLib_RemoveSection(SectionItem* section)
{
	if (!section)
		return;

	int indx;
	if ((indx = sectionList.getIndex(section)) != -1) {
		sectionList.remove(indx);
		SAFE_FREE((void**)&section->name);
		SAFE_FREE((void**)&section);
		bNeedRebuild = TRUE;
	}
}

IcolibItem* IcoLib_FindIcon(const char* pszIconName)
{
	int indx = iconList.getIndex((IcolibItem*)&pszIconName);
	return (indx != -1) ? iconList[indx] : 0;
}

IcolibItem* IcoLib_FindHIcon(HICON hIcon, bool &big)
{
	if (hIcon == NULL)
		return NULL;

	for (int i = 0; i < iconList.getCount(); i++) {
		IcolibItem *p = iconList[i];
		if ((void*)p == hIcon) {
			big = (p->source_small == NULL);
			return p;
		}
		if (p->source_small && p->source_small->icon == hIcon) {
			big = false;
			return p;
		}
		if (p->source_big && p->source_big->icon == hIcon) {
			big = true;
			return p;
		}
	}

	return NULL;
}

static void IcoLib_FreeIcon(IcolibItem* icon)
{
	if (!icon) return;

	SAFE_FREE((void**)&icon->name);
	SAFE_FREE((void**)&icon->description);
	SAFE_FREE((void**)&icon->default_file);
	SAFE_FREE((void**)&icon->temp_file);
	if (icon->section) {
		if (!--icon->section->ref_count)
			IcoLib_RemoveSection(icon->section);
		icon->section = NULL;
	}
	IconSourceItem_Release(icon->source_small);
	IconSourceItem_Release(icon->source_big);
	IconSourceItem_Release(icon->default_icon);
	SafeDestroyIcon(&icon->temp_icon);
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_AddIcon

MIR_APP_DLL(HANDLE) IcoLib_AddIcon(SKINICONDESC *sid, int hLangpack)
{
	bool utf = (sid->flags & SIDF_UNICODE) != 0;
	bool utf_path = (sid->flags & SIDF_PATH_UNICODE) != 0;

	mir_cslock lck(csIconList);

	IcolibItem *item = IcoLib_FindIcon(sid->pszName);
	if (!item) {
		item = (IcolibItem*)mir_calloc(sizeof(IcolibItem));
		item->name = sid->pszName;
		iconList.insert(item);
	}
	else IcoLib_FreeIcon(item);

	item->name = mir_strdup(sid->pszName);
	if (utf) {
		item->description = mir_u2t(sid->description.w);
		item->section = IcoLib_AddSection(sid->section.w, TRUE);
	}
	else {
		item->description = mir_a2t(sid->description.a);
		WCHAR *pwszSection = sid->section.a ? mir_a2u(sid->section.a) : NULL;
		item->section = IcoLib_AddSection(pwszSection, TRUE);
		SAFE_FREE((void**)&pwszSection);
	}

	if (item->section) {
		item->section->ref_count++;
		item->orderID = ++item->section->maxOrder;
	}
	else item->orderID = 0;

	if (sid->defaultFile.a) {
		WCHAR fileFull[MAX_PATH];
		if (utf_path)
			PathToAbsoluteT(sid->defaultFile.w, fileFull);
		else
			PathToAbsoluteT(_A2T(sid->defaultFile.a), fileFull);
		item->default_file = mir_wstrdup(fileFull);
	}
	item->default_indx = sid->iDefaultIndex;

	item->cx = sid->cx;
	item->cy = sid->cy;
	item->hLangpack = hLangpack;

	if (sid->hDefaultIcon) {
		bool big;
		IcolibItem* def_item = IcoLib_FindHIcon(sid->hDefaultIcon, big);
		if (def_item) {
			item->default_icon = big ? def_item->source_big : def_item->source_small;
			item->default_icon->ref_count++;
		}
		else {
			int cx = item->cx ? item->cx : g_iIconSX;
			int cy = item->cy ? item->cy : g_iIconSY;
			item->default_icon = CreateStaticIconSourceItem(cx, cy);
			if (GetIconData(sid->hDefaultIcon, &item->default_icon->icon_data, &item->default_icon->icon_size))
				IconSourceItem_Release(item->default_icon);
		}
	}

	if (item->section)
		item->section->flags = sid->flags & SIDF_SORTED;

	return item;
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_ReleaseIcon

static int ReleaseIconInternal(IcolibItem *item, bool big)
{
	if (item == NULL)
		return 1;

	IconSourceItem *source = big && !item->cx ? item->source_big : item->source_small;
	if (source && source->icon_ref_count) {
		if (iconEventActive)
			source->icon_ref_count--;
		else
			IconSourceItem_ReleaseIcon(source);
		return 0;
	}

	return 1;
}

MIR_APP_DLL(int) IcoLib_ReleaseIcon(HICON hIcon, bool big)
{
	if (hIcon == NULL)
		return 1;

	mir_cslock lck(csIconList);
	return ReleaseIconInternal(IcoLib_FindHIcon(hIcon, big), big);
}

MIR_APP_DLL(int) IcoLib_Release(const char *szIconName, bool big)
{
	if (szIconName == NULL)
		return 1;

	mir_cslock lck(csIconList);
	return ReleaseIconInternal(IcoLib_FindIcon(szIconName), big);
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_RemoveIcon

static int IcoLib_RemoveIcon_Internal(int i)
{
	IcolibItem *item = iconList[i];
	IcoLib_FreeIcon(item);
	iconList.remove(i);
	SAFE_FREE((void**)&item);
	return 0;
}

MIR_APP_DLL(void) IcoLib_RemoveIcon(const char *pszIconName)
{
	mir_cslock lck(csIconList);

	int i = iconList.indexOf((IcolibItem*)&pszIconName);
	if (i != -1)
		IcoLib_RemoveIcon_Internal(i);
}

MIR_APP_DLL(void) IcoLib_RemoveIconByHandle(HANDLE hIcoLib)
{
	mir_cslock lck(csIconList);

	int i = iconList.getIndex((IcolibItem*)hIcoLib);
	if (i != -1)
		IcoLib_RemoveIcon_Internal(i);
}

void KillModuleIcons(int hLangpack)
{
	if (!bModuleInitialized)
		return;

	mir_cslock lck(csIconList);
	for (int i = iconList.getCount() - 1; i >= 0; i--) {
		IcolibItem *item = iconList[i];
		if (item->hLangpack == hLangpack) {
			IcoLib_FreeIcon(item);
			iconList.remove(i);
			SAFE_FREE((void**)&item);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// IconItem_GetDefaultIcon

HICON IconItem_GetDefaultIcon(IcolibItem* item, bool big)
{
	HICON hIcon = NULL;

	if (item->default_icon && !big) {
		IconSourceItem_Release(item->source_small);
		item->source_small = item->default_icon;
		item->source_small->ref_count++;
		hIcon = IconSourceItem_GetIcon(item->source_small);
	}

	if (!hIcon && item->default_file) {
		int cx = item->cx ? item->cx : (big ? g_iIconX : g_iIconSX);
		int cy = item->cy ? item->cy : (big ? g_iIconY : g_iIconSY);
		IconSourceItem *def_icon = GetIconSourceItem(item->default_file, item->default_indx, cx, cy);
		if (big) {
			if (def_icon != item->source_big) {
				IconSourceItem_Release(item->source_big);
				item->source_big = def_icon;
				if (def_icon) {
					def_icon->ref_count++;
					hIcon = IconSourceItem_GetIcon(def_icon);
				}
			}
			else IconSourceItem_Release(def_icon);
		}
		else {
			if (def_icon != item->default_icon) {
				IconSourceItem_Release(item->default_icon);
				item->default_icon = def_icon;
				if (def_icon) {
					IconSourceItem_Release(item->source_small);
					item->source_small = def_icon;
					def_icon->ref_count++;
					hIcon = IconSourceItem_GetIcon(def_icon);
				}
			}
			else IconSourceItem_Release(def_icon);
		}
	}
	return hIcon;
}

/////////////////////////////////////////////////////////////////////////////////////////
// IconItem_GetIcon

HICON IconItem_GetIcon(HANDLE hIcoLib, bool big)
{
	IcolibItem *item = (IcolibItem*)hIcoLib;
	if (item == NULL)
		return NULL;

	big = big && !item->cx;
	IconSourceItem* &source = big ? item->source_big : item->source_small;

	DBVARIANT dbv = { 0 };
	if (!source && !db_get_ts(NULL, "SkinIcons", item->name, &dbv)) {
		TCHAR tszFullPath[MAX_PATH];
		PathToAbsoluteT(dbv.ptszVal, tszFullPath);
		int cx = item->cx ? item->cx : (big ? g_iIconX : g_iIconSX);
		int cy = item->cy ? item->cy : (big ? g_iIconY : g_iIconSY);
		source = GetIconSourceItemFromPath(tszFullPath, cx, cy);
		db_free(&dbv);
	}

	HICON hIcon = NULL;
	if (source)
		hIcon = IconSourceItem_GetIcon(source);

	if (!hIcon)
		hIcon = IconItem_GetDefaultIcon(item, big);

	if (!hIcon)
		return hIconBlank;

	return hIcon;
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_GetIcon
// lParam: pszIconName
// wParam: PLOADIMAGEPARAM or NULL.
// if wParam == NULL, default is used:
//     uType = IMAGE_ICON
//     cx/cyDesired = GetSystemMetrics(SM_CX/CYSMICON)
//     fuLoad = 0

MIR_APP_DLL(HICON) IcoLib_GetIcon(const char* pszIconName, bool big)
{
	if (!pszIconName)
		return hIconBlank;

	mir_cslock lck(csIconList);
	IcolibItem* item = IcoLib_FindIcon(pszIconName);
	return (item) ? IconItem_GetIcon(item, big) : NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_GetIconHandle
// lParam: pszIconName

MIR_APP_DLL(HANDLE) IcoLib_GetIconHandle(const char *pszIconName)
{
	if (!pszIconName)
		return NULL;

	mir_cslock lck(csIconList);
	return IcoLib_FindIcon(pszIconName);
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_GetIconByHandle
// lParam: icolib item handle
// wParam: 0

MIR_APP_DLL(HICON) IcoLib_GetIconByHandle(HANDLE hItem, bool big)
{
	if (hItem == NULL)
		return NULL;

	mir_cslock lck(csIconList);
	IcolibItem *pi = (IcolibItem*)hItem;
	if (iconList.getIndex(pi) != -1)
		return IconItem_GetIcon(pi, big);

	return hIconBlank;
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_IsManaged
// lParam: NULL
// wParam: HICON

MIR_APP_DLL(HANDLE) IcoLib_IsManaged(HICON hIcon)
{
	mir_cslock lck(csIconList);

	bool big;
	return IcoLib_FindHIcon(hIcon, big);
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_AddRef
// lParam: NULL
// wParam: HICON

MIR_APP_DLL(int) IcoLib_AddRef(HICON hIcon)
{
	mir_cslock lck(csIconList);

	bool big;
	IcolibItem *item = IcoLib_FindHIcon(hIcon, big);
	if (item) {
		IconSourceItem* source = big && !item->cx ? item->source_big : item->source_small;
		if (source->icon_ref_count) {
			source->icon_ref_count++;
			return 0;
		}
	}

	return 1;
}

static int SkinSystemModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_OPT_INITIALISE, SkinOptionsInit);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Module initialization and finalization procedure

int LoadIcoLibModule(void)
{
	bModuleInitialized = TRUE;

	hIconBlank = LoadIconEx(g_hInst, MAKEINTRESOURCE(IDI_BLANK), 0);

	hIcons2ChangedEvent = CreateHookableEvent(ME_SKIN2_ICONSCHANGED);
	hIconsChangedEvent = CreateHookableEvent(ME_SKIN_ICONSCHANGED);

	HookEvent(ME_SYSTEM_MODULESLOADED, SkinSystemModulesLoaded);

	return 0;
}

void UnloadIcoLibModule(void)
{
	if (!bModuleInitialized)
		return;

	DestroyHookableEvent(hIconsChangedEvent);
	DestroyHookableEvent(hIcons2ChangedEvent);

	for (int i = iconList.getCount() - 1; i >= 0; i--) {
		IcolibItem* p = iconList[i];
		iconList.remove(i);
		IcoLib_FreeIcon(p);
		mir_free(p);
	}

	for (int i = iconSourceList.getCount() - 1; i >= 0; i--) {
		IconSourceItem* p = iconSourceList[i];
		iconSourceList.remove(i);
		IconSourceFile_Release(&p->file);
		SafeDestroyIcon(&p->icon);
		SAFE_FREE((void**)&p->icon_data);
		SAFE_FREE((void**)&p);
	}

	for (int i = iconSourceFileList.getCount() - 1; i >= 0; i--) {
		IconSourceFile* p = iconSourceFileList[i];
		iconSourceFileList.remove(i);
		SAFE_FREE((void**)&p->file);
		SAFE_FREE((void**)&p);
	}

	for (int i = 0; i < sectionList.getCount(); i++) {
		SAFE_FREE((void**)&sectionList[i]->name);
		mir_free(sectionList[i]);
	}

	SafeDestroyIcon(&hIconBlank);
	bModuleInitialized = false;
}
