#include "skype.h"
#include "skypeapi.h"
#include "gchat.h"
#include "contacts.h"
#include "debug.h"
#include "utf8.h"
#include "pthread.h"

#pragma warning (push)
#pragma warning (disable: 4100) // unreferenced formal parameter
#include <m_langpack.h>
#include <m_userinfo.h>
#include <m_history.h>
#include <m_contacts.h>
#pragma warning (pop)

#ifndef DWLP_USER
#define DWLP_USER DWL_USER
#endif

#ifdef _UNICODE
#define STR "%S"
#else
#define STR "%s"
#endif

/* VC6 PSDK compatibility */
#if !defined(_WIN64) && !defined(SetWindowLongPtr)
#define SetWindowLongPtr  SetWindowLong
#define GetWindowLongPtr  GetWindowLong
#endif

#pragma warning (disable: 4706) // assignment within conditional expression

extern HANDLE hInitChat;
extern HINSTANCE hInst;
extern char protocol, g_szProtoName[];
extern DWORD mirandaVersion;

static gchat_contacts *chats=NULL;
static int chatcount=0;
static CRITICAL_SECTION m_GCMutex;

// TODO: Disable groupchat for Protocol verisons <5

/****************************************************************************/
/*                  Chat management helper functions                        */
/****************************************************************************/

/* Get the gchat_contacts entry for the chat with the id szChatId
   If the chat doesn't already exist in the list, it is added.

   Parameters: szChatId - String with the chat ID of the chat to be found
   Returns:    Pointer to the gchat_contacts entry for the given id.
			   NULL on failure (not enough memory)
*/
gchat_contacts *GetChat(const TCHAR *szChatId) {
	int i;

	for (i=0;i<chatcount;i++)
		if (!_tcscmp(chats[i].szChatName, szChatId)) return &chats[i];
	if (chats = (gchat_contacts *)realloc(chats, sizeof(gchat_contacts)*(++chatcount))) {
		memset(&chats[chatcount-1], 0, sizeof(gchat_contacts));
		chats[chatcount-1].szChatName=_tcsdup(szChatId);
		return &chats[chatcount-1];
	}
	return NULL;
}

/* Removes the gchat_contacts entry for the chat with the id szChatId,
   if it exists.
  
   Parameters: szChatId - String with the chat ID to be removed from list
 */
void RemChat(TCHAR *szChatId) {
	int i;

	for (i=0;i<chatcount;i++)
		if (!_tcscmp(chats[i].szChatName, szChatId)) {
			if (chats[i].szChatName) free(chats[i].szChatName);
			if (chats[i].mJoinedContacts) free(chats[i].mJoinedContacts);
			if (i<--chatcount) memmove(&chats[i], &chats[i+1], (chatcount-i)*sizeof(gchat_contacts));
			chats = (gchat_contacts *)realloc(chats, sizeof(gchat_contacts)*chatcount);
			return;
		}
}

/* Checks, if the contact with the handle hContact exists in the groupchat
   given in gc

  Parameters: gc       - gchat_contacts entry for the chat session to be searched
			  who      - Name of member
  Returns:    -1  = Not found
              >=0 = Number of found item
 */
static int ExistsChatContact(gchat_contacts *gc, const TCHAR *who) {
	int i;

	for (i=0;i<gc->mJoinedCount;i++)
		if (_tcscmp(gc->mJoinedContacts[i].who, who)==0) return i;
	return -1;
}

gchat_contact *GetChatContact(gchat_contacts *gc, const TCHAR *who) {
	int i = ExistsChatContact (gc, who);

	if (i==-1) return NULL;
	return &gc->mJoinedContacts[i];
}

/* Adds contact with the name who to the groupchat given in gc

  Parameters: gc   -
  Returns:    -1  = Contact not found
			  -2  = On failure
			  >=0 = Number of added item
 */
static int AddChatContact(gchat_contacts *gc, char *who, TCHAR *pszRole)
{
	int i = -2;
	MCONTACT hContact;
	GCDEST gcd = {0};
	GCEVENT gce = {0};
	CONTACTINFO ci = {0};
	TCHAR *twho;

	LOG (("AddChatContact %s", who));
	if (!(twho = make_nonutf_tchar_string((const unsigned char*)who)))
		return -2;
	if ((i=ExistsChatContact(gc, twho))>=0) return i;
	hContact=find_contact(who);

	gcd.pszModule = SKYPE_PROTONAME;
	gcd.ptszID = gc->szChatName;
	gcd.iType = GC_EVENT_JOIN;

	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = &gcd;
	gce.ptszStatus = pszRole?pszRole:_T("USER");
	gce.time = (DWORD)time(NULL);
	gce.dwFlags = GCEF_ADDTOLOG | GC_TCHAR;

	ci.cbSize = sizeof(ci);
	ci.szProto = SKYPE_PROTONAME;
	ci.dwFlag = CNF_DISPLAY | CNF_TCHAR;
	ci.hContact = hContact;

	if (hContact && !CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&ci)) gce.ptszNick=ci.pszVal; 
	else gce.ptszNick=twho;
        
	gce.ptszUID=twho;
	if (!CallService(MS_GC_EVENT, 0, (LPARAM)&gce)) {
		if ((gc->mJoinedContacts=(gchat_contact*)realloc(gc->mJoinedContacts, (gc->mJoinedCount+1)*sizeof(gchat_contact))))
		{
			gc->mJoinedContacts[i=gc->mJoinedCount].hContact=hContact;
			_tcscpy (gc->mJoinedContacts[i].szRole, gce.ptszStatus);
			_tcscpy (gc->mJoinedContacts[i].who, twho);
			gc->mJoinedCount++;
		}
	}
    if (ci.pszVal) mir_free (ci.pszVal);
	free_nonutf_tchar_string (twho);
	return i;
}

void RemChatContact(gchat_contacts *gc, const TCHAR *who) {
	int i;

	if (!gc) return;
	for (i=0;i<gc->mJoinedCount;i++)
		if (_tcscmp(gc->mJoinedContacts[i].who, who)==0) {
			if (i<--gc->mJoinedCount) 
				memmove(&gc->mJoinedContacts[i], &gc->mJoinedContacts[i+1], (gc->mJoinedCount-i)*sizeof(gchat_contact));
			if (gc->mJoinedCount) gc->mJoinedContacts = (gchat_contact*)realloc(gc->mJoinedContacts, sizeof(gchat_contact)*gc->mJoinedCount);
			else {free (gc->mJoinedContacts); gc->mJoinedContacts = NULL; }
			return;
		}
}

MCONTACT find_chat(LPCTSTR chatname) {
	char *szProto;
	int tCompareResult;
	MCONTACT hContact;
	DBVARIANT dbv;

	for (hContact=db_find_first();hContact != NULL;hContact=db_find_next(hContact)) {
		szProto = (char*)CallService( MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0 );
		if (szProto!=NULL && !strcmp(szProto, SKYPE_PROTONAME) &&
			db_get_b(hContact, SKYPE_PROTONAME, "ChatRoom", 0)==1)
		{
			if (db_get_ts(hContact, SKYPE_PROTONAME, "ChatRoomID", &dbv)) continue;
            tCompareResult = _tcscmp(dbv.ptszVal, chatname);
			db_free(&dbv);
			if (tCompareResult) continue;
			return hContact; // already there, return handle
		}
	}
	return NULL;
}

#ifdef _UNICODE
MCONTACT find_chatA(char *chatname) {
	char *szProto;
	int tCompareResult;
	MCONTACT hContact;
	DBVARIANT dbv;

	for (hContact=db_find_first();hContact != NULL;hContact=db_find_next(hContact)) {
		szProto = (char*)CallService( MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0 );
		if (szProto!=NULL && !strcmp(szProto, SKYPE_PROTONAME) &&
			db_get_b(hContact, SKYPE_PROTONAME, "ChatRoom", 0)==1)
		{
			if (db_get_s(hContact, SKYPE_PROTONAME, "ChatRoomID", &dbv)) continue;
            tCompareResult = strcmp(dbv.pszVal, chatname);
			db_free(&dbv);
			if (tCompareResult) continue;
			return hContact; // already there, return handle
		}
	}
	return NULL;
}
#endif



int  __cdecl AddMembers(char *szSkypeMsg) {
	BYTE *contactmask=NULL;
	DBVARIANT dbv2;
	CONTACTINFO ci={0};
	char *ptr, *who, *nextoken;
	TCHAR *szChatId;
	int i, iRet = 0;
	gchat_contacts *gc;

	LOG(("AddMembers STARTED")); 
	if (!(ptr=strstr(szSkypeMsg, " MEMBERS"))) return -1;
	EnterCriticalSection(&m_GCMutex);
	ptr[0]=0;
	szChatId = make_nonutf_tchar_string((const unsigned char*)szSkypeMsg+5);
	ptr+=9;
	if (find_chat(szChatId) && (gc=GetChat(szChatId)) && 
		!db_get_s(NULL, SKYPE_PROTONAME, SKYPE_NAME, &dbv2))
	{
		char *pszMemObjs, *token;

		if (protocol>=7 && (pszMemObjs = SkypeGet ("CHAT", szSkypeMsg+5, "MEMBEROBJECTS"))) {
			// Add new contacts (protocol 7+ with memberobjects, supports roles)
			for (token=strtok_r(pszMemObjs, ", ", &nextoken); token; token=strtok_r(NULL, ", ", &nextoken)) {
				if (!(who = SkypeGet ("CHATMEMBER", token, "IDENTITY"))) continue;
				if (strcmp(who, dbv2.pszVal)) {
					char *pszRole;
					TCHAR *ptszRole = NULL;

					if (pszRole = SkypeGet ("CHATMEMBER", token, "ROLE"))
						ptszRole = make_nonutf_tchar_string((const unsigned char*)pszRole);

					i=AddChatContact(gc, who, ptszRole);
					free_nonutf_tchar_string (ptszRole);
					if (pszRole) free (pszRole);
					if (i>=0 && !contactmask && !(contactmask = (unsigned char*)calloc(gc->mJoinedCount, 1))) i=-2;
					if (!(contactmask= (unsigned char *) realloc(contactmask, gc->mJoinedCount))) {
						iRet = -1;
						free (who);
						break;
					}
					contactmask[i]=TRUE;
				}
				free (who);
			}
			free (pszMemObjs);
		}
		else
		{
			// Add new contacts (normal)
			for (who=strtok_r(ptr, " ", &nextoken); who; who=strtok_r(NULL, " ", &nextoken)) {
				if (strcmp(who, dbv2.pszVal)) {
					i=AddChatContact(gc, who, NULL);
					if (i>=0 && !contactmask && !(contactmask = (unsigned char*)calloc(gc->mJoinedCount, 1))) i=-2;
					if (i<0 || !(contactmask= (unsigned char *) realloc(contactmask, gc->mJoinedCount))) {
						iRet = -1;
						break;
					}
					contactmask[i]=TRUE;
				}
			}
		}
		// Quit contacts which are no longer there
		if (iRet == 0 && contactmask) {
			GCDEST gcd = {0};
			GCEVENT gce = {0};

			gcd.pszModule = SKYPE_PROTONAME;
			gcd.ptszID = szChatId;
			gcd.iType = GC_EVENT_QUIT;

			gce.cbSize = sizeof(GCEVENT);
			gce.pDest = &gcd;
			gce.time = (DWORD)time(NULL);
			gce.dwFlags = GCEF_ADDTOLOG | GC_TCHAR;
        
			ci.cbSize = sizeof(ci);
			ci.szProto = SKYPE_PROTONAME;
			ci.dwFlag = CNF_DISPLAY;

			for (i=0;i<gc->mJoinedCount;i++)
			if (!contactmask[i]) 
			{
				ci.hContact = gc->mJoinedContacts[i].hContact;
				ci.dwFlag = CNF_TCHAR;
				if (ci.hContact && !CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&ci)) gce.ptszNick=ci.pszVal; 
				else gce.ptszNick=gc->mJoinedContacts[i].who;
				RemChatContact(gc, gc->mJoinedContacts[i].who);
				gce.ptszUID = gc->mJoinedContacts[i].who;
				CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
				if (ci.pszVal) {
					mir_free (ci.pszVal);
					ci.pszVal=NULL;
				}
			}
	// We don't do this, because the dialog group-chat may have been started intentionally
	/*
			if (gc->mJoinedCount == 1) {
				// switch back to normal session
				KillChatSession(&gcd);
			}
	*/
		}
		if (contactmask) free(contactmask);
		db_free(&dbv2);
	} else iRet = -1;
	free_nonutf_tchar_string (szChatId);
	LeaveCriticalSection(&m_GCMutex);
	LOG(("AddMembers DONE"));
	return iRet;
}

void AddMembersThread(char *szSkypeMsg)
{
	AddMembers (szSkypeMsg);
	free (szSkypeMsg);
}

/****************************************************************************/
/*                           Window procedures                              */
/****************************************************************************/
INT_PTR CALLBACK InputBoxDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
		SetDlgItemText(hwndDlg, IDC_TEXT, (TCHAR*)lParam);
		return TRUE;
	}

	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDOK:
		{	
			GetDlgItemText(hwndDlg, IDC_TEXT, (TCHAR*)GetWindowLongPtr(hwndDlg, DWLP_USER), (MAX_BUF-1)*sizeof(TCHAR));
			EndDialog(hwndDlg, 1);
			break;
		}
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			break;
	}	}
	return FALSE;
}

/****************************************************************************/
/*                      Core Chat management functions                      */
/****************************************************************************/

/* We have a new Groupchat

   This hook is called when a new chat is initialised.
   Parameters:  wParam = (char *)Name of new chat session [Has to be ASCIIZ/UTF8]
				lParam = 1 - Create groupchat, but don't open it
				         0 - Default - open groupchat after init
*/
int __cdecl  ChatInit(WPARAM wParam, LPARAM lParam) {
	GCSESSION gcw = {0};
	GCEVENT gce = {0};
	GCDEST gcd = {0};
	DBVARIANT dbv, dbv2;
	char *szChatName;
	int iRet = -1;

	UNREFERENCED_PARAMETER(lParam);

	if (!wParam) return -1;

	gcw.cbSize = sizeof(GCSESSION);
	gcw.iType = GCW_CHATROOM;
	gcw.pszModule = SKYPE_PROTONAME;
	gcw.dwFlags = GC_TCHAR;

	if (!(szChatName = SkypeGet ("CHAT", (char *)wParam, "FRIENDLYNAME")) || !*szChatName)
		gcw.ptszName=TranslateT("Unknown"); else {
#ifdef _UNICODE
		gcw.ptszName=make_unicode_string((const unsigned char*)szChatName);
		free (szChatName);
		szChatName = (char*)gcw.ptszName;
#else
		gcw.ptszName=szChatName;
#endif
	}
	gcw.ptszID = make_nonutf_tchar_string((const unsigned char*)wParam);
	gcw.ptszStatusbarText = NULL;
	EnterCriticalSection(&m_GCMutex);
	if (!CallService(MS_GC_NEWSESSION, 0, (LPARAM)&gcw)) {
		char *szChatRole;

		gce.cbSize = sizeof(GCEVENT);
		gcd.pszModule = SKYPE_PROTONAME;
		gcd.ptszID = (TCHAR*)gcw.ptszID;
		gcd.iType = GC_EVENT_ADDGROUP;
		gce.pDest = &gcd;
		gce.ptszStatus = _T("CREATOR");
		gce.dwFlags = GC_TCHAR;
		// BUG: Groupchat returns nonzero on success here in earlier versions, so we don't check
		// it here
		CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
		gce.ptszStatus = _T("MASTER");
		CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
		gce.ptszStatus = _T("HELPER");
		CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
		gce.ptszStatus = _T("USER");
		CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
		gce.ptszStatus = _T("LISTENER");
		CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
		gce.ptszStatus = _T("APPLICANT");
		CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

		gcd.iType = GC_EVENT_JOIN;
		gce.ptszStatus = NULL;
		if (protocol >=7 && (szChatRole = SkypeGet ("CHAT", (char *)wParam, "MYROLE"))) {
			if (strncmp(szChatRole, "ERROR", 5))
			{
#ifdef _UNICODE
				gce.ptszStatus = make_unicode_string((const unsigned char*)szChatRole);
				free (szChatRole);
#else
				gce.ptszStatus = szChatRole;
#endif
			}
		}
		if (!gce.ptszStatus) gce.ptszStatus=_tcsdup(_T("CREATOR"));

		if (!db_get_ts(NULL, SKYPE_PROTONAME, "Nick", &dbv)) {
			if (!db_get_ts(NULL, SKYPE_PROTONAME, SKYPE_NAME, &dbv2)) {
				gce.ptszNick = dbv.ptszVal;
				gce.ptszUID = dbv2.ptszVal;
				gce.time = 0;
				gce.bIsMe = TRUE;
				gce.dwFlags |= GCEF_ADDTOLOG;
				if (!CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce)) {
					gce.cbSize = sizeof(GCEVENT);
					gcd.iType = GC_EVENT_CONTROL;
					gce.pDest = &gcd;
					if (!lParam) CallService(MS_GC_EVENT, SESSION_INITDONE, (LPARAM)&gce);
					CallService(MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gce);
					CallService(MS_GC_EVENT, lParam ? WINDOW_HIDDEN : WINDOW_VISIBLE, (LPARAM)&gce);
					SkypeSend ("GET CHAT %s MEMBERS", (char *)wParam);
					iRet = 0;
				}
				else {LOG (("ChatInit: Joining 'me' failed."));}
			}
			db_free(&dbv2);
		}
		free ((void*)gce.ptszStatus);
		db_free(&dbv);
	}
	free (szChatName);
	free_nonutf_tchar_string ((void*)gcw.ptszID);
	LeaveCriticalSection(&m_GCMutex);
	return iRet;
}

/* Open new Groupchat

   Parameters:  szChatId = (char *)Name of new chat session
*/
int  __cdecl ChatStart(char *szChatId, BOOL bJustCreate) {
	LOG(("ChatStart: New groupchat started"));
	if (!szChatId || NotifyEventHooks(hInitChat, (WPARAM)szChatId, bJustCreate)) return -1;
	return 0;
}


void KillChatSession(GCDEST *gcd) {
	GCEVENT gce = {0};

	EnterCriticalSection(&m_GCMutex);
	LOG(("KillChatSession: Groupchatsession terminated."));
	gce.cbSize = sizeof(GCEVENT);
	gce.dwFlags = GC_TCHAR;
	gce.pDest = gcd;
	gcd->iType = GC_EVENT_CONTROL;
	if (SkypeSend ("ALTER CHAT "STR" LEAVE", gcd->ptszID) == 0)
	{
		CallService(MS_GC_EVENT, SESSION_OFFLINE, (LPARAM)&gce);
		CallService(MS_GC_EVENT, SESSION_TERMINATE, (LPARAM)&gce);
	}
	LeaveCriticalSection(&m_GCMutex);
}

void InviteUser(const TCHAR *szChatId)
{
	HMENU tMenu = CreatePopupMenu();
	MCONTACT hContact = db_find_first(), hInvitedUser;
	DBVARIANT dbv;
	HWND tWindow;
	POINT pt;
	gchat_contacts *gc;
	int j;

	if (!szChatId || !(gc=GetChat(szChatId))) return;

	// add the heading
	AppendMenu(tMenu, MF_STRING|MF_GRAYED|MF_DISABLED, (UINT_PTR)0, TranslateT("&Invite user..."));
	AppendMenu(tMenu, MF_SEPARATOR, (UINT_PTR)1, NULL);
    
	// generate a list of contact
	while (hContact) {
		char *szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact,0 );
		if (szProto && !strcmp(SKYPE_PROTONAME, szProto) &&
			!db_get_b(hContact, SKYPE_PROTONAME, "ChatRoom", 0) &&
			 db_get_w(hContact, SKYPE_PROTONAME, "Status", ID_STATUS_OFFLINE)!=ID_STATUS_OFFLINE) 
		{
			BOOL alreadyInSession = FALSE;
			for (j=0; j<gc->mJoinedCount; j++) {
				if (gc->mJoinedContacts[j].hContact==hContact) {
					alreadyInSession = TRUE;
					break;
				}
			}
            if (!alreadyInSession)
				AppendMenu(tMenu, MF_STRING, (UINT_PTR)hContact, 
					(TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR));
        }
		hContact = db_find_next(hContact);
	}

	tWindow = CreateWindow(_T("EDIT"),_T(""),0,1,1,1,1,NULL,NULL,hInst,NULL);

	GetCursorPos (&pt);
	hInvitedUser = (MCONTACT)TrackPopupMenu(tMenu, TPM_NONOTIFY | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, tWindow, NULL);
	DestroyMenu(tMenu);
	DestroyWindow(tWindow);

	if (!hInvitedUser || db_get_s(hInvitedUser, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) 
		return;
	SkypeSend ("ALTER CHAT "STR" ADDMEMBERS %s", szChatId, dbv.pszVal);
	db_free(&dbv);

}

static void KickUser (MCONTACT hContact, GCHOOK *gch)
{
	char *ptr;

	EnterCriticalSection(&m_GCMutex);
	if (SkypeSend ("ALTER CHAT "STR" KICK "STR, gch->pDest->ptszID, gch->ptszUID)!=-1) {
		if (ptr=SkypeRcv("ALTER CHAT KICK", 2000)) {
			if (strncmp(ptr, "ERROR", 5)) {
				GCDEST gcd = {0};
				GCEVENT gce = {0};
				CONTACTINFO ci = {0};
				DBVARIANT dbv;

				gcd.pszModule = SKYPE_PROTONAME;
				gcd.ptszID = (TCHAR*)gch->pDest->ptszID;
				gcd.iType = GC_EVENT_KICK;

				gce.cbSize = sizeof(GCEVENT);
				gce.pDest = &gcd;
				gce.time = (DWORD)time(NULL);
				gce.dwFlags = GCEF_ADDTOLOG | GC_TCHAR;
				gce.ptszUID= gch->ptszUID;

				ci.cbSize = sizeof(ci);
				ci.szProto = SKYPE_PROTONAME;
				ci.dwFlag = CNF_DISPLAY | CNF_TCHAR;
				ci.hContact = hContact;
				if (hContact && !CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&ci)) gce.ptszNick=ci.pszVal; 
				else gce.ptszNick=gce.ptszUID;
        
				if (!db_get_ts(NULL, SKYPE_PROTONAME, "Nick", &dbv)) {
					gce.ptszStatus = dbv.ptszVal;		
					CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
					RemChatContact (GetChat(gcd.ptszID), gch->ptszUID);
					db_free(&dbv);
				}
				if (ci.pszVal) mir_free (ci.pszVal);
			}
			free (ptr);
		}
	}
	LeaveCriticalSection(&m_GCMutex);
}

void SetChatTopic(TCHAR *szChatId, TCHAR *szTopic, BOOL bSet)
{
	GCDEST gcd = {0};
	GCEVENT gce = {0};
	MCONTACT hContact = find_chat (szChatId);
	char *szUTFTopic;

	gce.cbSize = sizeof(GCEVENT);
	gcd.pszModule = SKYPE_PROTONAME;
	gcd.ptszID = szChatId;
	gcd.iType = GC_EVENT_TOPIC;
	gce.pDest = &gcd;
	gce.ptszText = szTopic;
	gce.dwFlags = GCEF_ADDTOLOG | GC_TCHAR;
	gce.time = (DWORD)time (NULL);
	gce.dwFlags = GC_TCHAR;
	CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

	gcd.iType = GC_EVENT_SETSBTEXT;
	CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

	if (bSet) {
#ifdef _UNICODE
		szUTFTopic=(char*)make_utf8_string(szTopic);
#else
		if (utf8_encode(szTopic, &szUTFTopic)==-1) szUTFTopic = NULL;
#endif
		if (szUTFTopic) {
			SkypeSend ("ALTER CHAT "STR" SETTOPIC %s", szChatId, szUTFTopic);
			free (szUTFTopic);
		}
		testfor ("ALTER CHAT SETTOPIC", INFINITE);
	}

	if (hContact)
		db_set_ts(hContact, SKYPE_PROTONAME, "Nick", szTopic);
}


int GCEventHook(WPARAM wParam,LPARAM lParam) {
	GCHOOK *gch = (GCHOOK*) lParam;
	gchat_contacts *gc = GetChat(gch->pDest->ptszID);

	UNREFERENCED_PARAMETER(wParam);

	if(gch) {
		if (!_stricmp(gch->pDest->pszModule, SKYPE_PROTONAME)) {

			switch (gch->pDest->iType) {
			case GC_SESSION_TERMINATE: {
				MCONTACT hContact;

				if (gc->mJoinedCount == 1) {
					// switch back to normal session
                    // I don't know if this behaviour isn't a bit annoying, therefore, we
					// don't do this now, until a user requests this feature :)
                    
					// open up srmm dialog when quit while 1 person left
//					CallService(MS_MSG_SENDMESSAGE, (WPARAM)gc->mJoinedContacts[0].hContact, 0);

					RemChatContact(gc, gc->mJoinedContacts[0].who);
				}
				// Delete Chatroom from Contact list, as we don't need it anymore...?
				if (hContact = find_chat(gc->szChatName))
					CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0); 
				RemChat(gc->szChatName);

				break;
			}
			case GC_USER_MESSAGE:
				if(gch && gch->ptszText && _tcslen(gch->ptszText) > 0) {
					DBVARIANT dbv, dbv2;
					CCSDATA ccs = {0};
					GCDEST gcd = { 0 };
					GCEVENT gce = { sizeof(gce), &gcd };
					TCHAR *pEnd;

					// remove the ending linebreak
					for (pEnd = &gch->ptszText[_tcslen(gch->ptszText) - 1];
						 *pEnd==_T('\r') || *pEnd==_T('\n'); pEnd--) *pEnd=0;
                    // Send message to the chat-contact    
					if (ccs.hContact = find_chat(gch->pDest->ptszID)) {
#ifdef _UNICODE
						// If PREF_UTF is supported, just convert it to UTF8 and pass the buffer to PSS_MESSAGE
						if (mirandaVersion >= 0x070000) {
							ccs.lParam = (LPARAM)make_utf8_string(gch->ptszText);
							ccs.wParam = PREF_UTF;
							CallProtoService (SKYPE_PROTONAME, PSS_MESSAGE, 0, (LPARAM)&ccs);
							free ((void*)ccs.lParam);
						} else {
						// Otherwise create this strange dual miranda-format
							ccs.lParam = (LPARAM)calloc(3, _tcslen(gch->ptszText)+1);
							wcstombs ((char*)ccs.lParam, gch->ptszText, _tcslen(gch->ptszText)+1);
							_tcscpy ((TCHAR*)((char*)ccs.lParam+strlen((char*)ccs.lParam)+1), gch->ptszText);
							ccs.wParam = PREF_UNICODE;
							CallProtoService (SKYPE_PROTONAME, PSS_MESSAGE, 0, (LPARAM)&ccs);
							free ((void*)ccs.lParam);
						}
#else
						ccs.lParam = (LPARAM)gch->ptszText;
						ccs.wParam = PREF_TCHAR;
						CallProtoService (SKYPE_PROTONAME, PSS_MESSAGE, 0, (LPARAM)&ccs);
#endif
					}

					// Add our line to the chatlog	
					gcd.pszModule = gch->pDest->pszModule;
					gcd.ptszID = gch->pDest->ptszID;
					if ( _tcsncmp(gch->ptszText, _T("/me "), 4)==0 && _tcslen(gch->ptszText)>4) {
						gce.ptszText = gch->ptszText+4;
						gcd.iType = GC_EVENT_ACTION;
					}
					else {
						gce.ptszText = gch->ptszText;
						gcd.iType = GC_EVENT_MESSAGE;
					}

					if (db_get_ts(NULL, SKYPE_PROTONAME, "Nick", &dbv))
						gce.ptszNick = TranslateT("Me");
					else
						gce.ptszNick = dbv.ptszVal;
					db_get_ts(NULL, SKYPE_PROTONAME, SKYPE_NAME, &dbv2);
					gce.ptszUID = dbv2.ptszVal;
					gce.time = (DWORD)time(NULL);
					gce.dwFlags = GCEF_ADDTOLOG;
					gce.bIsMe = TRUE;
					CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
					if (dbv.pszVal) db_free(&dbv);
					if (dbv2.pszVal) db_free(&dbv2);
				}
				break;
			case GC_USER_CHANMGR:
				InviteUser(gch->pDest->ptszID);
				break;
			case GC_USER_PRIVMESS: {
				MCONTACT hContact = find_contactT(gch->ptszUID);
				if (hContact) CallService(MS_MSG_SENDMESSAGE, (WPARAM)hContact, 0);
				break;

			}
			case GC_USER_LOGMENU:
				switch(gch->dwData) {
				case 10: InviteUser(gch->pDest->ptszID); break;
				case 20: KillChatSession(gch->pDest); break;
                case 30: 
					{
						TCHAR *ptr, buf[MAX_BUF];

						ptr = SkypeGetT ("CHAT", (TCHAR*)gch->pDest->ptszID, "TOPIC");
						_tcscpy(buf, ptr);
						free(ptr);
						if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_INPUTBOX), NULL, InputBoxDlgProc, (LPARAM)&buf))
							SetChatTopic((TCHAR*)gch->pDest->ptszID, buf, TRUE);
						break;
					}
				}
				break;
			case GC_USER_NICKLISTMENU: {
				MCONTACT hContact = find_contactT(gch->ptszUID);

				switch(gch->dwData) {
				case 10:CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)hContact, 0); break;
				case 20:CallService(MS_HISTORY_SHOWCONTACTHISTORY, (WPARAM)hContact, 0); break;
				case 30: KickUser(hContact, gch); break;
				case 110: KillChatSession(gch->pDest); break;
				}
				break;
			}			
			default:
				break;
			}
		}

	}
	return 0;
}

int __cdecl  GCMenuHook(WPARAM wParam,LPARAM lParam) {
	GCMENUITEMS *gcmi= (GCMENUITEMS*) lParam;
	DBVARIANT dbv;
	TCHAR* szInvite  = TranslateT("&Invite user...");
	TCHAR* szLeave   = TranslateT("&Leave chat session");
	TCHAR* szTopic   = TranslateT("Set &Topic...");
	TCHAR* szDetails = TranslateT("User &details");
	TCHAR* szHistory = TranslateT("User &history");
	TCHAR* szKick    = TranslateT("&Kick user");

	static struct gc_item Item_log[] = {
		{NULL, 10, MENU_ITEM, FALSE},
		{NULL, 30, MENU_ITEM, FALSE},
		{NULL, 20, MENU_ITEM, FALSE}
	};
	static struct gc_item Item_nicklist_me[] = {
		{NULL, 20, MENU_ITEM, FALSE},
		{_T(""), 100, MENU_SEPARATOR, FALSE},
		{NULL, 110, MENU_ITEM, FALSE}
	};
	static struct gc_item Item_nicklist[] = {
		{NULL, 10, MENU_ITEM, FALSE},
		{NULL, 20, MENU_ITEM, FALSE},
		{NULL, 30, MENU_ITEM, FALSE}
	};

	UNREFERENCED_PARAMETER(wParam);

	Item_log[0].pszDesc  = szInvite;
	Item_log[1].pszDesc  = szTopic;
	Item_log[2].pszDesc  = szLeave;
	Item_nicklist_me[0].pszDesc  = szHistory;
	Item_nicklist_me[2].pszDesc  = szLeave;
	Item_nicklist[0].pszDesc  = szDetails;
	Item_nicklist[1].pszDesc  = szHistory;
	Item_nicklist[2].pszDesc  = szKick;

	LOG (("GCMenuHook started."));
	if(gcmi) {
		if (!_stricmp(gcmi->pszModule, SKYPE_PROTONAME)) {
			switch (gcmi->Type)
			{
			case MENU_ON_LOG:
				gcmi->nItems = sizeof(Item_log)/sizeof(Item_log[0]);
				gcmi->Item = &Item_log[0];
                LOG (("GCMenuHook: Items in log window: %d", gcmi->nItems));
				break;
			case MENU_ON_NICKLIST:
				if (db_get_ts(NULL, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) return -1;
				if (!lstrcmp(dbv.ptszVal, gcmi->pszUID)) {
					gcmi->nItems = sizeof(Item_nicklist_me)/sizeof(Item_nicklist_me[0]);
					gcmi->Item = &Item_nicklist_me[0];
				} else {
					gchat_contacts *gcs = GetChat(gcmi->pszID);
					gchat_contact *gc = gcs?GetChatContact(gcs, gcmi->pszUID):NULL;
					gcmi->nItems = sizeof(Item_nicklist)/sizeof(Item_nicklist[0]);

					Item_nicklist[2].bDisabled = FALSE;
					if (gc && !gc->hContact)
					{
						gcmi->nItems -= 2;
						gcmi->Item = &Item_nicklist[2];
					}
					else
						gcmi->Item = &Item_nicklist[0];
					/*
					if (protocol<7) Item_nicklist[2].bDisabled = TRUE;
					else {
						TCHAR *szChatRole;
						if (szChatRole = SkypeGetT ("CHAT", gcmi->pszID, "MYROLE")) {
							if (_tcscmp(szChatRole, _T("MASTER")) && _tcscmp(szChatRole, _T("CREATOR")))
								Item_nicklist[2].bDisabled = TRUE;
							free (szChatRole);
						}
					}*/
					
				}
				db_free(&dbv);
				break;
			}
        } else {LOG (("GCMenuHook: ERROR: Not our protocol."));}
	} else {LOG (("GCMenuHook: ERROR: No gcmi"));}
	LOG (("GCMenuHook: terminated."));
	return 0;
}

INT_PTR GCOnLeaveChat(WPARAM wParam,LPARAM lParam)
{
	MCONTACT hContact = (MCONTACT)wParam;
	DBVARIANT dbv;

	UNREFERENCED_PARAMETER(lParam);

	if (db_get_ts(hContact, SKYPE_PROTONAME, "ChatRoomID", &dbv) == 0)
	{
		GCDEST gcd = {0};

		gcd.pszModule = SKYPE_PROTONAME;
		gcd.iType = GC_EVENT_CONTROL;
		gcd.ptszID = dbv.ptszVal;
		KillChatSession(&gcd);
		db_free(&dbv);
	}
	return 0;
}
 
INT_PTR GCOnJoinChat(WPARAM wParam,LPARAM lParam)
{
	MCONTACT hContact = (MCONTACT)wParam;
	DBVARIANT dbv;

	UNREFERENCED_PARAMETER(lParam);

	if (db_get_s(hContact, SKYPE_PROTONAME, "ChatRoomID", &dbv) == 0)
	{
		ChatStart (dbv.pszVal, FALSE);
		db_free(&dbv);
	}
	return 0;
}

void GCInit(void)
{
	InitializeCriticalSection (&m_GCMutex);
}

void GCExit(void)
{
	int i;

	DeleteCriticalSection (&m_GCMutex);
	for (i=0;i<chatcount;i++) {
		if (chats[i].szChatName) free(chats[i].szChatName);
		if (chats[i].mJoinedContacts) free(chats[i].mJoinedContacts);
	}
	if (chats) free (chats);
	chats = NULL;
	chatcount = 0;
}