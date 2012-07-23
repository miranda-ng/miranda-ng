/*
 * Contactlist management functions
 */

#include "skype.h"
#include "skypeapi.h"
#include "debug.h"
#include "pthread.h"
#include "gchat.h"
#include "voiceservice.h"

#pragma warning (push)
#pragma warning (disable: 4100) // unreferenced formal parameter
#include "../../include/m_langpack.h"
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
	CLISTMENUITEM mi={0};

	mi.cbSize=sizeof(mi);
	mi.position=-2000005000;
	mi.flags=CMIF_NOTOFFLINE|CMIF_TCHAR;
	mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_CALL));
	mi.pszContactOwner=SKYPE_PROTONAME;
	mi.ptszName=LPGENT("Call (Skype)");
	mi.pszService=SKYPE_CALL;
	
	return mi;
}

CLISTMENUITEM SkypeOutCallItem(void) {
	CLISTMENUITEM mi={0};

	mi.cbSize=sizeof(mi);
	mi.position=-2000005000;
	mi.flags=CMIF_HIDDEN|CMIF_TCHAR;
	mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_CALLSKYPEOUT));
	mi.ptszName=LPGENT("Call using SkypeOut");
	mi.pszService=SKYPEOUT_CALL;

	return mi;
}

CLISTMENUITEM HupItem(void) {
	CLISTMENUITEM mi={0};

	mi.cbSize=sizeof(mi);
	mi.position=-2000005000;
	mi.flags=CMIF_NOTOFFLINE|CMIF_TCHAR;
	mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_HANGUP));
	mi.pszContactOwner=SKYPE_PROTONAME;
	mi.ptszName=LPGENT("Hang up call (Skype)");
	mi.pszService=SKYPE_CALLHANGUP;

	return mi;
}

CLISTMENUITEM SkypeOutHupItem(void) {
	CLISTMENUITEM mi={0};

	mi.cbSize=sizeof(mi);
	mi.position=-2000005000;
	mi.flags=CMIF_TCHAR;
	mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_HANGUP));
	mi.ptszName=LPGENT("Hang up SkypeOut call");
	mi.pszService=SKYPEOUT_CALL;
	return mi;
}

CLISTMENUITEM HoldCallItem(void) {
	CLISTMENUITEM mi={0};

	mi.cbSize=sizeof(mi);
	mi.position=-2000005000;
	mi.flags=CMIF_HIDDEN|CMIF_NOTOFFLINE|CMIF_TCHAR;
	mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_HOLD));
	mi.ptszName=LPGENT("Hold call");
	mi.pszService=SKYPE_HOLDCALL;
	return mi;
}

CLISTMENUITEM ResumeCallItem(void) {
	CLISTMENUITEM mi={0};

	mi.cbSize=sizeof(mi);
	mi.position=-2000005000;
	mi.flags=CMIF_HIDDEN|CMIF_NOTOFFLINE|CMIF_TCHAR;
	mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_RESUME));
	mi.ptszName=LPGENT("Resume call");
	mi.pszService=SKYPE_HOLDCALL;
	return mi;
}

CLISTMENUITEM FileTransferItem(void) {
	CLISTMENUITEM mi={0};

	// Stolen from file.c of Miranda core
	mi.cbSize=sizeof(mi);
	mi.position=-2000020000;
	mi.flags=CMIF_HIDDEN|CMIF_NOTOFFLINE|CMIF_TCHAR;
	mi.hIcon=LoadSkinnedIcon(SKINICON_EVENT_FILE);
	mi.ptszName=LPGENT("&File");
	mi.pszContactOwner=SKYPE_PROTONAME;
	mi.pszService=SKYPE_SENDFILE;
	return mi;
}

CLISTMENUITEM ChatInitItem(void) {
	CLISTMENUITEM mi={0};

	mi.cbSize=sizeof(mi);
	mi.position=-2000020000;
	mi.flags=CMIF_HIDDEN|CMIF_NOTOFFLINE|CMIF_TCHAR;
	mi.hIcon=LoadIcon( hInst, MAKEINTRESOURCE( IDI_INVITE ));
	mi.ptszName=LPGENT("&Open groupchat");
	mi.pszContactOwner=SKYPE_PROTONAME;
	mi.pszService=SKYPE_CHATNEW;
	return mi;
}

HANDLE add_contextmenu(HANDLE hContact) {
	CLISTMENUITEM mi;
	
	UNREFERENCED_PARAMETER(hContact);

	if (!HasVoiceService()) {
		mi=CallItem();
		hMenuCallItem=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0,(LPARAM)&mi);
		mi=HupItem();
		hMenuCallHangup=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
	}
	
	mi=SkypeOutCallItem();
	hMenuSkypeOutCallItem=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0,(LPARAM)&mi);

	if (!HasVoiceService()) {
		mi=HoldCallItem();
		hMenuHoldCallItem=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0,(LPARAM)&mi);
	}

    // We cannot use flag PF1_FILESEND for sending files, as Skype opens its own
	// sendfile-Dialog.
	mi=FileTransferItem();
	hMenuFileTransferItem=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0,(LPARAM)&mi);
    
   	mi=ChatInitItem();
	hMenuChatInitItem=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0,(LPARAM)&mi);


	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=-2000005000;
	mi.flags=CMIF_TCHAR;
	mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_IMPORT));
	mi.pszContactOwner=SKYPE_PROTONAME;
	mi.ptszName=LPGENT("Import Skype history");
	mi.pszService=SKYPE_IMPORTHISTORY;
	return (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0,(LPARAM)&mi);
}

HANDLE add_mainmenu(void) {
	CLISTMENUITEM mi={0};

	mi.cbSize=sizeof(mi);
	mi.position=-2000005000;
	mi.flags=CMIF_TCHAR;
	mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_ADD));
	mi.pszContactOwner=SKYPE_PROTONAME;
	mi.ptszName=LPGENT("Add Skype contact");
	mi.pszService=SKYPE_ADDUSER;
	return (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM, (WPARAM)NULL,(LPARAM)&mi);

}

int __cdecl  PrebuildContactMenu(WPARAM wParam, LPARAM lParam) {
	DBVARIANT dbv;
	CLISTMENUITEM mi;
	char *szProto;
	BOOL callAvailable = FALSE;
	BOOL hangupAvailable = FALSE;

	UNREFERENCED_PARAMETER(lParam);

	if (!(szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0))) return 0;

	if (!HasVoiceService()) {
		// Clear hold-Item in case it exists
		mi=HoldCallItem();
		mi.flags|=CMIM_ALL;
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuHoldCallItem,(LPARAM)&mi);
	}

	if (!strcmp(szProto, SKYPE_PROTONAME)) {
		if (!HasVoiceService()) {
			if (!DBGetContactSetting((HANDLE)wParam, SKYPE_PROTONAME, "CallId", &dbv)) {
				if (DBGetContactSettingByte((HANDLE)wParam, SKYPE_PROTONAME, "OnHold", 0))
					mi=ResumeCallItem(); else mi=HoldCallItem();
				mi.flags=CMIM_ALL;
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuHoldCallItem,(LPARAM)&mi);

				callAvailable = FALSE;
				hangupAvailable = TRUE;

				DBFreeVariant(&dbv);
			} else { callAvailable = TRUE; hangupAvailable = FALSE; }
        
			if (DBGetContactSettingByte((HANDLE)wParam, SKYPE_PROTONAME, "ChatRoom", 0)!=0) {
				callAvailable = FALSE;
				hangupAvailable = FALSE;
			}

			mi = CallItem();
			mi.flags |= CMIM_ALL | (!callAvailable?CMIF_HIDDEN:0);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuCallItem,(LPARAM)&mi);

			mi = HupItem();
			mi.flags |= CMIM_ALL | (!hangupAvailable?CMIF_HIDDEN:0);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuCallHangup,(LPARAM)&mi);
		}

		// Clear SkypeOut menu in case it exists
		mi=SkypeOutCallItem();
		mi.flags|=CMIM_ALL;
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuSkypeOutCallItem,(LPARAM)&mi);

		// File sending and groupchat-creation works starting with protocol version 5
		if (protocol>=5) {
			mi=FileTransferItem();
            if (DBGetContactSettingByte((HANDLE)wParam, SKYPE_PROTONAME, "ChatRoom", 0)==0)
			    mi.flags ^= CMIF_HIDDEN;
			mi.flags |= CMIM_FLAGS;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuFileTransferItem,(LPARAM)&mi);
            
            mi=ChatInitItem();
			if (DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "UseGroupchat", 0) &&
				DBGetContactSettingByte((HANDLE)wParam, SKYPE_PROTONAME, "ChatRoom", 0)==0)
					mi.flags ^= CMIF_HIDDEN;
			mi.flags |= CMIM_FLAGS;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuChatInitItem,(LPARAM)&mi);
		}

	} else if (bSkypeOut) {
		if (!DBGetContactSetting((HANDLE)wParam, SKYPE_PROTONAME, "CallId", &dbv)) {
			mi=SkypeOutHupItem();
			DBFreeVariant(&dbv);
		} else {
			mi=SkypeOutCallItem();
			if(!DBGetContactSetting((HANDLE)wParam,"UserInfo","MyPhone0",&dbv)) {
				DBFreeVariant(&dbv);
				mi.flags=0;
			}
		}
		mi.flags|=CMIM_ALL;
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)(HANDLE)hMenuSkypeOutCallItem,(LPARAM)&mi);
	}

	return 0;
}

/*
int ClistDblClick(WPARAM wParam, LPARAM lParam) {
	char *szProto;

	szProto = (char*)CallService( MS_PROTO_GETCONTACTBASEPROTO, wParam, 0 );
	if (szProto!=NULL && !strcmp(szProto, SKYPE_PROTONAME) && 
		DBGetContactSettingWord((HANDLE)wParam, SKYPE_PROTONAME, "Status", ID_STATUS_OFFLINE)==ID_STATUS_ONTHEPHONE) {
			SkypeCall(wParam, 0);
	}

	return 0;
}
*/

HANDLE find_contact(char *name) {
	char *szProto;
	int tCompareResult;
	HANDLE hContact;
	DBVARIANT dbv;

	// already on list?
	for (hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);hContact != NULL;hContact=(HANDLE)CallService( MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0)) 
	{
		szProto = (char*)CallService( MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0 );
		if (szProto!=NULL && !strcmp(szProto, SKYPE_PROTONAME) &&	DBGetContactSettingByte(hContact, SKYPE_PROTONAME, "ChatRoom", 0)==0)	
		{
			if (DBGetContactSettingString(hContact, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) continue;
            tCompareResult = strcmp(dbv.pszVal, name);
			DBFreeVariant(&dbv);
			if (tCompareResult) continue;
			return hContact; // already there, return handle
		}
	}
	return NULL;
}
HANDLE find_contactT(TCHAR *name) {
	char *szProto;
	int tCompareResult;
	HANDLE hContact;
	DBVARIANT dbv;

	// already on list?
	for (hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);hContact != NULL;hContact=(HANDLE)CallService( MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0)) 
	{
		szProto = (char*)CallService( MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0 );
		if (szProto!=NULL && !strcmp(szProto, SKYPE_PROTONAME) &&	DBGetContactSettingByte(hContact, SKYPE_PROTONAME, "ChatRoom", 0)==0)	
		{
			if (DBGetContactSettingTString(hContact, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) continue;
            tCompareResult = _tcscmp(dbv.ptszVal, name);
			DBFreeVariant(&dbv);
			if (tCompareResult) continue;
			return hContact; // already there, return handle
		}
	}
	return NULL;
}


HANDLE add_contact(char *name, DWORD flags) {
	HANDLE hContact;

	// already on list?
	if (hContact=find_contact(name)) {
		if (!(flags & PALF_TEMPORARY) && DBGetContactSettingByte(hContact, "CList", "NotOnList", 1)) {
			DBDeleteContactSetting( hContact, "CList", "NotOnList" );
			DBDeleteContactSetting( hContact, "CList", "Hidden" );
		}
		LOG(("add_contact: Found %s", name));
		return hContact; // already there, return handle
	}
	// no, so add
	
	LOG(("add_contact: Adding %s", name));
	hContact=(HANDLE)CallServiceSync(MS_DB_CONTACT_ADD, 0, 0);
	if (hContact) {
		if (CallServiceSync(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact,(LPARAM)SKYPE_PROTONAME)!=0) {
			LOG(("add_contact: Ouch! MS_PROTO_ADDTOCONTACT failed for some reason"));
			CallServiceSync(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
			return NULL;
		}
		if (name[0]) DBWriteContactSettingString(hContact, SKYPE_PROTONAME, SKYPE_NAME, name);

   		if (flags & PALF_TEMPORARY ) {
			DBWriteContactSettingByte(hContact, "CList", "NotOnList", 1);
			DBWriteContactSettingByte(hContact, "CList", "Hidden", 1);
		}
		if (name[0]) {
			SkypeSend("GET USER %s DISPLAYNAME", name);
		} else {LOG(("add_contact: Info: The contact added has no name."));}
	} else {LOG(("add_contact: Ouch! MS_DB_CONTACT_ADD failed for some reason"));}
	LOG(("add_contact succeeded"));
	return hContact;
}

void logoff_contacts(BOOL bCleanup) {
	HANDLE hContact;
	char *szProto;
	DBVARIANT dbv={0};

	LOG(("logoff_contacts: Logging off contacts."));
	for (hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);hContact != NULL;hContact=(HANDLE)CallService( MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0)) {
		szProto = (char*)CallService( MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0 );
		if (szProto!=NULL && !strcmp(szProto, SKYPE_PROTONAME))
		{
			if (DBGetContactSettingWord(hContact, SKYPE_PROTONAME, "Status", ID_STATUS_OFFLINE)!=ID_STATUS_OFFLINE)
				DBWriteContactSettingWord(hContact, SKYPE_PROTONAME, "Status", ID_STATUS_OFFLINE);

			DBDeleteContactSetting(hContact, SKYPE_PROTONAME, "CallId");
			if (DBGetContactSettingByte(hContact, SKYPE_PROTONAME, "ChatRoom", 0)==1)
			{
				if (DBGetContactSettingTString(hContact, SKYPE_PROTONAME, "ChatRoomID", &dbv)) continue;
				RemChat (dbv.ptszVal);
				DBFreeVariant(&dbv);
			}
			if (DBGetContactSettingString(hContact, SKYPE_PROTONAME, "Typing_Stream", &dbv) == 0)
			{
				if (bCleanup) SkypeSend ("ALTER APPLICATION libpurple_typing DISCONNECT %s", dbv.pszVal);
				DBFreeVariant(&dbv);
				DBDeleteContactSetting(hContact, SKYPE_PROTONAME, "Typing_Stream");
			}

		}
	}
	if (bCleanup && (protocol>=5 || bIsImoproxy)) SkypeSend ("DELETE APPLICATION libpurple_typing");
}
