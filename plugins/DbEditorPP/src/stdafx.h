#ifndef _COMMONHEADERS_H
#define _COMMONHEADERS_H
//=====================================================
//	Includes
//=====================================================

#include <Windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <io.h>
#include <time.h>
#include <stddef.h>
#include <shlwapi.h>
#include <process.h>
#include <string.h>
#include <win2k.h>
#include <malloc.h>

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_database.h>
#include <m_db_int.h>
#include <m_gui.h>
#include <m_hotkeys.h>
#include <m_icolib.h>
#include <m_langpack.h>
#include <m_metacontacts.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_userinfo.h>
#include <m_utils.h>

#include "m_toptoolbar.h"

#include "resource.h"
#include "version.h"

//=======================================================
//	Definitions
//=======================================================

#define MODULENAME   "DBEditorpp"
#define modFullname  "Database Editor++"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMOption<bool> bExpandSettingsOnOpen, bRestoreOnOpen, bWarnOnDelete;
	CMOption<WORD> iPopupDelay;
	CMOption<DWORD> iPopupBkColor, iPopupTxtColor;

	CMPlugin();

	int Load() override;
	int Unload() override;
};

#define msg(a)		MessageBox(hwnd2mainWindow,a,_A2T(modFullname),MB_OK)
#define dlg(a,b)	MessageBox(hwnd2mainWindow,a,_A2T(modFullname),b)

#define FLD_SIZE	256
#define MSG_SIZE	256
#define NAME_SIZE	128

#define WM_FINDITEM (WM_USER + 1) // onyl for the main window, wparam is ItemIfno* lparam is 0

/***********************
	ModuleTreeInfoStruct
	this gets dumped as the lparam for each module tree item
	************************/

	// types
#define CONTACT_ROOT_ITEM 0
#define CONTACT           1
#define MODULE            2
#define STUB              4
#define EMPTY             8

struct ModuleTreeInfoStruct
{
	int type; // from above types
	MCONTACT hContact;
};

struct SettingListInfo
{
	MCONTACT hContact;
	int selectedItem; // item that is currently selected
	char module[FLD_SIZE];
	// for edit
	HWND hwnd2Edit;
	char setting[FLD_SIZE];
	int subitem;
};

struct DBsetting
{
	MCONTACT hContact;
	char *module;
	char *setting;
	DBVARIANT dbv;
};

struct ModuleAndContact
{
	char module[FLD_SIZE];
	MCONTACT hContact;
};

// find window
#define FW_MODULE 0
#define FW_SETTINGNAME 1
#define FW_SETTINGVALUE 2

struct ItemInfo
{
	int type; // above types
	MCONTACT hContact;
	char module[FLD_SIZE];
	char setting[FLD_SIZE];
};

// watchwindow
struct WatchListArrayStruct
{
	DBsetting *item; // gotta malloc this
	int count;
	int size;
};

// module setting enum
struct ModSetLinkLinkItem
{
	char *name;
	ModSetLinkLinkItem *next;
};

struct ModuleSettingLL
{
	ModSetLinkLinkItem *first;
	ModSetLinkLinkItem *last;
};

struct ColumnsSettings
{
	wchar_t *name;
	int index;
	char *dbname;
	int defsize;
};

struct ColumnsSortParams
{
	HWND hList;
	int column;
	int last;
};

enum ICONS
{
	IMAGE_EMPTY,
	IMAGE_BINARY,
	IMAGE_BYTE,
	IMAGE_WORD,
	IMAGE_DWORD,
	IMAGE_STRING,
	IMAGE_UNICODE,
	IMAGE_HANDLE,
	IMAGE_SETTINGS,
	IMAGE_CLOSED,
	IMAGE_OPENED,
	IMAGE_CONTACTS,
	IMAGE_ONLINE,
	IMAGE_OFFLINE
};

//=======================================================
//  Variables
//=======================================================
extern HWND hwnd2mainWindow;

extern int g_Mode;
extern int g_Hex;
extern int g_Order;
extern int g_Inline;

extern MCONTACT hRestore;

extern MIDatabase *g_db;

extern bool g_bServiceMode;
extern bool g_bUsePopups;

#define NAMEORDERCOUNT	9

#define MODE_UNLOADED	1
#define MODE_LOADED		2
#define MODE_ALL		3

#define HEX_BYTE		1
#define HEX_WORD		2
#define HEX_DWORD		4

#ifdef _UNICODE
#define GetValue(a,b,c,d,e)	GetValueW(a,b,c,d,e)
#else
#define GetValue(a,b,c,d,e)	GetValueA(a,b,c,d,e)
#endif

//main
char *StringFromBlob(BYTE *blob, WORD len);
int WriteBlobFromString(MCONTACT hContact, const char *module, const char *setting, const char *value, int len);
wchar_t *DBVType(BYTE type);
DWORD getNumericValue(DBVARIANT *dbv);
int setNumericValue(MCONTACT hContact, const char *module, const char *setting, DWORD value, int type);
int IsRealUnicode(wchar_t *value);
int setTextValue(MCONTACT hContact, const char *module, const char *setting, wchar_t *value, int type);
int GetValueA(MCONTACT hContact, const char *module, const char *setting, char *value, int length);
int GetValueW(MCONTACT hContact, const char *module, const char *setting, WCHAR *value, int length);
int GetContactName(MCONTACT hContact, const char *proto, wchar_t *value, int maxlen);
int ApplyProtoFilter(MCONTACT hContact);
void loadListSettings(HWND hwnd, ColumnsSettings *cs);
void saveListSettings(HWND hwnd, ColumnsSettings *cs);
INT_PTR CALLBACK ColumnsCompare(LPARAM lParam1, LPARAM lParam2, LPARAM myParam);

// main_window
void openMainWindow();

// deletemodules
int deleteModule(MCONTACT hContact, const char *module, int confirm);
void deleteModuleDlg();

// renamemodule
int renameModule(MCONTACT hContact, const char *oldName, const char *newName);
void renameModuleDlg();
void addModuleDlg(MCONTACT hContact);

// moduletree
void insertItem(MCONTACT hContact, const char *module, HTREEITEM hParent);
HTREEITEM findItemInTree(MCONTACT hContact, const char *module);
void replaceTreeItem(MCONTACT hContact, const char *module, const char *newModule);
void refreshTree(BOOL restore);
void freeTree(MCONTACT hContact);

// settinglist
int ListView_GetItemTextA(HWND hwndLV, int i, int iSubItem, char *pszText, int cchTextMax);
int ListView_SetItemTextA(HWND hwndLV, int i, int iSubItem, const char *pszText);
void ClearListView();
void DeleteSettingsFromList(MCONTACT hContact, const char *module, const char *setting);
void addListHandle(MCONTACT hContact);
void PopulateSettings(MCONTACT hContact, const char *module);
void SelectSetting(const char *setting);
void settingChanged(MCONTACT hContact, const char *module, const char *setting, DBVARIANT *dbv);

// settingsdlg
void editSetting(MCONTACT hContact, const char *module, const char *setting);
void copySetting(MCONTACT hContact, const char *module, const char *setting);
void newSetting(MCONTACT hContact, const char *module, int type);

// exportimport
void exportDB(MCONTACT hContact, const char *module); // hContact == -1 export entire db. module == NULL export entire contact
void ImportSettingsMenuItem(MCONTACT hContact);
void ImportSettingsFromFileMenuItem(MCONTACT hContact, const char *filePath); // ansi!

// find window
void newFindWindow();

// copymodule
void copyModuleMenuItem(MCONTACT hContact, const char *module);
void copyModule(const char *module, MCONTACT hContactFrom, MCONTACT hContactTo);
int CloneContact(MCONTACT hContact);

// options
int OptInit(WPARAM wParam, LPARAM lParam);

// watchedvars
int WatchedArrayIndex(MCONTACT hContact, const char *module, const char *setting, int strict);
int addSettingToWatchList(MCONTACT hContact, const char *module, const char *setting);
void freeWatchListItem(int item);
void PopulateWatchedWindow();
void freeAllWatches();
void openWatchedVarWindow();
void popupWatchedVar(MCONTACT hContact, const char *module, const char *setting);

// modsettingenum
int EnumModules(ModuleSettingLL *msll);
int EnumSettings(MCONTACT hContact, const char *module, ModuleSettingLL *msll);
void FreeModuleSettingLL(ModuleSettingLL *msll);
int IsModuleEmpty(MCONTACT hContact, const char *module);
int LoadResidentSettings();
void FreeResidentSettings();
int IsResidentSetting(const char *module, const char *setting);
int EnumResidentSettings(const char *module, ModuleSettingLL *msll);
int fixResidentSettings();

// icons
void IcoLibRegister();
HICON LoadSkinnedDBEIcon(int icon);
HIMAGELIST LoadIcons();
int GetProtoIconIndex(const char *proto);

#endif //_COMMONHEADERS_H
