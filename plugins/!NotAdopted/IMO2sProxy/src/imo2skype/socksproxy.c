/* Module:  imo2skypeproxy.c
   Purpose: Implementation of imo2skype API as Skype proxy (for Miranda Skype plugin)
   Author:  leecher
   Date:    01.09.2009

   Changelog:
    1.00  - 01.09.2009 - Initial release
    1.01  - 02.09.2009 - Little bugfix for Onlinestatus, introduced mutex for send thread.
    1.02  - 02.09.2009 - Added support for incoming call notifications and expired sessions.
    1.03  - 03.09.2009 - As WIN32-port of libcurl is a total mess, Win32-version now uses WinINET.
    1.04  - 04.09.2009 - Added support for searching, adding and removing buddies
    1.05  - 06.09.2009 - * Mapped "busy" to DND, not NA, as forum user AL|EN proposed.
                         * Mood text will now also be sent on status change of a contact
                           (doesn't really have an effect though)
                         * New Parameter -d (daemonize) to launch proxy in background
                         * Fixed a bug with a crash when encoding umlauts on Win32
    1.06  - 25.09.2009 - Added parameter -t for local timestamps.
    1.07  - 07.10.2009 - Bugfix: Added parsing of multiple JSON messages
                         Added support for writing to logfile while in daemon mode (-l)
    1.08  - 18.10.2009 - Added support for voice calls via imo.im flash on WIN32 (-i)
                       - Limited memory queues to 50 entries to reduce memory usage.
    1.09  - 07.11.2009 - Split command line main module and proxy module to make proxy
                         accessible by different layers (i.e. new Miranda Plugin layer)
    1.10  - 14.12.2009 - Bug in the cJson library. The authors used the String as second 
                         sprintf-Parameter which contains the format string instead of 
                         ["%s", StringParam] causing problems sending Messages containt a %
                       - Added some space in the dialog for translations and added Translate() 
                         function to some strings in the Wrapper-DLL
                       - More verbose output if you enable logging
                       - Fixes a Bug that caused the plugin to block after going offline 
                         and reconnecting back online
                       - Hopefully fixed a severe threading problem: The Send-Mutex was not
                         covering a full transaction, but only 1 send causing the receiver
                         the receive garbled data causing "An existing connection was forcibly
                         closed by the remote host." error.
                       - Imo2S_Exit freed the temporary buffer too early resulting in a crash
                         on exit.
	1.12  - 19.04.2010 - POST interface changed from www.imo.im/amy to s.imo.im/amy
	1.13  - 19.12.2010 - imo.im modified their interface, so tried to adapt the communication
						 routines at low level to be compatible again. There still may be
						 many bugs. Don't forget to turn off history logging in the options
						 at https://imo.im
						 There also seems to be a nice new feature called "reflection" so that
						 you can login from multiple sessions and messages etc. entered there
						 will be reflected to the current session. This may be addressed in one
						 of the next builds...
*/

#include <stdio.h>
#include "imo2sproxy.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <process.h> 
#define thread_t HANDLE
#define LockMutex(x) EnterCriticalSection (&x)
#define UnlockMutex(x) LeaveCriticalSection(&x)
#define InitMutex(x) InitializeCriticalSection(&x)
#define ExitMutex(x) DeleteCriticalSection(&x)
#define strcasecmp stricmp
#define strncasecmp stricmpn
#define mutex_t CRITICAL_SECTION
#define SHUT_RD SD_RECEIVE
#define SHUT_WR SD_SEND
#define SHUT_RDWR SD_BOTH
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#define SOCKET int
#define thread_t pthread_t
#define mutex_t pthread_mutex_t
#define INVALID_SOCKET -1
#define SOCKET_ERROR INVALID_SOCKET
#define closesocket close
#define LockMutex(x) pthread_mutex_lock(&x)
#define UnlockMutex(x) pthread_mutex_unlock(&x)
#define InitMutex(x)  pthread_mutex_init(&x, NULL);
#define ExitMutex(x) 
#endif
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "memlist.h"
#include "socksproxy.h"

// -----------------------------------------------------------------------------

#define AUTHENTICATE	0x01
#define CAPABILITIES	0x02
#define USE_AUTHENTICATION 0x01

typedef struct
{
	IMO2SPROXY vtbl; // Must be first!
	IMO2SPROXY_CFG *pCfg;
	SOCKSPROXY_CFG *pMyCfg;

	SOCKET listen_fd;
	volatile int iRunning;
	mutex_t loopmutex;
} IMO2SPROXY_INST;

typedef struct 
{
	SOCKET hSock;
	thread_t hThread;
	IMOSAPI *hInst;
	int iConnectionStat;
	mutex_t connected;
	mutex_t sendmutex;
	IMO2SPROXY_INST *hProxy;
} CONNINST;


// -----------------------------------------------------------------------------

static void EventHandler(char *pszMsg, void *pUser);
static void DispatcherThread(void *pUser);
static int Dispatcher_Start(CONNINST *pInst);
static int Dispatcher_Stop(CONNINST *pInst);
static char *GetError(void);
static int RcvPacket (CONNINST *pInst, void *buf, int len);
static int SendPacket (CONNINST *pInst, void *buf, int len);
static void FreeConnection (CONNINST *pInst);
static void CleanConnections (TYP_LIST *hList);
static SOCKET Init(unsigned long int lHost, short sPort, int iMaxConn);
static void Loop(SOCKET listen_fd);
static void Exit(SOCKET listen_fd);

static int Imo2sproxy_Open(IMO2SPROXY *hInst);
static void Imo2sproxy_Loop(IMO2SPROXY *hInst);
static void Imo2sproxy_Exit(IMO2SPROXY *hInst);

// -----------------------------------------------------------------------------
static void EventHandler(char *pszMsg, void *pUser)
{
	CONNINST *pInst = (CONNINST*)pUser;
	unsigned int uiLen = strlen (pszMsg);
	static BOOL bFirstLogin = TRUE;

	if (pInst->hProxy->pCfg->bVerbose && pInst->hProxy->pCfg->fpLog)
	{
		fprintf (pInst->hProxy->pCfg->fpLog, "%03d> %s\n", pInst->hSock, pszMsg);
		fflush (pInst->hProxy->pCfg->fpLog);
	}
	if (bFirstLogin && strncmp (pszMsg, "CONNSTATUS", 10) == 0 &&
		strcmp(pszMsg+11, "CONNECTING"))
	{
		pInst->iConnectionStat = (strcmp(pszMsg+11, "ONLINE")==0);
		UnlockMutex (pInst->connected);
		bFirstLogin = FALSE;
	}
	LockMutex(pInst->sendmutex);
	if (!(SendPacket (pInst, &uiLen, sizeof(uiLen)) && SendPacket (pInst, pszMsg, uiLen)))
	{
		//Dispatcher_Stop(pInst);
		//FreeConnection (pInst);
	}
	UnlockMutex(pInst->sendmutex);
}

// -----------------------------------------------------------------------------

static void DispatcherThread(void *pUser)
{
	CONNINST *pInst = (CONNINST*)pUser;
	char *pszUser, *pszPass, *pszError, *pszMsgBuf=NULL;
	unsigned int uiLength, cbMsgBuf=0, bAuthenticated = 0, iConnected=0, iLogin=1;
	char command=0, reply=0;

	if (pInst->hProxy->pCfg->bVerbose && pInst->hProxy->pCfg->fpLog)
		fprintf (pInst->hProxy->pCfg->fpLog, "Imo2sproxy::DispatcherThread()\n");

	if (!(pInst->hInst = Imo2S_Init(EventHandler, pInst, pInst->hProxy->pCfg->iFlags)))
	{
		pInst->hProxy->pCfg->logerror (stderr, "Connection %d: Cannot start Imo2Skype instance.\n", pInst->hSock);
		FreeConnection (pInst);
		return;
	}

	// FIXME: Static user+pass from cmdline, until there is a possibility for 
	// a client to authenticate
	pszUser = pInst->hProxy->pCfg->pszUser;
	pszPass = pInst->hProxy->pCfg->pszPass;

	// FIXME: We should enable logging dependent on a loglevel rather than just enabling it
	if (pInst->hProxy->pCfg->bVerbose)
		Imo2S_SetLog (pInst->hInst, pInst->hProxy->pCfg->fpLog);


	while (pInst->hProxy->iRunning)
	{
		if (RcvPacket(pInst, &uiLength, sizeof(uiLength))<=0) break;
		LockMutex(pInst->sendmutex);

		if (uiLength == 0)
		{
			if (RcvPacket(pInst, &command, 1)<=0)
			{
				UnlockMutex(pInst->sendmutex);
				break;
			}
			switch (command)
			{
				case AUTHENTICATE:
					if (pInst->hProxy->pMyCfg->pszAuthPass) reply=1;
					break;
				case CAPABILITIES:
					if (pInst->hProxy->pMyCfg->pszAuthPass) reply=USE_AUTHENTICATION;
					break;
			}
			if (SendPacket (pInst, &reply, 1)<=0)
			{
				UnlockMutex(pInst->sendmutex);
				break;
			}
			UnlockMutex(pInst->sendmutex);
			continue;
		}

		if (uiLength >= cbMsgBuf)
		{
			pszMsgBuf = realloc (pszMsgBuf, uiLength+1);
			if (!pszMsgBuf)
			{
				UnlockMutex(pInst->sendmutex);
				break;
			}
			cbMsgBuf=uiLength+1;
		}

		if (RcvPacket(pInst, pszMsgBuf, uiLength)<=0)
		{
			UnlockMutex(pInst->sendmutex);
			break;
		}

		if (command)
		{
			if (command == AUTHENTICATE)
			{
				bAuthenticated = pInst->hProxy->pMyCfg->pszAuthPass && strcmp(pInst->hProxy->pMyCfg->pszAuthPass, pszMsgBuf) == 0;
				if (SendPacket (pInst, &bAuthenticated, 1)<=0)
				{
					UnlockMutex(pInst->sendmutex);
					break;
				}
			}
			command = 0;
		}
		UnlockMutex(pInst->sendmutex);

		if (iLogin)
		{
			if (Imo2S_Login (pInst->hInst, pszUser, pszPass, &pszError) != 1)
			{
				pInst->hProxy->pCfg->logerror (stderr, "Connection %d: Cannot login with (%s/****): %s\n", 
					pInst->hSock, pszUser, pszError);
				FreeConnection (pInst);
				return;
			}
			iLogin = 0;
		}

		if (pInst->hProxy->pMyCfg->pszAuthPass && !bAuthenticated)continue;
		pszMsgBuf[uiLength]=0;
		if (pInst->hProxy->pCfg->bVerbose && pInst->hProxy->pCfg->fpLog)
		{
			fprintf (pInst->hProxy->pCfg->fpLog, "%03d< [%s]\n", pInst->hSock, pszMsgBuf);
			fflush (pInst->hProxy->pCfg->fpLog);
		}
		if (!iConnected)
		{
			LockMutex (pInst->connected);
			iConnected = pInst->iConnectionStat;
			if (!iConnected)
			{
				pInst->hProxy->pCfg->logerror(stderr, "Invalid username / password");
			}
		}
		Imo2S_Send (pInst->hInst, pszMsgBuf);	
	}

	FreeConnection (pInst);
	if (pszMsgBuf) free (pszMsgBuf);
	return;
}

// -----------------------------------------------------------------------------

#ifdef WIN32
static int Dispatcher_Start(CONNINST *pInst)
{
	DWORD ThreadID;

    return (pInst->hThread=(thread_t)_beginthreadex(NULL, 0, 
		(unsigned(__stdcall *)(void*))DispatcherThread, pInst, 0, &ThreadID))!=0; 
	
}

static int Dispatcher_Stop(CONNINST *pInst)
{
	return pInst->hThread?TerminateThread (pInst->hThread, 0):1;
}

static char *GetError(void)
{
	static char szMessage[1024];
	DWORD dwErr = WSAGetLastError();

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErr, 0, szMessage, sizeof(szMessage), NULL);
	return szMessage;
}

#else
static int Dispatcher_Start(CONNINST *pInst)
{
	return pthread_create(&pInst->hThread, NULL, DispatcherThread, pInst)==0;
}

static int Dispatcher_Stop(CONNINST *pInst)
{
	if (!pInst->hThread || pthread_cancel(pInst->hThread))
	{
		pInst->hThread=0;
		return 1;
	}
	return 0;
}

static char *GetError(void)
{
	return strerror (errno);
}

#endif

// -----------------------------------------------------------------------------

static int RcvPacket (CONNINST *pInst, void *buf, int len)
{
	int iReceived = recv (pInst->hSock, buf, len, 0);
	if (iReceived <= 0)
		pInst->hProxy->pCfg->logerror(stderr, "Connection %d (Receive): %s", pInst->hSock, GetError());
	return iReceived;
}

// -----------------------------------------------------------------------------

static int SendPacket (CONNINST *pInst, void *buf, int len)
{
	int iSent;
	
	iSent = send (pInst->hSock, buf, len, 0);
	if (iSent <= 0)
		pInst->hProxy->pCfg->logerror (stderr, "Connection %d (Send): %s", pInst->hSock, GetError());
	return iSent;
}
// -----------------------------------------------------------------------------

static void FreeConnection (CONNINST *pInst)
{
	if (pInst->hProxy->pCfg->bVerbose && pInst->hProxy->pCfg->fpLog)
	{
		fprintf (pInst->hProxy->pCfg->fpLog, "Closed connection %d\n", pInst->hSock);
		fflush (pInst->hProxy->pCfg->fpLog);
	}
	if (pInst->hSock != INVALID_SOCKET)
		closesocket(pInst->hSock);
	pInst->hSock = INVALID_SOCKET;
	pInst->hThread = 0;
	ExitMutex (pInst->connected);
	ExitMutex (pInst->sendmutex);
	if (pInst->hInst)
	{
		IMOSAPI *hInst = pInst->hInst;
		pInst->hInst = NULL;
		Imo2S_Exit(hInst);
	}
}

// -----------------------------------------------------------------------------

static void CleanConnections (TYP_LIST *hList)
{
	unsigned int i;
	CONNINST *hInst;

	for (i=0; i<List_Count(hList); i++)
	{
		hInst = List_ElementAt (hList, i);
		if (hInst->hThread == 0)
		{
			free (List_RemoveElementAt(hList, i));
			i--;
		}
	}
}

// -----------------------------------------------------------------------------
// PUBLIC
// -----------------------------------------------------------------------------

void SocksProxy_Defaults (SOCKSPROXY_CFG *pMyCfg)
{
	memset (pMyCfg, 0, sizeof(SOCKSPROXY_CFG));
	pMyCfg->lAddr = htonl(INADDR_ANY);
	pMyCfg->sPort = 1401;
	pMyCfg->iMaxConn = SOMAXCONN;
}

// -----------------------------------------------------------------------------

IMO2SPROXY *SocksProxy_Init (IMO2SPROXY_CFG *pCfg, SOCKSPROXY_CFG *pMyCfg)
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
	struct sockaddr_in sock={0};
	int yes = 1;
	IMO2SPROXY_INST *hProxy = (IMO2SPROXY_INST*)hInst;

#ifdef WIN32
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		hProxy->pCfg->logerror (stderr, "WSAStartup failed");
		return INVALID_SOCKET;
	}
#endif
	if (hProxy->pCfg->bVerbose && hProxy->pCfg->fpLog)
		fprintf (hProxy->pCfg->fpLog, "Socksproxy:Loop(Start)\n");
	hProxy->listen_fd = socket(PF_INET, SOCK_STREAM, 0);
	if(hProxy->listen_fd == INVALID_SOCKET) return -1;
	if (setsockopt(hProxy->listen_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(int))<0)
	{
		hProxy->pCfg->logerror (stderr, "Cannot set socket options to SO_REUSEADDR");
		closesocket(hProxy->listen_fd);
		return -1;
	}
	sock.sin_family = AF_INET;
	sock.sin_addr.s_addr = hProxy->pMyCfg->lAddr;
	sock.sin_port = htons(hProxy->pMyCfg->sPort);

	if (bind(hProxy->listen_fd, (struct sockaddr *) &sock, sizeof(sock)) != 0)
	{
		hProxy->pCfg->logerror (stderr, "Cannot bind socket");
		closesocket(hProxy->listen_fd);
		return -1;
	}

	if (listen(hProxy->listen_fd, hProxy->pMyCfg->iMaxConn) < 0)
	{
		hProxy->pCfg->logerror (stderr, "Cannot listen on socket");
		closesocket(hProxy->listen_fd);
		return -1;
	}

	return 0;
}

// -----------------------------------------------------------------------------

static void Imo2sproxy_Loop(IMO2SPROXY *hInst)
{
	struct sockaddr_in sock;
	int socklen;
	SOCKET new_fd;
	TYP_LIST *hConns = List_Init(32);
	CONNINST *pInst;
	IMO2SPROXY_INST *hProxy = (IMO2SPROXY_INST*)hInst;
	fd_set fdListen;

	if (hProxy->pCfg->bVerbose && hProxy->pCfg->fpLog)
		fprintf (hProxy->pCfg->fpLog, "Socksproxy:Loop(Start)\n");
	hProxy->iRunning = 1;
	LockMutex(hProxy->loopmutex);
	while (hProxy->iRunning)
	{
		FD_ZERO(&fdListen);
		FD_SET(hProxy->listen_fd, &fdListen);
		socklen = sizeof(sock);
		if (select (0, &fdListen, NULL, NULL, NULL) != SOCKET_ERROR && FD_ISSET(hProxy->listen_fd, &fdListen))
		{
			new_fd = accept(hProxy->listen_fd, (struct sockaddr *) &sock, &socklen); 
			if (hProxy->pCfg->bVerbose && hProxy->pCfg->fpLog)
			{
				fprintf (hProxy->pCfg->fpLog, "Connection from %s:%d -> Connection: %d\n", inet_ntoa(sock.sin_addr),
					ntohs(sock.sin_port), new_fd);
				fflush (hProxy->pCfg->fpLog);
			}
			if (new_fd != INVALID_SOCKET && (pInst = calloc (1, sizeof(CONNINST))))
			{
				CleanConnections (hConns);
				List_Push(hConns, pInst);
				pInst->hSock = new_fd;
				pInst->hProxy = hProxy;
				InitMutex(pInst->connected);
				LockMutex(pInst->connected);
				InitMutex(pInst->sendmutex);
				Dispatcher_Start(pInst);
			}
		}
	}
	if (hProxy->pCfg->bVerbose && hProxy->pCfg->fpLog)
		fprintf (hProxy->pCfg->fpLog, "Socksproxy:Loop(End)\n");

	CleanConnections (hConns);
	while (pInst=List_Pop(hConns))
	{
		Dispatcher_Stop(pInst);
		FreeConnection(pInst);
		free (pInst);
	}
	List_Exit(hConns);
	UnlockMutex(hProxy->loopmutex);
}


// -----------------------------------------------------------------------------

static void Imo2sproxy_Exit(IMO2SPROXY *hInst)
{
	IMO2SPROXY_INST *hProxy = (IMO2SPROXY_INST*)hInst;

	if (hProxy->pCfg->bVerbose && hProxy->pCfg->fpLog)
		fprintf (hProxy->pCfg->fpLog, "Socksproxy:Exit()\n");

	hProxy->iRunning = 0;
	shutdown (hProxy->listen_fd, SHUT_RDWR);
	closesocket (hProxy->listen_fd);
	LockMutex(hProxy->loopmutex);

/*
#ifdef WIN32
	WSACleanup();
#endif
*/
	UnlockMutex(hProxy->loopmutex);
	ExitMutex(hProxy->loopmutex);
	free (hProxy);
}

