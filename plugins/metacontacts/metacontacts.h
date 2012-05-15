/*
MetaContacts Plugin for Miranda IM.

Copyright © 2004 Universite Louis PASTEUR, STRASBOURG.

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

/** @file metacontacts.h 
*
* Header declaring functions that are accessed in multiple files.
*/

#define _CRT_SECURE_NO_DEPRECATE

#define _WIN32_WINNT 0x0400
#define _WIN32_IE 0x0300

#define MIRANDA_VER 0x0800

#include <m_stdhdr.h>

#include <windows.h>
#include <commctrl.h>
#include <string.h>
#include <time.h>
#include "resource.h"

#include "m_metacontacts.h"

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_clui.h>
#include <m_skin.h>
#include <m_langpack.h>
#include <m_protomod.h>
#include <m_database.h>
#include <m_system.h>
#include <m_protocols.h>
#include <m_userinfo.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_utils.h>
#include <m_ignore.h>
#include <m_clc.h>
#include <m_contacts.h>
#include <m_message.h>
#include <m_ignore.h>
#include <m_icolib.h>
#include <m_langpack.h>
#include <m_popup.h>
#include <m_cluiframes.h>
#include "../updater/m_updater.h"

#define META_PROTO				"MetaContacts"
#define META_FILTER				"MetaContactsFilter"
#define META_ID					"MetaID"
#define META_FILTER_ID			"MetaFilterID"
#define META_LINK				"MetaLink"

#define META_HIDDEN_GROUP		"MetaContacts Hidden Group"
#define MAX_CONTACTS			20

// I can't think of a way around this - mental block

INT_PTR TranslateMenuFunc(HANDLE hContact, int i);
extern HANDLE hMenuContact[MAX_CONTACTS];

INT_PTR MenuFunc0(WPARAM wParam, LPARAM lParam);
INT_PTR MenuFunc1(WPARAM wParam, LPARAM lParam);
INT_PTR MenuFunc2(WPARAM wParam, LPARAM lParam);
INT_PTR MenuFunc3(WPARAM wParam, LPARAM lParam);
INT_PTR MenuFunc4(WPARAM wParam, LPARAM lParam);
INT_PTR MenuFunc5(WPARAM wParam, LPARAM lParam);
INT_PTR MenuFunc6(WPARAM wParam, LPARAM lParam);
INT_PTR MenuFunc7(WPARAM wParam, LPARAM lParam);
INT_PTR MenuFunc8(WPARAM wParam, LPARAM lParam);
INT_PTR MenuFunc9(WPARAM wParam, LPARAM lParam);
INT_PTR MenuFunc10(WPARAM wParam, LPARAM lParam);
INT_PTR MenuFunc11(WPARAM wParam, LPARAM lParam);
INT_PTR MenuFunc12(WPARAM wParam, LPARAM lParam);
INT_PTR MenuFunc13(WPARAM wParam, LPARAM lParam);
INT_PTR MenuFunc14(WPARAM wParam, LPARAM lParam);
INT_PTR MenuFunc15(WPARAM wParam, LPARAM lParam);
INT_PTR MenuFunc16(WPARAM wParam, LPARAM lParam);
INT_PTR MenuFunc17(WPARAM wParam, LPARAM lParam);
INT_PTR MenuFunc18(WPARAM wParam, LPARAM lParam);
INT_PTR MenuFunc19(WPARAM wParam, LPARAM lParam);

extern HINSTANCE hInstance;
extern PLUGININFOEX pluginInfo;

// contact menu items
extern HANDLE	hMenuConvert,
				hMenuAdd,
				hMenuEdit,
				hMenuDelete,
				hMenuDefault,
				hMenuForceDefault;

// main menu item
extern HANDLE hMenuOnOff;

extern DWORD nextMetaID;
extern int mcStatus;

INT_PTR Meta_Convert(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_AddTo(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_Edit(WPARAM wParam,LPARAM lParam);
void Meta_RemoveContactNumber(HANDLE hMeta, int number);
INT_PTR Meta_Delete(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_Default(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_ForceDefault(WPARAM wParam,LPARAM lParam);

INT_PTR Meta_IsEnabled();

int Meta_EqualDBV(DBVARIANT *dbv, DBVARIANT *id);
int Meta_ModifyMenu(WPARAM wParam,LPARAM lParam);
BOOL Meta_Assign(HANDLE src, HANDLE dest, BOOL set_as_default);
HANDLE Meta_GetHandle(const char *protocol, DBVARIANT *id);
int Meta_SetNick(char *proto);
HANDLE Meta_GetMostOnline(HANDLE hMeta);
HANDLE Meta_GetMostOnlineSupporting(HANDLE hMeta, int pflagnum, unsigned long capability);
int Meta_HideLinkedContacts(void);
int Meta_SetHandles(void);
int Meta_UnhideLinkedContacts(void);
int Meta_GetContactNumber(HANDLE hContact);
HANDLE Meta_GetContactHandle(HANDLE hMeta, int contact_number);
void Meta_GetStatusString(int status, char *buf, size_t size);
void Meta_RestoreGroup(HANDLE hContact);
void Meta_SetGroup(HANDLE hContact);
int Meta_HideMetaContacts(int hide);
int Meta_SuppressStatus(int suppress);
int Meta_CopyContactNick(HANDLE hMeta, HANDLE hContact);
void Meta_CopyData(HANDLE hMeta);
int Meta_SetAllNicks();
int Meta_IsHiddenGroup(const char *group_name);
int Meta_SwapContacts(HANDLE hMeta, DWORD contact_number1, DWORD contact_number2);
// function to copy history from one contact to another - courtesy JdGordon with mods (thx)
void copyHistory(HANDLE hContactFrom,HANDLE hContactTo);
// inverse
//void Meta_RemoveHistory(HANDLE hContactRemoveFrom, HANDLE hContactSource);
void Meta_FixStatus(HANDLE hMeta);

char *Meta_GetUniqueIdentifier(HANDLE hContact, DWORD *pused);

INT_PTR Meta_GetCaps(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_GetName(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_LoadIcon(WPARAM wParam,LPARAM lParam); 
INT_PTR Meta_SetStatus(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_GetStatus(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_SendMessage(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_RecvMessage(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_ContactMenuFunc(WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK Meta_SelectDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK Meta_EditDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

void Meta_InitServices();
void Meta_CloseHandles();

enum MenuDisplayNameType {DNT_UID = 0, DNT_DID = 1};
enum MenuFunctionType {FT_MSG = 0, FT_MENU = 1, FT_INFO = 2};
enum CListDisplayNameType {CNNT_NICK = 0, CNNT_DISPLAYNAME = 1};

typedef struct tag_MetaOptions {
	BOOL set_default_on_recv;
	BOOL always_use_default;
	BOOL suppress_status;
	int menu_contact_label;
	int clist_contact_name;
	int menu_function;
	BOOL suppress_proto;
	BOOL copy_subcontact_history;
	int days_history;
	int set_status_from_offline_delay;
	BOOL subcontact_windows;
	BOOL metahistory;
	BOOL subhistory;
	BOOL copydata;
	BOOL lockHandle;
	BOOL temp_default;
	BOOL flash_meta_message_icon;
	BOOL copy_userinfo;
	BOOL use_proto_recv;
} MetaOptions;

extern MetaOptions options;

INT_PTR CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcOptsPriorities(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
int Meta_WriteOptions(MetaOptions *opt);
int Meta_ReadOptions(MetaOptions *opt);

int GetDefaufaultPrio(int status);
int GetRealPriority(char *proto, int status);

// API function headers
INT_PTR MetaAPI_GetMeta(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_GetDefault(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_GetDefaultNum(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_GetMostOnline(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_GetNumContacts(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_GetContact(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_SetDefaultContactNum(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_SetDefaultContact(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_ForceSendContactNum(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_ForceSendContact(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_UnforceSendContact(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_ForceDefault(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_GetForceState(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_GetProtoName(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_ConvertToMeta(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_AddToMeta(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_RemoveFromMeta(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_DisableHiddenGroup(WPARAM wParam, LPARAM lParam);

// extended db get/write setting functions, that handle unicode
INT_PTR MyDBWriteContactSetting(HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv);
INT_PTR MyDBGetContactSetting(HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv);

// IcoLib support
void InitIcons(void);
void DeinitIcons(void);

typedef enum {I_MENUOFF, I_MENU, I_CONVERT, I_ADD, I_EDIT, I_SETDEFAULT, I_REMOVE} IconIndex;
HICON LoadIconEx(IconIndex i);
void ReleaseIconEx(HICON hIcon);

extern HANDLE hEventDefaultChanged, hEventForceSend, hEventUnforceSend, hSubcontactsChanged;

extern POINT menuMousePoint;

extern BOOL message_window_api_enabled;

#define MAX_PROTOCOLS	20
extern int proto_count;
extern char proto_names[MAX_PROTOCOLS * 128];

// used for the 'jabber' hack - i.e. hide contacts instead of moving them to the hidden group
#define JABBER_UNIQUE_ID_SETTING "jid"

// delay setting status from offline - to help reduce innapropriate status notification popups
#define DEFAULT_SET_STATUS_SLEEP_TIME		15000		// milliseconds

// service from clist_meta_mw, existence means we don't need to hide subcontacts (woohoo - thanks FYR)
#define MS_CLUI_METASUPPORT  "CLUI/MetaContactSupport"

extern BOOL meta_group_hack_disabled;

#ifndef MS_CLUI_GETVERSION
#define MS_CLUI_GETVERSION      "CLUI/GetVersion"


extern BOOL os_unicode_enabled;

#endif
