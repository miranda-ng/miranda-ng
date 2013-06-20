#include "imo2sproxy.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include "skypepluginlink.h"
#include "include/newpluginapi.h"
#include "include/m_system.h"

#define LockMutex(x) EnterCriticalSection (&x)
#define UnlockMutex(x) LeaveCriticalSection(&x)
#define InitMutex(x) InitializeCriticalSection(&x)
#define ExitMutex(x) DeleteCriticalSection(&x)
#define mutex_t CRITICAL_SECTION


// -----------------------------------------------------------------------------

struct tag_proxyInst;
typedef struct tag_proxyInst IMO2SPROXY_INST;

typedef struct
{
	IMOSAPI *hInst;
	IMO2SPROXY_INST *hProxy;
	mutex_t sendmutex;
	mutex_t rcvmutex;
	int iConnectionStat;
} CONNINST;

struct tag_proxyInst
{
	IMO2SPROXY vtbl; // Must be first!
	IMO2SPROXY_CFG *pCfg;
	SKYPEPLUGINLINK_CFG *pMyCfg;
	HANDLE hService;
	CONNINST stClient;	// Currently only 1 connection
};


static CONNINST *m_pConn = NULL;

// -----------------------------------------------------------------------------

static void EventHandler(char *pszMsg, void *pUser);
static int InitProxy(IMO2SPROXY_INST *hProxy);

static int Imo2sproxy_Open(IMO2SPROXY *hInst);
static void Imo2sproxy_Loop(IMO2SPROXY *hInst);
static void Imo2sproxy_Exit(IMO2SPROXY *hInst);

// -----------------------------------------------------------------------------
static void EventHandler(char *pszMsg, void *pUser)
{
	CONNINST *pInst = (CONNINST*)pUser;
	COPYDATASTRUCT cds;
	DWORD dwRes = 0;

	//LockMutex (pInst->sendmutex);
	if (pInst->hProxy->pCfg->bVerbose && pInst->hProxy->pCfg->fpLog)
	{
		fprintf (pInst->hProxy->pCfg->fpLog, "> %s\n", pszMsg);
		fflush (pInst->hProxy->pCfg->fpLog);
	}
	cds.dwData = 0;
	cds.cbData = strlen(pszMsg)+1;
	cds.lpData = pszMsg;
	CallService (SKYPE_PROTONAME PSS_SKYPEAPIMSG, 0, (LPARAM)&cds);
	//UnlockMutex (pInst->sendmutex);
}

// -----------------------------------------------------------------------------

INT_PTR CallIn(WPARAM wParam,LPARAM lParam)
{
	CONNINST *pInst = (CONNINST*)m_pConn;
	PCOPYDATASTRUCT pCopyData = (PCOPYDATASTRUCT)lParam;

	if (!pInst) return -1;

	if (pInst->hProxy->pMyCfg->bDelayLogin && pInst->iConnectionStat < 1)
	{
		LockMutex (pInst->rcvmutex);
		if (InitProxy(pInst->hProxy)<0)
		{
			UnlockMutex(pInst->rcvmutex);
			return -1;
		}
		UnlockMutex(pInst->rcvmutex);
	}

	if (pInst->hProxy->pCfg->bVerbose && pInst->hProxy->pCfg->fpLog)
	{
		fprintf (pInst->hProxy->pCfg->fpLog, "< [%s]\n", pCopyData->lpData);
		fflush (pInst->hProxy->pCfg->fpLog);
	}
	Imo2S_Send (pInst->hInst, pCopyData->lpData);
	return 0;
}

// -----------------------------------------------------------------------------

static void FreeConnection (CONNINST *pInst)
{
	if (!pInst || !pInst->hProxy) return;
	if (pInst->hProxy->pCfg->bVerbose && pInst->hProxy->pCfg->fpLog)
	{
		fprintf (pInst->hProxy->pCfg->fpLog, "SkypePluginLink: Closed connection\n");
		fflush (pInst->hProxy->pCfg->fpLog);
	}

	if (pInst->hProxy->hService)
	{
		DestroyServiceFunction(pInst->hProxy->hService);
		pInst->hProxy->hService = NULL;
		CallService (SKYPE_PROTONAME SKYPE_REGPROXY, 0, 0);
	}
	ExitMutex(pInst->sendmutex);
	ExitMutex(pInst->rcvmutex);
	if (pInst->hInst)
	{
		IMOSAPI *hInst = pInst->hInst;
		pInst->hInst = NULL;
		Imo2S_Exit(hInst);
	}
}

// -----------------------------------------------------------------------------

static int InitProxy(IMO2SPROXY_INST *hProxy)
{
	char *pszError;

	if (!hProxy->stClient.hInst)
	{
		if (!(hProxy->stClient.hInst = Imo2S_Init(EventHandler, &hProxy->stClient, hProxy->pCfg->iFlags)))
		{
			hProxy->pCfg->logerror (stderr, "SkypePluginLink: Cannot start Imo2Skype instance.\n");
			return -1;
		}
	}

	// FIXME: We should enable logging dependent on a loglevel rather than just enabling it
	Imo2S_SetLog (hProxy->stClient.hInst, hProxy->pCfg->fpLog);

	if (hProxy->stClient.iConnectionStat == 0 ||
		(hProxy->stClient.iConnectionStat = Imo2S_Login (hProxy->stClient.hInst, 
		hProxy->pCfg->pszUser, hProxy->pCfg->pszPass, &pszError)) != 1)
	{
		hProxy->pCfg->logerror (stderr, "SkypePluginLink: Cannot login with (%s/****): %s\n", 
			hProxy->pCfg->pszUser, pszError);
		return -1;
	}
	return 0;
}

// -----------------------------------------------------------------------------
// PUBLIC
// -----------------------------------------------------------------------------

void SkypePluginLink_Defaults (SKYPEPLUGINLINK_CFG *pMyCfg)
{
	memset (pMyCfg, 0, sizeof(SKYPEPLUGINLINK_CFG));
	pMyCfg->bDelayLogin = TRUE;
}

// -----------------------------------------------------------------------------

IMO2SPROXY *SkypePluginLink_Init (IMO2SPROXY_CFG *pCfg, SKYPEPLUGINLINK_CFG *pMyCfg)
{
	IMO2SPROXY_INST *pstInst = calloc(sizeof(IMO2SPROXY_INST), 1);

	pstInst->vtbl.Open = Imo2sproxy_Open;
	pstInst->vtbl.Loop = Imo2sproxy_Loop;
	pstInst->vtbl.Exit = Imo2sproxy_Exit;
	pstInst->pCfg = pCfg;
	pstInst->pMyCfg = pMyCfg;
	return (IMO2SPROXY*)pstInst;
}

// -----------------------------------------------------------------------------
// IMPLEMENTATION
// -----------------------------------------------------------------------------
static int Imo2sproxy_Open(IMO2SPROXY *hInst)
{
	IMO2SPROXY_INST *hProxy = (IMO2SPROXY_INST*)hInst;
	WNDCLASS WndClass ={0}; 

	if (hProxy->pCfg->bVerbose && hProxy->pCfg->fpLog)
		fprintf (hProxy->pCfg->fpLog, "SkypePluginLink:Open(Start)\n");

	// Only 1 connection instance, so we can init it right here
	InitMutex(hProxy->stClient.sendmutex);
	InitMutex(hProxy->stClient.rcvmutex);
	hProxy->stClient.hProxy = hProxy;
	m_pConn = &hProxy->stClient;

	// Register with Skype Plugin
	hProxy->hService = CreateServiceFunction("IMO2SPROXY" PSS_SKYPEAPIMSG, CallIn);
	CallService (SKYPE_PROTONAME SKYPE_REGPROXY, 0, (LPARAM)"IMO2SPROXY" PSS_SKYPEAPIMSG);
	hProxy->stClient.iConnectionStat = -1;

	if (!hProxy->pMyCfg->bDelayLogin)
	{
		if (InitProxy(hProxy)<0)
		{
			FreeConnection(&hProxy->stClient);
			return -1;
		}
	}
	return 0;
}

// -----------------------------------------------------------------------------

static void Imo2sproxy_Loop(IMO2SPROXY *hInst)
{
	// No loop needed for call-in
}


// -----------------------------------------------------------------------------

static void Imo2sproxy_Exit(IMO2SPROXY *hInst)
{
	IMO2SPROXY_INST *hProxy = (IMO2SPROXY_INST*)hInst;

	if (hProxy->pCfg->bVerbose && hProxy->pCfg->fpLog)
		fprintf (hProxy->pCfg->fpLog, "SkypePluginLink:Exit()\n");

	FreeConnection (&hProxy->stClient);
	free (hProxy);
}

