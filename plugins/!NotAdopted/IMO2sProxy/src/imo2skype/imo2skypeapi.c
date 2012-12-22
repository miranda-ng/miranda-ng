/* Module:  imo2skypeapi.c
   Purpose: Simple wrapper for imo.im Webservice to SKYPE API to maintain compatibility with Skype-Plugins
   Author:  leecher
   Date:    30.08.2009
*/

#define VOICECALL_VERSION 1221873445
#define IVC_VERSION "98a29c15e305a7af04634b03d5e1425d6c67806e"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h> 
#include <direct.h>
#include "w32browser.h"
#define strcasecmp stricmp
#define strncasecmp strnicmp
#define thread_t HANDLE
#define vsnprintf _vsnprintf
#define GWL_PINST		0
#define GWL_ORIGWPRC	1*sizeof(PVOID)
#else
#define thread_t pthread_t
#include <unistd.h>
#include <pthread.h>
#endif
#include "imo_skype.h"
#include "imo_request.h"
#include "fifo.h"
#include "memlist.h"
#include "buddylist.h"
#include "avatarlist.h"
#include "msgqueue.h"
#include "callqueue.h"
#include "imo2skypeapi.h"

typedef struct
{
	char *pszImoStat;
	char *pszSkypeStat;
} STATMAP;

struct _tagIMOSAPI
{
    IMOSKYPE *hInst;
    NICKENTRY myUser;
    char *pszPass;
    char *pszLogBuf;
	char *pszClientName;
    int cbBuf;
    TYP_LIST *hBuddyList;
	TYP_LIST *hAvatarList;
    TYP_LIST *hMsgQueue;
    TYP_LIST *hCallQueue;
    int iProtocol;
    int iLoginStat;
    IMO2SCB Callback;
    void *pUser;
	FILE *fpLog;
    thread_t hThread;
    int bFriendsPending;
    int iFlags;
	int iShuttingDown;
	volatile time_t tSetMoodText;
	char *pszCmdID;
	char *pszBuddiesToAdd;
};

static STATMAP m_stMap[] =
{
	{"available", "ONLINE"},
	{"available", "SKYPEME"},
	{"offline", "OFFLINE"},
	{"away", "AWAY"},
	{"busy", "DND"},
	{"busy", "NA"},
	{"invisible", "INVISIBLE"}
};

static int StartCallSWF (IMOSAPI *pInst, CALLENTRY *pCall);
static int StatusCallback (cJSON *pMsg, void *pUser);
static void DispatcherThread(void *pUser);
static int Dispatcher_Start(IMOSAPI *pInst);
static int Dispatcher_Stop(IMOSAPI *pInst);
static void Send(IMOSAPI *pInst, const char *pszMsg, ...);
static void HandleMessage(IMOSAPI *pInst, char *pszMsg);


// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------

IMOSAPI *Imo2S_Init(IMO2SCB Callback, void *pUser, int iFlags)
{
	IMOSAPI *pInst = calloc(1, sizeof(IMOSAPI));

	if (!pInst) return NULL;
	if (!(pInst->pszLogBuf = malloc(pInst->cbBuf=512)) ||
	    !(pInst->hInst = ImoSkype_Init(StatusCallback, pInst)) ||
	    !(pInst->hBuddyList = BuddyList_Init()) ||
		!(pInst->hAvatarList = AvatarList_Init()) ||
	    !(pInst->hMsgQueue = MsgQueue_Init()) ||
#ifdef WIN32
		((iFlags & IMO2S_FLAG_ALLOWINTERACT) && W32Browser_Init(0)==-1) ||
#endif
	    !(pInst->hCallQueue = CallQueue_Init()))
	{
		Imo2S_Exit(pInst);
		return NULL;
	}
	pInst->Callback = Callback;
	pInst->pUser = pUser;
	pInst->iFlags = iFlags;
	pInst->myUser.iBuddyStatus = 3;
	strcpy (pInst->myUser.szStatus, "OFFLINE");
	pInst->iProtocol = 3;
	return pInst;
}

// -----------------------------------------------------------------------------

void Imo2S_SetLog (IMOSAPI *pInst, FILE *fpLog)
{
	pInst->fpLog = fpLog;
}

// -----------------------------------------------------------------------------

void Imo2S_Exit (IMOSAPI *pInst)
{
	if (!pInst) return;
	pInst->iShuttingDown = 1;
	if (pInst->fpLog) fprintf (pInst->fpLog, "Imo2S_Exit()\n");
	if (pInst->iLoginStat == 1) Imo2S_Logout(pInst);
	if (pInst->hInst) ImoSkype_Exit(pInst->hInst);
	if (pInst->hBuddyList) BuddyList_Exit(pInst->hBuddyList);
	if (pInst->hAvatarList) AvatarList_Exit(pInst->hAvatarList);
	if (pInst->hMsgQueue) MsgQueue_Exit(pInst->hMsgQueue);
	if (pInst->hCallQueue) CallQueue_Exit(pInst->hCallQueue);
	if (pInst->pszPass) free (pInst->pszPass);
	if (pInst->pszLogBuf) free(pInst->pszLogBuf);
	if (pInst->pszClientName) free(pInst->pszClientName);
#ifdef WIN32
	if (pInst->iFlags & IMO2S_FLAG_ALLOWINTERACT) W32Browser_Exit();
#endif
	BuddyList_FreeEntry(&pInst->myUser, TRUE);
	memset (pInst, 0, sizeof(IMOSAPI));
	free (pInst);
}

// -----------------------------------------------------------------------------

int Imo2S_Login (IMOSAPI *pInst, char *pszUser, char *pszPass, char **ppszError)
{
	// In case this module is passing in the original values...
	char *pszLocalUser, *pszLocalPass;

	if (pInst->fpLog) fprintf (pInst->fpLog, "Imo2S_Login(%s, ****)\n", pszUser);
	if (pInst->iLoginStat == 1) return pInst->iLoginStat;
	if (!pInst->hInst) pInst->hInst=ImoSkype_Init(StatusCallback, pInst);
	pszLocalUser = strdup(pszUser);
	if (pInst->myUser.pszUser) free (pInst->myUser.pszUser);
	pInst->myUser.pszUser = pszLocalUser;
	pszLocalPass = strdup(pszPass);
	if (pInst->pszPass) free (pInst->pszPass);
	pInst->pszPass = pszLocalPass;
	Send(pInst, "CONNSTATUS CONNECTING");
	pInst->iLoginStat = ImoSkype_Login(pInst->hInst, pszLocalUser, pszLocalPass);
	if (pInst->iLoginStat == 1)
	{
		ImoSkype_GetUnreadMsgs(pInst->hInst);
		if (IMO_API_VERSION > 0) ImoSkype_GetAlpha(pInst->hInst);
		Dispatcher_Start(pInst);
	}
	else
		if (ppszError) *ppszError = ImoSkype_GetLastError(pInst->hInst);
	return pInst->iLoginStat;
}

// -----------------------------------------------------------------------------

void Imo2S_Logout(IMOSAPI *pInst)
{
	if (pInst->fpLog) fprintf (pInst->fpLog, "Imo2S_Logout()\n");
	Dispatcher_Stop(pInst);
	if (ImoSkype_Logout(pInst->hInst) == 1)
	{
		pInst->iLoginStat = 0;
		strcpy (pInst->myUser.szStatus, "OFFLINE");
	}


	// If we relogin, user information won't be re-propagated if we
	// reuse the same connection. Therefore also dispose the connection
	// to imo.im service (wouldn't be necessary, but to ensure proper
	// repropagation of contacts on login, we have to do it, sorry)
	if (!pInst->iShuttingDown)
	{
		ImoSkype_Exit(pInst->hInst);
		pInst->hInst = NULL;
	}
}

// -----------------------------------------------------------------------------

int Imo2S_Send (IMOSAPI *pInst, char *pszMsg)
{
	char *pszDup = strdup(pszMsg);
	char *pszRealMsg = pszMsg;

	if (*pszRealMsg=='#')
	{
		char *p;
		if (p = strchr (pszRealMsg, ' ')) pszRealMsg=p+1;
	}
	if (pInst->fpLog) fprintf (pInst->fpLog, "Imo2S_Send(%s)\n", pszMsg);
	if (strlen(pszRealMsg)>15 && strncasecmp (pszRealMsg, "SET ", 4)== 0 &&
		(strncasecmp (pszRealMsg+4, "USERSTATUS", 10)==0 ||
		 strncasecmp (pszRealMsg+4, "CONNSTATUS", 10)==0))
	{
//		if (pInst->fpLog) fprintf (pInst->fpLog, "Imo2S_Send: iLoginStat = %d\n", 
//			pInst->iLoginStat);
		if (pInst->iLoginStat == 0)
		{
			if (pInst->myUser.pszUser && pInst->pszPass && strncasecmp (pszRealMsg+15, "OFFLINE", 7))
			{
				Imo2S_Login(pInst, pInst->myUser.pszUser, pInst->pszPass, NULL);
			}
		}
		else
		{
			HandleMessage (pInst, pszDup);
			if (strncasecmp (pszRealMsg+15, "OFFLINE", 7) == 0)
				Imo2S_Logout(pInst);
			free (pszDup);
			return 0;
		}
	}
	if (pInst->iLoginStat != 1) return -1;
	HandleMessage(pInst, pszDup);
	free (pszDup);
	return 0;
}

// -----------------------------------------------------------------------------
// Static
// -----------------------------------------------------------------------------


static int StatusCallback (cJSON *pMsg, void *pUser)
{
	char *pszName;
	BOOL bAdded;
	cJSON *pContent, *pProto;
	IMOSAPI *pInst = (IMOSAPI*)pUser;
	int m, iSize = cJSON_GetArraySize(pMsg);

	if (pInst->fpLog)
	{
		char *pszMsg = cJSON_Print(pMsg);
		fprintf (pInst->fpLog, "Imo2S::StatusCallback():%s\n", pszMsg);
		free(pszMsg);
	}

	for (m=0; m<iSize; m++)
	{
		pContent = cJSON_GetArrayItem(pMsg, m);
		if (!pContent) return 0;
		pszName = cJSON_GetObjectItem(pContent,"name")->valuestring;

		if ((pProto = cJSON_GetObjectItem(pContent,"proto")) && strcasecmp (pProto->valuestring, "prpl-skype"))
			continue;

		if (!strcmp(pszName, "recv_im"))
		{
			// I got a message!
			cJSON *pEdata = cJSON_GetObjectItem(pContent,"edata");

			if (pEdata)
			{
				MSGENTRY *pMsg;
				
				// imo.im sometimes seems to send information about messages you sent yourself.
				// We have to ignore them.
				if (strcmp(cJSON_GetObjectItem(pEdata, "buid")->valuestring, pInst->myUser.pszUser) &&
				   (pMsg = MsgQueue_Insert(pInst->hMsgQueue, pEdata)))
				{
					if (pInst->iFlags & IMO2S_FLAG_CURRTIMESTAMP) time(&pMsg->timestamp);
					Send(pInst, "%sMESSAGE %d STATUS %s", pInst->iProtocol>=3?"CHAT":"", 
						pMsg->hdr.uMsgNr, pMsg->szStatus);
				}
			}
		}
		else if (!strcmp(pszName, "signed_on"))
		{
			// I just signed on. 
			cJSON *pEdata = cJSON_GetObjectItem(pContent,"edata");
			char *pszAlias;

			Send(pInst, "CONNSTATUS ONLINE");
			if (strcmp(pInst->myUser.szStatus, "OFFLINE")==0)
				strcpy (pInst->myUser.szStatus, "ONLINE");
			Send(pInst, "USERSTATUS %s", pInst->myUser.szStatus);
			if (pEdata && (pszAlias = cJSON_GetObjectItem(pEdata, "alias")->valuestring))
			{
				pInst->myUser.pszAlias = strdup(pszAlias);
			}
			Send(pInst, "CURRENTUSERHANDLE %s", cJSON_GetObjectItem(pContent, "uid")->valuestring);
		}
		else if (!strcmp(pszName, "buddy_icon"))
		{
			// Here are the Avatars for the buddies
			//We have to track them in a seperate list as the buddies may get populated lateron
			cJSON *pArray = cJSON_GetObjectItem(pContent,"edata"), *pItem;
			int i, iCount;

			if (pArray)
			{
				for (i=0, iCount = cJSON_GetArraySize(pArray); i<iCount; i++)
				{
					if (pItem = cJSON_GetArrayItem(pArray, i))
						AvatarList_Insert(pInst->hAvatarList, pItem);
				}
			}
		}
		else if (!strcmp(pszName, "disconnect"))
		{
			// I got disconnected (wrong user/pass?)
			cJSON *pEdata = cJSON_GetObjectItem(pContent,"edata");
			//char *pszMsg;

			Send(pInst, "CONNSTATUS OFFLINE");
			Send(pInst, "USERSTATUS OFFLINE");
			strcpy (pInst->myUser.szStatus, "OFFLINE");
			/*
			if (pEdata && (pszMsg = cJSON_GetObjectItem(pEdata, "msg")->valuestring))
			{
				if (strcmp(pszMsg, "uidpassword")==0)
				{
					fprintf (stderr, "Invalid username / password combination!\n");
				}
			}
			*/
		}
		else if ((bAdded = !strcmp(pszName, "buddy_added")) || !strcmp(pszName, "buddy_status"))
		{
			// Here comes the contact list
			cJSON *pArray = cJSON_GetObjectItem(pContent,"edata"), *pItem;
			char *pszLastBUID = NULL, *pszLastGroup = NULL;
			int i, iCount;

			if (pArray)
			{
				TYP_LIST *hListMemToAdd = NULL;

				for (i=0, iCount = cJSON_GetArraySize(pArray); i<iCount; i++)
				{
					if (pItem = cJSON_GetArrayItem(pArray, i))
					{
						char szQuery[256];
						char *pszBUID = cJSON_GetObjectItem(pItem, "buid")->valuestring;
						cJSON *pGroup = cJSON_GetObjectItem(pItem, "group");
						cJSON *pDisplay = cJSON_GetObjectItem(pItem, "display");

						if (bAdded) 
						{
							/*
							if (BuddyList_Find (pInst->hBuddyList, cJSON_GetObjectItem(pItem, "buid")->valuestring))
								bAdded=FALSE;
							else
							*/
							BuddyList_Insert(pInst->hBuddyList, pItem);
						}
						else BuddyList_SetStatus(pInst->hBuddyList, pItem);

						if ((!pGroup || strcmp(pGroup->valuestring, "Skype")==0 ||
							strcmp(pGroup->valuestring, "Offline")==0) &&
							!pDisplay)
						{
							// Normal user, not groupchat, so output this
							sprintf (szQuery, "GET USER %s ONLINESTATUS", pszBUID);
							HandleMessage (pInst, szQuery);
							sprintf (szQuery, "GET USER %s MOOD_TEXT", pszBUID);
							HandleMessage (pInst, szQuery);
						}
						else
						{
							// Groupchat created/added
							MSGENTRY * pMsgEntry = NULL;

							if (bAdded && pInst->pszBuddiesToAdd)
							{
								// Process pending ADDs
								char *pTok, szQuery[256];

								for (pTok = strtok(pInst->pszBuddiesToAdd, ", "); pTok; pTok=strtok(NULL, ", "))
									ImoSkype_GroupInvite(pInst->hInst, pszBUID, pTok);
								free (pInst->pszBuddiesToAdd);
								pInst->pszBuddiesToAdd = NULL;
								sprintf (szQuery, "GET CHAT %s STATUS", pszBUID);
								HandleMessage (pInst, szQuery);
							}
							else
							{
								if (bAdded || !pDisplay)
								{
									char szQuery[256], szGroup[256]={0};
									NICKENTRY *pChat;

									if (pGroup) sprintf (szGroup, "%s;", pGroup->valuestring);
									if (pGroup && (pChat=BuddyList_Find(pInst->hBuddyList, szGroup)) && 
										pChat->hGCMembers)
									{
										sprintf (szQuery, "GET CHAT %s MEMBERS", szGroup);
										HandleMessage (pInst, szQuery);
									}
									else
									{
										if ((bAdded || !pChat) && (!pszLastBUID || strcmp(pszLastBUID, pszBUID)) &&
											(!pszLastGroup || (pGroup && strcmp(pszLastGroup, pGroup->valuestring))))
										{
											if (!hListMemToAdd) hListMemToAdd=List_Init(0);
											List_Push(hListMemToAdd, pItem);
											pszLastBUID = pszBUID;
											if (pGroup) pszLastGroup = pGroup->valuestring;
										}
									}
								}
								/*
								else
								{
									pMsgEntry = MsgQueue_AddEvent (pInst->hMsgQueue, pszBUID, "SETTOPIC");
									if (pDisplay->valuestring) pMsgEntry->pszMessage = strdup(pDisplay->valuestring);
								}
								*/
								if (pMsgEntry)
									Send(pInst, "%sMESSAGE %d STATUS %s", pInst->iProtocol>=3?"CHAT":"", 
										pMsgEntry->hdr.uMsgNr, pMsgEntry->szStatus);
								
							}
						}
					}
				}
				if (hListMemToAdd)
				{
					int nCount;

					for (i=0, nCount=List_Count(hListMemToAdd); i<nCount; i++)
					{
						cJSON *pItem = List_ElementAt(hListMemToAdd, i);
						cJSON *pInviter = cJSON_GetObjectItem(pItem, "inviter");
						MSGENTRY *pMsgEntry;

						if (pMsgEntry = MsgQueue_AddEvent (pInst->hMsgQueue, cJSON_GetObjectItem(pItem, "buid")->valuestring, "ADDEDMEMBERS")) // MULTI_SUBSCRIBED
						{
							if (pInviter) pMsgEntry->pszAuthor = strdup(pInviter->valuestring);
							Send(pInst, "%sMESSAGE %d STATUS %s", pInst->iProtocol>=3?"CHAT":"", 
								pMsgEntry->hdr.uMsgNr, pMsgEntry->szStatus);
						}
					}

					List_Exit(hListMemToAdd);
					hListMemToAdd = NULL;
				}
				if (bAdded && pInst->bFriendsPending)
				{
					char szMsg[]="SEARCH FRIENDS";
					pInst->bFriendsPending = 0;
					HandleMessage (pInst, szMsg);
				}
			}
		}
		else if (!strcmp(pszName, "buddy_removed"))
		{
			// Here comes the contact list
			cJSON *pArray = cJSON_GetObjectItem(pContent,"edata"), *pItem;
			int i, iCount;

			if (pArray)
			{
				for (i=0, iCount = cJSON_GetArraySize(pArray); i<iCount; i++)
				{
					char *pszUser;
					cJSON *pBuid;

					if (iCount==1) pItem=pArray; else pItem = cJSON_GetArrayItem(pArray, i);
					if (pItem && (pBuid = cJSON_GetObjectItem(pItem, "buid")) &&
						(pszUser = pBuid->valuestring))
					{
						NICKENTRY *pNick = BuddyList_Find (pInst->hBuddyList, pszUser);

						if (pNick && pNick->pGroup)
						{
							MSGENTRY *pMsg = MsgQueue_AddEvent (pInst->hMsgQueue, pszUser, "KICKED");
							cJSON *pUid = cJSON_GetObjectItem(pContent,"uid");
							if (pUid) pMsg->pszAuthor = strdup(pUid->valuestring);
							Send(pInst, "%sMESSAGE %d STATUS %s", pInst->iProtocol>=3?"CHAT":"", 
								pMsg->hdr.uMsgNr, pMsg->szStatus);
						}
						else
							Send (pInst, "USER %s BUDDYSTATUS 1", pszUser);
						if (pNick)
							BuddyList_Remove (pInst->hBuddyList, pNick);
					}
				}
			}
		}
		else
		if (/*!strcmp(pszName, "recv") || */!strcmp(pszName, "streams_info"))
		{
			cJSON 	*pEdata = cJSON_GetObjectItem(pContent,"edata"),
				*pType = cJSON_GetObjectItem(pContent,"type"), *pVal;

			if (pType && pEdata)
			{
				if (strcasecmp(pType->valuestring, "call")==0 || strcasecmp(pType->valuestring, "video")==0 ||
					strcasecmp(pType->valuestring, "av")==0)
				{
					// Rring, rrring...
					int iDirection = ((pVal = cJSON_GetObjectItem(pEdata, "is_initiator")) && pVal->type == cJSON_True)?CALL_OUTGOING:CALL_INCOMING;
					CALLENTRY *pCall = CallQueue_Insert (pInst->hCallQueue, pEdata, iDirection);
					if (pCall)
					{
						Send (pInst, "CALL %d STATUS %s", pCall->hdr.uMsgNr, pCall->szStatus);
						if ((pInst->iFlags & IMO2S_FLAG_ALLOWINTERACT) && iDirection == CALL_OUTGOING)
							StartCallSWF (pInst, pCall);
					}
				}
			}
		}
		else
		if (!strcmp(pszName, "ended"))
		{
			cJSON 	*pEdata = cJSON_GetObjectItem(pContent,"edata"),
				*pType = cJSON_GetObjectItem(pContent,"type");

			if (pType && pEdata)
			{
				if (strcasecmp(pType->valuestring, "call")==0 || strcasecmp(pType->valuestring, "video")==0)
				{
					// No call ID, so just hangup all calls to this user
					int i, nCount = List_Count(pInst->hCallQueue);
					char *pszUser = cJSON_GetObjectItem(pEdata, "buid")->valuestring;
				
					for (i=0; i<nCount; i++)
					{
						CALLENTRY *pCall = (CALLENTRY*)List_ElementAt(pInst->hCallQueue, i);
					
						if (!strcmp(pCall->pszUser, pszUser) && strcmp (pCall->szStatus, "FINISHED"))
						{
							char szQuery[256];

							sprintf (szQuery, "SET CALL %d STATUS FINISHED", pCall->hdr.uMsgNr);
							HandleMessage (pInst, szQuery);
						}
					}
				}
			}	
		}
		else
		if (!strcmp(pszName, "reflect")) // Status reflections. We may want to support more of them in the future
		{
			cJSON 	*pEdata = cJSON_GetObjectItem(pContent,"edata"),
				*pType = cJSON_GetObjectItem(pContent,"type"), *pRname, *pValue;

			if (pEdata && pType && (pRname = cJSON_GetObjectItem(pEdata,"r_name")))
			{
				if (strcasecmp(pType->valuestring, "account") == 0)
				{
					if (strcasecmp (pRname->valuestring, "set_status") == 0 &&
						(pValue = cJSON_GetObjectItem(pEdata,"primitive")) )
					{
						unsigned int i;

						for (i=0; i<sizeof(m_stMap)/sizeof(m_stMap[0]); i++)
						{
							if (!strcasecmp(m_stMap[i].pszImoStat, pValue->valuestring))
							{
								strcpy (pInst->myUser.szStatus, m_stMap[i].pszSkypeStat);
								Send (pInst, "USERSTATUS %s", pInst->myUser.szStatus);
								break;
							}
						}

						// Just ensure that on autoaway we notify imo.im that we are still there
						if (!strcmp (pValue->valuestring, "away"))
							ImoSkype_KeepAlive(pInst->hInst);

						if (pValue = cJSON_GetObjectItem(pEdata,"status"))
						{
							if (pInst->myUser.pszStatusText) free (pInst->myUser.pszStatusText);
							pInst->myUser.pszStatusText = NULL;
							if (*pValue->valuestring)
								pInst->myUser.pszStatusText = strdup(pValue->valuestring);
						}

					}
				}
				else
				if (strcasecmp(pType->valuestring, "conv") == 0)
				{
					if (strcasecmp (pRname->valuestring, "send_im") == 0)
					{
						MSGENTRY *pMsg;
						
						if (pMsg = MsgQueue_AddReflect(pInst->hMsgQueue, pEdata, pInst->hBuddyList))
						{
							if (pInst->iFlags & IMO2S_FLAG_CURRTIMESTAMP) time(&pMsg->timestamp);
							Send(pInst, "%sMESSAGE %d STATUS %s", pInst->iProtocol>=3?"CHAT":"", 
								pMsg->hdr.uMsgNr, pMsg->szStatus);
						}
					}

				}
			}
		}
		else
		if (!strcmp(pszName, "expired") && m>=iSize-1)
		{
			// Session expired, so we have to reconnect
			Send(pInst, "CONNSTATUS OFFLINE");
			Send(pInst, "USERSTATUS OFFLINE");
			strcpy (pInst->myUser.szStatus, "OFFLINE");
			pInst->iLoginStat = 0;
			Dispatcher_Stop(pInst);
			if (pInst->myUser.pszUser && pInst->pszPass)
				Imo2S_Login(pInst, pInst->myUser.pszUser, pInst->pszPass, NULL);
		}
		else
		if (!strcmp(pszName, "ack"))
		{
			MSGENTRY *pMsg;
			cJSON 	*pRqId = cJSON_GetObjectItem(pContent,"request_id");

			if (pRqId && (pMsg = MsgQueue_FindByRqId(pInst->hMsgQueue, pRqId->valueint)))
			{
				cJSON 	*pResp, *pTS;

				strcpy (pMsg->szStatus, "SENT");
				if ((pResp = cJSON_GetObjectItem(pContent,"response")) &&
					(pTS   = cJSON_GetObjectItem(pResp,"timestamp")))
					pMsg->timestamp = pTS->valueint;
				Send (pInst, "MESSAGE %d STATUS %s", pMsg->hdr.uMsgNr, pMsg->szStatus);
			}
		}
		else
		{
			char *pszMsg = cJSON_Print(pMsg);
			fprintf (stderr, "%s\n\n", pszMsg);
			free (pszMsg);
		}
	}
	return 0;
}

// -----------------------------------------------------------------------------

#ifdef WIN32
// Set call status to finished when closing phone applet window just in case
// there is no callback by imo.im upon closure
static LRESULT CallWndFilter(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case  WM_DESTROY:
	{
		IMOSAPI *pInst = (IMOSAPI*)GetWindowLong (hWnd, GWL_PINST);
		int i, nCount;
	
		if (!pInst) break;
		for (i=0, nCount = List_Count(pInst->hCallQueue); i<nCount; i++)
		{
			CALLENTRY *pCall = (CALLENTRY*)List_ElementAt(pInst->hCallQueue, i);
		
			if (pCall->hCallWnd == hWnd)
			{
				char szQuery[256];

				sprintf (szQuery, "SET CALL %d STATUS FINISHED", pCall->hdr.uMsgNr);
				HandleMessage (pInst, szQuery);
				break;
			}
		}
		break;
	}
	}
	return  CallWindowProc((WNDPROC)GetWindowLong (hWnd, GWL_ORIGWPRC), hWnd, message, wParam, lParam);
}
#endif
		

// -----------------------------------------------------------------------------

static int StartCallSWF (IMOSAPI *pInst, CALLENTRY *pCall)
{
	char szSWF[256], szFlashVars[1024], szID[18];
	char szHTML[2048];
	static unsigned int id=100;


	/* The flash plugin basically opens rtmp://[host]/mchat in case of type=imo, otherwise rtmp://[host]/ivc 
	   Maybe this can also be used with a SIP-phone or Asterisk?
	 */
#ifndef WIN32
	FILE *fpTemp;
	int iFound;

	sprintf (pCall->szCallFile, "xdg-open %s.html", tmpnam(NULL));
	if (!(fpTemp=fopen(pCall->szCallFile, "w"))) return -1;
#else
	// Windows:
	// Ensure that we are allowed to load the .swf from imo.im
#ifndef CSIDL_APPDATA
#define CSIDL_APPDATA                   0x001a
#endif
	static BOOL bInit = TRUE;

	if (bInit)
	{
		char szPath[MAX_PATH]={0};
		HMODULE hModule;
		typedef HMODULE  (__stdcall *SHGETFOLDERPATH)(HWND, int, HANDLE, DWORD, LPTSTR);

		if (hModule = LoadLibrary("SHFOLDER.DLL"))
		{
			SHGETFOLDERPATH fnShGetFolderPath = (SHGETFOLDERPATH)GetProcAddress(hModule, "SHGetFolderPathA");
   
			 if (fnShGetFolderPath && fnShGetFolderPath( NULL, CSIDL_APPDATA, NULL, 0, szPath)==S_OK)
			 {
				 strcat (szPath, "\\Macromedia\\Flash Player");
				 if (GetFileAttributes (szPath) != 0xFFFFFFFF)
				 {
					 strcat (szPath, "\\#Security");
					 _mkdir (szPath);
					 strcat (szPath, "\\FlashPlayerTrust");
					 _mkdir (szPath);
					 strcat (szPath, "\\imo2sproxy.cfg");
					 if (GetFileAttributes (szPath) == 0xFFFFFFFF)
					 {
						 FILE *fp = fopen (szPath, "w");

						 if (fp)
						 {
							 fputs ("https://imo.im/images/nchat.swf\nabout:blank\n", fp);
							 fclose (fp);
						 }
					 }
				 }
			 }
		  FreeLibrary(hModule);
		}
		bInit = FALSE;
	}
#endif
	sprintf (szSWF, "https://imo.im/images/nchat.swf?%s", IVC_VERSION);
	ImoRq_CreateID (szID, 16);
	//sprintf (szID, "imo%d", id++);

	if (*pCall->szSendStream)
		sprintf (szFlashVars, 
		"type=skype&amp;"
		"send_stream=%s&amp;"
		"recv_stream=%s&amp;"
		"id=skype#%s&amp;"
		"host=video0.imo.im&amp;"
		"debug=conv net&amp;"
		"buddy_icon=https://imo.im/assets/main/icn_default.png&amp;"
		"setCookie=setCookie&amp;"
		"getCookie=getCookie&amp;"
		"removeCookie=removeCookie&amp;"
		"log=log&amp;"
		"init_callback=init_callback&amp;"
		"video_options_changed_cb=video_options_changed_cb&amp;"
		"options_changed_cb=options_changed_cb&amp;"
		"mic_changed_cb=mic_changed_cb&amp;"
		"cam_changed_cb=cam_changed_cb&amp;"
		"connection_status_cb=connection_status_cb",
		pCall->szSendStream, pCall->szRecvStream, pCall->szSendStream);
	else
		return -1;
	//	sprintf (szFlashVars, "type=imo&amp;conv=%s&amp;role=%d&amp;host=%s&amp;id=%s", pCall->szConv, pCall->iRole, pCall->szIP, szID);

	strcat (szFlashVars, "&amp;audio_only=true");
	// Currently not supported 
	sprintf (szHTML, "<html>\n"
		"<head><script language='javascript'>\n"
		"function log(t, s) { document.all.log.innerHTML+=s+'<br>'; }\n"
		"function setCookie (N, L, M) { alert ('setcookie av_chat_'+L+' = '+M); }\n"
		"function getCookie (M, L) { alert ('getcookie av_chat_'+L); }\n"
		"function removeCookie (M, L) { alert ('removecookie av_chat_'+L); }\n"
		"function init_callback(L) { }\n"
		"function video_options_changed_cb(v, t, s) { }\n"
		"function options_changed_cb(v, t, s) { }\n"
		"function mic_changed_cb(u, s) { }\n"
		"function cam_changed_cb(u, t) { }\n"
		"function connection_status_cb (u, s) { }\n"
		"</script></head>"
		"<body>\n"
		"<object width=\"400\" height=\"208\" type=\"application/x-shockwave-flash\" data=\"%s\" id=\"flash_widget_%s\" "
		"style=\"visibility: visible; width: 400px; height: 208px;\">\n"
		"<param name=\"movie\" value=\"%s\">\n"
		"<param name=\"allowscriptaccess\" value=\"always\">\n"
		"<param name=\"bgcolor\" value=\"#ffffff\">"
		"<param name=\"flashvars\" value=\"%s\">\n"
		"</object><br><div id=\"log\"></div>\n"
		"</body></html>\n",
		szSWF, szID, szSWF, szFlashVars);

#ifdef WIN32
	//OutputDebugString (szHTML);
	sprintf (szSWF, "Voicechat with %s", pCall->pszUser);

	/*

	{
		FILE *fpOut = fopen("C:\\TEMP\\CALL.HTML", "w");

		fputs (szHTML, fpOut);
		fclose (fpOut);
		system ("\"C:\\Programme\\Internet Explorer\\IEXPLORE.EXE\" C:\\TEMP\\CALL.HTML");
	}
	*/

	if ((pCall->hCallWnd = W32Browser_ShowHTMLStr (szHTML,420, 280, szSWF))>0)
	{
		// Hook WndProc to handle WM_DESTROY so that we generate a CALL %d STATUS FINISHED on
		// closing the chat window
#ifdef _WIN64
		SetWindowLongPtr ((HWND)pCall->hCallWnd, GWL_PINST, (LONG_PTR)pInst);
		SetWindowLongPtr ((HWND)pCall->hCallWnd, GWL_ORIGWPRC, 
			GetWindowLongPtr ((HWND)pCall->hCallWnd, GWLP_WNDPROC));
		SetWindowLongPtr ((HWND)pCall->hCallWnd, GWLP_WNDPROC, (LONG_PTR)CallWndFilter);
#else
		SetWindowLong ((HWND)pCall->hCallWnd, GWL_PINST, (LONG)pInst);
		SetWindowLong ((HWND)pCall->hCallWnd, GWL_ORIGWPRC, GetWindowLong ((HWND)pCall->hCallWnd, GWL_WNDPROC));
		SetWindowLong((HWND)pCall->hCallWnd, GWL_WNDPROC, (LONG)CallWndFilter);
#endif
		return 0;
	}
	return -1;
#else
	fprintf (fpTemp, "%s", szHTML);
	fclose (fpTemp);
	iFound = system(pCall->szCallFile);
	return (iFound == -1 || iFound == 127)?-1:0;
#endif
}

// -----------------------------------------------------------------------------

static void DispatcherThread(void *pUser)
{
	IMOSAPI *pInst = (IMOSAPI*)pUser;
	time_t t = 0, tcur;

	if (pInst->fpLog) fprintf (pInst->fpLog, "Imo2S::DispatcherThread() start\n");
	while (!pInst->iShuttingDown)
	{
#if defined(WIN32) && defined(WIN32)
		char szBuf[128];

		sprintf (szBuf, "DispatcherThread %d loops.\n", GetCurrentThreadId());
		OutputDebugString (szBuf);
#endif
		if (time(&tcur)>=t+300)
		{
			t=tcur;
			//ImoSkype_Ping (pInst->hInst);
			ImoSkype_KeepAlive(pInst->hInst);
		}

		// Set status in case this is needed
		if (pInst->tSetMoodText && tcur>=pInst->tSetMoodText)
		{
			int i;

			pInst->tSetMoodText = 0;
			for (i=0; i<sizeof(m_stMap)/sizeof(m_stMap[0]); i++)
			{
				if (!strcasecmp(m_stMap[i].pszSkypeStat, pInst->myUser.szStatus))
				{
					ImoSkype_SetStatus(pInst->hInst, m_stMap[i].pszImoStat, 
						pInst->myUser.pszStatusText?pInst->myUser.pszStatusText:"");
					break;
				}
			}
		}
		ImoSkype_Poll(pInst->hInst);
	}
}

// -----------------------------------------------------------------------------

#ifdef WIN32
static int Dispatcher_Start(IMOSAPI *pInst)
{
	DWORD ThreadID;

	if (pInst->fpLog) fprintf (pInst->fpLog, "Imo2S::Dispatcher_Start()\n");
    return (pInst->hThread=(thread_t)_beginthreadex(NULL, 0, 
		(unsigned(__stdcall *)(void*))DispatcherThread, pInst, 0, &ThreadID))!=0; 
	
}

static int Dispatcher_Stop(IMOSAPI *pInst)
{
	int iRet, iOldShutdown;

	if (pInst->fpLog)
	{
		fprintf (pInst->fpLog, "Imo2S::Dispatcher_Stop()\n");
		fflush(pInst->fpLog);
	}

	// Shutdown polling socket and wait some time if thread terminates
	// gracefully, otherwise kill it
	iOldShutdown = pInst->iShuttingDown;
	pInst->iShuttingDown = 1;
	ImoSkype_CancelPolling (pInst->hInst);
	if (WaitForSingleObject (pInst->hThread, 2000) == WAIT_TIMEOUT)
		iRet = TerminateThread (pInst->hThread, 0);
	else iRet = 1;
	pInst->iShuttingDown = iOldShutdown;

	if (iRet) 
	{
		CloseHandle (pInst->hThread);
		pInst->hThread = 0;
	}

	if (pInst->fpLog)
	{
		fprintf (pInst->fpLog, "Imo2S::Dispatcher_Stop() done.\n");
		pInst->fpLog = NULL;
	}

	return iRet;
}

#else
static int Dispatcher_Start(IMOSAPI *pInst)
{
	if (pInst->fpLog) fprintf (pInst->fpLog, "Imo2S::Dispatcher_Start()\n");
	return pthread_create(&pInst->hThread, NULL, DispatcherThread, pInst)==0;
}

static int Dispatcher_Stop(IMOSAPI *pInst)
{
	if (pInst->fpLog) fprintf (pInst->fpLog, "Imo2S::Dispatcher_Stop()\n");
	if (pthread_cancel(pInst->hThread))
	{
		pInst->hThread=0;
		return 1;
	}
	return 0;
}
#endif

// -----------------------------------------------------------------------------

static void Send(IMOSAPI *pInst, const char *pszMsg, ...)
{
	va_list ap;
	int iLen, iLenCmdID;
	char *pszLogBuf = pInst->pszLogBuf;
	int cbBuf = pInst->cbBuf;

	iLenCmdID = pInst->pszCmdID?strlen(pInst->pszCmdID)+1:0;
	do
	{
		cbBuf = pInst->cbBuf - iLenCmdID;
		pszLogBuf = pInst->pszLogBuf + iLenCmdID;
		va_start(ap, pszMsg);
		iLen = vsnprintf (pszLogBuf, cbBuf, pszMsg, ap);
		va_end(ap);
#ifndef WIN32
		if (iLen>=cbBuf) iLen=-1;
#endif
		if (iLen == -1)
		{
			char *pNewBuf;
			
			if (!(pNewBuf = realloc(pInst->pszLogBuf, pInst->cbBuf*2)))
			{
				break;
			}
			pInst->cbBuf*=2;
			pInst->pszLogBuf = pNewBuf;
		}
	} while (iLen == -1);
	if (pInst->pszCmdID && iLenCmdID>1)
	{
		memcpy (pInst->pszLogBuf, pInst->pszCmdID, iLenCmdID);
		pInst->pszLogBuf[iLenCmdID-1]=' ';
	}

//printf ("%s\n", szBuf);
	pInst->Callback(pInst->pszLogBuf, pInst->pUser);
}

// -----------------------------------------------------------------------------

static void HandleMessage(IMOSAPI *pInst, char *pszMsg)
{
	char *pszCmd=strtok(pszMsg, " ");

	if (!pInst || !pszCmd || !pInst->hInst) return;
	if (*pszCmd=='#')
	{
		// This is a PROTOCOL 4 feature, but we will support it just in case...
		pInst->pszCmdID = pszCmd;
		if (!(pszCmd=strtok(NULL, " ")))
		{
			pInst->pszCmdID = NULL;
			return;
		}
	}
	else pInst->pszCmdID = NULL;

	if (strcasecmp(pszCmd, "PROTOCOL") == 0)
	{
		if (pszCmd = strtok(NULL, " "))
		{
			pInst->iProtocol = atoi(pszCmd);
			if (pInst->iProtocol>3) pInst->iProtocol=3;
		}

		Send (pInst, "PROTOCOL %d", pInst->iProtocol);
		return;
	}
	else
	if (strcasecmp(pszCmd, "PING") == 0)
	{
		Send (pInst, "PONG");
		return;
	}
	else
	if (strcasecmp(pszCmd, "SEARCH") == 0)
	{
		TYP_FIFO *hFifo;

		if (!(pszCmd = strtok(NULL, " ")))
		{
			Send (pInst, "ERROR 2 Invalid command");
			return;
		}
		
		if (strcasecmp(pszCmd, "FRIENDS") == 0)
		{
			unsigned int nCount =List_Count(pInst->hBuddyList);
			
			if (!nCount)
			{
				pInst->bFriendsPending = 1;
				return;
			}
			if(hFifo=Fifo_Init(512))
			{
				unsigned int i;
				char *pszUsers;

				for (i=0; i<nCount; i++)
				{
					NICKENTRY *pEntry = List_ElementAt(pInst->hBuddyList, i);
					if (!pEntry->pGroup && !pEntry->hGCMembers)
					{
						if (i>0) Fifo_AddString (hFifo, ", ");
						Fifo_AddString (hFifo, pEntry->pszUser);
					}
				}
				if (pszUsers = Fifo_Get(hFifo, NULL))
					Send (pInst, "USERS %s", pszUsers);				
				else
					pInst->bFriendsPending = 1;
				Fifo_Exit(hFifo);
			}
		}
		else if (strcasecmp(pszCmd, "MISSEDMESSAGES") == 0)
		{
			unsigned int nCount=List_Count(pInst->hMsgQueue);

			if(nCount && (hFifo=Fifo_Init(512)))
			{
				unsigned int i, j=0;

				for (i=0; i<nCount; i++)
				{
					MSGENTRY *pEntry = List_ElementAt(pInst->hMsgQueue, i);
					char szNr[32];

					if (!strcmp(pEntry->szStatus, "RECEIVED"))
					{
						if (j>0) Fifo_AddString (hFifo, ", ");
						sprintf (szNr, "%d", pEntry->hdr.uMsgNr);
						Fifo_AddString (hFifo, szNr);
						j++;
					}
				}
				Send (pInst, "MESSAGES %s", Fifo_Get(hFifo, NULL));
				Fifo_Exit(hFifo);
			}
		}
		else if (strcasecmp(pszCmd, "USERS") == 0)
		{
			// There is no possibility to search Skype users in imo.im
			// therefore just return that the user exists even if it doesn't :(
			Send (pInst, "USERS %s", pszCmd+6);
			// We add the user as a temporary contact to our list so that the
			// client can get the empty properties and set the buddystatus.
			BuddyList_AddTemporaryUser (pInst->hBuddyList, pszCmd+6);
		}
		else if (strcasecmp(pszCmd, "USERSWAITINGMYAUTHORIZATION") == 0)
		{
			Send (pInst, "USERS");
		}
		else if (strcasecmp(pszCmd, "ACTIVECALLS") == 0)
		{
			char szCalls[512];
			int i, nCount, iOffs=0;

			iOffs = sprintf (szCalls, "CALLS");
			for (i=0, nCount=List_Count(pInst->hCallQueue); i<nCount; i++)
			{
				if (i) iOffs+=sprintf(&szCalls[iOffs], ", ");
				iOffs+=sprintf(&szCalls[iOffs], "%d", ((CALLENTRY*)List_ElementAt (pInst->hCallQueue, i))->hdr.uMsgNr);
				if (iOffs+6>=sizeof(szCalls)) break;
			}
			Send (pInst, szCalls);
		}
		else if (strcasecmp(pszCmd, "RECENTCHATS") == 0)
		{
			char szChats[2056];
			int i, nCount, iOffs=0, j=0;

			iOffs = sprintf (szChats, "CHATS ");
			for (i=0, nCount=List_Count(pInst->hBuddyList); i<nCount; i++)
			{
				NICKENTRY *pChat = (NICKENTRY*)List_ElementAt (pInst->hBuddyList, i);

				if (BuddyList_IsGroupchat(pChat))
				{
					if (j) iOffs+=sprintf(&szChats[iOffs], ", ");
					iOffs+=sprintf(&szChats[iOffs], "%s", pChat->pszUser);
					if (iOffs+6>=sizeof(szChats)) break;
					j++;
				}
			}
			Send (pInst, szChats);
		}
		return;
	}
	else
	if (strcasecmp(pszCmd, "GET") == 0)
	{
		if (!(pszCmd = strtok(NULL, " ")))
		{
			Send (pInst, "ERROR 7 Invalid property");
			return;
		}

		if (strcasecmp(pszCmd, "USER") == 0)
		{
			NICKENTRY *pUser = NULL;

			if (pszCmd = strtok(NULL, " "))
			{
				if (!strcasecmp (pszCmd, pInst->myUser.pszUser))
					pUser = &pInst->myUser;
				else
					pUser = BuddyList_Find(pInst->hBuddyList, pszCmd);
			}

			if (!pUser)			
			{
				Send (pInst, "ERROR 26 Invalid user handle");
				return;
			}
			if (!(pszCmd = strtok(NULL, " ")))
			{
				Send (pInst, "ERROR 10 Invalid property");
				return;
			}

			if (!strcasecmp (pszCmd, "HANDLE"))
			{
				if (pUser->pszAlias) Send (pInst, "USER %s HANDLE %s", pUser->pszUser, pUser->pszAlias);
			}
			else if (!strcasecmp (pszCmd, "FULLNAME")) /* Workaround */
				Send (pInst, "USER %s FULLNAME %s", pUser->pszUser, pUser->pszAlias);
			else if (!strcasecmp (pszCmd, "DISPLAYNAME"))
				Send (pInst, "USER %s DISPLAYNAME %s", pUser->pszUser, pUser->pszAlias);
			else if (!strcasecmp (pszCmd, "HASCALLEQUIPMENT"))
				Send (pInst, "USER %s HASCALLEQUIPMENT TRUE", pUser->pszUser);
			else if (!strcasecmp (pszCmd, "BUDDYSTATUS"))
				Send (pInst, "USER %s BUDDYSTATUS %d", pUser->pszUser, pUser->iBuddyStatus);
			else if (!strcasecmp (pszCmd, "ISAUTHORIZED"))
				Send (pInst, "USER %s ISAUTHORIZED TRUE", pUser->pszUser);
			else if (!strcasecmp (pszCmd, "MOOD_TEXT"))
				Send (pInst, "USER %s MOOD_TEXT %s", pUser->pszUser, 
					pUser->pszStatusText?pUser->pszStatusText:"");
			else if (!strcasecmp (pszCmd, "SEX"))
				Send (pInst, "USER %s SEX UNKNOWN", pUser->pszUser);
			else if (!strcasecmp (pszCmd, "BIRTHDAY"))
				Send (pInst, "USER %s BIRTHDAY 0", pUser->pszUser);
			else if (!strcasecmp (pszCmd, "ONLINESTATUS"))
			{
				unsigned int i;

				for (i=0; i<sizeof(m_stMap)/sizeof(m_stMap[0]); i++)
				{
					if (!strcasecmp(m_stMap[i].pszImoStat, pUser->szStatus))
					{
						Send (pInst, "USER %s ONLINESTATUS %s", pUser->pszUser, m_stMap[i].pszSkypeStat);
						break;
					}
				}
			}
			else if (!strcasecmp (pszCmd, "AVATAR"))
			{
				char *pszFile, *pAvatarBuf;
				unsigned int dwLength;
				AVATARENTRY *pAvatar;
				FILE *fp;

				if (!(pszFile = strtok(NULL, " ")) || strcasecmp (pszFile, "1"))
				{
					Send (pInst, "ERROR 116 GET invalid ID");
					return;
				}
				if (!(pszFile = strtok(NULL, "\n")))
				{
					Send (pInst, "ERROR 7 GET: invalid WHAT");
					return;
				}
				if (fp=fopen(pszFile, "r"))
				{
					fseek (fp, 0, SEEK_END);
					if (ftell(fp))
					{
						fclose(fp);
						Send (pInst, "ERROR 124 GET Destination file is not empty");
						return;
					}
					fclose(fp);
				}
				if (!(pAvatar = AvatarList_Find(pInst->hAvatarList, pUser->pszUser)) ||
					!(pAvatarBuf = ImoSkype_GetAvatar (pInst->hInst, pAvatar->pszIcon, &dwLength)))
				{
					// FIXME: Normally we should return default avatar if pAvatar is NULL.
					// Default avatar from imo.im is PNG, but JPEG is expected, so we 
					// return an error here
					Send (pInst, "ERROR 122 GET Unable to load avatar");
					return;
				}
				if (!(fp=fopen(pszFile, 
#ifdef WIN32
					"wb"
#else
					"w"
#endif
					)))
				{
					Send (pInst, "ERROR 121 GET File path doesn't exist");
					return;
				}
				fwrite (pAvatarBuf, dwLength, 1, fp);
				fclose (fp);
				Send (pInst, "USER %s AVATAR 1 %s", pUser->pszUser, pszFile);
			}
			else
			{
				Send(pInst, "ERROR 10 Invalid propery");
			}
			return;
		}
		else
		if (strcasecmp(pszCmd, "CURRENTUSERHANDLE") == 0)
		{
			if (pInst->myUser.pszUser)
				Send(pInst, "CURRENTUSERHANDLE %s", pInst->myUser.pszUser);
		}
		else
		if (strcasecmp(pszCmd, "USERSTATUS") == 0)
		{
			Send(pInst, "USERSTATUS %s", pInst->myUser.szStatus);
		}
		else
		if (strcasecmp(pszCmd, "MESSAGE") == 0 || strcasecmp(pszCmd, "CHATMESSAGE") == 0)
		{
			MSGENTRY *pEntry;
			char *pszMessage = pszCmd;

			if (!(pszCmd = strtok(NULL, " ")) || !(pEntry = MsgQueue_Find(pInst->hMsgQueue, atol(pszCmd))))
			{
				Send (pInst, "ERROR 14 Invalid message id");
				return;
			}
			if (!(pszCmd = strtok(NULL, " ")))
			{
				Send (pInst, "ERROR 10 Invalid property");
				return;
			}
			if (!strcasecmp (pszCmd, "TIMESTAMP"))
				Send (pInst, "%s %d TIMESTAMP %ld", pszMessage, pEntry->hdr.uMsgNr, pEntry->timestamp);
			else if (!strcasecmp (pszCmd, "PARTNER_HANDLE") || !strcasecmp (pszCmd, "FROM_HANDLE"))
			{
				char *pszUser = strdup(pEntry->pszAuthor?pEntry->pszAuthor:pEntry->pszUser), *p, *pszRealUser=pszUser;

				if (p=strtok(pszUser, ";")) 
				{
					if (!(pszRealUser=strtok(NULL, ";"))) pszRealUser=p;
				}
				Send (pInst, "%s %d %s %s", pszMessage, pEntry->hdr.uMsgNr, pszCmd, pszRealUser);
				free (pszUser);
			}
			else if (!strcasecmp (pszCmd, "PARTNER_DISPNAME"))
				Send (pInst, "%s %d PARTNER_DISPNAME %s", pszMessage, pEntry->hdr.uMsgNr, pEntry->pszAlias);
			else if (!strcasecmp (pszCmd, "TYPE"))
				Send (pInst, "%s %d TYPE %s", pszMessage, pEntry->hdr.uMsgNr, pEntry->szType);
			else if (!strcasecmp (pszCmd, "STATUS"))
				Send (pInst, "%s %d STATUS %s", pszMessage, pEntry->hdr.uMsgNr, pEntry->szStatus);
			else if (!strcasecmp (pszCmd, "FAILUREREASON"))
				Send (pInst, "%s %d FAILUREREASON %s", pszMessage, pEntry->hdr.uMsgNr, pEntry->szFailure);
			else if (!strcasecmp (pszCmd, "BODY"))
				Send (pInst, "%s %d BODY %s", pszMessage, pEntry->hdr.uMsgNr, pEntry->pszMessage);
			else if (!strcasecmp (pszCmd, "CHATNAME"))
			{
				if (pEntry->pszAuthor || !pEntry->pszMessage || strchr(pEntry->pszUser, ';')) // Groupchat
				{
					char *pszUser = strdup(pEntry->pszUser), *p;

					if (p=strchr(pszUser, ';')) p[1]=0;
					Send (pInst, "%s %d CHATNAME %s", pszMessage, pEntry->hdr.uMsgNr, pszUser);
					free (pszUser);
				}
				else
					Send (pInst, "%s %d CHATNAME #%s/$%s", pszMessage, pEntry->hdr.uMsgNr, pInst->myUser.pszUser, pEntry->pszUser);
			}
			else if (!strcasecmp (pszCmd, "USERS"))
			{
				// On KICK, this is the user who GOT kicked
				char *pszUser = strchr(pEntry->pszUser, ';');
				if (pszUser) pszUser++; else pszUser=pEntry->pszUser;
				Send (pInst, "%s %d USERS %s", pszMessage, pEntry->hdr.uMsgNr, pszUser);
			}
			else
				Send (pInst, "ERROR 10 Invalid property / not implemented");
			return;		
		}
		else
		if (strcasecmp(pszCmd, "PRIVILEGE") == 0)
		{
			if (!(pszCmd = strtok(NULL, " ")) ||
			    (strcasecmp (pszCmd, "SKYPEOUT") &&
			     strcasecmp (pszCmd, "SKYPEIN") &&
			     strcasecmp (pszCmd, "VOICEMAIL")))
			{
				Send (pInst, "ERROR 40 Unknown Privilege");
				return;
			}
			Send (pInst, "PRIVILEGE %s FALSE", pszCmd);
			return;
		}
		else
		if (strcasecmp(pszCmd, "CHAT") == 0)
		{
			char *pszChat;
			NICKENTRY *pChat=NULL;

			// A $ sign is in the name of a dialog, otherwise it's a groupchat ID
			if (!(pszChat = strtok(NULL, " ")) ||
				(!strchr(pszChat, '$') && !(pChat=BuddyList_Find(pInst->hBuddyList, pszChat))))
			{
				Send (pInst, "ERROR 14 Invalid message id");
				return;
			}
			

			if (!(pszCmd = strtok(NULL, " ")))
			{
				Send (pInst, "ERROR 10 Invalid property");
				return;
			}

			if (strcasecmp(pszCmd, "NAME") == 0)
				Send (pInst, "CHAT %s NAME %s", pszChat, pszChat);
			else if (strcasecmp(pszCmd, "STATUS") == 0)
				// A $ sign is in the name of a dialog, otherwise it's a groupchat ID
				Send (pInst, "CHAT %s STATUS %s", pszChat, pChat?"MULTI_SUBSCRIBED":"LEGACY_DIALOG");
			else if (strcasecmp(pszCmd, "ADDER") == 0)
				Send (pInst, "CHAT %s ADDER %s", pszChat, pInst->myUser.pszUser);
			else if (strcasecmp(pszCmd, "TYPE") == 0)
				Send (pInst, "CHAT %s TYPE %s", pszChat, pChat?"MULTICHAT":"DIALOG");
			else if (strcasecmp(pszCmd, "MYROLE") == 0)
				Send (pInst, "CHAT %s MYROLE USER", pszChat);
			else if (strcasecmp(pszCmd, "MYSTATUS") == 0)
				Send (pInst, "CHAT %s MYSTATUS SUBSCRIBED", pszChat);
			else if ((strcasecmp(pszCmd, "TOPIC") == 0 || strcasecmp(pszCmd, "FRIENDLYNAME") == 0) && pChat)
				Send (pInst, "CHAT %s %s %s", pszChat, pszCmd, pChat->pszDisplay);
			else if ((strcasecmp(pszCmd, "ACTIVEMEMBERS") == 0 || strcasecmp(pszCmd, "MEMBERS") == 0) && pChat)
			{
				char szMembers[1024]={0};
				int i, nCount, iOffs=0;

				iOffs = sprintf (szMembers, "CHAT %s %s ", pszChat, pszCmd);
				for (i=0, nCount=List_Count(pChat->hGCMembers); i<nCount; i++)
				{
					NICKENTRY *pMemb = (NICKENTRY*)List_ElementAt (pChat->hGCMembers, i);
					char szUser[512], *pTok;

					if (i) iOffs+=sprintf(&szMembers[iOffs], " ");
					strcpy (szUser, pMemb->pszUser);
					pTok=strtok(szUser, ";");
					pTok=strtok(NULL, ";");
					iOffs+=sprintf(&szMembers[iOffs], "%s", pTok);
					if (iOffs+6>=sizeof(szMembers)) break;
				}
				Send (pInst, szMembers);
			}
			else
				Send(pInst, "ERROR 7 Invalid property / not implemented");
			return;
		}
		else
		if (strcasecmp(pszCmd, "CALL") == 0)
		{
			CALLENTRY *pEntry;

			if (!(pszCmd = strtok(NULL, " ")) || !(pEntry = CallQueue_Find(pInst->hCallQueue, atol(pszCmd))))
			{
				Send (pInst, "ERROR 11 Invalid call id");
				return;
			}
			if (!(pszCmd = strtok(NULL, " ")))
			{
				Send (pInst, "ERROR 10 Invalid property");
				return;
			}
			if (!strcasecmp (pszCmd, "TIMESTAMP"))
				Send (pInst, "CALL %d TIMESTAMP %ld", pEntry->hdr.uMsgNr, pEntry->timestamp);
			else if (!strcasecmp (pszCmd, "PARTNER_HANDLE"))
				Send (pInst, "CALL %d PARTNER_HANDLE %s", pEntry->hdr.uMsgNr, pEntry->pszUser);
			else if (!strcasecmp (pszCmd, "PARTNER_DISPNAME"))
			{
				NICKENTRY *pNick = BuddyList_Find (pInst->hBuddyList, pEntry->pszUser);
				
				if (pNick)
					Send (pInst, "CALL %d PARTNER_DISPNAME %s", pEntry->hdr.uMsgNr, pNick->pszAlias);
			}
			else if (!strcasecmp (pszCmd, "CONF_ID"))
				Send (pInst, "CALL %d CONF_ID 0", pEntry->hdr.uMsgNr);
			else if (!strcasecmp (pszCmd, "TYPE"))
				Send (pInst, "CALL %d TYPE %s", pEntry->hdr.uMsgNr, pEntry->iDirection==CALL_INCOMING?"INCOMING_P2P":"OUTGOING_P2P");
			else if (!strcasecmp (pszCmd, "STATUS"))
				Send (pInst, "CALL %d STATUS %s", pEntry->hdr.uMsgNr, pEntry->szStatus);
			else if (!strcasecmp (pszCmd, "VIDEO_STATUS"))
				Send (pInst, "CALL %d VIDEO_STATUS VIDEO_NONE", pEntry->hdr.uMsgNr);
			else if (!strcasecmp (pszCmd, "VIDEO_SEND_STATUS") || !strcasecmp (pszCmd, "VIDEO_RECEIVE_STATUS"))
				Send (pInst, "CALL %d %s NOT_AVAILABLE", pEntry->hdr.uMsgNr, pszCmd);
			else if (!strcasecmp (pszCmd, "FAILUREREASON"))
				Send (pInst, "CALL %d FAILUREREASON UNKNOWN", pEntry->hdr.uMsgNr);
			else if (!strcasecmp (pszCmd, "DURATION"))
				Send (pInst, "CALL %d DURATION 0", pEntry->hdr.uMsgNr);
			else if (!strcasecmp (pszCmd, "CONF_PARTICIPANTS_COUNT"))
				Send (pInst, "CALL %d CONF_PARTICIPANTS_COUNT 0", pEntry->hdr.uMsgNr);
			else
				Send (pInst, "ERROR 10 Invalid property / not implemented");
			return;					
		}
		else
		if (strcasecmp(pszCmd, "SKYPEVERSION") == 0)
		{
			Send (pInst, "SKYPEVERSION 3.8.0.188"); // Fake
		}
		else
		{
			Send(pInst, "ERROR 7 Invalid property / not implemented");
		}
		return;
	}
	else
	if (strcasecmp(pszCmd, "SET") == 0)
	{
		if (!(pszCmd = strtok(NULL, " ")))
		{
			Send (pInst, "ERROR 7 Invalid property");
			return;
		}

		if (strcasecmp(pszCmd, "USER") == 0)
		{
			char *pszUser;
			NICKENTRY *pUser = NULL;

			if (!(pszUser = strtok(NULL, " ")))
			{
				Send (pInst, "ERROR 26 Invalid user handle");
				return;
			}

			if (!(pszCmd = strtok(NULL, " ")))
			{
				Send (pInst, "ERROR 7 Invalid property");
				return;
			}

			pUser = BuddyList_Find(pInst->hBuddyList, pszUser);

			if (strcasecmp(pszCmd, "BUDDYSTATUS") == 0)
			{
				int iStatus = -1;

				if (pszCmd = strtok(NULL, " "))
					iStatus = atoi(pszCmd);

				if (!pUser && iStatus < 2)
				{
					Send (pInst, "ERROR 26 Invalid user handle");
					return;
				}
				if (iStatus == 2 || (iStatus > 2 && !pUser) || iStatus != pUser->iBuddyStatus)
				{
					switch (iStatus)
					{
					case 1:
						if (ImoSkype_DelBuddy (pInst->hInst, pUser->pszUser, 
								strcmp(pUser->szStatus, "OFFLINE")?"Skype":"Offline") == 1)
							pUser->iBuddyStatus = iStatus;
						break;
					case 2:
					case 3:
						ImoSkype_AddBuddy (pInst->hInst, pszUser);
						return;
					default:
						Send (pInst, "ERROR 518 Invalid status given for BUDDYSTATUS");
						return;
					}
				}
				Send (pInst, "USER %s BUDDYSTATUS %d", pUser->pszUser, pUser->iBuddyStatus);
				return;
			}
			else if (!pUser)
			{
				Send (pInst, "ERROR 26 Invalid user handle");
				return;
			}
			else
			{
			// ISAUTHORIZED
			Send (pInst, "ERROR 7 Not implemented");
			}
		}
		else
		if (strcasecmp(pszCmd, "USERSTATUS") == 0)
		{
			unsigned int i;

			if (!(pszCmd = strtok(NULL, " ")))
			{
				Send (pInst, "ERROR 28 Unknown userstatus");
				return;
			}

			if (strcasecmp(pInst->myUser.szStatus, pszCmd))
			{
				for (i=0; i<sizeof(m_stMap)/sizeof(m_stMap[0]); i++)
				{
					if (!strcasecmp(m_stMap[i].pszSkypeStat, pszCmd))
					{
						pInst->tSetMoodText = 0;
						if (ImoSkype_SetStatus(pInst->hInst, m_stMap[i].pszImoStat, 
							pInst->myUser.pszStatusText?pInst->myUser.pszStatusText:"")>0)
							strcpy (pInst->myUser.szStatus, pszCmd);
						Send (pInst, "USERSTATUS %s", pInst->myUser.szStatus);
						break;
					}
				}
				if (i==sizeof(m_stMap)/sizeof(m_stMap[0]))
					Send (pInst, "ERROR 28 Unknown userstatus");
			} else Send (pInst, "USERSTATUS %s", pInst->myUser.szStatus);
			return;
		}
		else
		if (strcasecmp(pszCmd, "MESSAGE") == 0 || strcasecmp(pszCmd, "CHATMESSAGE") == 0)
		{
			MSGENTRY *pEntry;
			char *pszMessage = pszCmd;

			if (!(pszCmd = strtok(NULL, " ")) || !(pEntry = MsgQueue_Find(pInst->hMsgQueue, atol(pszCmd))))
			{
				Send (pInst, "ERROR 14 Invalid message id");
				return;
			}
			if (!(pszCmd = strtok(NULL, " ")))
			{
				Send (pInst, "ERROR 10 Invalid property");
				return;
			}
			if (!strcasecmp (pszCmd, "SEEN"))
			{
				strcpy (pEntry->szStatus, "READ");
				Send (pInst, "%s %d STATUS %s", pszMessage, pEntry->hdr.uMsgNr, pEntry->szStatus);
			}
			else
				Send (pInst, "ERROR 10 Invalid property / not implemented");
			return;
		}
		else
		if (strcasecmp(pszCmd, "CALL") == 0)
		{
			CALLENTRY *pEntry;

			if (!(pszCmd = strtok(NULL, " ")) || !(pEntry = CallQueue_Find(pInst->hCallQueue, atol(pszCmd))))
			{
				Send (pInst, "ERROR 11 Invalid call id");
				return;
			}
			if (!(pszCmd = strtok(NULL, " ")))
			{
				Send (pInst, "ERROR 10 Invalid property");
				return;
			}
			if (!strcasecmp (pszCmd, "STATUS"))
			{
				if (!(pszCmd = strtok(NULL, " ")))
				{
					Send (pInst, "ERROR 21 Unknown/disallowed call prop");
					return;
				}
				strcpy (pEntry->szStatus, pszCmd);
				Send (pInst, "CALL %d STATUS %s", pEntry->hdr.uMsgNr, pEntry->szStatus);
				
				// {RINGING, INPROGRESS, ONHOLD, FINISHED}
				if (pInst->iFlags & IMO2S_FLAG_ALLOWINTERACT)
				{
					if (strcasecmp (pEntry->szStatus, "INPROGRESS") == 0 && pEntry->iDirection == CALL_INCOMING)
					{
						StartCallSWF (pInst, pEntry);
					}
					else
					if (strcasecmp (pEntry->szStatus, "FINISHED") == 0 && pEntry->hCallWnd)
					{
						if (*pEntry->szCallFile)
						{
							unlink(pEntry->szCallFile);
							*pEntry->szCallFile=0;
						}
#ifdef WIN32
						if (pEntry->hCallWnd)
						{
							W32Browser_CloseWindow (pEntry->hCallWnd);
							pEntry->hCallWnd = NULL;
						}
#endif

						// On incoming call, hang up
					}
				}

				// Currently we don't support calls, just hang up
				/* FIXME: Hangup! */
				return;
			}
			return;
		}
		else
		if (strcasecmp(pszCmd, "PROFILE") == 0)
		{
			if (!(pszCmd = strtok(NULL, " ")))
			{
				Send (pInst, "ERROR 10 Invalid property");
				return;
			}

			if (!strcasecmp (pszCmd, "MOOD_TEXT"))
			{
				if (!pInst->myUser.pszStatusText || 
					strcasecmp(pInst->myUser.pszStatusText, pszCmd+10))
				{
					time_t t;
					// Delay setting of MOOD_TEXT, as next command in chain may change online status
					// and we want to prevent double calls
					// The polling-thread will take care of this event
					pInst->tSetMoodText = time(&t)+15;
					if (pInst->myUser.pszStatusText) free (pInst->myUser.pszStatusText);
					pInst->myUser.pszStatusText = strdup (pszCmd+10);
				}
			}
			else
			{
				Send (pInst, "ERROR 552 Invalid property");
				return;
			}
		}
		else
		{
			Send (pInst, "ERROR 7 Invalid property");
			return;
		}
	}
	else
	if (strcasecmp(pszCmd, "MESSAGE") == 0 || strcasecmp(pszCmd, "CHATMESSAGE") == 0)
	{
		NICKENTRY *pUser;
		unsigned int uMsgId;
		MSGENTRY *pMsg;

		if (!(pszCmd = strtok(NULL, " ")) || !(pUser = BuddyList_Find(pInst->hBuddyList, pszCmd)))
		{
			Send (pInst, "ERROR 26 Invalid user handle");
			return;
		}
		pszCmd+=strlen(pszCmd)+1;
		if (!*pszCmd)
		{
			Send (pInst, "ERROR 43 Cannot send empty message");
			return;
		}
		if (!(pMsg = MsgQueue_AddSent (pInst->hMsgQueue, pUser->pszUser, pUser->pszAlias, pszCmd, &uMsgId)))
		{
			Send (pInst, "ERROR 9901 Internal error");
			return;
		}
		if (IMO_API_VERSION == 0) Send (pInst, "MESSAGE %d STATUS SENDING", uMsgId);
		if (ImoSkype_SendMessage(pInst->hInst, pUser->pszUser, pszCmd, &pMsg->uRqId)>0)
		{
			if (IMO_API_VERSION == 0) strcpy (pMsg->szStatus, "SENT");
		}
		else
		{
			strcpy (pMsg->szStatus, "FAILED");
			strncpy (pMsg->szFailure, ImoSkype_GetLastError(pInst->hInst), sizeof(pMsg->szFailure));
		}
		Send (pInst, "MESSAGE %d STATUS %s", uMsgId, pMsg->szStatus);
	}
	else
	if (strcasecmp(pszCmd, "CHAT") == 0)
	{
		if (!(pszCmd = strtok(NULL, " ")))
		{
			Send (pInst, "ERROR 536 CREATE: no object or type given");
			return;
		}
		if (strcasecmp(pszCmd, "CREATE") == 0)
		{
			char szName[128];	// No name given here, so make up a name
			time_t t;

			time(&t);
			sprintf (szName, "%d", t);
			if (ImoSkype_CreateSharedGroup(pInst->hInst, szName)<1) return;

			// Now buddy_added should be called which gives us the ID of the new groupchat 
			// where we can then add members to. However this is not known here yet, so feed 
			// list with users to add to instance and let it be processed by buddy_added handler
			if (pszCmd = strtok(NULL, "\n"))
			{
				if (pInst->pszBuddiesToAdd) free(pInst->pszBuddiesToAdd);
				pInst->pszBuddiesToAdd = strdup(pszCmd);
			}
		}
		else
			Send (pInst, "ERROR 503 CHAT: Invalid or unknown action");
		return;
	}
	else
	if (strcasecmp(pszCmd, "CALL") == 0)
	{
		NICKENTRY *pUser;

		if (!(pszCmd = strtok(NULL, " ")) || !(pUser = BuddyList_Find(pInst->hBuddyList, pszCmd)))
		{
			Send (pInst, "ERROR 26 Invalid user handle");
			return;
		}
	
		ImoSkype_StartVoiceCall (pInst->hInst, pUser->pszUser);
		return;
	}
	else
	if (strcasecmp(pszCmd, "OPEN") == 0)
	{
		return;
	}
	else
	if (strcasecmp(pszCmd, "CREATE") == 0)
	{
		if (!(pszCmd = strtok(NULL, " ")))
		{
			Send (pInst, "ERROR 536 CREATE: no object or type given");
			return;
		}
		if (strcasecmp(pszCmd, "APPLICATION") == 0)
		{
			if (!(pszCmd = strtok(NULL, " ")) || strcasecmp(pszCmd, "libpurple_typing"))
				Send (pInst, "ERROR 540 CREATE APPLICATION: Missing or invalid name");
			else
				Send (pInst, "CREATE APPLICATION libpurple_typing");		
		}
		else
			Send (pInst, "ERROR 537 CREATE: Unknown object type given");
		return;
	}
	else
	if (strcasecmp(pszCmd, "DELETE") == 0)
	{
		if (!(pszCmd = strtok(NULL, " ")))
		{
			Send (pInst, "ERROR 538 DELETE: no object or type given");
			return;
		}
		if (strcasecmp(pszCmd, "APPLICATION") == 0)
		{
			if (!(pszCmd = strtok(NULL, " ")) || strcasecmp(pszCmd, "libpurple_typing"))
				Send (pInst, "ERROR 542 DELETE APPLICATION: Missing or invalid application name");
			else
				Send (pInst, "DELETE APPLICATION libpurple_typing");		
		}
		else
			Send (pInst, "ERROR 539 DELETE: Unknown object type given");
		return;
	}
	else
	if (strcasecmp(pszCmd, "ALTER") == 0)
	{
		if (!(pszCmd = strtok(NULL, " ")))
			Send (pInst, "ERROR 526 ALTER: no object type given");
		else
		{
			if (strcasecmp(pszCmd, "APPLICATION") == 0)
			{
				if (!(pszCmd = strtok(NULL, " ")) || strcasecmp(pszCmd, "libpurple_typing") ||
					!(pszCmd = strtok(NULL, " ")))
					Send (pInst, "ERROR 545 ALTER: missing or invalid action");
				else
				{
					NICKENTRY *pUser;

					if (strcasecmp (pszCmd, "CONNECT") == 0)
					{
						if (!(pszCmd = strtok(NULL, " ")) || !(pUser = BuddyList_Find(pInst->hBuddyList, pszCmd)))
							Send (pInst, "ERROR 547 ALTER APPLICATION CONNECT: Invalid user handle");
						else
						{
							Send (pInst, "ALTER APPLICATION libpurple_typing CONNECT %s", pszCmd);
							Send (pInst, "APPLICATION CONNECTING %s", pszCmd);
							Send (pInst, "APPLICATION libpurple_typing STREAMS %s:1", pszCmd);
							// FIXME: Shouldn't we enumerate all STREAMS here? dunno...
						}
					} else
					if (strcasecmp (pszCmd, "DATAGRAM") == 0)
					{
						char *pSep;

						if (!(pszCmd = strtok(NULL, " ")) || !(pSep = strchr(pszCmd, ':')))
							Send (pInst, "ERROR 551 ALTER APPLICATION DATAGRAM: Missing or invalid stream identifier");
						else
						{
							*pSep=0;
							if (!(pUser = BuddyList_Find(pInst->hBuddyList, pszCmd)))
								Send (pInst, "ERROR 551 ALTER APPLICATION DATAGRAM: Missing or invalid stream identifier");
							else
							{
								*pSep=':';
								if (!(pszCmd = strtok(NULL, " ")))
									Send (pInst, "ERROR 541 APPLICATION: Operation failed");
								else
								{
									if (!strcmp (pszCmd, "PURPLE_TYPING"))
										ImoSkype_Typing (pInst->hInst, pUser->pszUser, "typing");
									else if (!strcmp (pszCmd, "PURPLE_TYPED"))
										ImoSkype_Typing (pInst->hInst, pUser->pszUser, "typed");
									else if (!strcmp (pszCmd, "PURPLE_NOT_TYPING"))
										ImoSkype_Typing (pInst->hInst, pUser->pszUser, "not_typing");
								}
							}
						}
					}
				}

			}
			else 
			if (strcasecmp(pszCmd, "CHAT") == 0)
			{
				char *pszChat;
				NICKENTRY *pChat;

				if (!(pszChat = strtok(NULL, " ")) ||
					!(pChat=BuddyList_Find(pInst->hBuddyList, pszChat)))
				{
					Send (pInst, "ERROR 14 Invalid message id");
					return;
				}
				

				if (!(pszCmd = strtok(NULL, " ")))
				{
					Send (pInst, "ERROR 503 CHAT: Invalid or unknown action");
					return;
				}

				if (strcasecmp (pszCmd, "ADDMEMBERS") == 0)
				{
					while(pszCmd = strtok(NULL, ", "))
					{
						ImoSkype_GroupInvite (pInst->hInst, pChat->pszUser, pszCmd);
					}
				}
				else if (strcasecmp (pszCmd, "KICK") == 0)
				{
					while(pszCmd = strtok(NULL, ", "))
					{
						ImoSkype_GroupKick (pInst->hInst, pChat->pszUser, pszCmd);
					}
				}
				else if (strcasecmp (pszCmd, "SETTOPIC") == 0)
				{
					if(pszCmd = strtok(NULL, ""))
					{
						ImoSkype_GroupTopic (pInst->hInst, pChat->pszUser, pszCmd);
					}
				}
				else if (strcasecmp (pszCmd, "LEAVE") == 0)
				{
					if(pszCmd = strtok(NULL, ""))
					{
						ImoSkype_GroupLeave (pInst->hInst, pChat->pszUser);
					}
				}
				else Send (pInst, "ERROR 503 CHAT: Invalid or unknown action");
			} else
				Send (pInst, "ERROR 527 ALTER: unknown object type given");
		}
		return;
	}
	else
	if (strcasecmp(pszCmd, "NAME") == 0)
	{
		if (pszCmd = strtok(NULL, " "))
		{
			if (pInst->pszClientName) free(pInst->pszClientName);
			pInst->pszClientName = strdup(pszCmd);
		}
		Send (pInst, "OK");
		return;
	}
	else
	{
		Send (pInst, "ERROR 2 Not Implemented");
	}
}

// -----------------------------------------------------------------------------

