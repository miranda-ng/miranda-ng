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

#pragma once

#define ICOLIB_MAGIC 0xDEADBEEF

#define SECTIONPARAM_MAKE(index, level, flags) MAKELONG((index)&0xFFFF, MAKEWORD(level, flags))
#define SECTIONPARAM_INDEX(lparam) LOWORD(lparam)
#define SECTIONPARAM_LEVEL(lparam) LOBYTE(HIWORD(lparam))
#define SECTIONPARAM_FLAGS(lparam) HIBYTE(HIWORD(lparam))
#define SECTIONPARAM_HAVEPAGE	0x0001

struct SectionItem : public MZeroedObject
{
	ptrW name;
	int  flags, maxOrder, ref_count;
};

struct IconSourceFile
{
	int ref_count;
	wchar_t file[MAX_PATH];
};

struct IconSourceItemKey
{
	IconSourceFile* file;
	int   indx;
	int   cx, cy;
};

class IconSourceItem : public MZeroedObject
{
	IconSourceItemKey key;
	int   ref_count;

	uint8_t* icon_data;
	int   icon_size;

public:
	IconSourceItem(const IconSourceItemKey&);
	~IconSourceItem();

	__inline void addRef() { ref_count++; }
	int release();

	HICON getIcon();
	int   getIconData(HICON icon);
	int   releaseIcon();

	static int compare(const IconSourceItem *p1, const IconSourceItem *p2);

public:
	HICON icon;
	int   icon_ref_count;
};

struct IcolibItem : public MZeroedObject
{
	char*           name;
	uint32_t           signature = ICOLIB_MAGIC;
	SectionItem*    section;
	int             orderID;
	wchar_t*        description;
	IconSourceFile* default_file;
	int             default_indx;
	int             cx, cy;
	HPLUGIN         pPlugin;

	IconSourceItem* source_small;
	IconSourceItem* source_big;
	IconSourceItem* default_icon;

	wchar_t*        temp_file;
	HICON           temp_icon;
	BOOL            temp_reset;

	__inline ~IcolibItem() { clear(); }
	__inline wchar_t* getDescr() const { return TranslateW_LP(description, pPlugin); }

	void clear();
};

// extracticon.c

UINT _ExtractIconEx(LPCTSTR lpszFile, int iconIndex, int cxIcon, int cyIcon, HICON *phicon, UINT flags);

void SafeDestroyIcon(HICON &icon);

IconSourceItem* GetIconSourceItem(const wchar_t* file, int indx, int cxIcon, int cyIcon);

IcolibItem* IcoLib_FindHIcon(HICON hIcon, bool &big);
IcolibItem* IcoLib_FindIcon(const char* pszIconName);

int SkinOptionsInit(WPARAM, LPARAM);

extern mir_cs csIconList;
extern LIST<IcolibItem> iconList;
extern LIST<SectionItem> sectionList;

extern BOOL bNeedRebuild;
extern HICON hIconBlank;
extern HANDLE hIconsChangedEvent;
