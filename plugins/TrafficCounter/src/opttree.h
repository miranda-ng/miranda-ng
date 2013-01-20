/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-07  George Hazan
Copyright ( C ) 2007     Maxim Mluhov
Copyright ( C ) 2007     Victor Pavlychko

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

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

// дыруш
#define OPTTREE_CHECK	0
#define OPTTREE_INVISIBLE 2
#define OPTTREE_NOTRANSLATE 4

typedef struct {
  int       iconIndex;
  TCHAR		*szOptionName;
  int       groupId;
  DWORD     dwFlag;
  HTREEITEM hItem;
  char	    *szSettingName;
} OPTTREE_OPTION;

BOOL OptTree_ProcessMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, BOOL *result, int idcTree, OPTTREE_OPTION *options, int optionCount);

DWORD OptTree_GetOptions(HWND hwnd, int idcTree, OPTTREE_OPTION *options, int optionCount, char *szSettingName);
void OptTree_SetOptions(HWND hwnd, int idcTree, OPTTREE_OPTION *options, int optionCount, DWORD dwOptions, char *szSettingName);

void OptTree_Translate(HWND hwndTree);

#endif // __opttree_h__
