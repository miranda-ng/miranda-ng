/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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
static volatile LONG iStaticCount = 1;
HANDLE hIconsChangedEvent;

HICON hIconBlank = nullptr;

BOOL bNeedRebuild = FALSE;

mir_cs csIconList;

static int sttCompareSections(const SectionItem *p1, const SectionItem *p2)
{
	return mir_wstrcmp(p1->name, p2->name);
}

LIST<SectionItem> sectionList(20, sttCompareSections);

static int sttCompareIconSourceFiles(const IconSourceFile *p1, const IconSourceFile *p2)
{
	return mir_wstrcmpi(p1->file, p2->file);
}

static LIST<IconSourceFile> iconSourceFileList(10, sttCompareIconSourceFiles);

static LIST<IconSourceItem> iconSourceList(20, &IconSourceItem::compare);

static int sttCompareIcons(const IcolibItem *p1, const IcolibItem *p2)
{
	return mir_strcmp(p1->name, p2->name);
}

LIST<IcolibItem> iconList(20, sttCompareIcons);

/////////////////////////////////////////////////////////////////////////////////////////
// Utility functions

void SafeDestroyIcon(HICON &hIcon)
{
	if (hIcon != nullptr) {
		DestroyIcon(hIcon);
		hIcon = nullptr;
	}
}

// Helper functions to manage Icon resources

static IcolibItem *Handle2Ptr(HANDLE hIcoLib)
{
	IcolibItem *p = (IcolibItem *)hIcoLib;
	if (p == nullptr)
		return nullptr;

	__try {
		if (p->signature != ICOLIB_MAGIC)
			p = nullptr;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		p = nullptr;
	}
	return p;
}

static IconSourceFile* IconSourceFile_Get(const wchar_t *file, bool isPath)
{
	if (!file)
		return nullptr;

	IconSourceFile key;
	if (isPath)
		PathToAbsoluteW(file, key.file); /// TODO: convert path to long - eliminate duplicate items
	else
		wcsncpy_s(key.file, file, _TRUNCATE);

	IconSourceFile *p = iconSourceFileList.find(&key);
	if (p != nullptr) {
		p->ref_count++;
		return p;
	}

	// store only needed number of chars
	size_t cbLen = wcslen(key.file) + 1;
	p = (IconSourceFile*)mir_alloc(sizeof(int) + sizeof(wchar_t)*cbLen);
	p->ref_count = 1;
	wcsncpy_s(p->file, cbLen, key.file, _TRUNCATE);
	iconSourceFileList.insert(p);
	return p;
}

static int IconSourceFile_Release(IconSourceFile *pItem)
{
	if (pItem == nullptr || !pItem->ref_count)
		return 1;

	if (--pItem->ref_count <= 0) {
		int indx;
		if ((indx = iconSourceFileList.getIndex(pItem)) != -1) {
			iconSourceFileList.remove(indx);
			mir_free(pItem);
		}
	}
	return 0;
}

static int BytesPerScanLine(int PixelsPerScanline, int BitsPerPixel, int Alignment)
{
	Alignment--;
	int bytes = ((PixelsPerScanline * BitsPerPixel) + Alignment) & ~Alignment;
	return bytes / 8;
}

static int InitializeBitmapInfoHeader(HBITMAP bitmap, BITMAPINFOHEADER *bi)
{
	DIBSECTION DS;
	DS.dsBmih.biSize = 0;
	int bytes = GetObject(bitmap, sizeof(DS), &DS);
	if (bytes == 0) // Failure
		return 1;

	if ((bytes >= (sizeof(DS.dsBm) + sizeof(DS.dsBmih))) && (DS.dsBmih.biSize >= uint32_t(sizeof(DS.dsBmih))))
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

static int InternalGetDIBSizes(HBITMAP bitmap, int *InfoHeaderSize, int *ImageSize)
{
	BITMAPINFOHEADER bi;
	if (InitializeBitmapInfoHeader(bitmap, &bi)) // Failure
		return 1;
	
	*InfoHeaderSize = sizeof(bi);
	if (bi.biBitCount > 8) {
		if ((bi.biCompression & BI_BITFIELDS) != 0)
			*InfoHeaderSize += 12;
	}
	else {
		if (bi.biClrUsed == 0)
			*InfoHeaderSize += sizeof(RGBQUAD) * (int)(1 << bi.biBitCount);
		else
			*InfoHeaderSize += sizeof(RGBQUAD) * bi.biClrUsed;
	}
	*ImageSize = bi.biSizeImage;
	return 0; // Success
}

static int InternalGetDIB(HBITMAP bitmap, HPALETTE palette, void *bitmapInfo, void *Bits)
{
	if (InitializeBitmapInfoHeader(bitmap, (BITMAPINFOHEADER*)bitmapInfo)) return 1; // Failure

	HDC DC = CreateCompatibleDC(nullptr);
	HPALETTE oldPal;
	if (palette) {
		oldPal = SelectPalette(DC, palette, FALSE);
		RealizePalette(DC);
	}
	else oldPal = nullptr;

	int result = GetDIBits(DC, bitmap, 0, ((BITMAPINFOHEADER*)bitmapInfo)->biHeight, Bits, (BITMAPINFO*)bitmapInfo, DIB_RGB_COLORS) == 0;

	if (oldPal) SelectPalette(DC, oldPal, FALSE);
	DeleteDC(DC);
	return result;
}

#define VER30 0x00030000

IconSourceItem::IconSourceItem(const IconSourceItemKey &_key)
	: key(_key),
	ref_count(1)
{
}

IconSourceItem::~IconSourceItem()
{
	IconSourceFile_Release(key.file);
	SafeDestroyIcon(icon);
	mir_free(icon_data);
}

HICON IconSourceItem::getIcon()
{
	if (icon) {
		icon_ref_count++;
		return icon;
	}

	if (icon_size) {
		icon = CreateIconFromResourceEx(icon_data, icon_size, TRUE, VER30, key.cx, key.cy, LR_COLOR);
		if (icon) {
			icon_ref_count++;
			return icon;
		}
	}

	_ExtractIconEx(key.file->file, key.indx, key.cx, key.cy, &icon, LR_COLOR);
	if (icon)
		icon_ref_count++;

	return icon;
}

int IconSourceItem::getIconData(HICON hIcon)
{
	icon_size = 0;

	ICONINFO iconInfo;
	if (!GetIconInfo(hIcon, &iconInfo))
		return 1; // Failure

	int MonoInfoSize, ColorInfoSize, MonoBitsSize, ColorBitsSize;
	if (InternalGetDIBSizes(iconInfo.hbmMask, &MonoInfoSize, &MonoBitsSize)
		|| InternalGetDIBSizes(iconInfo.hbmColor, &ColorInfoSize, &ColorBitsSize)) {
		DeleteObject(iconInfo.hbmColor);
		DeleteObject(iconInfo.hbmMask);
		return 1; // Failure
	}

	mir_ptr<uint8_t>
		MonoInfo((uint8_t*)mir_calloc(MonoInfoSize)),
		MonoBits((uint8_t*)mir_calloc(MonoBitsSize)),
		ColorInfo((uint8_t*)mir_calloc(ColorInfoSize)),
		ColorBits((uint8_t*)mir_calloc(ColorBitsSize));

	if (InternalGetDIB(iconInfo.hbmMask, nullptr, MonoInfo, MonoBits)
		|| InternalGetDIB(iconInfo.hbmColor, nullptr, ColorInfo, ColorBits)) {
		DeleteObject(iconInfo.hbmColor);
		DeleteObject(iconInfo.hbmMask);
		return 1; // Failure
	}

	icon_size = ColorInfoSize + ColorBitsSize + MonoBitsSize;
	icon_data = (uint8_t*)mir_alloc(icon_size);

	uint8_t *buf = icon_data;
	((BITMAPINFOHEADER*)(uint8_t*)ColorInfo)->biHeight *= 2; // color height includes mono bits
	memcpy(buf, ColorInfo, ColorInfoSize);
	buf += ColorInfoSize;
	memcpy(buf, ColorBits, ColorBitsSize);
	buf += ColorBitsSize;
	memcpy(buf, MonoBits, MonoBitsSize);

	DeleteObject(iconInfo.hbmColor);
	DeleteObject(iconInfo.hbmMask);
	return 0; // Success
}

int IconSourceItem::releaseIcon()
{
	if (this == nullptr || icon_ref_count == 0)
		return 1; // Failure

	icon_ref_count--;
	if (!icon_ref_count) {
		if (!icon_size)
			if (getIconData(icon))
				icon_size = 0; // Failure
		SafeDestroyIcon(icon);
	}

	return 0; // Success
}

int IconSourceItem::compare(const IconSourceItem *p1, const IconSourceItem *p2)
{
	if (p1->key.indx < p2->key.indx)
		return -1;

	if (p1->key.indx > p2->key.indx)
		return 1;

	if (p1->key.cx < p2->key.cx)
		return -1;

	if (p1->key.cx > p2->key.cx)
		return 1;

	if (p1->key.cy < p2->key.cy)
		return -1;

	if (p1->key.cy > p2->key.cy)
		return 1;

	if (p1->key.file == p2->key.file)
		return 0;

	return (p1->key.file > p2->key.file) ? 1 : -1;
}

IconSourceItem* GetIconSourceItem(const wchar_t *file, int indx, int cxIcon, int cyIcon)
{
	if (!file)
		return nullptr;

	IconSourceFile *r_file = IconSourceFile_Get(file, true);
	IconSourceItemKey key = { r_file, indx, cxIcon, cyIcon };
	int ix;
	if ((ix = iconSourceList.getIndex((IconSourceItem*)&key)) != -1) {
		IconSourceFile_Release(r_file);
		iconSourceList[ix]->addRef();
		return iconSourceList[ix];
	}

	IconSourceItem *newItem = new IconSourceItem(key);
	iconSourceList.insert(newItem);
	return newItem;
}

IconSourceItem* GetIconSourceItemFromPath(const wchar_t *path, int cxIcon, int cyIcon)
{
	if (!path)
		return nullptr;

	wchar_t file[MAX_PATH];
	mir_wstrncpy(file, path, _countof(file));
	wchar_t *comma = wcsrchr(file, ',');

	int n;
	if (!comma)
		n = 0;
	else {
		n = _wtoi(comma + 1);
		*comma = 0;
	}
	return GetIconSourceItem(file, n, cxIcon, cyIcon);
}

IconSourceItem* CreateStaticIconSourceItem(int cxIcon, int cyIcon)
{
	wchar_t tszName[100];
	mir_snwprintf(tszName, L"*StaticIcon_%d", iStaticCount++);

	IconSourceItemKey key = { IconSourceFile_Get(tszName, false), 0, cxIcon, cyIcon };
	IconSourceItem *newItem = new IconSourceItem(key);
	iconSourceList.insert(newItem);
	return newItem;
}

int IconSourceItem::release()
{
	if (this == nullptr || ref_count <= 0)
		return 1;

	ref_count--;
	if (!ref_count) {
		int indx;
		if ((indx = iconSourceList.getIndex(this)) != -1) {
			iconSourceList.remove(indx);
			delete this;
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Service functions

static SectionItem* IcoLib_AddSection(wchar_t *sectionName, BOOL create_new)
{
	if (!sectionName)
		return nullptr;

	int indx;
	if ((indx = sectionList.getIndex((SectionItem*)&sectionName)) != -1)
		return sectionList[indx];

	if (create_new) {
		SectionItem *newItem = new SectionItem();
		newItem->name = mir_wstrdup(sectionName);
		sectionList.insert(newItem);
		bNeedRebuild = TRUE;
		return newItem;
	}

	return nullptr;
}

static void IcoLib_RemoveSection(SectionItem *section)
{
	if (!section)
		return;

	int indx;
	if ((indx = sectionList.getIndex(section)) != -1) {
		sectionList.remove(indx);
		delete section;
		bNeedRebuild = TRUE;
	}
}

IcolibItem* IcoLib_FindIcon(const char *pszIconName)
{
	int indx = iconList.getIndex((IcolibItem*)&pszIconName);
	return (indx != -1) ? iconList[indx] : nullptr;
}

IcolibItem* IcoLib_FindHIcon(HICON hIcon, bool &big)
{
	if (hIcon == nullptr)
		return nullptr;

	for (auto &p : iconList) {
		if ((void*)p == hIcon) {
			big = (p->source_small == nullptr);
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

	return nullptr;
}

void IcolibItem::clear()
{
	mir_free(name);
	mir_free(description);
	mir_free(temp_file);
	if (section) {
		if (!--section->ref_count)
			IcoLib_RemoveSection(section);
		section = nullptr;
	}
	IconSourceFile_Release(default_file);
	source_small->release(); source_small = nullptr;
	source_big->release(); source_big = nullptr;
	default_icon->release(); default_icon = nullptr;
	SafeDestroyIcon(temp_icon);
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_AddIcon

MIR_APP_DLL(HANDLE) IcoLib_AddIcon(const SKINICONDESC *sid, HPLUGIN pPlugin)
{
	mir_cslock lck(csIconList);

	IcolibItem *item = IcoLib_FindIcon(sid->pszName);
	if (!item) {
		item = new IcolibItem();
		item->name = sid->pszName;
		iconList.insert(item);
	}
	else item->clear();

	item->name = mir_strdup(sid->pszName);
	if (sid->flags & SIDF_UNICODE) {
		item->description = mir_wstrdup(sid->description.w);
		item->section = IcoLib_AddSection(sid->section.w, TRUE);
	}
	else {
		item->description = mir_a2u(sid->description.a);
		item->section = IcoLib_AddSection(_A2T(sid->section.a), TRUE);
	}

	if (item->section) {
		item->section->ref_count++;
		item->orderID = ++item->section->maxOrder;
	}
	else item->orderID = 0;

	if (sid->defaultFile.a) {
		if (sid->flags & SIDF_PATH_UNICODE)
			item->default_file = IconSourceFile_Get(sid->defaultFile.w, true);
		else
			item->default_file = IconSourceFile_Get(_A2T(sid->defaultFile.a), true);
	}
	item->default_indx = sid->iDefaultIndex;

	item->cx = sid->cx;
	item->cy = sid->cy;
	item->pPlugin = pPlugin;

	if (sid->hDefaultIcon) {
		bool big;
		IcolibItem *def_item = IcoLib_FindHIcon(sid->hDefaultIcon, big);
		if (def_item) {
			item->default_icon = big ? def_item->source_big : def_item->source_small;
			item->default_icon->addRef();
		}
		else {
			int cx = item->cx ? item->cx : g_iIconSX;
			int cy = item->cy ? item->cy : g_iIconSY;
			item->default_icon = CreateStaticIconSourceItem(cx, cy);
			if (item->default_icon->getIconData(sid->hDefaultIcon)) {
				item->default_icon->release();
				item->default_icon = nullptr;
			}
		}
	}

	if (item->section)
		item->section->flags = sid->flags & SIDF_SORTED;

	if (pPlugin && item->default_indx)
		((CMPluginBase*)pPlugin)->addIcolib(item);

	return item;
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_ReleaseIcon

static int ReleaseIconInternal(IcolibItem *item, bool big)
{
	if (item == nullptr)
		return 1;

	IconSourceItem *source = big && !item->cx ? item->source_big : item->source_small;
	if (source && source->icon_ref_count) {
		source->releaseIcon();
		return 0;
	}

	return 1;
}

MIR_APP_DLL(int) IcoLib_ReleaseIcon(HICON hIcon, bool big)
{
	if (hIcon == nullptr)
		return 1;

	mir_cslock lck(csIconList);

	// this call might change the 'big' parameter
	// if inserted into the call of ReleaseIconInternal(), the unchanged 'big' will be passed first
	IcolibItem *pItem = IcoLib_FindHIcon(hIcon, big);
	return ReleaseIconInternal(pItem, big);
}

MIR_APP_DLL(int) IcoLib_Release(const char *szIconName, bool big)
{
	if (szIconName == nullptr)
		return 1;

	mir_cslock lck(csIconList);
	return ReleaseIconInternal(IcoLib_FindIcon(szIconName), big);
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_RemoveIcon

MIR_APP_DLL(void) IcoLib_RemoveIcon(const char *pszIconName)
{
	mir_cslock lck(csIconList);

	int i = iconList.indexOf((IcolibItem*)&pszIconName);
	if (i != -1) {
		iconList.remove(i);
		delete iconList[i];
	}
}

MIR_APP_DLL(void) IcoLib_RemoveIconByHandle(HANDLE hIcoLib)
{
	auto *pItem = Handle2Ptr(hIcoLib);

	mir_cslock lck(csIconList);
	int i = iconList.getIndex(pItem);
	if (i != -1) {
		iconList.remove(i);
		delete iconList[i];
	}
}

void KillModuleIcons(CMPluginBase *pPlugin)
{
	if (!bModuleInitialized)
		return;

	mir_cslock lck(csIconList);
	for (auto &it : iconList.rev_iter())
		if (it->pPlugin == pPlugin)
			delete iconList.removeItem(&it);
}

/////////////////////////////////////////////////////////////////////////////////////////
// IconItem_GetDefaultIcon

HICON IconItem_GetDefaultIcon(IcolibItem *item, bool big)
{
	HICON hIcon = nullptr;

	if (item->default_icon && !big) {
		item->source_small->release();
		item->source_small = item->default_icon;
		item->source_small->addRef();
		hIcon = item->source_small->getIcon();
	}

	if (!hIcon && item->default_file) {
		int cx = item->cx ? item->cx : (big ? g_iIconX : g_iIconSX);
		int cy = item->cy ? item->cy : (big ? g_iIconY : g_iIconSY);
		IconSourceItem *def_icon = GetIconSourceItem(item->default_file->file, item->default_indx, cx, cy);
		if (big) {
			if (def_icon != item->source_big) {
				item->source_big->release();
				item->source_big = def_icon;
				if (def_icon) {
					def_icon->addRef();
					hIcon = def_icon->getIcon();
				}
			}
			else def_icon->release();
		}
		else {
			if (def_icon != item->default_icon) {
				item->default_icon->release();
				item->default_icon = def_icon;
				if (def_icon) {
					item->source_small->release();
					item->source_small = def_icon;
					def_icon->addRef();
					hIcon = def_icon->getIcon();
				}
			}
			else def_icon->release();
		}
	}
	return hIcon;
}

/////////////////////////////////////////////////////////////////////////////////////////
// IconItem_GetIcon

HICON IconItem_GetIcon(HANDLE hIcoLib, bool big)
{
	auto *item = Handle2Ptr(hIcoLib);
	if (item == nullptr)
		return nullptr;

	big = big && !item->cx;
	IconSourceItem* &source = big ? item->source_big : item->source_small;
	if (source == nullptr) {
		ptrW tszCustomPath(db_get_wsa(0, "SkinIcons", item->name));
		if (tszCustomPath != nullptr) {
			wchar_t tszFullPath[MAX_PATH];
			PathToAbsoluteW(tszCustomPath, tszFullPath);
			int cx = item->cx ? item->cx : (big ? g_iIconX : g_iIconSX);
			int cy = item->cy ? item->cy : (big ? g_iIconY : g_iIconSY);
			source = GetIconSourceItemFromPath(tszFullPath, cx, cy);
		}
	}

	HICON hIcon = nullptr;
	if (source)
		hIcon = source->getIcon();

	if (!hIcon)
		hIcon = IconItem_GetDefaultIcon(item, big);

	if (!hIcon)
		return hIconBlank;

	return hIcon;
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_GetIcon
// lParam: pszIconName
// wParam: PLOADIMAGEPARAM or nullptr.
// if wParam == nullptr, default is used:
//     uType = IMAGE_ICON
//     cx/cyDesired = GetSystemMetrics(SM_CX/CYSMICON)
//     fuLoad = 0

MIR_APP_DLL(HICON) IcoLib_GetIcon(const char* pszIconName, bool big)
{
	if (!pszIconName)
		return hIconBlank;

	mir_cslock lck(csIconList);
	IcolibItem *item = IcoLib_FindIcon(pszIconName);
	return (item) ? IconItem_GetIcon(item, big) : nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_GetIconHandle
// lParam: pszIconName

MIR_APP_DLL(HANDLE) IcoLib_GetIconHandle(const char *pszIconName)
{
	if (!pszIconName)
		return nullptr;

	mir_cslock lck(csIconList);
	return IcoLib_FindIcon(pszIconName);
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_GetIconByHandle
// lParam: icolib item handle
// wParam: 0

MIR_APP_DLL(HICON) IcoLib_GetIconByHandle(HANDLE hItem, bool big)
{
	IcolibItem *pi = Handle2Ptr(hItem);
	if (pi == nullptr)
		return nullptr;

	mir_cslock lck(csIconList);
	if (iconList.getIndex(pi) != -1)
		return IconItem_GetIcon(pi, big);

	return hIconBlank;
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_IsManaged
// lParam: nullptr
// wParam: HICON

MIR_APP_DLL(HANDLE) IcoLib_IsManaged(HICON hIcon)
{
	mir_cslock lck(csIconList);

	bool big;
	return IcoLib_FindHIcon(hIcon, big);
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_AddRef
// lParam: nullptr
// wParam: HICON

MIR_APP_DLL(int) IcoLib_AddRef(HICON hIcon)
{
	mir_cslock lck(csIconList);

	bool big;
	IcolibItem *item = IcoLib_FindHIcon(hIcon, big);
	if (item) {
		IconSourceItem *source = big && !item->cx ? item->source_big : item->source_small;
		if (source->icon_ref_count) {
			source->icon_ref_count++;
			return 0;
		}
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Module initialization and finalization procedure

int LoadIcoLibModule(void)
{
	bModuleInitialized = TRUE;

	hIconBlank = LoadIconEx(g_plugin.getInst(), MAKEINTRESOURCE(IDI_BLANK), 0);

	hIconsChangedEvent = CreateHookableEvent(ME_SKIN_ICONSCHANGED);

	HookEvent(ME_OPT_INITIALISE, SkinOptionsInit);
	return 0;
}

void UnloadIcoLibModule(void)
{
	if (!bModuleInitialized)
		return;

	DestroyHookableEvent(hIconsChangedEvent);

	for (auto &p : iconList)
		delete p;		
	iconList.destroy();

	for (auto &p : iconSourceList)
		delete p;
	iconSourceList.destroy();

	for (auto &p : iconSourceFileList)
		mir_free(p);
	iconSourceFileList.destroy();

	for (auto &p : sectionList)
		delete p;

	SafeDestroyIcon(hIconBlank);
	bModuleInitialized = false;
}
