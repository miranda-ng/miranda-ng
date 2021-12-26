/* ========================================================================

Custom Status List
__________________

Custom Status List plugin for Miranda-IM (www.miranda-im.org)
Follower of Custom Status History List by HANAX
Copyright © 2006-2008 HANAX
Copyright © 2007-2009 jarvis

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

DESCRIPTION:

Offers List of your Custom Statuses.

============================================================================

//====[ INCLUDES AND DEFINITIONS ]======================================== */

#ifndef __CSLIST_H
#define __CSLIST_H 1

#include <windows.h>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_system.h>
#include <m_langpack.h>
#include <m_clistint.h>
#include <m_icolib.h>
#include <m_protosvc.h>
#include <m_xstatus.h>
#include <m_options.h>
#include <m_hotkeys.h>

#include <m_variables.h>

#include "resource.h"
#include "version.h"

//====[ BASIC DEFINITIONS ]==================================================

#define MODULENAME  LPGENW("Custom Status List")
#define MODNAME     "CSList"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
};

//====[ LIMITS ]=============================================================

#define EXTRASTATUS_TITLE_LIMIT			64   // limit of chars for extra status title
#define EXTRASTATUS_MESSAGE_LIMIT		2048 // limit of chars for extra status message

//====[ DEFAULT DB VALUES ]==================================================

#define DEFAULT_ICQ_XSTATUS_ENABLED                 1
#define DEFAULT_PLUGIN_SHOW_RELNOTES                0
#define DEFAULT_PLUGIN_SHOW_NONSTANDARD_XSTATUSES   1
#define DEFAULT_PLUGIN_CONFIRM_ITEMS_DELETION       1
#define DEFAULT_PLUGIN_DELETE_AFTER_IMPORT          0
#define DEFAULT_ITEMS_COUNT                         0
#define DEFAULT_ITEM_ICON                           0
#define DEFAULT_ITEM_IS_FAVOURITE                   0
#define DEFAULT_ALLOW_EXTRA_ICONS                   1
#define DEFAULT_REMEMBER_WINDOW_POSITION            1


//====[ FUNCTION MACROS ]====================================================

// set which row is selected (cannot be only focused, but fully selected - second param ;))
#define ListView_GetSelectedItemMacro( hwnd )   ListView_GetNextItem( hwnd, -1, LVNI_FOCUSED | LVNI_SELECTED );
#ifndef ListView_SetSelectionMark
#define ListView_SetSelectionMark( x, y )     0
#endif

// --
typedef void(__cdecl *pForAllProtosFunc)(char*, void *);


//====[ STRUCTURES ]=========================================================

struct StatusItem // list item structure
{
	int     m_iIcon;
	wchar_t   m_tszTitle[EXTRASTATUS_TITLE_LIMIT];
	wchar_t   m_tszMessage[EXTRASTATUS_MESSAGE_LIMIT];
	BOOL    m_bFavourite;

	StatusItem()
	{
		m_iIcon = 0;
		mir_wstrcpy(m_tszTitle, L"");
		mir_wstrcpy(m_tszMessage, L"");
		m_bFavourite = FALSE;
	}

	StatusItem(const StatusItem& p)
	{
		m_iIcon = p.m_iIcon;
		mir_wstrcpy(m_tszTitle, p.m_tszTitle);
		mir_wstrcpy(m_tszMessage, p.m_tszMessage);
		m_bFavourite = p.m_bFavourite;
	}

	~StatusItem()
	{
	}
};

static struct CSForm { // icons + buttons
	int     idc;
	wchar_t*  ptszTitle;
	wchar_t*  ptszDescr;
	char *  pszIconIcoLib;
	int     iconNoIcoLib;
	HANDLE  hIcoLibItem;

} forms[] = {

	{ -1, LPGENW("Main Menu"), LPGENW("Main Icon"), "icon", IDI_CSLIST },
	{ IDC_ADD, LPGENW("Add new item"), LPGENW("Add"), "add", IDI_ADD },
	{ IDC_MODIFY, LPGENW("Modify selected item"), LPGENW("Modify"), "modify", IDI_MODIFY },
	{ IDC_REMOVE, LPGENW("Delete selected item"), LPGENW("Remove"), "remove", IDI_REMOVE },
	{ IDC_FAVOURITE, LPGENW("Set/unset current item as favorite"), LPGENW("Favorite"), "favourite", IDI_FAVOURITE },
	{ IDC_UNDO, LPGENW("Undo changes"), LPGENW("Undo changes"), "undo", IDI_UNDO },
	{ IDC_IMPORT, LPGENW("Import statuses from database"), LPGENW("Import"), "import", IDI_IMPORT },
	{ IDC_FILTER, LPGENW("Filter list"), LPGENW("Filter"), "filter", IDI_FILTER },
	{ IDCLOSE, LPGENW("Close without changing custom status"), LPGENW("No change"), "nochng", IDI_CLOSE },
	{ IDC_CANCEL, LPGENW("Clear custom status (reset to None) and close"), LPGENW("Clear"), "clear", IDI_UNSET },
	{ IDOK, LPGENW("Set custom status to selected one and close"), LPGENW("Set"), "apply", IDI_APPLY }
};

//====[ MY BITCHY LIST IMPLEMENTATION x)) ]==================================
// TODO: Optimize it all x))

template< class T > struct ListItem
{
	T* m_item;
	ListItem* m_next;

	ListItem()
	{
		m_item = nullptr;
		m_next = nullptr;
	}

	~ListItem()
	{
		delete m_item;
	}

	ListItem(StatusItem* si)
	{
		m_item = si;
		m_next = nullptr;
	}

	ListItem(const ListItem& p)
	{
		m_item = p.item;
		m_next = nullptr;
	}
};


template< class T > struct List
{
private:
	ListItem< T >* m_items;
	unsigned int   m_count;

public:
	typedef int(*compareFunc)(const T* p1, const T* p2);
	compareFunc m_compare;

	List(compareFunc compFnc)
	{
		m_items = nullptr;
		m_count = 0;
		m_compare = compFnc;
	}

	~List()
	{
		destroy();
	}

	ListItem< T >* getListHead()
	{
		return m_items;
	}

	unsigned int getCount()
	{
		return m_count;
	}

	int add(T* csi)
	{
		int position = 0;
		ListItem< T >* item = new ListItem< T >(csi);
		if (m_items == nullptr)
			m_items = item;
		else
		{
			ListItem< T >* help = item;
			item->m_next = m_items;
			while (help->m_next != nullptr)
			{
				int cmp = m_compare(item->m_item, help->m_next->m_item);
				if (cmp == 1)
					help = help->m_next;
				else if (cmp == 0)
				{
					delete item;
					return -1;
				}
				else
					break;
				position++;
			}
			if (help != item)
			{
				item->m_next = help->m_next;
				help->m_next = item;
			}
			else m_items = item;
		}
		m_count++;
		return position;
	}

	int remove(const unsigned int item)
	{
		int position = 0;
		if (item >= m_count)
			return -1;

		ListItem< T >* help = m_items;
		ListItem< T >* removed;
		if (item == 0)
		{
			m_items = m_items->m_next;
			removed = help;
		}
		else
		{
			for (unsigned int i = 0; i < item - 1; i++)
			{
				help = help->m_next;
				position++;
			}
			removed = help->m_next;
			help->m_next = help->m_next->m_next;
		}
		delete removed;
		m_count--;
		return position;
	}

	T* get(const unsigned int item)
	{
		ListItem< T >* help = m_items;
		for (unsigned int i = 0; i < item; i++)
			help = help->m_next;
		return help->m_item;
	}

	T* operator[](const unsigned int item)
	{
		return get(item);
	}

	void destroy()
	{
		while (m_count > 0)
			remove(0);
	}
};


//====[ CLASSING -- FUNNY STUFF :) ]=========================================

struct CSWindow;
struct CSAMWindow;

struct CSListView
{
	HWND        m_handle;
	CSWindow*   m_parent;

	CSListView(HWND, CSWindow*);

	void    addItem(StatusItem* item, int itemNumber);
	void    initItems(ListItem< StatusItem >* items);
	void    reinitItems(ListItem< StatusItem >* items);
	void    removeItems();
	int     getPositionInList();
	void    setFullFocusedSelection(int selection);
};


struct CSItemsList
{
	List< StatusItem > *m_list;

	static int compareItems(const StatusItem* p1, const StatusItem* p2);
	void loadItems(char *protoName);
	void saveItems(char *protoName);

	CSItemsList(char *protoName);
	~CSItemsList();
};

struct CSWindow
{
	HWND            m_handle;
	BOOL            m_bExtraIcons;
	CSItemsList*    m_itemslist;
	CSListView*     m_listview;
	CSAMWindow*     m_addModifyDlg;
	HIMAGELIST      m_icons;
	int             m_statusCount;
	BOOL            m_bSomethingChanged;
	wchar_t*          m_filterString;
	char *          m_protoName;

	CSWindow(char *protoName);
	~CSWindow();

	void    initIcons();
	void    deinitIcons();
	void    initButtons();
	void    loadWindowPosition();
	BOOL    toggleButtons();
	void    toggleEmptyListMessage();
	void    toggleFilter();
	BOOL    itemPassedFilter(ListItem< StatusItem >* li);

	void __inline saveWindowPosition(HWND hwnd)
	{
		if (g_plugin.getByte("RememberWindowPosition", DEFAULT_REMEMBER_WINDOW_POSITION) == TRUE)
			Utils_SaveWindowPosition(hwnd, NULL, MODNAME, "Position");
	}
};

struct CSAMWindow
{
	uint16_t        m_action;
	StatusItem* m_item;
	CSWindow*   m_parent;
	BOOL        m_bChanged;

	HWND        m_handle;
	HWND        m_hCombo;
	HWND        m_hMessage;

	CSAMWindow(uint16_t action, CSWindow* parent);
	~CSAMWindow();

	void    exec();
	void    setCombo();
	void    fillDialog();
	void    checkFieldLimit(uint16_t action, uint16_t item);
	void    checkItemValidity();
};


// service functions
INT_PTR __cdecl  showList(WPARAM, LPARAM, LPARAM);

// protocols enumeration related functions
void addProtoStatusMenuItem(char *protoName);

// other functions
void RegisterHotkeys(char buf[200], wchar_t* accName, int Number);
void SetStatus(uint16_t code, StatusItem* item, char *protoName);

//====[ PROCEDURES ]=========================================================

INT_PTR CALLBACK CSWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
INT_PTR CALLBACK CSAMWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
INT_PTR CALLBACK CSOptionsProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

#endif /* __CSLIST_H */
