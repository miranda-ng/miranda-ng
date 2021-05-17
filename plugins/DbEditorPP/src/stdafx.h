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
#define modFullnameW L"Database Editor++"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMOption<bool> bExpandSettingsOnOpen, bRestoreOnOpen, bWarnOnDelete;
	CMOption<WORD> iPopupDelay;
	CMOption<DWORD> iPopupBkColor, iPopupTxtColor;

	CMPlugin();

	int Load() override;
	int Unload() override;
};

#define FLD_SIZE	256
#define MSG_SIZE	256
#define NAME_SIZE	128

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

// main
char *StringFromBlob(BYTE *blob, WORD len);
int WriteBlobFromString(MCONTACT hContact, const char *module, const char *setting, const char *value, int len);
wchar_t *DBVType(BYTE type);
DWORD getNumericValue(DBVARIANT *dbv);
int setNumericValue(MCONTACT hContact, const char *module, const char *setting, DWORD value, int type);
int IsRealUnicode(wchar_t *value);
int setTextValue(MCONTACT hContact, const char *module, const char *setting, wchar_t *value, int type);
int GetValueA(MCONTACT hContact, const char *module, const char *setting, char *value, int length);
int GetValueW(MCONTACT hContact, const char *module, const char *setting, wchar_t *value, int length);
int GetContactName(MCONTACT hContact, const char *proto, wchar_t *value, int maxlen);
int ApplyProtoFilter(MCONTACT hContact);
void loadListSettings(HWND hwnd, ColumnsSettings *cs);
void saveListSettings(HWND hwnd, ColumnsSettings *cs);
int CALLBACK ColumnsCompare(LPARAM lParam1, LPARAM lParam2, LPARAM myParam);

// main_window

class CMainDlg : public CDlgBase
{
	int m_splitterPos;
	int m_lastColumn = -1;

	MCONTACT m_hContact = 0;
	int m_selectedItem = -1; // item that is currently selected
	char m_module[FLD_SIZE];

	// for edit
	HWND m_hwnd2Edit = nullptr;
	char m_setting[FLD_SIZE];
	int m_subitem = 0;

	CSplitter m_splitter;
	CCtrlTreeView m_modules;
	CCtrlListView m_settings;

	void addModuleDlg(MCONTACT hContact);
	void deleteModuleDlg();

	int doContacts(HTREEITEM contactsRoot, ModuleSettingLL *modlist, MCONTACT hSelectedContact, const char *selectedModule, const char *selectedSetting);
	void doItems(ModuleSettingLL *modlist, int count);

	void ImportSettingsMenuItem(MCONTACT hContact);

	UI_MESSAGE_MAP(CMainDlg, CDlgBase);
		UI_MESSAGE(WM_COMMAND, OnCommand);
	UI_MESSAGE_MAP_END();

public:
	CMainDlg();

	__forceinline void msg(const wchar_t *pwszMessage) {
		MessageBoxW((this != nullptr) ? m_hwnd : 0, pwszMessage, modFullnameW, MB_OK);
	}

	__forceinline int dlg(const wchar_t *pwszMessage, int code) {
		return MessageBoxW((this != nullptr) ? m_hwnd : 0, pwszMessage, modFullnameW, code);
	}

	bool OnInitDialog() override;
	void OnDestroy() override;
	int Resizer(UTILRESIZECONTROL *urc) override;

	LRESULT OnCommand(UINT, WPARAM, LPARAM);

	void onChange_Splitter(CSplitter*);

	// find window
	#define FW_MODULE 0
	#define FW_SETTINGNAME 1
	#define FW_SETTINGVALUE 2
	void FindItem(int type, MCONTACT, const char *szModule, const char *szSetting);

	////////////////////////////////////////////////////////////////////////////////////////
	// module tree

	HTREEITEM findItemInTree(MCONTACT hContact, const char *module);
	BOOL findAndRemoveDuplicates(MCONTACT hContact, const char *module);
	void freeTree(MCONTACT hContact);
	void insertItem(MCONTACT hContact, const char *module, HTREEITEM hParent);
	void replaceTreeItem(MCONTACT hContact, const char *module, const char *newModule);

	void onItemExpand_Modules(CCtrlTreeView::TEventInfo *ev);
	void onSelChanged_Modules(CCtrlTreeView::TEventInfo *ev);
	void onBeginLabelEdit_Modules(CCtrlTreeView::TEventInfo *ev);
	void onEndLabelEdit_Modules(CCtrlTreeView::TEventInfo *ev);
	void onContextMenu_Modules(CContextMenuPos *pos);

	static void __cdecl PopulateModuleTreeThreadFunc(void *param);

	////////////////////////////////////////////////////////////////////////////////////////
	// settings list

	void ClearListView();
	void DeleteSettingsFromList(MCONTACT hContact, const char *module, const char *setting);
	void EditFinish(int selected);
	void EditLabel(int item, int subitem);
	bool EditLabelWndProc(HWND hwnd, UINT, WPARAM);
	bool ListWndProc(UINT, WPARAM wParam);
	void PopulateSettings(MCONTACT hContact, const char *module);
	void SelectSetting(const char *setting);

	void addListHandle(MCONTACT hContact);
	void addListItem(const char *setting, int resident);
	void deleteListItem(const char *setting);
	int  findListItem(const char *setting);
	void updateListItem(int index, const char *setting, DBVARIANT *dbv, int resident);

	void editSetting(MCONTACT hContact, const char *module, const char *setting);
	void copySetting(MCONTACT hContact, const char *module, const char *setting);
	void newSetting(MCONTACT hContact, const char *module, int type);

	void onSettingChanged(MCONTACT hContact, DBCONTACTWRITESETTING *cws);
	void onClick_Settings(CCtrlListView::TEventInfo *ev);
	void onDblClick_Settings(CCtrlListView::TEventInfo *ev);
	void onColumnClick_Settings(CCtrlListView::TEventInfo *ev);
	void onContextMenu_Settings(CContextMenuPos *pos);
};

extern CMainDlg *g_pMainWindow;

// deletemodules
int deleteModule(HWND hwndParent, MCONTACT hContact, const char *module, int confirm);

// renamemodule
int renameModule(MCONTACT hContact, const char *oldName, const char *newName);

// moduletree
void refreshTree(BOOL restore);

// exportimport
void exportDB(MCONTACT hContact, const char *module); // hContact == -1 export entire db. module == NULL export entire contact
void ImportSettingsFromFileMenuItem(MCONTACT hContact, const char *filePath); // ansi!

// find window
void newFindWindow();

// copymodule
void copyModuleMenuItem(HWND hwndParent, MCONTACT hContact, const char *module);
void copyModule(const char *module, MCONTACT hContactFrom, MCONTACT hContactTo);

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

int ListView_GetItemTextA(HWND hwndLV, int i, int iSubItem, char *pszText, int cchTextMax);
int ListView_SetItemTextA(HWND hwndLV, int i, int iSubItem, const char *pszText);

#endif //_COMMONHEADERS_H
