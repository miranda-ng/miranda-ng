/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org),
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

/* genmenu structs */

#define MS_CLIST_MAIN_MENU "MainMenu/Command"
#define MS_CLIST_STATUS_MENU "GlobalStatus/Command"

#define MENUITEM_SIGNATURE 0xDEADBEEF

struct TIntMenuObject;
struct TMO_IntMenuItem;

struct TMO_LinkedList : public OBJLIST<TMO_IntMenuItem>
{
	typedef LIST<TMO_IntMenuItem> CSuper;

	TMO_LinkedList() :
		OBJLIST<TMO_IntMenuItem>(1)
	{}

	void remove(TMO_IntMenuItem *p);
};

struct TMO_IntMenuItem : public MZeroedObject
{
	TMO_IntMenuItem();
	~TMO_IntMenuItem();

	uint32_t     signature;
	int          iCommand;
	int          iconId;           // icon index in the section's image list
	TMO_MenuItem mi;               // user-defined data
	char*        pszUniqName;      // unique name
	wchar_t*     ptszCustomName;
	HANDLE       hIcolibItem;      // handle of iconlib item
	HBITMAP      hBmp;
	int          originalPosition; // !!!!!!!!!!!!!!!!!!!!!!!!
	bool         customVisible;

	uint32_t     hotKey;
	WPARAM       execParam;
	void*        pUserData;

	TIntMenuObject *parent;
	TMO_LinkedList *owner;
	TMO_LinkedList submenu;

	void relink(TMO_LinkedList *pList);
};

struct TIntMenuObject : public MZeroedObject
{
	TIntMenuObject();
	~TIntMenuObject();

	int id;
	char *pszName;
	wchar_t *ptszDisplayName;

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

	TMO_LinkedList m_items;
	HIMAGELIST m_hMenuIcons;
	bool m_bUseUserDefinedItems;

	CMStringA getModule() const;
};

extern LIST<TIntMenuObject> g_menus;
extern bool g_bMenuIconsEnabled;

#define SEPARATORPOSITIONINTERVAL	100000

// internal usage
TMO_IntMenuItem* MO_GetIntMenuItem(HGENMENU);

int MO_ProcessCommandBySubMenuIdent(int menuID, int command, LPARAM lParam);

// function returns TRUE if the walk should be immediately stopped
typedef int (*pfnWalkFunc)(TMO_IntMenuItem*, const void*);

// returns the item, on which pfnWalkFunc returned TRUE
TMO_IntMenuItem* MO_RecursiveWalkMenu(const TMO_LinkedList &pList, pfnWalkFunc, const void* = nullptr);

TMO_IntMenuItem* MO_GetDefaultItem(const TMO_LinkedList &pList);

// general stuff
int InitGenMenu();
int UninitGenMenu();

void Menu_LoadAllFromDatabase(const TMO_LinkedList &pList, const char *param);

HMENU Menu_BuildGroupMenu(struct ClcGroup *group);

LPTSTR GetMenuItemText(TMO_IntMenuItem*);

int GenMenuOptInit(WPARAM wParam, LPARAM);

TIntMenuObject* GetMenuObjbyId(const int id);

int ProtocolOrderOptInit(WPARAM wParam, LPARAM);

#endif
