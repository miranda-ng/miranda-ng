#ifndef m_w7ui_h__
#define m_w7ui_h__

enum
{
	MIS_ICOLIB,
	MIS_GENERAL,
	MIS_PROTOCOL,
};

struct MIRANDAJUMPLISTITEM
{
	int iconSource;
	char *iconName;
	int iconIdx;
	WCHAR *szTitle;
	WCHAR *szPrefix;
	WCHAR *szArgument;
};

// Force jumplist rebuild
#define MS_JUMPLIST_REBUILD			"w7/JumpList/Rebuild"

// ---
#define ME_JUMPLIST_BUILDCATEGORIES	"w7/JumpList/BuildCategories"

// lParam = (WCHAR *)category name
#define ME_JUMPLIST_BUILDITEMS		"w7/JumpList/BuildItems"

// lParam = (WCHAR *)category name
#define MS_JUMPLIST_ADDCATEGORY		"w7/JumpList/AddCategory"

// lParam = (MIRANDAJUMPLISTITEM *)item
#define MS_JUMPLIST_ADDITEM			"w7/JumpList/AddItem"

// wParam = prefix
// lParam = argument
#define ME_JUMPLIST_PROCESS			"w7/JumpList/Process"

static void MJumpList_AddCategory(WCHAR *name)
{
	CallService(MS_JUMPLIST_ADDCATEGORY, 0, (LPARAM)name);
}

static void MJumpList_AddItem(char *mir_icon, WCHAR *title, WCHAR *prefix, WCHAR *argument)
{
	MIRANDAJUMPLISTITEM item = { MIS_ICOLIB, mir_icon, 0, title, prefix, argument };
	CallService(MS_JUMPLIST_ADDITEM, 0, (LPARAM)&item);
}

static void MJumpList_AddItem(int skinicon, WCHAR *title, WCHAR *prefix, WCHAR *argument)
{
	MIRANDAJUMPLISTITEM item = { MIS_GENERAL, 0, skinicon, title, prefix, argument };
	CallService(MS_JUMPLIST_ADDITEM, 0, (LPARAM)&item);
}

static void MJumpList_AddItem(char *proto, int skinicon, WCHAR *title, WCHAR *prefix, WCHAR *argument)
{
	MIRANDAJUMPLISTITEM item = { MIS_PROTOCOL, proto, skinicon, title, prefix, argument };
	CallService(MS_JUMPLIST_ADDITEM, 0, (LPARAM)&item);
}

#endif // m_w7ui_h__
