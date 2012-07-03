#ifndef __M_HOTKEYSSERVICE__
# define __M_HOTKEYSSERVICE__


#define HKS_SERVICE_NAME "HotkeysService"

// Modifiers for hotkeys
//#define MOD_ALT         0x0001
//#define MOD_CONTROL     0x0002
//#define MOD_SHIFT       0x0004
//#define MOD_WIN         0x0008
#define MOD_GLOBAL        0x0010

// FLAGS
// inherit modifiers from group (specified modifiers ignored)
#define HKS_ACTION_SHAREMODIFIERS 1
// register item, but disable
#define HKS_INITIALLY_DISABLED 2

#if defined(_MSC_VER)
#pragma warning(push)          /* save warning settings */
#pragma warning(disable:4201)  /* nonstandard extension used : nameless struct/union */
#endif

// Structure of hotkey
typedef union
{
	struct
	{
		WORD key;
		WORD modifiers;
	};
	DWORD hotkey;
	
} THKSHotkey, *PHKSHotkey;

#if defined(_MSC_VER)
#pragma warning(pop)  /* restore warning settings */
#endif

// ITEM TYPES
#define HKS_ITEM_MODULE 	1
#define HKS_ITEM_GROUP      2
#define HKS_ITEM_ACTION     3
#define HKS_ITEM_MACRO      4


// Structure passed to RegisterItem service.
// Used only for registration pursposes, so free it after call.
// name, owner and itemType field is mandatory.
// owner is item ID or must be 0 for 1st level items (module, single action or macro)
// itemType can be one of ITEM TYPES constants.
// flags can be combined from FLAGS constants.
// Group can contain other groups and items and optionally
//   set group modifiers. Only item can be tree leaf.
// If creating group, hotkey.modifiers will be used
//   as group modifiers (if nonzero)
// If creating action, hotkey is optional. If hotkey.key is filled, then
//   hotkey will be assigned to item (if unused).
// If creating macro, hotkey is mandatory.

typedef struct 
{
	int owner;
	char *name;
	int itemType;
	THKSHotkey hotkey;
	int flags;
	
} THKSItem, *PHKSItem;


// Register item
// wParam: item data in PKHSItem format
// lParam: 0
// Returns HANDLE called "ID" in loWord.
//   For actions and macros, hiWord returns state: 0 if ok, other if passed hotkey
//     can't be registered (for example already used)
//   In other cases hiWord is useless
#define MS_HKS_REGISTER_ITEM 	HKS_SERVICE_NAME "/RegisterItem"

// Unregister item
// If item is Group, then all subItems will be unregistered.
// wParam: item ID
// lParam: 0
// Returns: 0 on success, other on fail
#define MS_HKS_UNREGISTER_ITEM 	HKS_SERVICE_NAME "/UnregisterItem"

// Assign hotkey to item. If item is group, then only
//   modifiers are taken as group modifiers. Do not call on modules.
// Hotkey consists of modifiers in hiWord and key in loWord.
// wParam: item ID
// lParam: hotkey as PHKS_Hotkey to register
// Returns:
//   on success: hotkey
//   on error: 0
#define MS_HKS_ASSIGN_HOTKEY 	HKS_SERVICE_NAME "/AssignHotkey"

// Get hotkey assigned to item. Don't apply to modules.
// wParam: item ID
// lParam: 0
// Returns: hotkey assigned, 0 otherwise.
#define MS_HKS_GET_HOTKEY 		HKS_SERVICE_NAME "/GetAssignedHotkey"

// Unassign hotkey from item. Only valid on action and macro items.
// wParam: item ID
// lParam: 0
// Returns: 0 on success, other on fail
#define MS_HKS_UNASSIGN_HOTKEY 	HKS_SERVICE_NAME "/UnassignHotkey"

// Enable/Disable item.
// If item is group or module, then all subItems will be affected.
// wParam: item ID
// lParam: 1 to enable, anything else to disable
// Returns: 0 on success, other on fail
#define MS_HKS_ENABLE_ITEM 		HKS_SERVICE_NAME "/EnableItem"

// Hotkey to text
// wParam: hotkey to textify
// lParam: address to place string, space must be allocated
// Returns: 0
#define MS_HKS_TO_TEXT 			HKS_SERVICE_NAME "/HotkeyToText"

// Get hotkey from text
// wParam: text to convert to hotkey
// lParam: 0
// Returns: hotkey
#define MS_HKS_FROM_TEXT 		HKS_SERVICE_NAME "/HotkeyFromText"


typedef struct 
{
	int itemId;
	int moduleId;
	char *name;
	int itemType;
	THKSHotkey hotkey;
	
} THKSEvent, *PHKSEvent;

// Event when hotkey is pressed
// wParam: PHKSEvent
// lParam: 0
#define ME_HKS_KEY_PRESSED 		HKS_SERVICE_NAME "/HotkeyPressed"




// Util functions ////////////////////////////////////////////////////////////////////////


__inline static int HKS_RegisterModule(char *name)
{
	THKSItem item = {0};
	
	if (!ServiceExists(MS_HKS_REGISTER_ITEM))
		return -1;
	
	item.name = name;
	item.itemType = HKS_ITEM_MODULE;

	return LOWORD(CallService(MS_HKS_REGISTER_ITEM, (WPARAM) &item, 0));
}


__inline static int HKS_RegisterAction(int owner, char *name, int modifiers, char key, int flags)
{
	THKSItem item = {0};
	
	if (!ServiceExists(MS_HKS_REGISTER_ITEM))
		return -1;
	
	item.owner = owner;
	item.name = name;
	item.itemType = HKS_ITEM_ACTION;
	item.flags = flags;

	if(key != 0)
	{
		item.hotkey.key = (WORD) key;
		item.hotkey.modifiers = (WORD)modifiers;
	}

	return LOWORD(CallService(MS_HKS_REGISTER_ITEM, (WPARAM) &item, 0));
}


__inline static int HKS_RegisterGroup(int owner, char *name, int modifiers, int flags)
{
	THKSItem item = {0};
	
	if (!ServiceExists(MS_HKS_REGISTER_ITEM))
		return -1;
	
	item.owner = owner;
	item.name = name;
	item.itemType = HKS_ITEM_GROUP;
	item.flags = flags;
	item.hotkey.modifiers = (WORD)modifiers;

	return LOWORD(CallService(MS_HKS_REGISTER_ITEM, (WPARAM) &item, 0));
}

__inline static int HKS_Unregister(int id)
{
	if (!ServiceExists(MS_HKS_UNREGISTER_ITEM))
		return -1;
	
	return CallService(MS_HKS_UNREGISTER_ITEM, (WPARAM) id, 0);
}


#endif
