/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2010 Miranda ICQ/IM project,
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

typedef struct
{
	struct _tagIntMenuItem *first, // first element of submenu, or NULL
		                    *last;  // last element of submenu, or NULL
}
	TMO_LinkedList;

typedef struct _tagIntMenuItem
{
	DWORD        signature;
	int          iCommand;
	int          iconId;          // icon index in the section's image list
	TMO_MenuItem mi;              // user-defined data
	BOOL         OverrideShow;
	char*        UniqName;        // unique name
	TCHAR*       CustomName;
	HANDLE       hIcolibItem;     // handle of iconlib item
	HBITMAP      hBmp;
	int          originalPosition;

	struct _tagIntMenuItem *next; // next item in list
	struct TIntMenuObject  *parent;
	TMO_LinkedList         *owner;
	TMO_LinkedList         submenu;
}
	TMO_IntMenuItem,*PMO_IntMenuItem;

struct TIntMenuObject
{
	TIntMenuObject();
	~TIntMenuObject();

	__inline void* operator new( size_t size )
	{	return mir_calloc( size );
	}
	__inline void operator delete( void* p )
	{	mir_free( p );
	}

	char* Name;
	int   id;

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

	void freeItem( TMO_IntMenuItem* );
};

extern LIST<TIntMenuObject> g_menus;

#define SEPARATORPOSITIONINTERVAL	100000

//internal usage
HMENU BuildRecursiveMenu(HMENU hMenu, PMO_IntMenuItem, ListParam *param);
void GetMenuItemName( PMO_IntMenuItem pMenuItem, char* pszDest, size_t cbDestSize );

PMO_IntMenuItem MO_GetIntMenuItem( HGENMENU );

PMO_IntMenuItem MO_AddNewMenuItem( HANDLE menuobjecthandle, PMO_MenuItem pmi );
PMO_IntMenuItem MO_AddOldNewMenuItem( HANDLE menuobjecthandle, PMO_MenuItem pmi );

int MO_DrawMenuItem( LPDRAWITEMSTRUCT dis );
int MO_MeasureMenuItem( LPMEASUREITEMSTRUCT mis );
int MO_ModifyMenuItem( PMO_IntMenuItem menuHandle, PMO_MenuItem pmiparam );
int MO_ProcessCommand( PMO_IntMenuItem pimi, LPARAM lParam );
INT_PTR MO_ProcessHotKeys( HANDLE menuHandle, INT_PTR vKey );
int MO_SetOptionsMenuItem( PMO_IntMenuItem menuobjecthandle, int setting, INT_PTR value );
int MO_SetOptionsMenuObject( HANDLE menuobjecthandle, int setting, INT_PTR value );

INT_PTR MO_ProcessCommandByMenuIdent(WPARAM wParam,LPARAM lParam);
int MO_ProcessCommandBySubMenuIdent(int menuID, int command, LPARAM lParam);

// function returns TRUE if the walk should be immediately stopped
typedef int ( *pfnWalkFunc )( PMO_IntMenuItem, void* );

// returns the item, on which pfnWalkFunc returned TRUE
PMO_IntMenuItem MO_RecursiveWalkMenu( PMO_IntMenuItem, pfnWalkFunc, void* );

//general stuff
int InitGenMenu();
int UnitGenMenu();

int FindRoot( PMO_IntMenuItem pimi, void* param );

TMO_IntMenuItem * GetMenuItemByGlobalID(int globalMenuID);
BOOL	FindMenuHanleByGlobalID(HMENU hMenu, int globalID, struct _MenuItemHandles * dat);	//GenMenu.c

int GenMenuOptInit(WPARAM wParam, LPARAM lParam);
int GetMenuObjbyId(const int id);
int GetMenuItembyId(const int objpos,const int id);
INT_PTR MO_GetMenuItem(WPARAM wParam,LPARAM lParam);
void FreeAndNil(void **p);
static int RemoveFromList(int pos,void **lpList,int *ListElemCount,int ElemSize);
static int RemoveFromList(int pos,void **lpList,int *ListElemCount,int ElemSize);
#endif
