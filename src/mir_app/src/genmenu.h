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

#ifndef GENMENU_H
#define GENMENU_H
//general menu object module
#include "m_genmenu.h"

/* genmenu structs */

#define MENUITEM_SIGNATURE 0xDEADBEEF

struct TIntMenuObject;
struct TMO_IntMenuItem;

struct TMO_LinkedList
{
	TMO_IntMenuItem
		*first, // first element of submenu, or NULL
		*last;  // last element of submenu, or NULL
};

struct TMO_IntMenuItem
{
	DWORD        signature;
	int          iCommand;
	int          iconId;          // icon index in the section's image list
	TMO_MenuItem mi;              // user-defined data
	char*        UniqName;        // unique name
	TCHAR*       CustomName;
	HANDLE       hIcolibItem;     // handle of iconlib item
	HBITMAP      hBmp;
	int          originalPosition;
	DWORD        hotKey;
	WPARAM       execParam;
	void*        pUserData;

	TMO_IntMenuItem *next; // next item in list
	TIntMenuObject  *parent;
	TMO_LinkedList  *owner;
	TMO_LinkedList   submenu;
};

struct TIntMenuObject : public MZeroedObject
{
	TIntMenuObject();
	~TIntMenuObject();

	int id;
	char *pszName;
	TCHAR *ptszDisplayName;

	//ExecService
	//LPARAM lParam;//owner data
	//WPARAM wParam;//allways lparam from winproc
	char *ExecService;

	//CheckService called when building menu
	//return false to skip item.
	//LPARAM lParam;//0
	//WPARAM wParam;//CheckParam
	char *CheckService;//analog to check_proc

	//LPARAM lParam;//ownerdata
	//WPARAM wParam;//menuitemhandle
	char *FreeService;//callback service used to free ownerdata for menuitems

	//LPARAM lParam;//MENUITEMINFO filled with all needed data
	//WPARAM wParam;//menuitemhandle
	char *onAddService;//called just before add MENUITEMINFO to hMenu

	TMO_LinkedList m_items;
	HIMAGELIST m_hMenuIcons;
	BOOL m_bUseUserDefinedItems;

	void freeItem(TMO_IntMenuItem*);
};

extern LIST<TIntMenuObject> g_menus;

#define SEPARATORPOSITIONINTERVAL	100000

// internal usage
void GetMenuItemName(TMO_IntMenuItem *pMenuItem, char* pszDest, size_t cbDestSize);

TMO_IntMenuItem* MO_GetIntMenuItem(HGENMENU);

int MO_DrawMenuItem(LPDRAWITEMSTRUCT dis);
int MO_MeasureMenuItem(LPMEASUREITEMSTRUCT mis);

int MO_ProcessCommandBySubMenuIdent(int menuID, int command, LPARAM lParam);

// function returns TRUE if the walk should be immediately stopped
typedef int (*pfnWalkFunc)(TMO_IntMenuItem*, void*);

// returns the item, on which pfnWalkFunc returned TRUE
TMO_IntMenuItem *MO_RecursiveWalkMenu(TMO_IntMenuItem*, pfnWalkFunc, void*);

//general stuff
int InitGenMenu();
int UnitGenMenu();

TMO_IntMenuItem * GetMenuItemByGlobalID(int globalMenuID);
BOOL	FindMenuHanleByGlobalID(HMENU hMenu, int globalID, struct _MenuItemHandles * dat);	//GenMenu.c

LPTSTR GetMenuItemText(TMO_IntMenuItem*);

int GenMenuOptInit(WPARAM wParam, LPARAM);
int GetMenuItembyId(const int objpos, const int id);

TIntMenuObject* GetMenuObjbyId(const int id);

int ProtocolOrderOptInit(WPARAM wParam, LPARAM);

void FreeAndNil(void **p);
#endif
