/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project, 
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
typedef struct 
{
	TCHAR* name;
	int    flags;
	int    maxOrder;
	int    ref_count;
}
	SectionItem;

typedef struct
{
	TCHAR*       file;
	int          ref_count;
}
	IconSourceFile;

typedef struct
{
	IconSourceFile* file;
	int          indx;
	int          cx, cy;

	int          ref_count;

	HICON        icon;
	int          icon_ref_count;

	BYTE*        icon_data;
	int          icon_size;
}
	IconSourceItem;

typedef struct
{
	char*        name;
	SectionItem* section;
	int          orderID;
	TCHAR*       description;
	TCHAR*       default_file;
	int          default_indx;
	int          cx, cy;

	IconSourceItem* source_small;
	IconSourceItem* source_big;
	IconSourceItem* default_icon;

	TCHAR*       temp_file;
	HICON        temp_icon;
	BOOL         temp_reset;
}
	IconItem;

typedef struct
{
	char *paramName;
	DWORD value;
}
	TreeItem;

// extracticon.c
UINT _ExtractIconEx(LPCTSTR lpszFile, int iconIndex, int cxIcon, int cyIcon, HICON *phicon, UINT flags);
