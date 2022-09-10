#ifndef M_GENMENU_H__
#define M_GENMENU_H__

#ifndef M_CORE_H__
#include <m_core.h>
#endif

#ifndef M_NEWPLUGINAPI_H__
#include <newpluginapi.h>
#endif

// predefined menu objects
#define MO_MAIN    (-1)
#define MO_CONTACT (-2)
#define MO_PROTO   (-3)
#define MO_STATUS  (-4)

#define CMIF_GRAYED            0x0001
#define CMIF_CHECKED           0x0002
#define CMIF_HIDDEN            0x0004  // only works on contact menus
#define CMIF_NOTOFFLINE        0x0008  // item won't appear for contacts that are offline
#define CMIF_NOTONLINE         0x0010  //          "      online
#define CMIF_NOTONLIST         0x0020  // item won't appear on standard contacts
#define CMIF_NOTOFFLIST        0x0040  // item won't appear on contacts that have the 'NotOnList' setting
#define CMIF_UNMOVABLE         0x0080  // item's position cannot be changed
#define CMIF_SYSTEM            0x0100  // item's presence & position cannot be changed
#define CMIF_UNICODE           0x0200  // will use wchar_t* instead of char*
#define CMIF_KEEPUNTRANSLATED  0x0400 // don't translate a menu item
#define CMIF_CUSTOM            0x0800 // custom menu item. doesn't exist in a code
#define CMIF_DEFAULT           0x1000 // this menu item is the default one

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
	const CMPluginBase *pPlugin;
	MUUID uid;
};

#define SET_UID(M,A,B,C,D1,D2,D3,D4,D5,D6,D7,D8) { M.uid = { A, B, C, {D1,D2,D3,D4,D5,D6,D7,D8}}; }
#define UNSET_UID(M) { M.uid = MIID_LAST; }

#ifdef __cplusplus
struct CMenuItem : public TMO_MenuItem
{
	CMenuItem(HPLUGIN _p)
	{
		memset(this, 0, sizeof(CMenuItem));
		this->pPlugin = _p;
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

EXTERN_C MIR_APP_DLL(HGENMENU) Menu_CreateRoot(int hMenuObject, LPCWSTR ptszName, int position, HANDLE hIcoLib, HPLUGIN pPlugin);

/////////////////////////////////////////////////////////////////////////////////////////
// process a WM_DRAWITEM message for user context menus      v0.1.1.0+
// wParam, lParam, return value as for WM_MEASUREITEM
// See comments for clist/menumeasureitem

EXTERN_C MIR_APP_DLL(BOOL) Menu_DrawItem(LPARAM);

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

EXTERN_C MIR_APP_DLL(BOOL) Menu_MeasureItem(LPARAM);

/////////////////////////////////////////////////////////////////////////////////////////
// modify an existing menu item 
// returns 0 on success, nonzero on failure

EXTERN_C MIR_APP_DLL(int) Menu_ModifyItem(HGENMENU hMenuItem, const wchar_t *ptszName, HANDLE hIcon = INVALID_HANDLE_VALUE, int iFlags = -1);

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
// temporarily changes menu item's visibility

EXTERN_C MIR_APP_DLL(void) Menu_ShowItem(HGENMENU hMenuItem, bool bShow);

/////////////////////////////////////////////////////////////////////////////////////////
// turns a menu item's check on & off

EXTERN_C MIR_APP_DLL(void) Menu_SetChecked(HGENMENU hMenuItem, bool bSet);

/////////////////////////////////////////////////////////////////////////////////////////
// sets menu item's Visible checkbox programmatically and write it to the options

EXTERN_C MIR_APP_DLL(void) Menu_SetVisible(HGENMENU pimi, bool bVisible);

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
#define MCI_OPT_HOTKEY     2 // uint32_t value = MAKELONG(VK_*, VK_SHIFT)
#define MCI_OPT_EXECPARAM  3 // INT_PTR or void*, associated with this item
#define MCI_OPT_UID        4 // TMenuItem::uid as string like "2E407C55-5E89-4E83-9B79-15A803E7EE90"
#define MCI_OPT_DISABLED   5 // Hidden by default

EXTERN_C MIR_APP_DLL(int) Menu_ConfigureItem(HGENMENU hItem, int iOption, INT_PTR value);

__forceinline int Menu_ConfigureItem(HGENMENU hMenu, int iSetting, LPCSTR pszValue)
{	return Menu_ConfigureItem(hMenu, iSetting, INT_PTR(pszValue));
}

/////////////////////////////////////////////////////////////////////////////////////////
// returns HGENMENU of the root item or NULL

EXTERN_C MIR_APP_DLL(HGENMENU) Menu_GetProtocolRoot(PROTO_INTERFACE *pThis);

#endif // M_GENMENU_H__

