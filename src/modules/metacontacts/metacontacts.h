/*
former MetaContacts Plugin for Miranda IM.

Copyright © 2014 Miranda NG Team
Copyright © 2004-07 Scott Ellis
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

#define META_FILTER       "MetaContactsFilter"
#define META_ID           "MetaID"
#define META_FILTER_ID    "MetaFilterID"
#define META_LINK         "MetaLink"

#define MAX_CONTACTS       20

// I can't think of a way around this - mental block

INT_PTR TranslateMenuFunc(MCONTACT hContact, int i);

// contact menu items
void InitMenus();
extern DWORD nextMetaID;
extern int mcStatus;

INT_PTR Meta_Convert(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_AddTo(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_Edit(WPARAM wParam,LPARAM lParam);
void Meta_RemoveContactNumber(MCONTACT hMeta, int number);
INT_PTR Meta_Delete(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_Default(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_ForceDefault(WPARAM wParam,LPARAM lParam);

INT_PTR Meta_IsEnabled();

INT_PTR Meta_OnOff(WPARAM wParam, LPARAM lParam);
int Meta_EqualDBV(DBVARIANT *dbv, DBVARIANT *id);
int Meta_ModifyMenu(WPARAM wParam,LPARAM lParam);
BOOL Meta_Assign(MCONTACT src, MCONTACT dest, BOOL set_as_default);
int Meta_SetNick(char *proto);
int Meta_HideLinkedContacts(void);
int Meta_UnhideLinkedContacts(void);
int Meta_GetContactNumber(DBCachedContact *cc, MCONTACT hContact);
int Meta_HideMetaContacts(int hide);
int Meta_SuppressStatus(int suppress);
int Meta_CopyContactNick(DBCachedContact *cc, MCONTACT hContact);
void Meta_CopyData(DBCachedContact *cc);
int Meta_SetAllNicks();
int Meta_SwapContacts(DBCachedContact *cc, DWORD contact_number1, DWORD contact_number2);

MCONTACT Meta_GetHandle(const char *protocol, DBVARIANT *id);
MCONTACT Meta_GetMostOnline(DBCachedContact *cc);
MCONTACT Meta_GetMostOnlineSupporting(DBCachedContact *cc, int pflagnum, unsigned long capability);
MCONTACT Meta_GetContactHandle(DBCachedContact *cc, int contact_number);

DBCachedContact* CheckMeta(MCONTACT hMeta);

// function to copy history from one contact to another - courtesy JdGordon with mods (thx)
void Meta_FixStatus(DBCachedContact *cc);

char *Meta_GetUniqueIdentifier(MCONTACT hContact, DWORD *pused);

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

struct MetaOptions
{
	BYTE set_default_on_recv;
	BYTE always_use_default;
	BYTE suppress_status;
	BYTE subcontact_windows;
	BYTE metahistory;
	BYTE subhistory;
	BYTE copydata;
	BYTE lockHandle;
	BYTE temp_default;
	BYTE flash_meta_message_icon;
	BYTE copy_userinfo;
	BYTE use_proto_recv;

	int menu_contact_label;
	int menu_function;
	int clist_contact_name;
	int days_history;
	int set_status_from_offline_delay;
};

extern MetaOptions options;

int Meta_OptInit(WPARAM wParam, LPARAM lParam);
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
INT_PTR Mydb_get(MCONTACT hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv);

typedef enum {I_MENUOFF, I_MENU, I_CONVERT, I_ADD, I_EDIT, I_SETDEFAULT, I_REMOVE} IconIndex;
HICON LoadIconEx(IconIndex i);
HANDLE GetIconHandle(IconIndex i);

extern HANDLE hEventDefaultChanged, hEventForceSend, hEventUnforceSend, hSubcontactsChanged;

extern POINT menuMousePoint;

#define MAX_PROTOCOLS	20
extern char proto_names[MAX_PROTOCOLS * 128];

// used for the 'jabber' hack - i.e. hide contacts instead of moving them to the hidden group
#define JABBER_UNIQUE_ID_SETTING "jid"

// delay setting status from offline - to help reduce innapropriate status notification popups
#define DEFAULT_SET_STATUS_SLEEP_TIME		15000		// milliseconds

// service from clist_meta_mw, existence means we don't need to hide subcontacts (woohoo - thanks FYR)
#define MS_CLUI_METASUPPORT  "CLUI/MetaContactSupport"

#ifndef MS_CLUI_GETVERSION
#define MS_CLUI_GETVERSION      "CLUI/GetVersion"

#define szDelMsg LPGEN("You are going to remove all the contacts associated with this MetaContact.\nThis will delete the MetaContact.\n\nProceed Anyway?")

#endif
