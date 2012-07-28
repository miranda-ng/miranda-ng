/* Module:  imo_skype.c
   Purpose: Communication layer for imo.im Skype 
   Author:  leecher
   Date:    30.08.2009
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "imo_request.h"
#include "imo_skype.h"

#ifdef _WIN64
#pragma comment (lib, "bufferoverflowU.lib")
#endif

#define PROTO "prpl-skype"

struct _tagIMOSKYPE
{
	IMORQ *hRq;
	IMORQ *hPoll;
	char *pszUser;
	IMOSTATCB StatusCb;
	char *pszLastRes;
	void *pUser;
	int request_id;
};

static int CheckReturn (IMOSKYPE *hSkype, char *pszMsg, char *pszExpected);
static int ManageBuddy(IMOSKYPE *hSkype, char *pszAction, char *pszBuddy, char *pszGroup);

// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------

IMOSKYPE *ImoSkype_Init(IMOSTATCB StatusCb, void *pUser)
{
	IMOSKYPE *hSkype = calloc(1, sizeof(IMOSKYPE));

	if (!hSkype) return NULL;
	if (!(hSkype->hRq = ImoRq_Init()) || !(hSkype->hPoll = ImoRq_Clone(hSkype->hRq)))
	{
		ImoSkype_Exit(hSkype);
		return NULL;
	}
	hSkype->StatusCb = StatusCb;
	hSkype->pUser = pUser;
	return hSkype;
}

// -----------------------------------------------------------------------------

void ImoSkype_Exit(IMOSKYPE *hSkype)
{
	if (!hSkype) return;
	if (hSkype->hRq) ImoRq_Exit(hSkype->hRq);
	if (hSkype->hPoll) ImoRq_Exit(hSkype->hPoll);
	if (hSkype->pszUser) free(hSkype->pszUser);
	free (hSkype);
}

// -----------------------------------------------------------------------------

void ImoSkype_CancelPolling(IMOSKYPE *hSkype)
{
	if (hSkype->hPoll)
		ImoRq_Cancel(hSkype->hPoll);
}

// -----------------------------------------------------------------------------

char *ImoSkype_GetLastError(IMOSKYPE *hSkype)
{
	char *pszRet = ImoRq_GetLastError(hSkype->hRq);

	if (!pszRet || !*pszRet) return hSkype->pszLastRes;
	return pszRet;
}

// -----------------------------------------------------------------------------

char *ImoSkype_GetUserHandle(IMOSKYPE *hSkype)
{
	return hSkype->pszUser;
}

// -----------------------------------------------------------------------------

// -1	-	Error
// 0 	-	Login failed
// 1	-	Login successful
int ImoSkype_Login(IMOSKYPE *hSkype, char *pszUser, char *pszPass)
{
	cJSON *root;
	char *pszRet;
	int iRet = -1;

	if (!hSkype) return 0;
	if (IMO_API_VERSION == 0)
	{
		if (!(root=cJSON_CreateObject())) return 0;
		cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
		if (pszRet = ImoRq_PostToSys(hSkype->hRq, "cookie_login", "session", root, 1, NULL))
			iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	}

	if (!(root=cJSON_CreateObject())) return 0;
	if (hSkype->pszUser) free (hSkype->pszUser);
	cJSON_AddStringToObject(root, "uid", hSkype->pszUser = strdup(pszUser));
	cJSON_AddStringToObject(root, "proto", PROTO);
	cJSON_AddStringToObject(root, "passwd", pszPass);
	cJSON_AddNullToObject(root, "captcha");	// Uh-oh, thay may get annoying in the future! :(
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	if (pszRet = ImoRq_PostAmy(hSkype->hRq, "account_login", root))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	cJSON_Delete(root);

	return iRet;
}

// -----------------------------------------------------------------------------

// -1	-	Error
// 0 	-	Logout failed
// 1	-	Logout successful
int ImoSkype_Logout(IMOSKYPE *hSkype)
{
	cJSON *root;
	char *pszRet;
	int iRet = -1;

	if (!hSkype  || !(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	if (pszRet = ImoRq_PostToSys(hSkype->hRq, "signoff_all", "session", root, 1, NULL))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	return iRet;
}

// -----------------------------------------------------------------------------

// -1	-	Error
// 0	-	Received unknown answer
// 1	-	Got back information, called notification callback
// 2	-	Received PING [deprecated]
int ImoSkype_Poll(IMOSKYPE *hSkype)
{
	char *pszRet;

	if (!hSkype || !hSkype->hPoll) return 0;
	pszRet = ImoRq_PostSystem(hSkype->hPoll, "forward_to_server", NULL, NULL, NULL, 1);
	if (!pszRet) return -1;
	return CheckReturn (hSkype, pszRet, "ping");
}

// -----------------------------------------------------------------------------

int ImoSkype_KeepAlive(IMOSKYPE *hSkype)
{
	char *pszRet;

	/* In case we want to receive Promo-Infos...
	{
		cJSON *edata = cJSON_CreateObject(), *root;

		root=cJSON_CreateObject();
		cJSON_AddStringToObject(edata, "kind", "web");
		cJSON_AddNumberToObject(edata, "quantity", 1);
		cJSON_AddItemToObject(root, "edata", edata);
		if (pszRet = ImoRq_PostToSys(hSkype->hRq, "get_promos", "promo", root, 1))
			CheckReturn(hSkype, pszRet, "ok");
	}
	*/

	if (!hSkype) return 0;
	pszRet = ImoRq_UserActivity(hSkype->hPoll);
	if (!pszRet) return -1;
	return CheckReturn (hSkype, pszRet, "ok");
}

// -----------------------------------------------------------------------------

// pszStatus:
// Valid states:
//	typing
//	typed
//	not_typing
//
// -1	-	Error
// 0 	-	Typing notification failed
// 1	-	Typing notification  successful
int ImoSkype_Typing(IMOSKYPE *hSkype, char *pszBuddy, char *pszStatus)
{
	cJSON *root;
	char *pszRet;
	int iRet = -1;

	if (!hSkype || !hSkype->pszUser || !(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "uid", hSkype->pszUser);
	cJSON_AddStringToObject(root, "proto", PROTO);
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	cJSON_AddStringToObject(root, "buid", pszBuddy);
	cJSON_AddStringToObject(root, "typing_state", pszStatus);
	if (pszRet = ImoRq_PostAmy(hSkype->hRq, "im_typing", root))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	cJSON_Delete(root);
	return iRet;
}

// -----------------------------------------------------------------------------

// -1	-	Error
// 0 	-	Sending failed
// 1	-	Send pending
int ImoSkype_SendMessage(IMOSKYPE *hSkype, char *pszBuddy, char *pszMessage, int *prequest_id)
{
	cJSON *root;
	char *pszRet;
	int iRet = -1;

	if (!hSkype || !hSkype->pszUser || !(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	cJSON_AddStringToObject(root, "uid", hSkype->pszUser);
	cJSON_AddStringToObject(root, "proto", PROTO);
	cJSON_AddStringToObject(root, "buid", pszBuddy);
	cJSON_AddStringToObject(root, "msg", pszMessage);
	if (pszRet = (IMO_API_VERSION==0?ImoRq_PostAmy(hSkype->hRq, "send_im", root):ImoRq_PostToSys (hSkype->hRq, "send_im", "im", root, 0, &hSkype->request_id)))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	if (prequest_id) *prequest_id = hSkype->request_id;
	hSkype->request_id++;
	cJSON_Delete(root);
	return iRet;
}

// -----------------------------------------------------------------------------

// pszStatus:
// Valid states:
//	available
//	away
//	busy
//	invisible
//
// -1	-	Error
// 0 	-	Failed
// 1	-	OK
int ImoSkype_SetStatus(IMOSKYPE *hSkype, char *pszStatus, char *pszStatusMsg)
{
	cJSON *root;
	char *pszRet;
	int iRet = -1;

	/*
	if (!hSkype->pszUser || !(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "uid", hSkype->pszUser);
	cJSON_AddStringToObject(root, "proto", PROTO);
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	cJSON_AddStringToObject(root, "ad", "");
	cJSON_AddStringToObject(root, "primitive", pszStatus);
	cJSON_AddStringToObject(root, "status", pszStatusMsg);
	if (pszRet = ImoRq_PostAmy(hSkype->hRq, "set_status", root))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	*/

	if (!hSkype || !hSkype->pszUser || !(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	cJSON_AddStringToObject(root, "ad", "");
	cJSON_AddStringToObject(root, "primitive", pszStatus);
	cJSON_AddStringToObject(root, "status", pszStatusMsg);
	cJSON_AddFalseToObject (root, "auto_away");
	if (pszRet = ImoRq_PostToSys(hSkype->hRq, "set_status", "session", root, 0, NULL))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;

	cJSON_Delete(root);
	return iRet;
}

// -----------------------------------------------------------------------------

// -1	-	Error
// 0 	-	Failed
// 1	-	OK
int ImoSkype_AddBuddy(IMOSKYPE *hSkype, char *pszBuddy)
{
	return ManageBuddy (hSkype, "add_buddy", pszBuddy, "Offline");
}

// -----------------------------------------------------------------------------

// pszGroup = "Offline" if the user if offline, otherwise "Skype" or "Buddies"
// -1	-	Error
// 0 	-	Failed
// 1	-	OK
int ImoSkype_DelBuddy(IMOSKYPE *hSkype, char *pszBuddy, char *pszGroup)
{
	int iRet = ManageBuddy (hSkype, "del_buddy", pszBuddy, pszGroup);

	if (iRet<1 && strcmp(pszGroup, "Skype")==0)
		return ManageBuddy (hSkype, "del_buddy", pszBuddy, "Buddies");
	return iRet;
}
// -----------------------------------------------------------------------------

int ImoSkype_BlockBuddy(IMOSKYPE *hSkype, char *pszBuddy)
{
	return ManageBuddy (hSkype, "block_buddy", pszBuddy, NULL);
}

// -----------------------------------------------------------------------------

int ImoSkype_UnblockBuddy(IMOSKYPE *hSkype, char *pszBuddy)
{
	return ManageBuddy (hSkype, "unblock_buddy", pszBuddy, NULL);
}

// -----------------------------------------------------------------------------

int ImoSkype_ChangeAlias(IMOSKYPE *hSkype, char *pszBuddy, char *pszNewAlias)
{
	cJSON *root;
	char *pszRet;
	int iRet = -1;

	if (!hSkype->pszUser || !(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	cJSON_AddStringToObject(root, "uid", hSkype->pszUser);
	cJSON_AddStringToObject(root, "proto", PROTO);
	cJSON_AddStringToObject(root, "buid", pszBuddy);
	cJSON_AddStringToObject(root, "alias", pszNewAlias);
	if (pszRet = ImoRq_PostAmy(hSkype->hRq, "change_buddy_alias", root))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	cJSON_Delete(root);
	return iRet;
}

// -----------------------------------------------------------------------------

int ImoSkype_StartVoiceCall(IMOSKYPE *hSkype, char *pszBuddy)
{
	cJSON *root;
	char *pszRet;
	int iRet = -1;

	if (!hSkype->pszUser || !(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	cJSON_AddStringToObject(root, "uid", hSkype->pszUser);
	cJSON_AddStringToObject(root, "proto", PROTO);
	cJSON_AddStringToObject(root, "buid", pszBuddy);
	if (pszRet = ImoRq_PostToSys (hSkype->hRq, "start_audio_chat", "av", root, 1, NULL))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	return iRet;
}

// -----------------------------------------------------------------------------

int ImoSkype_Ping(IMOSKYPE *hSkype)
{
	char *pszRet;
	int iRet = -1;

	if (pszRet = ImoRq_Echo(hSkype->hRq))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	return iRet;
}

// -----------------------------------------------------------------------------

char *ImoSkype_GetAvatar(IMOSKYPE *hSkype, char *pszID, unsigned int *pdwLength)
{
	char szURL[256];

	sprintf (szURL, "%sb/%s", ImoRq_GetHost(hSkype->hRq), pszID);
	return ImoRq_HTTPGet (hSkype->hRq, szURL, pdwLength);
}

// -----------------------------------------------------------------------------

int ImoSkype_GetUnreadMsgs(IMOSKYPE *hSkype)
{
	cJSON *root;
	char *pszRet;
	int iRet = -1;

	if (!hSkype->pszUser || !(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	cJSON_AddStringToObject(root, "uid", hSkype->pszUser);
	cJSON_AddStringToObject(root, "proto", PROTO);
	if (pszRet = ImoRq_PostAmy(hSkype->hRq, "get_unread_msgs", root))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	cJSON_Delete(root);
	return iRet;
}

// -----------------------------------------------------------------------------

int ImoSkype_GetAlpha(IMOSKYPE *hSkype)
{
	cJSON *root;
	char *pszRet;
	int iRet = -1;

	if (!hSkype->pszUser || !(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	cJSON_AddStringToObject(root, "uid", hSkype->pszUser);
	cJSON_AddStringToObject(root, "proto", PROTO);
	if (pszRet = ImoRq_PostToSys (hSkype->hRq, "get_alpha_for_user", "alpha", root, 1, NULL))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	return iRet;
}

// -----------------------------------------------------------------------------


int ImoSkype_CreateSharedGroup(IMOSKYPE *hSkype, char *pszName)
{
	cJSON *root;
	char *pszRet;
	int iRet = -1;

	if (!hSkype->pszUser || !(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	cJSON_AddStringToObject(root, "uid", hSkype->pszUser);
	cJSON_AddStringToObject(root, "proto", PROTO);
	cJSON_AddFalseToObject(root, "is_native");	// TRUE would be great, but not yet supported?
	cJSON_AddStringToObject(root, "name", pszName);
	if (pszRet = ImoRq_PostAmy(hSkype->hRq, "create_shared_group", root))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	cJSON_Delete(root);
	return iRet;
}

// -----------------------------------------------------------------------------

int ImoSkype_GroupInvite(IMOSKYPE *hSkype, char *pszGroup, char *pszUser)
{
	cJSON *root;
	char *pszRet, *p, *pszGroupDup = strdup(pszGroup);
	int iRet = -1;

	if (p=strrchr(pszGroupDup, ';')) *p=0;
	if (!hSkype->pszUser || !(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	cJSON_AddStringToObject(root, "uid", hSkype->pszUser);
	cJSON_AddStringToObject(root, "proto", PROTO);
	cJSON_AddStringToObject(root, "gid", pszGroupDup);
	cJSON_AddStringToObject(root, "iproto", PROTO);
	cJSON_AddStringToObject(root, "iuid", hSkype->pszUser);
	cJSON_AddStringToObject(root, "ibuid", pszUser);
	if (pszRet = ImoRq_PostAmy(hSkype->hRq, "invite_to_group", root))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	free (pszGroupDup);
	cJSON_Delete(root);
	return iRet;
}

// -----------------------------------------------------------------------------

int ImoSkype_GroupKick(IMOSKYPE *hSkype, char *pszGroup, char *pszUser)
{
	cJSON *root;
	char *pszRet, *p, *pszGroupDup = strdup(pszGroup), szBUID[256];
	int iRet = -1;

	if (p=strrchr(pszGroupDup, ';')) *p=0;
	sprintf (szBUID, "%s;%s;"PROTO, pszGroupDup, pszUser);
	if (!hSkype->pszUser || !(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	cJSON_AddStringToObject(root, "uid", hSkype->pszUser);
	cJSON_AddStringToObject(root, "proto", PROTO);
	cJSON_AddStringToObject(root, "buid", szBUID);
	if (pszRet = ImoRq_PostAmy(hSkype->hRq, "kick_member", root))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	free (pszGroupDup);
	cJSON_Delete(root);
	return iRet;
}

// -----------------------------------------------------------------------------

int ImoSkype_GroupTopic(IMOSKYPE *hSkype, char *pszGroup, char *pszTopic)
{
	cJSON *root;
	char *pszRet, *p, *pszGroupDup = strdup(pszGroup);
	int iRet = -1;

	if (p=strrchr(pszGroupDup, ';')) *p=0;
	if (!hSkype->pszUser || !(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	cJSON_AddStringToObject(root, "uid", hSkype->pszUser);
	cJSON_AddStringToObject(root, "proto", PROTO);
	cJSON_AddStringToObject(root, "gid", pszGroupDup);
	cJSON_AddStringToObject(root, "topic", pszTopic);
	if (pszRet = ImoRq_PostAmy(hSkype->hRq, "set_group_topic", root))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	free (pszGroupDup);
	cJSON_Delete(root);
	return iRet;
}

// -----------------------------------------------------------------------------

int ImoSkype_GroupLeave(IMOSKYPE *hSkype, char *pszGroup)
{
	cJSON *root;
	char *pszRet, *p, *pszGroupDup = strdup(pszGroup);
	int iRet = -1;

	if (p=strrchr(pszGroupDup, ';')) *p=0;
	if (!hSkype->pszUser || !(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	cJSON_AddStringToObject(root, "uid", hSkype->pszUser);
	cJSON_AddStringToObject(root, "proto", PROTO);
	cJSON_AddStringToObject(root, "gid", pszGroupDup);
	if (pszRet = ImoRq_PostAmy(hSkype->hRq, "leave_group", root))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	free (pszGroupDup);
	cJSON_Delete(root);
	return iRet;
}

// -----------------------------------------------------------------------------
// Static
// -----------------------------------------------------------------------------
static void PostDisconnect(IMOSKYPE *hSkype)
{
	cJSON *arr, *root;

	if (arr = cJSON_CreateArray())
	{
		if (!(root=cJSON_CreateObject())) return;
		cJSON_AddStringToObject(root, "name", "disconnect");
		cJSON_AddItemToArray (arr, root);
		hSkype->StatusCb(arr, hSkype->pUser);
		cJSON_Delete(arr);
	}
}

// 0	-	Unexpected answer
// 1	-	Got back JSON data, notified callback
// 2	-	Received expected message pszExpected [deprecated]
static int CheckReturn (IMOSKYPE *hSkype, char *pszMsg, char *pszExpected)
{
	cJSON *root, *data, *msgs, *msg, *sys, *arr, *prefs, *pref;
	char *pszMethod, *pszSys;

	hSkype->pszLastRes = pszMsg;
	if (root = cJSON_Parse(pszMsg))
	{
		// Now let's see if this one is interesting for our system
		if ((data = cJSON_GetObjectItem(root,"method")) &&
			(pszMethod = data->valuestring) &&
			strcmp(pszMethod, "forward_to_client") == 0 &&
			(data = cJSON_GetObjectItem(root,"data")) &&
			(msgs = cJSON_GetObjectItem(data,"messages")))
		{
			int i, iCount = cJSON_GetArraySize(msgs);

			if (!iCount && pszExpected && strcmp(pszExpected, "ok") == 0) 
			{
				if ((sys = cJSON_GetObjectItem(data,"ack")) && (unsigned long)sys->valueint >= ImoRq_GetSeq(hSkype->hRq))
					return 2;	// imoim ACKnowledged this
				else
				{
					if (sys && sys->valueint==0)
					{
						// ACK error, reset to 0, better reset the connection
						PostDisconnect(hSkype);
					}
					return 0;	// No ACK, sequence number not incremented :(
				}
			}
			for (i=0; i<iCount; i++)
			{
				if (msg = cJSON_GetArrayItem(msgs, i))
				{
					// Is this for me?
					if ((sys = cJSON_GetObjectItem(msg,"to")) &&
						(pszSys = cJSON_GetObjectItem(sys, "system")->valuestring) &&
						strcmp (pszSys, "client") == 0)
					{
						if (sys = cJSON_GetObjectItem(msg,"seq"))
						{
							ImoRq_UpdateAck(hSkype->hRq, sys->valueint+1);
							ImoRq_UpdateAck(hSkype->hPoll, sys->valueint+1);
						}

						// Callback is only called for system IM
						if ((sys = cJSON_GetObjectItem(msg,"from")) &&
						(pszSys = cJSON_GetObjectItem(sys, "system")->valuestring))
						{
							if ((strcmp (pszSys, "im") == 0 || strcmp (pszSys, "av") == 0) &&
							(data = cJSON_GetObjectItem(msg,"data")) &&
							(arr = cJSON_CreateArray()))
							{
								// Pack data into array for Callback backwards
								// compatibility
								cJSON *next;

								next = data->next;
								data->next = NULL;
								cJSON_AddItemToArray (arr, data);
								hSkype->StatusCb(arr, hSkype->pUser);
								data->next = next;
								free(arr);
							}
							else if (strcmp (pszSys, "internal") == 0 &&
							(data = cJSON_GetObjectItem(msg,"data")) &&
							(arr = cJSON_CreateArray()))
							{
								// Pack ACK msgs into a fake "ack" method so that callback 
								// function can dispatch them without interface change
								cJSON *next;

								cJSON_AddStringToObject (data, "name", "ack");
								next = data->next;
								data->next = NULL;
								cJSON_AddItemToArray (arr, data);
								hSkype->StatusCb(arr, hSkype->pUser);
								data->next = next;
								free(arr);
							}
							// Ensure to disable annoying autoaway
							else if (strcmp (pszSys, "preference") == 0 && 
								(data = cJSON_GetObjectItem(msg,"data")) &&
								(prefs = cJSON_GetObjectItem(data, "preferences")))
							{
								int j, nPrefs = cJSON_GetArraySize(prefs);
								cJSON *kind, *thispref, *value, *prefdata;

								for (j=0; j<nPrefs; j++)
								{
									if ((pref = cJSON_GetArrayItem(prefs, j)) && 
										(kind = cJSON_GetObjectItem(pref, "kind")) &&
										strcmp (kind->valuestring, "impref") == 0 &&
										(thispref = cJSON_GetObjectItem(pref, "pref")) &&
										strcmp (thispref->valuestring, "auto_away") == 0 &&
										(value = cJSON_GetObjectItem(pref, "value")) &&
										value->type == cJSON_True && hSkype->pszUser &&
										(prefdata = cJSON_CreateObject()))
									{
										cJSON *accs, *acc, *setprefs, *newpref;

										if (accs = cJSON_CreateArray())
										{
											if (acc = cJSON_CreateObject())
											{
												cJSON_AddStringToObject(acc, "uid", hSkype->pszUser);
												cJSON_AddStringToObject(acc, "proto", PROTO);
												cJSON_AddItemToArray (accs, acc);
											}
											cJSON_AddItemToObject(prefdata, "accounts", accs);
										}
										if (setprefs = cJSON_CreateArray())
										{
											if (newpref = cJSON_CreateObject())
											{
												cJSON_AddStringToObject(newpref, "kind", kind->valuestring);
												cJSON_AddStringToObject(newpref, "pref", thispref->valuestring);
												cJSON_AddFalseToObject (newpref, "value");
												cJSON_AddItemToArray (setprefs, newpref);
											}
											cJSON_AddItemToObject(prefdata, "preferences", setprefs);
										}
										ImoRq_PostToSys (hSkype->hRq, "set", "preference", prefdata, 1, NULL);
									}
								}
							}
							else if (strcmp (pszSys, "reset") == 0)
							{
								// System requested to reset connection
								// Let's issue a "disconnect" to the callback to let it handle 
								// this situation
							}
						}
					}
				}
			}
		}
		cJSON_Delete(root);
		return 1;
	}
	else
	{
		if (pszExpected && strcmp(pszMsg, pszExpected)==0)
			return 2;
	}
	return 0;
}

// -----------------------------------------------------------------------------

static int ManageBuddy(IMOSKYPE *hSkype, char *pszAction, char *pszBuddy, char *pszGroup)
{
	cJSON *root;
	char *pszRet;
	int iRet = -1;

	if (!hSkype->pszUser || !(root=cJSON_CreateObject())) return 0;
	cJSON_AddStringToObject(root, "ssid", ImoRq_SessId(hSkype->hRq));
	cJSON_AddStringToObject(root, "uid", hSkype->pszUser);
	cJSON_AddStringToObject(root, "proto", PROTO);
	cJSON_AddStringToObject(root, "buid", pszBuddy);
	if (pszGroup) cJSON_AddStringToObject(root, "group", pszGroup);
	if (pszRet = ImoRq_PostAmy(hSkype->hRq, pszAction, root))
		iRet = CheckReturn(hSkype, pszRet, "ok")>0;
	cJSON_Delete(root);
	return iRet;
}
