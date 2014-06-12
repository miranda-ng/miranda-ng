/*
 * Contactlist management functions
 */

#include "skype.h"
#include "skypeapi.h"
#include "debug.h"
#include "pthread.h"
#include "gchat.h"

#pragma warning (push)
#pragma warning (disable: 4100) // unreferenced formal parameter
#include <m_langpack.h>
#pragma warning (pop)

#pragma warning (disable: 4706) // assignment within conditional expression

// Imported Globals
extern HINSTANCE hInst;
extern BOOL bSkypeOut, bIsImoproxy;
extern char protocol, g_szProtoName[];

// Handles
static HANDLE hMenuCallItem, hMenuCallHangup, hMenuSkypeOutCallItem, hMenuHoldCallItem, hMenuFileTransferItem, hMenuChatInitItem;

// Check if alpha blending icons are supported
// Seems to be not neccessary
/*
BOOL SupportAlphaIcons(void) {
HANDLE hMod;
DLLVERSIONINFO tDVI={0};
BOOL retval=FALSE;
FARPROC pDllGetVersion;

if (!(hMod=LoadLibrary("comctl32.dll"))) return FALSE;
if (pDllGetVersion=GetProcAddress(hMod, "DllGetVersion")) {
tDVI.cbSize=sizeof(tDVI);
if (!pDllGetVersion ((DLLVERSIONINFO *)&tDVI)) {
if (GetDeviceCaps(GetDC(NULL), BITSPIXEL)*GetDeviceCaps(GetDC(NULL), PLANES)>=32 &&
tDVI.dwMajorVersion>=6)
retval=TRUE;
}
}
FreeLibrary(hMod);
return retval;
}
*/

CLISTMENUITEM CallItem(void) {
	CLISTMENUITEM mi = { 0 };

	mi.cbSize = sizeof(mi);
	mi.position = -2000005000;
	mi.flags = CMIF_NOTOFFLINE | CMIF_TCHAR;
	mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CALL));
	mi.pszContactOwner = SKYPE_PROTONAME;
	mi.ptszName = LPGENT("Call (Skype)");
	mi.pszService = SKYPE_CALL;

	return mi;
}

CLISTMENUITEM SkypeOutCallItem(void) {
	CLISTMENUITEM mi = { 0 };

	mi.cbSize = sizeof(mi);
	mi.position = -2000005000;
	mi.flags = CMIF_HIDDEN | CMIF_TCHAR;
	mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CALLSKYPEOUT));
	mi.ptszName = LPGENT("Call using SkypeOut");
	mi.pszService = SKYPEOUT_CALL;

	return mi;
}

CLISTMENUITEM HupItem(void) {
	CLISTMENUITEM mi = { 0 };

	mi.cbSize = sizeof(mi);
	mi.position = -2000005000;
	mi.flags = CMIF_NOTOFFLINE | CMIF_TCHAR;
	mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_HANGUP));
	mi.pszContactOwner = SKYPE_PROTONAME;
	mi.ptszName = LPGENT("Hang up call (Skype)");
	mi.pszService = SKYPE_CALLHANGUP;

	return mi;
}

CLISTMENUITEM SkypeOutHupItem(void) {
	CLISTMENUITEM mi = { 0 };

	mi.cbSize = sizeof(mi);
	mi.position = -2000005000;
	mi.flags = CMIF_TCHAR;
	mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_HANGUP));
	mi.ptszName = LPGENT("Hang up SkypeOut call");
	mi.pszService = SKYPEOUT_CALL;
	return mi;
}

CLISTMENUITEM HoldCallItem(void) {
	CLISTMENUITEM mi = { 0 };

	mi.cbSize = sizeof(mi);
	mi.position = -2000005000;
	mi.flags = CMIF_HIDDEN | CMIF_NOTOFFLINE | CMIF_TCHAR;
	mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_HOLD));
	mi.ptszName = LPGENT("Hold call");
	mi.pszService = SKYPE_HOLDCALL;
	return mi;
}

CLISTMENUITEM ResumeCallItem(void) {
	CLISTMENUITEM mi = { 0 };

	mi.cbSize = sizeof(mi);
	mi.position = -2000005000;
	mi.flags = CMIF_HIDDEN | CMIF_NOTOFFLINE | CMIF_TCHAR;
	mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_RESUME));
	mi.ptszName = LPGENT("Resume call");
	mi.pszService = SKYPE_HOLDCALL;
	return mi;
}

CLISTMENUITEM FileTransferItem(void) {
	CLISTMENUITEM mi = { 0 };

	// Stolen from file.c of Miranda core
	mi.cbSize = sizeof(mi);
	mi.position = -2000020000;
	mi.flags = CMIF_HIDDEN | CMIF_NOTOFFLINE | CMIF_TCHAR;
	mi.hIcon = LoadSkinnedIcon(SKINICON_EVENT_FILE);
	mi.ptszName = LPGENT("&File");
	mi.pszContactOwner = SKYPE_PROTONAME;
	mi.pszService = SKYPE_SENDFILE;
	return mi;
}

CLISTMENUITEM ChatInitItem(void) {
	CLISTMENUITEM mi = { 0 };

	mi.cbSize = sizeof(mi);
	mi.position = -2000020000;
	mi.flags = CMIF_HIDDEN | CMIF_NOTOFFLINE | CMIF_TCHAR;
	mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_INVITE));
	mi.ptszName = LPGENT("&Open group chat");
	mi.pszContactOwner = SKYPE_PROTONAME;
	mi.pszService = SKYPE_CHATNEW;
	return mi;
}

HANDLE add_contextmenu(MCONTACT hContact) {
	CLISTMENUITEM mi;

	UNREFERENCED_PARAMETER(hContact);

	mi = CallItem();
	hMenuCallItem = Menu_AddContactMenuItem(&mi);
	mi = HupItem();
	hMenuCallHangup = Menu_AddContactMenuItem(&mi);

	mi = SkypeOutCallItem();
	hMenuSkypeOutCallItem = Menu_AddContactMenuItem(&mi);

	mi = HoldCallItem();
	hMenuHoldCallItem = Menu_AddContactMenuItem(&mi);

	// We cannot use flag PF1_FILESEND for sending files, as Skype opens its own
	// sendfile-Dialog.
	mi = FileTransferItem();
	hMenuFileTransferItem = Menu_AddContactMenuItem(&mi);

	mi = ChatInitItem();
	hMenuChatInitItem = Menu_AddContactMenuItem(&mi);


	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = -2000005000;
	mi.flags = CMIF_TCHAR;
	mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_IMPORT));
	mi.pszContactOwner = SKYPE_PROTONAME;
	mi.ptszName = LPGENT("Import Skype history");
	mi.pszService = SKYPE_IMPORTHISTORY;
	return Menu_AddContactMenuItem(&mi);
}

HANDLE add_mainmenu(void) {
	CLISTMENUITEM mi = { 0 };

	mi.cbSize = sizeof(mi);
	mi.position = -2000005000;
	mi.flags = CMIF_TCHAR;
	mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ADD));
	mi.pszContactOwner = SKYPE_PROTONAME;
	mi.ptszName = LPGENT("Add Skype contact");
	mi.pszService = SKYPE_ADDUSER;
	return Menu_AddMainMenuItem(&mi);

}

int __cdecl  PrebuildContactMenu(WPARAM wParam, LPARAM lParam) {
	DBVARIANT dbv;
	CLISTMENUITEM mi;
	char *szProto;
	BOOL callAvailable = FALSE;
	BOOL hangupAvailable = FALSE;

	UNREFERENCED_PARAMETER(lParam);

	if (!(szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0))) return 0;

	// Clear hold-Item in case it exists
	mi = HoldCallItem();
	mi.flags |= CMIM_ALL;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuHoldCallItem, (LPARAM)&mi);

	if (!strcmp(szProto, SKYPE_PROTONAME)) {
		if (!db_get((MCONTACT)wParam, SKYPE_PROTONAME, "CallId", &dbv)) {
			if (db_get_b((MCONTACT)wParam, SKYPE_PROTONAME, "OnHold", 0))
				mi = ResumeCallItem(); else mi = HoldCallItem();
			mi.flags = CMIM_ALL;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuHoldCallItem, (LPARAM)&mi);

			callAvailable = FALSE;
			hangupAvailable = TRUE;

			db_free(&dbv);
		}
		else { callAvailable = TRUE; hangupAvailable = FALSE; }

		if (db_get_b((MCONTACT)wParam, SKYPE_PROTONAME, "ChatRoom", 0) != 0) {
			callAvailable = FALSE;
			hangupAvailable = FALSE;
		}

		mi = CallItem();
		mi.flags |= CMIM_ALL | (!callAvailable ? CMIF_HIDDEN : 0);
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuCallItem, (LPARAM)&mi);

		mi = HupItem();
		mi.flags |= CMIM_ALL | (!hangupAvailable ? CMIF_HIDDEN : 0);
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuCallHangup, (LPARAM)&mi);

		// Clear SkypeOut menu in case it exists
		mi = SkypeOutCallItem();
		mi.flags |= CMIM_ALL;
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuSkypeOutCallItem, (LPARAM)&mi);

		// File sending and groupchat-creation works starting with protocol version 5
		if (protocol >= 5) {
			mi = FileTransferItem();
			if (db_get_b((MCONTACT)wParam, SKYPE_PROTONAME, "ChatRoom", 0) == 0)
				mi.flags ^= CMIF_HIDDEN;
			mi.flags |= CMIM_FLAGS;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuFileTransferItem, (LPARAM)&mi);
		}

		if (protocol >= 5 || bIsImoproxy) {
			mi = ChatInitItem();
			if (db_get_b(NULL, SKYPE_PROTONAME, "UseGroupchat", 0) &&
				db_get_b((MCONTACT)wParam, SKYPE_PROTONAME, "ChatRoom", 0) == 0)
				mi.flags ^= CMIF_HIDDEN;
			mi.flags |= CMIM_FLAGS;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuChatInitItem, (LPARAM)&mi);
		}

	}
	else if (bSkypeOut) {
		if (!db_get((MCONTACT)wParam, SKYPE_PROTONAME, "CallId", &dbv)) {
			mi = SkypeOutHupItem();
			db_free(&dbv);
		}
		else {
			mi = SkypeOutCallItem();
			if (!db_get((MCONTACT)wParam, "UserInfo", "MyPhone0", &dbv)) {
				db_free(&dbv);
				mi.flags = 0;
			}
		}
		mi.flags |= CMIM_ALL;
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuSkypeOutCallItem, (LPARAM)&mi);
	}

	return 0;
}

/*
int ClistDblClick(WPARAM wParam, LPARAM lParam) {
char *szProto;

szProto = (char*)CallService( MS_PROTO_GETCONTACTBASEPROTO, wParam, 0 );
if (szProto!=NULL && !strcmp(szProto, SKYPE_PROTONAME) &&
db_get_w((HANDLE)wParam, SKYPE_PROTONAME, "Status", ID_STATUS_OFFLINE)==ID_STATUS_ONTHEPHONE) {
SkypeCall(wParam, 0);
}

return 0;
}
*/

MCONTACT find_contact(char *name)
{
	int tCompareResult;
	DBVARIANT dbv;

	// already on list?
	for (MCONTACT hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact))
	{
		char *szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, hContact, 0);
		if (szProto != NULL && !strcmp(szProto, SKYPE_PROTONAME) && db_get_b(hContact, SKYPE_PROTONAME, "ChatRoom", 0) == 0)
		{
			if (db_get_s(hContact, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) continue;
			tCompareResult = strcmp(dbv.pszVal, name);
			db_free(&dbv);
			if (tCompareResult) continue;
			return hContact; // already there, return handle
		}
	}
	return NULL;
}

MCONTACT find_contactT(TCHAR *name)
{
	int tCompareResult;
	DBVARIANT dbv;

	// already on list?
	for (MCONTACT hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact))
	{
		char *szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, hContact, 0);
		if (szProto != NULL && !strcmp(szProto, SKYPE_PROTONAME) && db_get_b(hContact, SKYPE_PROTONAME, "ChatRoom", 0) == 0)
		{
			if (db_get_ts(hContact, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) continue;
			tCompareResult = _tcscmp(dbv.ptszVal, name);
			db_free(&dbv);
			if (tCompareResult) continue;
			return hContact; // already there, return handle
		}
	}
	return NULL;
}


MCONTACT add_contact(char *name, DWORD flags)
{
	MCONTACT hContact;

	// already on list?
	if (hContact = find_contact(name)) {
		if (!(flags & PALF_TEMPORARY) && db_get_b(hContact, "CList", "NotOnList", 1)) {
			db_unset(hContact, "CList", "NotOnList");
			db_unset(hContact, "CList", "Hidden");
		}
		LOG(("add_contact: Found %s", name));
		return hContact; // already there, return handle
	}
	// no, so add

	LOG(("add_contact: Adding %s", name));
	hContact = (MCONTACT)CallServiceSync(MS_DB_CONTACT_ADD, 0, 0);
	if (hContact) {
		if (CallServiceSync(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)SKYPE_PROTONAME) != 0) {
			LOG(("add_contact: Ouch! MS_PROTO_ADDTOCONTACT failed for some reason"));
			CallServiceSync(MS_DB_CONTACT_DELETE, hContact, 0);
			return NULL;
		}
		if (name[0]) db_set_s(hContact, SKYPE_PROTONAME, SKYPE_NAME, name);

		if (flags & PALF_TEMPORARY) {
			db_set_b(hContact, "CList", "NotOnList", 1);
			db_set_b(hContact, "CList", "Hidden", 1);
		}
		if (name[0]) {
			SkypeSend("GET USER %s DISPLAYNAME", name);
		}
		else { LOG(("add_contact: Info: The contact added has no name.")); }
	}
	else { LOG(("add_contact: Ouch! MS_DB_CONTACT_ADD failed for some reason")); }
	LOG(("add_contact succeeded"));
	return hContact;
}

void logoff_contacts(BOOL bCleanup) {
	MCONTACT hContact;
	char *szProto;
	DBVARIANT dbv = { 0 };

	LOG(("logoff_contacts: Logging off contacts."));
	for (hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact)) {
		szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, hContact, 0);
		if (szProto != NULL && !strcmp(szProto, SKYPE_PROTONAME))
		{
			if (db_get_w(hContact, SKYPE_PROTONAME, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
				db_set_w(hContact, SKYPE_PROTONAME, "Status", ID_STATUS_OFFLINE);

			db_unset(hContact, SKYPE_PROTONAME, "CallId");
			if (db_get_b(hContact, SKYPE_PROTONAME, "ChatRoom", 0) == 1)
			{
				if (db_get_ts(hContact, SKYPE_PROTONAME, "ChatRoomID", &dbv)) continue;
				RemChat(dbv.ptszVal);
				db_free(&dbv);
			}
			if (db_get_s(hContact, SKYPE_PROTONAME, "Typing_Stream", &dbv) == 0)
			{
				if (bCleanup) SkypeSend("ALTER APPLICATION libpurple_typing DISCONNECT %s", dbv.pszVal);
				db_free(&dbv);
				db_unset(hContact, SKYPE_PROTONAME, "Typing_Stream");
			}

		}
	}
	if (bCleanup && (protocol >= 5 || bIsImoproxy)) SkypeSend("DELETE APPLICATION libpurple_typing");
}
