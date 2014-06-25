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

#define MAX_CONTACTS 20

#define META_FILTER "MetaContactsFilter"

INT_PTR TranslateMenuFunc(MCONTACT hContact, int i);

// contact menu items
void InitMenus();
extern int mcStatus;

struct MetaSrmmData
{
	MCONTACT m_hMeta, m_hSub;
	HWND m_hWnd;
};
extern OBJLIST<MetaSrmmData> arMetaWindows;

INT_PTR Meta_Convert(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_AddTo(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_Edit(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_Delete(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_Default(WPARAM wParam,LPARAM lParam);

INT_PTR Meta_OnOff(WPARAM wParam, LPARAM lParam);
int  Meta_ModifyMenu(WPARAM wParam,LPARAM lParam);
BOOL Meta_Assign(MCONTACT src, MCONTACT dest, BOOL set_as_default);
void Meta_RemoveContactNumber(DBCachedContact *cc, int number, bool bUpdateInfo);
int  Meta_SetNick(char *proto);
int  Meta_HideLinkedContacts(void);
int  Meta_GetContactNumber(DBCachedContact *cc, MCONTACT hContact);
int  Meta_HideMetaContacts(bool hide);
int  Meta_SuppressStatus(int suppress);
int  Meta_CopyContactNick(DBCachedContact *cc, MCONTACT hContact);
int  Meta_SetAllNicks();
int  Meta_SwapContacts(DBCachedContact *cc, int contact_number1, int contact_number2);

MCONTACT Meta_GetMostOnline(DBCachedContact *cc);
MCONTACT Meta_GetMostOnlineSupporting(DBCachedContact *cc, int pflagnum, unsigned long capability);
MCONTACT Meta_GetContactHandle(DBCachedContact *cc, int contact_number);

DBCachedContact* CheckMeta(MCONTACT hMeta);

// function to copy history from one contact to another - courtesy JdGordon with mods (thx)
void Meta_FixStatus(DBCachedContact *ccMeta);
void Meta_UpdateSrmmIcon(DBCachedContact *ccMeta, int iStatus);

char *Meta_GetUniqueIdentifier(MCONTACT hContact, DWORD *pused);

INT_PTR Meta_GetCaps(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_GetName(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_LoadIcon(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_SetStatus(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_GetStatus(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_SendMessage(WPARAM wParam,LPARAM lParam);
INT_PTR Meta_ContactMenuFunc(WPARAM wParam, LPARAM lParam);

void Meta_InitServices();
void Meta_CloseHandles();

enum MenuDisplayNameType {DNT_UID = 0, DNT_DID = 1};
enum MenuFunctionType {FT_MSG = 0, FT_MENU = 1, FT_INFO = 2};
enum CListDisplayNameType {CNNT_NICK = 0, CNNT_DISPLAYNAME = 1};

struct MetaOptions
{
	bool bLockHandle;
	bool bSuppressStatus;

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

// API function headers
void CreateApiServices();

typedef enum {I_MENUOFF, I_MENU, I_CONVERT, I_ADD, I_EDIT, I_SETDEFAULT, I_REMOVE} IconIndex;
HICON LoadIconEx(IconIndex i);
HANDLE GetIconHandle(IconIndex i);

extern HANDLE hEventForceSend, hEventUnforceSend, hSubcontactsChanged;
extern POINT menuMousePoint;

#define MAX_PROTOCOLS	20

// used for the 'jabber' hack - i.e. hide contacts instead of moving them to the hidden group
#define JABBER_UNIQUE_ID_SETTING "jid"

// delay setting status from offline - to help reduce innapropriate status notification popups
#define DEFAULT_SET_STATUS_SLEEP_TIME		15000		// milliseconds

// service from clist_meta_mw, existence means we don't need to hide subcontacts (woohoo - thanks FYR)
#define MS_CLUI_METASUPPORT  "CLUI/MetaContactSupport"

#ifndef MS_CLUI_GETVERSION
#define MS_CLUI_GETVERSION      "CLUI/GetVersion"

#define szDelMsg LPGEN("You are going to remove all the contacts associated with this metacontact.\nThis will delete the metacontact.\n\nProceed anyway?")

#endif
