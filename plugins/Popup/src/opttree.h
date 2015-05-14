/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
© 2004-2007 Victor Pavlychko
© 2010 MPK
© 2010 Merlin_de

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

#ifndef __opttree_h__
#define __opttree_h__

#define OPTTREE_CHECK	0

typedef struct {
	int       iconIndex;
	LPTSTR    pszOptionName;
	int       groupId;
	DWORD     dwFlag;
	HTREEITEM hItem;
	LPTSTR    pszSettingName;
	BOOL      bState;
	int       Data;		//free for use (depend on data)
} OPTTREE_OPTION;

BOOL OptTree_ProcessMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, BOOL *result, int idcTree, OPTTREE_OPTION *options, int optionCount);
DWORD OptTree_GetOptions(HWND hwnd, int idcTree, OPTTREE_OPTION *options, int optionCount, LPTSTR pszSettingName = NULL);
void OptTree_SetOptions(HWND hwnd, int idcTree, OPTTREE_OPTION *options, int optionCount, DWORD dwOptions, LPTSTR pszSettingName = NULL);

void OptTree_Translate(HWND hwndTree);
HTREEITEM OptTree_FindNamedTreeItemAt(HWND hwndTree, HTREEITEM hItem, LPCTSTR name);
HTREEITEM OptTree_AddItem(HWND hwndTree, LPTSTR name, LPARAM lParam = 0, int iconIndex = -1);

#endif // __opttree_h__
