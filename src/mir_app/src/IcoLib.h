/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (�) 2012-16 Miranda NG project (http://miranda-ng.org),
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

#define SECTIONPARAM_MAKE(index, level, flags) MAKELONG((index)&0xFFFF, MAKEWORD(level, flags))
#define SECTIONPARAM_INDEX(lparam) LOWORD(lparam)
#define SECTIONPARAM_LEVEL(lparam) LOBYTE(HIWORD(lparam))
#define SECTIONPARAM_FLAGS(lparam) HIBYTE(HIWORD(lparam))
#define SECTIONPARAM_HAVEPAGE	0x0001

struct SectionItem : public MZeroedObject
{
	ptrT name;
	int  flags, maxOrder, ref_count;
};

struct IconSourceFile
{
	int ref_count;
	TCHAR file[MAX_PATH];
};

struct IconSourceItem : public MZeroedObject
{
	__inline IconSourceItem(IconSourceFile *_file, int _indx, int _cxIcon, int _cyIcon) :
		file(_file), indx(_indx), cx(_cxIcon), cy(_cyIcon), ref_count(1)
		{}

	IconSourceFile* file;
	int          indx;
	int          cx, cy;

	int          ref_count;

	HICON        icon;
	int          icon_ref_count;

	BYTE*        icon_data;
	int          icon_size;
};

struct IcolibItem : public MZeroedObject
{
	char*           name;
	SectionItem*    section;
	int             orderID;
	TCHAR*          description;
	IconSourceFile* default_file;
	int             default_indx;
	int             cx, cy;
	int             hLangpack;

	IconSourceItem* source_small;
	IconSourceItem* source_big;
	IconSourceItem* default_icon;

	TCHAR*          temp_file;
	HICON           temp_icon;
	BOOL            temp_reset;

	__inline ~IcolibItem() { clear(); }
	__inline TCHAR* getDescr() const { return TranslateTH(hLangpack, description); }

	void clear();
};

// extracticon.c

UINT _ExtractIconEx(LPCTSTR lpszFile, int iconIndex, int cxIcon, int cyIcon, HICON *phicon, UINT flags);

void __fastcall SafeDestroyIcon(HICON &icon);

int   IconSourceItem_Release(IconSourceItem* &pitem);
int   IconSourceItem_ReleaseIcon(IconSourceItem *item);
HICON IconSourceItem_GetIcon(IconSourceItem *item);
IconSourceItem* GetIconSourceItem(const TCHAR* file, int indx, int cxIcon, int cyIcon);

IcolibItem* IcoLib_FindHIcon(HICON hIcon, bool &big);
IcolibItem* IcoLib_FindIcon(const char* pszIconName);

int SkinOptionsInit(WPARAM, LPARAM);

extern mir_cs csIconList;
extern LIST<IcolibItem> iconList;
extern LIST<SectionItem> sectionList;

extern BOOL bNeedRebuild;
extern int iconEventActive;
extern HICON hIconBlank;
extern HANDLE hIcons2ChangedEvent, hIconsChangedEvent;
