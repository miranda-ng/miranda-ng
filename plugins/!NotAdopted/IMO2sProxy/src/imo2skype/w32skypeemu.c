#include <stdio.h>
#include "imo2sproxy.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define LockMutex(x) EnterCriticalSection (&x)
#define UnlockMutex(x) LeaveCriticalSection(&x)
#define InitMutex(x) InitializeCriticalSection(&x)
#define ExitMutex(x) DeleteCriticalSection(&x)
#define strcasecmp stricmp
#define strncasecmp stricmpn
#define mutex_t CRITICAL_SECTION
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "memlist.h"
#include "w32skypeemu.h"

#ifndef _WIN64
#if WINVER<0x0500
#define SetWindowLongPtr SetWindowLong
#define GetWindowLongPtr GetWindowLong
#endif
#ifndef LONG_PTR
#define LONG_PTR LONG
#endif
#ifndef GWLP_USERDATA
#define GWLP_USERDATA GWL_USERDATA
#endif
#endif

// Skype API defines
#define SKYPECONTROLAPI_ATTACH_SUCCESS 0
#define SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION 1
#define SKYPECONTROLAPI_ATTACH_REFUSED 2
#define SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE 3
#define SKYPECONTROLAPI_ATTACH_API_AVAILABLE 0x8001

// -----------------------------------------------------------------------------

typedef struct
{
	IMO2SPROXY vtbl; // Must be first!
	IMO2SPROXY_CFG *pCfg;
	W32SKYPEEMU_CFG *pMyCfg;
	HWND hWndDispatch;
	TYP_LIST *hClients;
	mutex_t loopmutex;
	DWORD dwThreadID;
} IMO2SPROXY_INST;

typedef struct 
{
	HWND hWnd;
	IMOSAPI *hInst;
	IMO2SPROXY_INST *hProxy;
	mutex_t sendmutex;
	int iConnectionStat;
} CONNINST;

static UINT m_ControlAPIAttach = 0;
static UINT m_ControlAPIDiscover = 0;

// -----------------------------------------------------------------------------

static void EventHandler(char *pszMsg, void *pUser);
static CONNINST *FindClient(IMO2SPROXY_INST *hProxy, HWND hWnd);
static void CleanConnections (TYP_LIST *hList);
static void FreeConnection (CONNINST *pInst);
static LONG APIENTRY WndProc(HWND hWnd, UINT message, UINT wParam, LONG lParam);

static int Imo2sproxy_Open(IMO2SPROXY *hInst);
static void Imo2sproxy_Loop(IMO2SPROXY *hInst);
static void Imo2sproxy_Exit(IMO2SPROXY *hInst);

// -----------------------------------------------------------------------------
static void EventHandler(char *pszMsg, void *pUser)
{
	CONNINST *pInst = (CONNINST*)pUser;
	COPYDATASTRUCT cds;
	DWORD dwRes = 0;

	LockMutex(pInst->sendmutex);
	if (pInst->hProxy->pCfg->bVerbose && pInst->hProxy->pCfg->fpLog)
	{
		fprintf (pInst->hProxy->pCfg->fpLog, "%08X> %s\n", pInst->hWnd, pszMsg);
		fflush (pInst->hProxy->pCfg->fpLog);
	}
	cds.dwData = 0;
	cds.cbData = strlen(pszMsg)+1;
	cds.lpData = pszMsg;
	SendMessageTimeout (pInst->hWnd, WM_COPYDATA, (WPARAM)pInst->hProxy->hWndDispatch, (LPARAM)&cds, SMTO_NORMAL, 1000, &dwRes);
	UnlockMutex(pInst->sendmutex);
}

// -----------------------------------------------------------------------------

static CONNINST *FindClient(IMO2SPROXY_INST *hProxy, HWND hWnd)
{
	int i, nCount;
	CONNINST *pConn;

	for (i=0, nCount=List_Count(hProxy->hClients); i<nCount; i++)
	{
		if ((pConn=(CONNINST*)List_ElementAt (hProxy->hClients, i))->hWnd == hWnd)
			return pConn;
	}
	return NULL;
}

// -----------------------------------------------------------------------------

static void CleanConnections (TYP_LIST *hList)
{
	unsigned int i;
	CONNINST *pInst;

	for (i=0; i<List_Count(hList); i++)
	{
		pInst = List_ElementAt (hList, i);
		if (!IsWindow (pInst->hWnd))
		{
			if (pInst->hInst) FreeConnection(pInst);
			free (List_RemoveElementAt(hList, i));
			i--;
		}
	}
}

// -----------------------------------------------------------------------------

static void FreeConnection (CONNINST *pInst)
{
	if (pInst->hProxy->pCfg->bVerbose && pInst->hProxy->pCfg->fpLog)
	{
		fprintf (pInst->hProxy->pCfg->fpLog, "Closed connection %08X\n", pInst->hWnd);
		fflush (pInst->hProxy->pCfg->fpLog);
	}
	ExitMutex (pInst->sendmutex);
	if (pInst->hInst)
	{
		IMOSAPI *hInst = pInst->hInst;
		pInst->hInst = NULL;
		Imo2S_Exit(hInst);
	}
}

// -----------------------------------------------------------------------------

static LONG APIENTRY WndProc(HWND hWnd, UINT message, UINT wParam, LONG lParam) 
{ 
    switch (message) 
    { 
		case WM_CREATE:
		{
			LPCREATESTRUCT lpCr = (LPCREATESTRUCT)lParam;

			SetWindowLongPtr (hWnd, GWLP_USERDATA, (LONG_PTR)lpCr->lpCreateParams);
			SetTimer (hWnd, 0, 60000, NULL);
			break;
		}
        case WM_COPYDATA: 
		{
			PCOPYDATASTRUCT pCopyData = (PCOPYDATASTRUCT)lParam;
			CONNINST *pInst;
			IMO2SPROXY_INST *hProxy = (IMO2SPROXY_INST*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

			if (pInst = FindClient (hProxy, (HWND)wParam))
			{
				if (pInst->hProxy->pMyCfg->bDelayLogin && pInst->iConnectionStat < 1)
				{
					char *pszError;

					if ((pInst->iConnectionStat = Imo2S_Login (pInst->hInst, hProxy->pCfg->pszUser, hProxy->pCfg->pszPass, &pszError)) != 1)
					{
						pInst->hProxy->pCfg->logerror (stderr, "Connection %08X: Cannot login with (%s/****): %s\n", 
							pInst->hWnd, hProxy->pCfg->pszUser, pszError);
						FreeConnection(pInst);
						free (List_Pop(hProxy->hClients));
						PostMessage ((HWND)wParam, m_ControlAPIAttach, (WPARAM)hWnd, SKYPECONTROLAPI_ATTACH_REFUSED);
						return 0;
					}
				}
				LockMutex(pInst->sendmutex);
				if (pInst->hProxy->pCfg->bVerbose && pInst->hProxy->pCfg->fpLog)
				{
					fprintf (pInst->hProxy->pCfg->fpLog, "%08X< [%s]\n", pInst->hWnd, pCopyData->lpData);
					fflush (pInst->hProxy->pCfg->fpLog);
				}
				Imo2S_Send (pInst->hInst, pCopyData->lpData);
				UnlockMutex(pInst->sendmutex);
			}
			return 1;
		}
		case WM_TIMER:
			// Housekeeping timer
			CleanConnections (((IMO2SPROXY_INST*)GetWindowLongPtr(hWnd, GWLP_USERDATA))->hClients);
			break;
		case WM_DESTROY:
			KillTimer (hWnd, 0);
			break;
		default:
			if (message == m_ControlAPIDiscover)
			{
				CONNINST *pInst;
				IMO2SPROXY_INST *hProxy = (IMO2SPROXY_INST*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
				char *pszError;
				
				if (!(pInst = FindClient (hProxy, (HWND)wParam)))
				{
					pInst = (CONNINST*)calloc (1, sizeof(CONNINST));
					if (!pInst) break;
					List_Push(hProxy->hClients, pInst);
					pInst->hProxy = hProxy;
					pInst->hWnd = (HWND)wParam;
					if (hProxy->pCfg->bVerbose && hProxy->pCfg->fpLog)
						fprintf (hProxy->pCfg->fpLog, "Imo2sproxy::SkypeControlAPIDiscover\n");

					if (!(pInst->hInst = Imo2S_Init(EventHandler, pInst, hProxy->pCfg->iFlags)))
					{
						hProxy->pCfg->logerror (stderr, "Connection %08X: Cannot start Imo2Skype instance.\n", pInst->hWnd);
						free (List_Pop(hProxy->hClients));
						PostMessage ((HWND)wParam, m_ControlAPIAttach, (WPARAM)hWnd, SKYPECONTROLAPI_ATTACH_REFUSED);
						return 0;
					}

					// FIXME: We should enable logging dependent on a loglevel rather than just enabling it
					if (hProxy->pCfg->bVerbose)
						Imo2S_SetLog (pInst->hInst, hProxy->pCfg->fpLog);

					InitMutex(pInst->sendmutex);
					
					if (!pInst->hProxy->pMyCfg->bDelayLogin)
					{
						PostMessage ((HWND)wParam, m_ControlAPIAttach, (WPARAM)hWnd, SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE);
						if ((pInst->iConnectionStat = Imo2S_Login (pInst->hInst, hProxy->pCfg->pszUser, hProxy->pCfg->pszPass, &pszError)) != 1)
						{
							pInst->hProxy->pCfg->logerror (stderr, "Connection %08X: Cannot login with (%s/****): %s\n", 
								pInst->hWnd, hProxy->pCfg->pszUser, pszError);
							FreeConnection(pInst);
							free (List_Pop(hProxy->hClients));
							PostMessage ((HWND)wParam, m_ControlAPIAttach, (WPARAM)hWnd, SKYPECONTROLAPI_ATTACH_REFUSED);
							return 0;
						}
						PostMessage ((HWND)wParam, m_ControlAPIAttach, (WPARAM)hWnd, SKYPECONTROLAPI_ATTACH_API_AVAILABLE);
					}
					else
					{
						SendMessage ((HWND)wParam, m_ControlAPIAttach, (WPARAM)hWnd, SKYPECONTROLAPI_ATTACH_SUCCESS);
					}
					return 0;
				}
				else
					SendMessage ((HWND)wParam, m_ControlAPIAttach, (WPARAM)hWnd, SKYPECONTROLAPI_ATTACH_SUCCESS);
				return 0;
			}
			break;
	}
	return (DefWindowProc(hWnd, message, wParam, lParam)); 
}



// -----------------------------------------------------------------------------
// PUBLIC
// -----------------------------------------------------------------------------

void W32SkypeEmu_Defaults (W32SKYPEEMU_CFG *pMyCfg)
{
	memset (pMyCfg, 0, sizeof(W32SKYPEEMU_CFG));
	// Login on first command from Client, immediately send SKYPECONTROLAPI_ATTACH_SUCCESS
	// This is necessary for some broken plugins, i.e. current SVN build of Miranda
	// Skype plugin, which doesn't correctly poll again if SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE
	// is sent.
	pMyCfg->bDelayLogin = TRUE;
}

// -----------------------------------------------------------------------------

IMO2SPROXY *W32SkypeEmu_Init (IMO2SPROXY_CFG *pCfg, W32SKYPEEMU_CFG *pMyCfg)
{
	IMO2SPROXY_INST *pstInst = calloc(sizeof(IMO2SPROXY_INST), 1);

	pstInst->vtbl.Open = Imo2sproxy_Open;
	pstInst->vtbl.Loop = Imo2sproxy_Loop;
	pstInst->vtbl.Exit = Imo2sproxy_Exit;
	pstInst->pCfg = pCfg;
	pstInst->pMyCfg = pMyCfg;
	InitMutex(pstInst->loopmutex);
	return (IMO2SPROXY*)pstInst;
}

// -----------------------------------------------------------------------------
// IMPLEMENTATION
// -----------------------------------------------------------------------------
static int Imo2sproxy_Open(IMO2SPROXY *hInst)
{
	IMO2SPROXY_INST *hProxy = (IMO2SPROXY_INST*)hInst;
	WNDCLASS WndClass ={0}; 

	// Start Skype connection 
	if (hProxy->pCfg->bVerbose && hProxy->pCfg->fpLog)
		fprintf (hProxy->pCfg->fpLog, "W32SkypeEmu:Open(Start)\n");
	if ((!m_ControlAPIAttach && !(m_ControlAPIAttach=RegisterWindowMessage("SkypeControlAPIAttach"))) || 
		(!m_ControlAPIDiscover && !(m_ControlAPIDiscover=RegisterWindowMessage("SkypeControlAPIDiscover")))) 
	{
		hProxy->pCfg->logerror (stderr, "Cannot register Window messages!");
		return -1;
	}
  
	// Create window class
	WndClass.style = CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS; 
	WndClass.lpfnWndProc = (WNDPROC)WndProc; 
	WndClass.hInstance =  GetModuleHandle(NULL);
	WndClass.lpszClassName = "Imo2SProxyDispatchWindow"; 
	RegisterClass(&WndClass);

	if (!(hProxy->hWndDispatch=CreateWindowEx( WS_EX_APPWINDOW|WS_EX_WINDOWEDGE,
		WndClass.lpszClassName, "", WS_BORDER|WS_SYSMENU|WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 128, 128, NULL, 0, (HINSTANCE)WndClass.hInstance, (LPVOID)hProxy)))
	{
		hProxy->pCfg->logerror (stderr, "Unable to create dispatch window!");
		UnregisterClass (WndClass.lpszClassName, WndClass.hInstance);
		return -1;
	}

	hProxy->hClients = List_Init(32);
	return 0;
}

// -----------------------------------------------------------------------------

static void Imo2sproxy_Loop(IMO2SPROXY *hInst)
{
	IMO2SPROXY_INST *hProxy = (IMO2SPROXY_INST*)hInst;
	MSG Message;

	if (hProxy->pCfg->bVerbose && hProxy->pCfg->fpLog)
		fprintf (hProxy->pCfg->fpLog, "W32SkypeEmu:Loop(Start)\n");

	// Pump da messages
	hProxy->dwThreadID = GetCurrentThreadId();
	LockMutex(hProxy->loopmutex);
	while (GetMessage(&Message, NULL, 0, 0)) 
    { 
        TranslateMessage(&Message); 
        DispatchMessage(&Message);
    }
	
	if (hProxy->pCfg->bVerbose && hProxy->pCfg->fpLog)
		fprintf (hProxy->pCfg->fpLog, "W32SkypeEmu:Loop(End)\n");
	UnlockMutex(hProxy->loopmutex);
}


// -----------------------------------------------------------------------------

static void Imo2sproxy_Exit(IMO2SPROXY *hInst)
{
	IMO2SPROXY_INST *hProxy = (IMO2SPROXY_INST*)hInst;
	CONNINST *pInst;

	if (hProxy->pCfg->bVerbose && hProxy->pCfg->fpLog)
		fprintf (hProxy->pCfg->fpLog, "W32SkypeEmu:Exit()\n");

	if (hProxy->hWndDispatch) DestroyWindow (hProxy->hWndDispatch);
	if (hProxy->dwThreadID) PostThreadMessage (hProxy->dwThreadID, WM_QUIT, 0, 0);
	LockMutex(hProxy->loopmutex);

	// Kill 'em all!
	if (hProxy->hClients)
	{
		while (pInst=List_Pop(hProxy->hClients))
		{
			FreeConnection(pInst);
			free (pInst);
		}
		List_Exit (hProxy->hClients);
	}

	UnregisterClass ("Imo2SProxyDispatchWindow", GetModuleHandle(NULL));

	UnlockMutex(hProxy->loopmutex);
	ExitMutex(hProxy->loopmutex);

	free (hProxy);
}

