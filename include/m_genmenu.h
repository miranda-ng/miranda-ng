#ifndef M_GENMENU_H__
#define M_GENMENU_H__

#ifndef M_CORE_H__
#include <m_core.h>
#endif

#if defined MIR_APP_EXPORTS
	typedef struct TMO_IntMenuItem* HGENMENU;
#else
	DECLARE_HANDLE(HGENMENU);
#endif

// predefined menu objects
#define MO_MAIN    (-1)
#define MO_CONTACT (-2)
#define MO_PROTO   (-3)
#define MO_STATUS  (-4)

#define CMIF_GRAYED     1
#define CMIF_CHECKED    2
#define CMIF_HIDDEN     4    //only works on contact menus
#define CMIF_NOTOFFLINE 8	  //item won't appear for contacts that are offline
#define CMIF_NOTONLINE  16	  //          "      online
#define CMIF_NOTONLIST  32   //item won't appear on standard contacts
#define CMIF_NOTOFFLIST 64   //item won't appear on contacts that have the 'NotOnList' setting

#define CMIF_UNICODE        512      //will return TCHAR* instead of char*
#if defined(_UNICODE)
#define CMIF_TCHAR       CMIF_UNICODE      //will return TCHAR* instead of char*
#else
#define CMIF_TCHAR       0       //will return char*, as usual
#endif

#define CMIF_KEEPUNTRANSLATED  1024 // don't translate a menu item
#define CMIF_DEFAULT           4096 // this menu item is the default one

struct TMO_MenuItem
{
	int position;
	const char *pszService;
	HGENMENU root;
	MAllStrings name;
	int flags; // set of CMIF_* constants
	union {
		HICON hIcon;
		HANDLE hIcolibItem;
	};
	int hLangpack;
};

#ifdef __cplusplus
struct CMenuItem : public TMO_MenuItem
{
	CMenuItem()
	{
		memset(this, 0, sizeof(CMenuItem));
		this->hLangpack = ::hLangpack;
	}
};
#endif

/*
This structure passed to CheckService.
*/

struct TCheckProcParam
{
	void *MenuItemOwnerData;
	HGENMENU MenuItemHandle;
	WPARAM wParam;
	LPARAM lParam;
};

struct ProcessCommandParam
{
	HMENU menu;
	int ident;
	LPARAM lParam;
};

/////////////////////////////////////////////////////////////////////////////////////////
// Builds a menu from menu object's description
// Returns hMenu on success or NULL on failure

EXTERN_C MIR_APP_DLL(HMENU) Menu_Build(HMENU parent, int hMenuObject, WPARAM wParam = 0, LPARAM lParam = 0);

/////////////////////////////////////////////////////////////////////////////////////////
// Passes custom lParam to the ExecMenuService for the specified menu item
// Returns TRUE if command was processed, FALSE otherwise

EXTERN_C MIR_APP_DLL(BOOL) Menu_ProcessCommand(HGENMENU hMenuItem, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////////////////
// if menu not known call this
// LOWORD(wparam) menuident (from WM_COMMAND message)
// It automatically finds right menuobject and menuitem and calls Menu_ProcessCommand
// returns TRUE if command was processed, FALSE otherwise

EXTERN_C MIR_APP_DLL(BOOL) Menu_ProcessCommandById(int command, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////////////////
// Adds a menu item to genmenu
// Returns HGENMENU on success, or NULL on failure

EXTERN_C MIR_APP_DLL(HGENMENU) Menu_AddItem(int hMenuObject, TMO_MenuItem *pItem, void *pUserData);

/////////////////////////////////////////////////////////////////////////////////////////
// Adds new submenu
// Returns HGENMENU on success, or NULL on failure

EXTERN_C MIR_APP_DLL(HGENMENU) Menu_CreateRoot(int hMenuObject, LPCWSTR ptszName, int position, HANDLE hIcoLib = NULL, int hLang = hLangpack);

/////////////////////////////////////////////////////////////////////////////////////////
// process a WM_DRAWITEM message for user context menus      v0.1.1.0+
// wParam, lParam, return value as for WM_MEASUREITEM
// See comments for clist/menumeasureitem

EXTERN_C MIR_APP_DLL(BOOL) Menu_DrawItem(DRAWITEMSTRUCT *dis);

/////////////////////////////////////////////////////////////////////////////////////////
// enables or disables a menu item

EXTERN_C MIR_APP_DLL(void) Menu_EnableItem(HGENMENU hMenuItem, bool bEnable);

/////////////////////////////////////////////////////////////////////////////////////////
// Retrieves a default menu item for the menu passed
// Returns a menu handle on success or NULL on failure

EXTERN_C MIR_APP_DLL(HGENMENU) Menu_GetDefaultItem(HGENMENU hMenu);

/////////////////////////////////////////////////////////////////////////////////////////
// Retrieves user info from a menu item
// Returns ownerdata on success, NULL on failure
// Useful to get and free ownerdata before delete menu item.

EXTERN_C MIR_APP_DLL(void*) Menu_GetItemData(HGENMENU hMenuItem);

/////////////////////////////////////////////////////////////////////////////////////////
// Retrieves menu item structure by handle.
// Returns 0 and filled TMO_MenuItem structure on success, or -1 on failure

EXTERN_C MIR_APP_DLL(int) Menu_GetItemInfo(HGENMENU hMenuItem, TMO_MenuItem &pInfo);

/////////////////////////////////////////////////////////////////////////////////////////
// process a WM_MEASUREITEM message for user context menus   v0.1.1.0+
// wParam, lParam, return value as for WM_MEASUREITEM
// This is for displaying the icons by the menu items. If you don't call this
// and clist/menudrawitem whne drawing a menu returned by one of the three menu
// services below then it'll work but you won't get any icons

EXTERN_C MIR_APP_DLL(BOOL) Menu_MeasureItem(MEASUREITEMSTRUCT *mis);

/////////////////////////////////////////////////////////////////////////////////////////
// modify an existing menu item 
// returns 0 on success, nonzero on failure

EXTERN_C MIR_APP_DLL(int) Menu_ModifyItem(HGENMENU hMenuItem, const WCHAR *ptszName, HANDLE hIcon = INVALID_HANDLE_VALUE, int iFlags = -1);

/////////////////////////////////////////////////////////////////////////////////////////
// Tries to process a keystroke
// returns TRUE if a key was, FALSE otherwise
// this should be called in WM_KEYDOWN

EXTERN_C MIR_APP_DLL(BOOL) Menu_ProcessHotKey(int hMenuObject, int key);

/////////////////////////////////////////////////////////////////////////////////////////
// Removes a menu item from genmenu
// Returns 0 on success,-1 on failure.
// You must free ownerdata before this call.
// If MenuItemHandle is root, all children will be removed too.

EXTERN_C MIR_APP_DLL(int) Menu_RemoveItem(HGENMENU hMenuItem);

/////////////////////////////////////////////////////////////////////////////////////////
// changes menu item's visibility

EXTERN_C MIR_APP_DLL(void) Menu_ShowItem(HGENMENU hMenuItem, bool bShow);

/////////////////////////////////////////////////////////////////////////////////////////
// turns a menu item's check on & off

EXTERN_C MIR_APP_DLL(void) Menu_SetChecked(HGENMENU hMenuItem, bool bSet);

/////////////////////////////////////////////////////////////////////////////////////////
// Creates a new menu object
// szName = unique menu object identifier
// szDisplayName = menu display name (auto-translated by core)
// szCheckService = this service called when module build menu(MO_BUILDMENU).
//    Service is called with params wparam = PCheckProcParam, lparam = 0
//    if service returns FALSE, item is skipped.
// szExecService = this service called when user select menu item.
//    Service called with params wparam = ownerdata; lparam = lParam from MO_PROCESSCOMMAND
// 
// returns = MenuObjectHandle on success, NULL on failure

EXTERN_C MIR_APP_DLL(int) Menu_AddObject(LPCSTR szName, LPCSTR szDisplayName, LPCSTR szCheckService, LPCSTR szExecService);

/////////////////////////////////////////////////////////////////////////////////////////
// Removes the whole menu object with all submenus
// returns 0 on success, nonzero on failure
// Note: you must free all ownerdata structures, before you
// call this function. Menu_RemoveObject DOES NOT free it.

EXTERN_C MIR_APP_DLL(int) Menu_RemoveObject(int hMenuObject);

/////////////////////////////////////////////////////////////////////////////////////////
// tunes the whold menu object
// returns TRUE if it processed the command, FALSE otherwise

// enable ability user to edit menuitems via options page.
#define MCO_OPT_USERDEFINEDITEMS 1

// Set FreeService for menuobject. When freeing menuitem it will be called with
// wParam = MenuItemHandle
// lParam = mi.ownerdata
#define MCO_OPT_FREE_SERVICE 2

// Set onAddService for menuobject.
#define MCO_OPT_ONADD_SERVICE 3

// Set menu check service
#define MCO_OPT_CHECK_SERVICE 4

EXTERN_C MIR_APP_DLL(int) Menu_ConfigureObject(int hMenuObject, int iSetting, INT_PTR value);

__forceinline int Menu_ConfigureObject(int hMenuObject, int iSetting, LPCSTR pszValue)
{	return Menu_ConfigureObject(hMenuObject, iSetting, INT_PTR(pszValue));
}

/////////////////////////////////////////////////////////////////////////////////////////
// tunes a menu item
// returns TRUE if it processed the command, FALSE otherwise

#define MCI_OPT_UNIQUENAME 1 // a unique name to menuitem(used to store it in database when enabled OPT_USERDEFINEDITEMS)
#define MCI_OPT_HOTKEY     2 // DWORD value = MAKELONG(VK_*, VK_SHIFT)
#define MCI_OPT_EXECPARAM  3 // INT_PTR or void*, associated with this item

EXTERN_C MIR_APP_DLL(int) Menu_ConfigureItem(HGENMENU hItem, int iOption, INT_PTR value);

__forceinline int Menu_ConfigureItem(HGENMENU hMenu, int iSetting, LPCSTR pszValue)
{	return Menu_ConfigureItem(hMenu, iSetting, INT_PTR(pszValue));
}

/////////////////////////////////////////////////////////////////////////////////////////
// returns HGENMENU of the root item or NULL

EXTERN_C MIR_APP_DLL(HGENMENU) Menu_GetProtocolRoot(const char *szProtoName);

/////////////////////////////////////////////////////////////////////////////////////////
// kills all menu items & submenus that belong to the hLangpack given

EXTERN_C MIR_APP_DLL(void) KillModuleMenus(int hLangpack);

#endif // M_GENMENU_H__

