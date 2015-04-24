#ifndef _COMMONHEADERS_H
#define _COMMONHEADERS_H
//=====================================================
//	Includes
//=====================================================

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#include <tchar.h>

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
#include <m_utils.h>
#include <m_clist.h>
#include <m_clui.h>
#include <m_skin.h>
#include <m_langpack.h>
#include <m_protomod.h>
#include <m_database.h>
#include <m_db_int.h>
#include <m_system.h>
#include <m_protocols.h>
#include <m_userinfo.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_popup.h>
#include <m_icolib.h>
#include <m_hotkeys.h>
#include <m_string.h>

#include "m_toptoolbar.h"

#include "resource.h"
#include "Version.h"
#include "modsettingenum.h"

#define DEF_ICON 5
#define crlf_string "\r\n\0"

/////// icons support

void addIcons();
HICON LoadSkinnedDBEIcon(int icon);
int AddIconToList(HIMAGELIST hil, HICON hIcon);
void AddProtoIconsToList(HIMAGELIST hil, int newshift);
int GetProtoIcon(char *szProto);
extern MCONTACT hRestore;
/////////////////////

#ifndef NDEBUG
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

//=======================================================
//	Definitions
//=======================================================
#define modname			"DBEditorpp"
#define modFullname		"Database Editor++"
#define msg(a,b)		MessageBoxA(0,a,b,MB_OK)
#define nick_unknown	"(UNKNOWN)"
#define nick_unknownW	L"(UNKNOWN)"


#define WM_FINDITEM (WM_USER + 1) // onyl for the main window, wparam is ItemIfno* lparam is 0

#define mir_strlen(ptr) ((ptr == NULL) ? 0 : (int)strlen(ptr))
#define mir_strncpy(dst, src, len) strncpy(dst, src, len)[len - 1] = 0;
#define mir_strcmp(ptr1, ptr2) ((ptr1 && ptr2) ? strcmp(ptr1, ptr2) : 1) // (ptr1||ptr2)

#define ListView_SetItemTextW(hwndLV, i, iSubItem_, pszText_) \
{ LV_ITEMW _ms_lvi;\
  _ms_lvi.iSubItem = iSubItem_;\
  _ms_lvi.pszText = pszText_;\
  SendMessageW((hwndLV), LVM_SETITEMTEXTW, (WPARAM)(i), (LPARAM)(LV_ITEMW *)&_ms_lvi);\
}

#define ListView_InsertItemW(hwnd, pitem)   \
  SendMessageW((hwnd), LVM_INSERTITEMW, 0, (LPARAM)(const LV_ITEMW *)(pitem))


#define TreeView_InsertItemW(hwnd, lpis) \
  (HTREEITEM)SendMessageW((hwnd), TVM_INSERTITEMW, 0, (LPARAM)(LPTV_INSERTSTRUCTW)(lpis))

/***********************
	ModuleTreeInfoStruct
	this gets dumped as the lparam for each module tree item
	************************/
// types
#define CONTACT_ROOT_ITEM 0
#define CONTACT 1
#define MODULE 0x2
#define KNOWN_MODULE 2
#define STUB 4
#define EMPTY 8

typedef struct {
	int type; // from above types
	MCONTACT hContact;
} ModuleTreeInfoStruct;

typedef struct {
	MCONTACT hContact;
	char *module;
	HWND hwnd2Edit;
	int selectedItem; // item that is currently selected
	int clicks; // set to 0 when selection changes, 1 after another click.. cant edit till this is 1
} SettingListInfo;

#define WATCH_MODULE 1
#define WATCH_SETTING 0

struct DBsetting {
	DBVARIANT dbv;
	MCONTACT hContact;
	char *module;
	char *setting;
	int WatchModule; // above defines
};

typedef struct {
	char module[256];
	MCONTACT hContact;
} ModuleAndContact;

// find window
#define FW_MODULE 0
#define FW_SETTINGNAME 1
#define FW_SETTINGVALUE 2

typedef struct {
	int type; // above types
	MCONTACT hContact;
	char module[256];
	char setting[256];
} ItemInfo;

// watchwindow
struct WatchListArrayStruct{
	struct DBsetting *item; // gotta malloc this
	int count;
	int size;
};
extern WatchListArrayStruct WatchListArray;

//=======================================================
//  Variables
//=======================================================
extern HINSTANCE hInst;
extern HWND hwnd2mainWindow, hwnd2watchedVarsWindow, hwnd2importWindow;
extern HIMAGELIST himl;
extern HIMAGELIST himl2;
extern int Mode;
extern int Hex;
extern int Order;

extern MIDatabase *g_db;

extern BOOL usePopups;

#define NAMEORDERCOUNT	8

#define MODE_UNLOADED	1
#define MODE_LOADED		2
#define MODE_ALL		3

#define HEX_BYTE		1
#define HEX_WORD		2
#define HEX_DWORD		4

//main.c
int DBGetContactSettingStringStatic(MCONTACT hContact, char *szModule, char *szSetting, char *value, int maxLength);
int WriteBlobFromString(MCONTACT hContact, const char *szModule, const char *szSetting, const char *Value, int len);
int GetSetting(MCONTACT hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv);
int GetValue(MCONTACT hContact, const char *szModule, const char *szSetting, char *Value, int length);
int GetValueW(MCONTACT hContact, const char *szModule, const char *szSetting, WCHAR *Value, int length);
char *u2a(wchar_t *src);
wchar_t *a2u(char *src, wchar_t *buffer, int len);
WCHAR *GetContactName(MCONTACT hContact, const char *szProto, int unicode);
BOOL IsProtocolLoaded(char *pszProtocolName);

// main_window.c
INT_PTR CALLBACK MainDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// modules.c
int deleteModule(char *module, MCONTACT hContact, int fromMenu);
void deleteModuleGui();
void renameModule(char *oldName, char *newName, MCONTACT hContact);
INT_PTR CALLBACK AddModDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int CloneContact(MCONTACT hContact);

// moduletree.c
void replaceTreeItem(HWND hwnd, MCONTACT hContact, const char *module, const char *newModule);
void refreshTree(BOOL restore);
void __cdecl PopulateModuleTreeThreadFunc(LPVOID di);
void freeTree(HWND hwnd2Tree, MCONTACT hContact);
int findItemInTree(HWND hwnd2Tree, MCONTACT hContact, char *module);

// settinglist.c
void setupSettingsList(HWND hwnd2List);
void saveListSettings(HWND hwnd2List);
void ClearListview(HWND hwnd2Settings);
void DeleteSettingsFromList(HWND hSettings, MCONTACT hContact, char *module, char *setting);
void PopulateSettings(HWND hwnd2Settings, MCONTACT hContact, char *module);
void SelectSetting(char *setting);

// addeditsettingsdlg.c
INT_PTR CALLBACK EditSettingDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void editSetting(MCONTACT hContact, char *module, char *setting);
BOOL convertSetting(MCONTACT hContact, char *module, char *setting, int toType); // 0 = byte, 1 = word, 2 = dword, 3 = string

// exportimport.c
void exportDB(MCONTACT hContact, char *module); // hContact == -1 export entire db. module == NULL export entire contact
void ImportSettingsMenuItem(MCONTACT hContact);
void ImportSettingsFromFileMenuItem(MCONTACT hContact, char *FilePath);

// find window.c
INT_PTR CALLBACK FindWindowDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// copymodule.c
void copyModuleMenuItem(char *module, MCONTACT hContact);
void copyModule(char *module, MCONTACT hContactFrom, MCONTACT hContactTo);

// options.c
int OptInit(WPARAM wParam, LPARAM lParam);

// watchlist
int addSettingToWatchList(MCONTACT hContact, char *module, char *setting);
void freeWatchListItem(int item);
void PopulateWatchedWindow(HWND hwnd);
void freeAllWatches();
INT_PTR CALLBACK WatchDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void popupWatchedVar(MCONTACT hContact, const char *module, const char *setting);

#endif //_COMMONHEADERS_H
