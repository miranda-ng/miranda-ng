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

  ============================================================================

  File name      : $URL$
  Revision       : $Rev$
  Last change on : $Date$
  Last change by : $Author$

  ============================================================================


  DESCRIPTION:

  Offers List of your Custom Statuses.

  ============================================================================

// ====[ INCLUDES AND DEFINITIONS ]======================================== */

#ifndef __CSLIST_H
#define __CSLIST_H 1

#if defined UNICODE
#define WINVER          0x501
#define _WIN32_WINNT    0x501
#define _WIN32_IE	    0x600
#else
#define WINVER          0x400
#define _WIN32_WINNT    0x400
#define _WIN32_IE       0x400
#define LVIF_GROUPID    0
#endif
#define MIRANDA_VER   0x0A00

#pragma warning( disable: 4996 )
#pragma comment( lib, "comctl32.lib" )

#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

#include <win2k.h>
#include <newpluginapi.h>
#include <m_database.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_genmenu.h>
#include <m_skin.h>
#include <m_icolib.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_icq.h>
#include <m_jabber.h>
#include <m_options.h>
#include <m_hotkeys.h>
#include <m_variables.h>

// ====[ BASIC DEFINITIONS ]==================================================

#include "resource.h"
#include "version.h"

#define MODULENAME	       "Custom Status List"

// {C8CC7414-6507-4AF6-925A-83C1D2F7BE8C}
#define PLUGIN_GUUID                  { 0xc8cc7414, 0x6507, 0x4af6, { 0x92, 0x5a, 0x83, 0xc1, 0xd2, 0xf7, 0xbe, 0x8c } }

// ====[ LIMITS ]=============================================================

#define EXTRASTATUS_TITLE_LIMIT			64   // limit of chars for extra status title
#define EXTRASTATUS_MESSAGE_LIMIT		2048 // limit of chars for extra status message
#define XSTATUS_COUNT					32
#define XSTATUS_COUNT_EXTENDED			37
#define MOODS_COUNT						84

// ====[ SERVICES ]===========================================================

#define MS_PSEUDO_PROTO_PROTOACCSENUM         ( CSList::bAccountsSupported ? MS_PROTO_ENUMACCOUNTS : MS_PROTO_ENUMPROTOCOLS )

// ====[ DEFAULT DB VALUES ]==================================================

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


// ====[ FUNCTION MACROS ]====================================================

// set which row is selected (cannot be only focused, but fully selected - second param ;))
#define ListView_GetSelectedItemMacro( hwnd )   ListView_GetNextItem( hwnd, -1, LVNI_FOCUSED | LVNI_SELECTED );
#ifndef ListView_SetSelectionMark
  #define ListView_SetSelectionMark( x, y )     0
#endif

#define getByte( setting, error )           DBGetContactSettingByte( NULL, __INTERNAL_NAME, setting, error )
#define setByte( setting, value )           DBWriteContactSettingByte( NULL, __INTERNAL_NAME, setting, value )
#define getWord( setting, error )           DBGetContactSettingWord( NULL, __INTERNAL_NAME, setting, error )
#define setWord( setting, value )           DBWriteContactSettingWord( NULL, __INTERNAL_NAME, setting, value )
#define getDword( setting, error )          DBGetContactSettingDword( NULL, __INTERNAL_NAME, setting, error )
#define setDword( setting, value )          DBWriteContactSettingDword( NULL, __INTERNAL_NAME, setting, value )
#define getTString( setting, dest )         DBGetContactSettingTString( NULL, __INTERNAL_NAME, setting, dest )
#define setTString( setting, value )        DBWriteContactSettingTString( NULL, __INTERNAL_NAME, setting, value )
#define deleteSetting( setting )            DBDeleteContactSetting( NULL, __INTERNAL_NAME, setting )

// --
typedef void (__cdecl *pForAllProtosFunc)( char*, void* );


// ====[ STRUCTURES ]=========================================================

struct StatusItem { // list item structure
	int     iIcon;
	TCHAR   tszTitle[EXTRASTATUS_TITLE_LIMIT];
	TCHAR   tszMessage[EXTRASTATUS_MESSAGE_LIMIT];
	BOOL    bFavourite;

	StatusItem()
	{
		this->iIcon = 0;
		lstrcpy(this->tszTitle, _T(""));
		lstrcpy(this->tszMessage, _T(""));
		this->bFavourite = FALSE;
	}

	StatusItem( const StatusItem& p )
	{
		this->iIcon = p.iIcon;
		lstrcpy( this->tszTitle, p.tszTitle );
		lstrcpy( this->tszMessage, p.tszMessage );
		this->bFavourite = p.bFavourite;
	}

	~StatusItem( )
	{
	}
};

static struct CSForm { // icons + buttons
	int     idc;
	TCHAR*  ptszTitle;
	TCHAR*  ptszDescr;
	char*   pszIconIcoLib;
	int     iconNoIcoLib;
	HANDLE  hIcoLibItem;

} forms[] = {

	{ -1, LPGENT( "Main Menu" ), LPGENT( "Main Icon" ), "icon", IDI_CSLIST, NULL },
	{ IDC_ADD, LPGENT( "Add new item" ), LPGENT( "Add" ), "add", IDI_ADD, NULL },
	{ IDC_MODIFY, LPGENT( "Modify selected item" ), LPGENT( "Modify" ), "modify", IDI_MODIFY, NULL },
	{ IDC_REMOVE, LPGENT( "Delete selected item" ), LPGENT( "Remove" ), "remove", IDI_REMOVE, NULL },
	{ IDC_FAVOURITE, LPGENT( "Set/unset current item as favorite" ), LPGENT( "Favourite" ), "favourite", IDI_FAVOURITE, NULL },
	{ IDC_UNDO, LPGENT( "Undo changes" ), LPGENT( "Undo changes" ), "undo", IDI_UNDO, NULL },
	{ IDC_IMPORT, LPGENT( "Import statuses from database" ), LPGENT( "Import" ), "import", IDI_IMPORT, NULL },
	{ IDC_FILTER, LPGENT( "Filter list" ), LPGENT( "Filter" ), "filter", IDI_FILTER, NULL },
	{ IDCLOSE, LPGENT( "Close without changing custom status" ), LPGENT( "No change" ), "nochng", IDI_CLOSE, NULL },
	{ IDC_CANCEL, LPGENT( "Clear custom status (reset to None) and close" ), LPGENT( "Clear" ), "clear", IDI_UNSET, NULL },
	{ IDOK, LPGENT( "Set custom status to selected one and close" ), LPGENT( "Set" ), "apply", IDI_APPLY, NULL },
};

static struct CSXstatus { // combobox xstatus icons
	int iId;
	TCHAR* ptszTitle;

} xstatus_names[] = {

	{ 1,  LPGENT( "Angry" ) },
	{ 2,  LPGENT( "Taking a bath" ) },
	{ 3,  LPGENT( "Tired" ) },
	{ 4,  LPGENT( "Party" ) },
	{ 5,  LPGENT( "Drinking beer" ) },
	{ 6,  LPGENT( "Thinking" ) },
	{ 7,  LPGENT( "Eating" ) },
	{ 8,  LPGENT( "Watching TV" ) },
	{ 9,  LPGENT( "Meeting" ) },
	{ 10, LPGENT( "Coffee" ) },
	{ 11, LPGENT( "Listening to music" ) },
	{ 12, LPGENT( "Business" ) },
	{ 13, LPGENT( "Shooting" ) },
	{ 14, LPGENT( "Having fun" ) },
	{ 15, LPGENT( "On the phone" ) },
	{ 16, LPGENT( "Gaming" ) },
	{ 17, LPGENT( "Studying" ) },
	{ 18, LPGENT( "Shopping" ) },
	{ 19, LPGENT( "Feeling sick" ) },
	{ 20, LPGENT( "Sleeping" ) },
	{ 21, LPGENT( "Surfing" ) },
	{ 22, LPGENT( "Browsing" ) },
	{ 23, LPGENT( "Working" ) },
	{ 24, LPGENT( "Typing" ) },
	{ 25, LPGENT( "Picnic" ) },
	{ 26, LPGENT( "PDA" ) },
	{ 27, LPGENT( "On the phone" ) },
	{ 28, LPGENT( "I'm high" ) },
	{ 29, LPGENT( "On WC" ) },
	{ 30, LPGENT( "To be or not to be" ) },
	{ 31, LPGENT( "Watching pro7 on TV" ) },
	{ 32, LPGENT( "Love" ) },
	{ 33, LPGENT( "Searching" ) },
	{ 34, LPGENT( "Amorous" ) },       // I don't understand why this falls when 2 same named items appear O_o
	{ 35, LPGENT( "Journal" ) },    // edit: ..and now I REALLY don't understand why it's working now x))
	{ 36, LPGENT( "Sex" ) },
	{ 37, LPGENT( "Smoking" ) },    // -||-
};

static struct CSMoods { // combobox moods icons
	int iId;
	TCHAR* ptszTitle;
	char* szTag;
} moods_names[] = {
	{ 1, LPGENT("Afraid"),       "afraid"        },
	{ 2, LPGENT("Amazed"),       "amazed"        },
	{ 3, LPGENT("Amorous"),      "amorous"       },
	{ 4, LPGENT("Angry"),        "angry"         },
	{ 5, LPGENT("Annoyed"),      "annoyed"       },
	{ 6, LPGENT("Anxious"),      "anxious"       },
	{ 7, LPGENT("Aroused"),      "aroused"       },
	{ 8, LPGENT("Ashamed"),      "ashamed"       },
	{ 9, LPGENT("Bored"),        "bored"         },
	{ 10, LPGENT("Brave"),        "brave"         },
	{ 11, LPGENT("Calm"),         "calm"          },
	{ 12, LPGENT("Cautious"),     "cautious"      },
	{ 13, LPGENT("Cold"),         "cold"          },
	{ 14, LPGENT("Confident"),    "confident"     },
	{ 15, LPGENT("Confused"),     "confused"      },
	{ 16, LPGENT("Contemplative"),"contemplative" },
	{ 17, LPGENT("Contented"),    "contented"     },
	{ 18, LPGENT("Cranky"),       "cranky"        },
	{ 19, LPGENT("Crazy"),        "crazy"         },
	{ 20, LPGENT("Creative"),     "creative"      },
	{ 21, LPGENT("Curious"),      "curious"       },
	{ 22, LPGENT("Dejected"),     "dejected"      },
	{ 23, LPGENT("Depressed"),    "depressed"     },
	{ 24, LPGENT("Disappointed"), "disappointed"  },
	{ 25, LPGENT("Disgusted"),    "disgusted"     },
	{ 26, LPGENT("Dismayed"),     "dismayed"      },
	{ 27, LPGENT("Distracted"),   "distracted"    },
	{ 28, LPGENT("Embarrassed"),  "embarrassed"   },
	{ 29, LPGENT("Envious"),      "envious"       },
	{ 30, LPGENT("Excited"),      "excited"       },
	{ 31, LPGENT("Flirtatious"),  "flirtatious"   },
	{ 32, LPGENT("Frustrated"),   "frustrated"    },
	{ 33, LPGENT("Grateful"),     "grateful"      },
	{ 34, LPGENT("Grieving"),     "grieving"      },
	{ 35, LPGENT("Grumpy"),       "grumpy"        },
	{ 36, LPGENT("Guilty"),       "guilty"        },
	{ 37, LPGENT("Happy"),        "happy"         },
	{ 38, LPGENT("Hopeful"),      "hopeful"       },
	{ 39, LPGENT("Hot"),          "hot"           },
	{ 40, LPGENT("Humbled"),      "humbled"       },
	{ 41, LPGENT("Humiliated"),   "humiliated"    },
	{ 42, LPGENT("Hungry"),       "hungry"        },
	{ 43, LPGENT("Hurt"),         "hurt"          },
	{ 44, LPGENT("Impressed"),    "impressed"     },
	{ 45, LPGENT("In awe"),       "in_awe"        },
	{ 46, LPGENT("In love"),      "in_love"       },
	{ 47, LPGENT("Indignant"),    "indignant"     },
	{ 48, LPGENT("Interested"),   "interested"    },
	{ 49, LPGENT("Intoxicated"),  "intoxicated"   },
	{ 50, LPGENT("Invincible"),   "invincible"    },
	{ 51, LPGENT("Jealous"),      "jealous"       },
	{ 52, LPGENT("Lonely"),       "lonely"        },
	{ 53, LPGENT("Lost"),         "lost"          },
	{ 54, LPGENT("Lucky"),        "lucky"         },
	{ 55, LPGENT("Mean"),         "mean"          },
	{ 56, LPGENT("Moody"),        "moody"         },
	{ 57, LPGENT("Nervous"),      "nervous"       },
	{ 58, LPGENT("Neutral"),      "neutral"       },
	{ 59, LPGENT("Offended"),     "offended"      },
	{ 60, LPGENT("Outraged"),     "outraged"      },
	{ 61, LPGENT("Playful"),      "playful"       },
	{ 62, LPGENT("Proud"),        "proud"         },
	{ 63, LPGENT("Relaxed"),      "relaxed"       },
	{ 64, LPGENT("Relieved"),     "relieved"      },
	{ 65, LPGENT("Remorseful"),   "remorseful"    },
	{ 66, LPGENT("Restless"),     "restless"      },
	{ 67, LPGENT("Sad"),          "sad"           },
	{ 68, LPGENT("Sarcastic"),    "sarcastic"     },
	{ 69, LPGENT("Satisfied"),    "satisfied"     },
	{ 70, LPGENT("Serious"),      "serious"       },
	{ 71, LPGENT("Shocked"),      "shocked"       },
	{ 72, LPGENT("Shy"),          "shy"           },
	{ 73, LPGENT("Sick"),         "sick"          },
	{ 74, LPGENT("Sleepy"),       "sleepy"        },
	{ 75, LPGENT("Spontaneous"),  "spontaneous"   },
	{ 76, LPGENT("Stressed"),     "stressed"      },
	{ 77, LPGENT("Strong"),       "strong"        },
	{ 78, LPGENT("Surprised"),    "surprised"     },
	{ 79, LPGENT("Thankful"),     "thankful"      },
	{ 80, LPGENT("Thirsty"),      "thirsty"       },
	{ 81, LPGENT("Tired"),        "tired"         },
	{ 82, LPGENT("Undefined"),    "undefined"     },
	{ 83, LPGENT("Weak"),         "weak"          },
	{ 84, LPGENT("Worried"),      "worried"       },
};
// ====[ MY BITCHY LIST IMPLEMENTATION x)) ]==================================
// TODO: Optimize it all x))

template< class T > struct ListItem
{
	T* item;
	ListItem* next;
	
	ListItem( )
	{
		this->item = NULL;
		this->next = NULL;
	}

	~ListItem( )
	{
		delete this->item;
	}
	
	ListItem( StatusItem* si )
	{
		this->item = si;
		this->next = NULL;
	}

	ListItem( const ListItem& p )
	{
		this->item = p.item;
		this->next = NULL;
	}
};


template< class T > struct List
{
private:
	ListItem< T >* items;
	unsigned int count;
	
public:
	typedef int ( *compareFunc )( const T* p1, const T* p2 );
	compareFunc compare;

	List( compareFunc compFnc )
	{
		this->items = NULL;
		this->count = 0;
		this->compare = compFnc;
	}

	~List( )
	{
		this->destroy( );
	}

	ListItem< T >* getListHead( )
	{
		return items;
	}

	unsigned int getCount( )
	{
		return count;
	}

	int add( T* csi )
	{
		int position = 0;
		ListItem< T >* item = new ListItem< T >( csi );
		if ( this->items == NULL )
			this->items = item;
		else
		{
			ListItem< T >* help = item;
			item->next = items;
			while ( help->next != NULL )
			{
				int cmp = compare( item->item, help->next->item );
			    if ( cmp == 1 )
					help = help->next;
				else if ( cmp == 0 )
				{
					delete item;
					return -1;
				}
				else
				    break;
				position++;
			}
			if ( help != item )
			{
				item->next = help->next;
				help->next = item;
			}
			else
			    items = item;
		}
		this->count++;
		return position;
	}

	int remove( const unsigned int item )
	{
		int position = 0;
		if ( item < 0 || item >= this->count )
			return -1;

		ListItem< T >* help = items;
		ListItem< T >* removed;
		if ( item == 0 )
		{
			items = items->next;
			removed = help;
		}
		else
		{
			for ( unsigned int i = 0; i < item - 1; i++ )
			{
				help = help->next;
				position++;
			}
			removed = help->next;
			help->next = help->next->next;
		}
		delete removed;
		this->count--;
		return position;
	}
	
	T* get( const unsigned int item )
	{
		ListItem< T >* help = items;
		for ( unsigned int i = 0; i < item; i++ )
			help = help->next;
		return help->item;
	}

	T* operator[]( const unsigned int item )
	{
		return get( item );
	}

	void destroy( )
	{
		while ( this->count > 0 )
			this->remove( 0 );
	}
};


// ====[ CLASSING -- FUNNY STUFF :) ]=========================================

struct CSList;
struct CSWindow;
struct CSAMWindow;
struct CSItemsList;
struct CSListView;

// --------

struct CSListView
{
	HWND        handle;
	CSWindow*   parent;

	CSListView( HWND, CSWindow* );

	void    addItem( StatusItem* item, int itemNumber );
	void    initItems( ListItem< StatusItem >* items );
	void    reinitItems( ListItem< StatusItem >* items );
	void    removeItems( );
	int     getPositionInList( );
	void    setFullFocusedSelection( int selection );
};


struct CSItemsList
{
	List< StatusItem >* list;

	static int compareItems( const StatusItem* p1, const StatusItem* p2 );
	void loadItems(char* protoName);
	void saveItems(char* protoName);

	CSItemsList(char* protoName);
	~CSItemsList();
};


struct CSAMWindow
{
	WORD        action;
	StatusItem* item;
	CSWindow*   parent;
	BOOL        bChanged;

	HWND        handle;
	HWND        hCombo;
	HWND        hMessage;

	CSAMWindow( WORD action, CSWindow* parent );
	~CSAMWindow( );

	void    exec( );
	void    setCombo( );
	void    fillDialog( );
	void    checkFieldLimit( WORD action, WORD item );
	void    checkItemValidity();
};


struct CSWindow
{
	HWND            handle;
	BOOL            bExtraIcons;
	CSItemsList*    itemslist;
	CSListView*     listview;
	CSAMWindow*     addModifyDlg;
	HIMAGELIST      icons;
	BOOL            bSomethingChanged;
	TCHAR*          filterString;
	char* protoName;
	CSWindow(char* protoName);
	~CSWindow();

	void    initIcons( );
	void    deinitIcons( );
	void    initButtons( );
	static void __cdecl showWindow( void* arg );
	static void __cdecl closeWindow( void* arg );
	void    loadWindowPosition( );
	BOOL    toggleButtons( );
	void    toggleEmptyListMessage( );
	void    toggleFilter( );
	BOOL    itemPassedFilter( ListItem< StatusItem >* li );

	void __inline saveWindowPosition( HWND hwnd )
	{
		if ( getByte( "RememberWindowPosition", DEFAULT_REMEMBER_WINDOW_POSITION ) == TRUE )
		{
			RECT rect = { 0 };
			GetWindowRect( hwnd, &rect );
			setWord( "PositionX", rect.left );
			setWord( "PositionY", rect.top );
		}
	}
};


struct CSList
{
	// global variables
	static HINSTANCE    handle;
	static DWORD        dwMirandaVersion;
	static BOOL         bUnicodeCore;
	static BOOL         bAccountsSupported;

	// class components
	CSWindow*   mainWindow;

	// events
	HANDLE      hHookOnPluginsLoaded;
	HANDLE      hHookOnOptionsInit;
	HANDLE      hHookOnDBSettingChanged;
	HANDLE      hHookOnStatusMenuBuild;

	// services
	HANDLE      hServiceShowList;

	CSList( );
	~CSList( );

	// event functions
	static int  postLoading( WPARAM, LPARAM );
	static int  initOptions( WPARAM, LPARAM );
	static int  respondDBChange( WPARAM, LPARAM );
	static int  createMenuItems( WPARAM, LPARAM );

	// service functions
	static INT_PTR __cdecl  showList(WPARAM, LPARAM, LPARAM);
	void    closeList( HWND );

	// protocols enumeration related functions
	static void ForAllProtocols( pForAllProtosFunc pFunc, void* arg );
	static void __cdecl addProtoStatusMenuItem( char* protoName, void* arg );
	static void __cdecl countProtos( char* protoName, void* arg );
	static void __cdecl countPlusModProtos( char* protoName, void* arg );
	static void __cdecl importCustomStatusUIStatusesFromAllProtos( char* protoName, void* arg );

	// other functions
	void    initIcoLib( );
	void    registerHotkeys(char buf[200], TCHAR* accName, int Number);
	void    rebuildMenuItems( );
	void    setStatus(WORD code, StatusItem* item, char* protoName);
};

HINSTANCE CSList::handle = NULL;
DWORD CSList::dwMirandaVersion = 0x00000000;
BOOL CSList::bUnicodeCore = FALSE;
BOOL CSList::bAccountsSupported = FALSE;

// ====[ GLOBALS ]============================================================

static const MUUID interfaces[] = { PLUGIN_GUUID, MIID_LAST };

PLUGINLINK*  pluginLink = NULL;
PLUGININFOEX pluginInfoEx = {
    sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	0,
	PLUGIN_GUUID
};

CSList* cslist = NULL;


// ====[ INIT STUFF ]=========================================================

BOOL WINAPI DllMain( HINSTANCE, DWORD, LPVOID );

extern "C" __declspec( dllexport ) PLUGININFOEX* MirandaPluginInfoEx( DWORD );
extern "C" __declspec( dllexport ) PLUGININFO* MirandaPluginInfo( DWORD );
extern "C" __declspec( dllexport ) const MUUID* MirandaPluginInterfaces( void );

extern "C" __declspec( dllexport ) int Load( PLUGINLINK* );
extern "C" __declspec( dllexport ) int Unload( void );


// ====[ THREAD FORK ]========================================================

void ForkThread( pThreadFunc pFunc, void* arg );


// ====[ PROCEDURES ]=========================================================

INT_PTR CALLBACK CSWindowProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam );
INT_PTR CALLBACK CSAMWindowProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam );
INT_PTR CALLBACK CSRNWindowProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam );
INT_PTR CALLBACK CSOptionsProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam );


// ====[ HELPERS ]============================================================

HICON LoadIconExEx( const char*, int );

void __fastcall SAFE_FREE(void** p)
{
	if (*p)
	{
		free(*p);
		*p = NULL;
	}
}


#endif /* __CSLIST_H */
