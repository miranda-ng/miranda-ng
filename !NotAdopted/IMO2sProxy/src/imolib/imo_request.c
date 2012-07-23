/* Module:  imo_request.c
   Purpose: Posts XMLHHTP-Requests to imo.im server
   Author:  leecher
   Date:    30.08.2009
*/
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define LockMutex(x) EnterCriticalSection (&x)
#define UnlockMutex(x) LeaveCriticalSection(&x)
#define InitMutex(x) InitializeCriticalSection(&x)
#define ExitMutex(x) DeleteCriticalSection(&x)
#define mutex_t CRITICAL_SECTION
#else
#include <pthread.h>
#define LockMutex(x) pthread_mutex_lock(&x)
#define UnlockMutex(x) pthread_mutex_unlock(&x)
#define InitMutex(x)  pthread_mutex_init(&x, NULL);
#define ExitMutex(x) 
#define mutex_t pthread_mutex_t
#endif
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fifo.h"
#include "imo_request.h"
#include "io_layer.h"

#define SSID_LENGTH 16

struct _tagIMORQ
{
	IOLAYER *hIO;
	char szSessId[SSID_LENGTH+2];
    unsigned long send_ack;   // Some Sending ACK number
    unsigned long send_seq;   // Within one ACK there seems to be a SEQ-Number?
	unsigned long *psend_ack;	// Pointer to send_ack to use
	unsigned long *psend_seq;	// Pointer to send_seq to use
	mutex_t mutex;				// Mutex for securing psend_ack and psend_seq read/write
	BOOL bIsClone;				// Indicates that the current handle is a clone
	char *pszHost;				// Login host
	char szReqURL[32];			// Request-URL
};

static IOLAYER *(*IoLayer_Init)(void) =
#ifdef WIN32
IoLayerW32_Init;
#else
IoLayerCURL_Init;
#endif

// Forward declaration of private functions
static size_t add_data(char *data, size_t size, size_t nmemb, void *ctx);
static IMORQ *Init(void);

// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------

void ImoRq_SetIOLayer(IOLAYER *(*fp_Init)(void))
{
	IoLayer_Init = fp_Init;
}

// -----------------------------------------------------------------------------

IMORQ *ImoRq_Init(void)
{

	IMORQ *hRq;

	if (hRq = Init())
	{
		/* Create session ID */
		ImoRq_CreateID (hRq->szSessId, SSID_LENGTH+1);

		hRq->psend_seq = &hRq->send_seq;
		hRq->psend_ack = &hRq->send_ack;
		InitMutex(hRq->mutex);

		/* Disptch version of imo.im protocol */
		switch (IMO_API_VERSION)
		{
		case 0:
			hRq->pszHost = "https://o.imo.im/";
			break;
		case 1:
			hRq->pszHost = "https://imo.im/";
			break;
		}
		sprintf (hRq->szReqURL, "%simo", hRq->pszHost);

		/* Fetch start page to get cookies */
		if (hRq->hIO->Get (hRq->hIO, hRq->pszHost, NULL))

		/* Get new session ID from system */
		{
			char *pszRPC = ImoRq_ResetRPC (hRq), *pszRPCRes;
			if (pszRPC)
			{
				if ((pszRPCRes = strstr(pszRPC, "ssid\":\"")) || (pszRPCRes = strstr(pszRPC, "ssid\": \"")))
					strcpy (hRq->szSessId, strtok (pszRPCRes+7, "\""));
			}
		} else {
			ImoRq_Exit(hRq);
			hRq = NULL;
		}
	}

	return hRq;
}

// -----------------------------------------------------------------------------

IMORQ *ImoRq_Clone (IMORQ *hRq)
{
	IMORQ *hDup;

	if (!(hDup = Init())) return NULL;
	strcpy (hDup->szSessId, hRq->szSessId);
	hDup->psend_seq = hRq->psend_seq;
	hDup->psend_ack = hRq->psend_ack;
	hDup->mutex = hRq->mutex;
	hDup->bIsClone = TRUE;
	hDup->pszHost = hRq->pszHost;
	strcpy (hDup->szReqURL, hRq->szReqURL);
	return hDup;
}

// -----------------------------------------------------------------------------

void ImoRq_Exit (IMORQ *hRq)
{
	if (hRq->hIO) hRq->hIO->Exit(hRq->hIO);
	if (!hRq->bIsClone) ExitMutex (hRq->mutex);
	free (hRq);
}
// -----------------------------------------------------------------------------

void ImoRq_Cancel (IMORQ *hRq)
{
	if (hRq->hIO) hRq->hIO->Cancel(hRq->hIO);
}

// -----------------------------------------------------------------------------

char *ImoRq_PostImo(IMORQ *hRq, char *pszMethod, cJSON *data)
{
	TYP_FIFO *hPostString;
	char *pszData, *pszEscData;
	unsigned int uiCount = -1;

	if (!(pszData = cJSON_Print(data))) return NULL;
printf ("-> %s\n", pszData);
#ifdef _WIN32
OutputDebugString (pszData);
OutputDebugString ("\n");
#endif
	pszEscData = hRq->hIO->EscapeString(hRq->hIO, pszData);
	free (pszData);
	if (!pszEscData || !(hPostString = Fifo_Init(strlen(pszEscData)+32)))
	{
		if (pszEscData) hRq->hIO->FreeEscapeString (pszEscData);
		return NULL;
	}
	Fifo_AddString (hPostString, "method=");
	Fifo_AddString (hPostString, pszMethod);
	Fifo_AddString (hPostString, "&data=");
	Fifo_AddString (hPostString, pszEscData);
	hRq->hIO->FreeEscapeString (pszEscData);
	pszEscData =  Fifo_Get(hPostString, &uiCount);
	pszData = hRq->hIO->Post (hRq->hIO, hRq->szReqURL, pszEscData,
		uiCount-1, NULL);
	Fifo_Exit(hPostString);
printf ("<- %s\n", pszData);
	return pszData;
}

// -----------------------------------------------------------------------------

char *ImoRq_PostSystem(IMORQ *hRq, char *pszMethod, char *pszSysTo, char *pszSysFrom, cJSON *data, int bFreeData)
{
    cJSON *root, *msgs, *msg, *to, *from;
	char *pszRet;

    if (!(root=cJSON_CreateObject())) return NULL;
	LockMutex (hRq->mutex);
    cJSON_AddNumberToObject (root, "ack", *hRq->psend_ack);
    if (*hRq->szSessId) cJSON_AddStringToObject (root, "ssid", hRq->szSessId);
	else cJSON_AddNumberToObject (root, "ssid", 0);
    cJSON_AddItemToObject (root, "messages", (msgs = cJSON_CreateArray()));
	if (data)
	{
	    msg=cJSON_CreateObject();
	    cJSON_AddItemToObject(msg, "data", data);
	    to = cJSON_CreateObject();
	    cJSON_AddStringToObject (to, "system", pszSysTo);
	    cJSON_AddItemToObject(msg, "to", to);
	    from = cJSON_CreateObject();
	    cJSON_AddStringToObject (from, "system", pszSysFrom);
		if (*hRq->szSessId) cJSON_AddStringToObject (from, "ssid", hRq->szSessId);
		else cJSON_AddNumberToObject (from, "ssid", 0);
	    cJSON_AddItemToObject(msg, "from", from);
	    cJSON_AddNumberToObject (msg, "seq", (*hRq->psend_seq)++);
	    cJSON_AddItemToArray (msgs, msg);
	}
	UnlockMutex (hRq->mutex);
    pszRet = ImoRq_PostImo (hRq, pszMethod, root);
	if (data && !bFreeData)
	{
		msg->child = data->next;
		data->next = NULL;
	}
	cJSON_Delete (root);
	return pszRet;
}

// -----------------------------------------------------------------------------

char *ImoRq_ResetRPC(IMORQ *hRq)
{
	cJSON *root, *ssid;
	char *pszRet;

	if (!(root=cJSON_CreateObject())) return NULL;
	cJSON_AddStringToObject (root, "method", (IMO_API_VERSION==0?"get_ssid":"get_cookie_and_ssid"));
	ssid=cJSON_CreateObject();
	cJSON_AddStringToObject (ssid, "ssid", hRq->szSessId);
	if (IMO_API_VERSION > 0)
	{
		cJSON_AddStringToObject (ssid, "kind", "reui");
		cJSON_AddStringToObject (ssid, "version", "1336611734.48");
	}
	cJSON_AddItemToObject(root, "data", ssid);
	if (IMO_API_VERSION == 0) *hRq->szSessId = 0;
    pszRet = ImoRq_PostSystem (hRq, "rest_rpc", (IMO_API_VERSION==0?"ssid":"session"), "client", root, 1);
	LockMutex (hRq->mutex);
	*hRq->psend_seq=0;
	UnlockMutex (hRq->mutex);
	return pszRet;
}

// -----------------------------------------------------------------------------

char *ImoRq_UserActivity(IMORQ *hRq)
{
	cJSON *ssid;

	ssid=cJSON_CreateObject();
	cJSON_AddStringToObject (ssid, "ssid", hRq->szSessId);
	return ImoRq_PostToSys (hRq, "observed_user_activity", "session", ssid, 1, NULL);
}

// -----------------------------------------------------------------------------

char *ImoRq_Echo(IMORQ *hRq)
{
	cJSON *data;
	time_t t;
	char szTime[16], *pszRet;

	if (!(data=cJSON_CreateObject())) return NULL;
	sprintf (szTime, "%ld", time(&t)*1000);
	cJSON_AddStringToObject (data, "t", szTime);
	pszRet = ImoRq_PostImo (hRq, "echo", data);
	cJSON_Delete (data);
	return pszRet;
}

// -----------------------------------------------------------------------------

char *ImoRq_Reui_Session(IMORQ *hRq)
{
	cJSON *ssid;

	ssid=cJSON_CreateObject();
	cJSON_AddStringToObject (ssid, "ssid", hRq->szSessId);
	return ImoRq_PostToSys (hRq, "reui_session", "session", ssid, 1, NULL);
}

// -----------------------------------------------------------------------------

char *ImoRq_PostToSys(IMORQ *hRq, char *pszMethod, char *pszSysTo, cJSON *data, int bFreeData, int *pireqid)
{
	cJSON *root;
	char *pszRet;

	if (!(root=cJSON_CreateObject())) return NULL;
	cJSON_AddStringToObject (root, "method", pszMethod);
	if (pireqid) cJSON_AddNumberToObject(root, "request_id", *pireqid);
	cJSON_AddItemToObject(root, "data", data);
	pszRet = ImoRq_PostSystem (hRq, "forward_to_server", pszSysTo, "client", root, bFreeData);
	if (!bFreeData)
	{
		data->prev->next = data->next;
		if (data->next) data->next->prev = data->prev;
		data->prev = data->next = NULL;
		cJSON_Delete (root);
	}
	return pszRet;
}
// -----------------------------------------------------------------------------

char *ImoRq_PostAmy(IMORQ *hRq, char *pszMethod, cJSON *data)
{
	return ImoRq_PostToSys (hRq, pszMethod, "im", data, FALSE, NULL);
}

// -----------------------------------------------------------------------------

char *ImoRq_SessId(IMORQ *hRq)
{
	return hRq->szSessId;
}

// -----------------------------------------------------------------------------

char *ImoRq_GetLastError(IMORQ *hRq)
{
	return hRq->hIO->GetLastError (hRq->hIO);
}

// -----------------------------------------------------------------------------
char *ImoRq_GetHost(IMORQ *hRq)
{
	return hRq->pszHost;
}

// -----------------------------------------------------------------------------
void ImoRq_UpdateAck(IMORQ *hRq, unsigned long lAck)
{
	LockMutex (hRq->mutex);
	*hRq->psend_ack = lAck;
	UnlockMutex (hRq->mutex);
}
// -----------------------------------------------------------------------------
unsigned long ImoRq_GetSeq(IMORQ *hRq)
{
	unsigned long lRet;

	LockMutex (hRq->mutex);
	lRet = *hRq->psend_seq;
	UnlockMutex (hRq->mutex);
	return lRet;
}
// -----------------------------------------------------------------------------

void ImoRq_CreateID(char *pszID, int cbID)
{
	int i, r;
	time_t curtime;

	srand(time(&curtime));
	for (i=0; i<cbID; i++)
	{
		r = rand()%62;
		if (r<26) pszID[i]='A'+r; else
		if (r<52) pszID[i]='a'+(r-26); else
		pszID[i]='0'+(r-52);
	}
	pszID[i]=0;
	return;
}

// -----------------------------------------------------------------------------

char *ImoRq_HTTPGet(IMORQ *hRq, char *pszURL, unsigned int *pdwLength)
{
	return hRq->hIO->Get (hRq->hIO, pszURL, pdwLength);
}

// -----------------------------------------------------------------------------

static IMORQ *Init(void)
{
	IMORQ *hRq = calloc(1, sizeof(IMORQ));

	/* Setup CURL */
	if (!hRq) return NULL;
	if (!(hRq->hIO = IoLayer_Init()))
	{
		ImoRq_Exit(hRq);
		return NULL;
	}
	return hRq;
}

