/* Module:  io_layer_netlib.c
   Purpose: IO Layer for Internet communication using Miranda NETLIB
   Author:  leecher
   Date:    20.04.2011 :=)
*/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <stdlib.h>
#include "include/newpluginapi.h"
#include "include/m_netlib.h"
#include "fifo.h"
#include "memlist.h"
#include "io_layer.h"

#define USER_AGENT "Mozilla/5.0 (Windows; U; Windows NT 5.1; de; rv:1.9.2.13) Gecko/20101203 Firefox/3.6.13"

typedef struct
{
	IOLAYER vtbl;
	int iRecursion;
	NETLIBHTTPREQUEST *nlhrReply;
	LPVOID lpErrorBuf;
	HANDLE nlc;
} IOLAYER_INST;

typedef struct
{
	HANDLE hNetlib;
	TYP_LIST *hCookies;
	TYP_FIFO *hCookieStr;
	char *pszCookies;
	LONG lRefCount;
	CRITICAL_SECTION cs;
} IOLAYER_SINGLETON;

static IOLAYER_SINGLETON *m_hNL = NULL;

static void IoLayer_Exit (IOLAYER *hPIO);
static char *IoLayer_Post(IOLAYER *hPIO, char *pszURL, char *pszPostFields, unsigned int cbPostFields, unsigned int *pdwLength);
static char *IoLayer_Get(IOLAYER *hIO, char *pszURL, unsigned int *pdwLength);
static void IoLayer_Cancel(IOLAYER *hIO);
static char *IoLayer_GetLastError(IOLAYER *hIO);
static char *IoLayer_EscapeString(IOLAYER *hPIO, char *pszData);
static void IoLayer_FreeEscapeString(char *pszData);
static void FetchLastError (IOLAYER_INST *hIO);
static void add_cookies(char *pszCookies);
static void refresh_cookies();
static HANDLE *OpenConnection(NETLIBHTTPREQUEST *nlhr);

// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------

IOLAYER *IoLayerNETLIB_Init(void)
{
	IOLAYER_INST *hIO;
	
	if (CallService (MS_SYSTEM_GETVERSION, 0, 0) < 0x080000 ||	// Miranda HTTPS support starting with 0.8.0.0
		!(hIO = calloc(1, sizeof(IOLAYER_INST))))
		return NULL;
	
	// NETLIB only works as singleton
	if (!m_hNL) 
	{
		NETLIBUSER nlu={0};

		if (!(m_hNL = calloc (1, sizeof(IOLAYER_SINGLETON))))
		{
			free (hIO);
			return NULL;
		}
		m_hNL->lRefCount++;

		// Init Netlib
		nlu.cbSize = sizeof(nlu);
		nlu.flags = NUF_OUTGOING;
		nlu.szDescriptiveName = "imo2sproxy connection";
		nlu.szSettingsModule = "IMOPROXY";
		nlu.szHttpGatewayUserAgent = USER_AGENT;
		if (!(m_hNL->hNetlib = (HANDLE)CallService (MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu)) ||
			!(m_hNL->hCookies = List_Init(0)) ||
			!(m_hNL->hCookieStr = Fifo_Init(0)))
		{
			IoLayer_Exit((IOLAYER*)hIO);
			return NULL;
		}	
		m_hNL->pszCookies = "";	
		InitializeCriticalSection (&m_hNL->cs);
	} else m_hNL->lRefCount++;

	// Init Vtbl
	hIO->vtbl.Exit = IoLayer_Exit;
	hIO->vtbl.Post = IoLayer_Post;
	hIO->vtbl.Get = IoLayer_Get;
	hIO->vtbl.Cancel = IoLayer_Cancel;
	hIO->vtbl.GetLastError = IoLayer_GetLastError;
	hIO->vtbl.EscapeString = IoLayer_EscapeString;
	hIO->vtbl.FreeEscapeString = IoLayer_FreeEscapeString;

	return (IOLAYER*)hIO;
}
// -----------------------------------------------------------------------------

static void IoLayer_Exit (IOLAYER *hPIO)
{
	IOLAYER_INST *hIO = (IOLAYER_INST*)hPIO;

	if (hIO->nlhrReply) CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,(LPARAM)hIO->nlhrReply);
	if (hIO->lpErrorBuf) LocalFree(hIO->lpErrorBuf);
	if (InterlockedDecrement(&m_hNL->lRefCount) <= 0)
	{
		if (m_hNL->hNetlib) Netlib_CloseHandle (m_hNL->hNetlib);
		if (m_hNL->hCookies) {
			List_FreeElements(m_hNL->hCookies);
			List_Exit (m_hNL->hCookies);
		}
		if (m_hNL->hCookieStr) Fifo_Exit(m_hNL->hCookieStr);
		DeleteCriticalSection (&m_hNL->cs);
		free (m_hNL);
		m_hNL = NULL;
	}
	free (hIO);
}

// -----------------------------------------------------------------------------

static char *IoLayer_Post(IOLAYER *hPIO, char *pszURL, char *pszPostFields, unsigned int cbPostFields, unsigned int *pdwLength)
{
	IOLAYER_INST *hIO = (IOLAYER_INST*)hPIO;
	NETLIBHTTPREQUEST nlhr={0};
	NETLIBHTTPHEADER headers[5];
	int i;
	char *pszCookies = NULL;
	

	/*
	char szDbg[256];
	sprintf (szDbg, "Thread %d with hIO %08X requests %s\n", GetCurrentThreadId(), hIO, pszURL);
	OutputDebugString(szDbg);
	*/

	// Build basic request
	nlhr.cbSize = sizeof(nlhr);
	nlhr.flags = NLHRF_GENERATEHOST | NLHRF_SMARTREMOVEHOST | NLHRF_REDIRECT;
	if (!pdwLength) nlhr.flags |= NLHRF_DUMPASTEXT;	// pdwLength needed -> Binary data
	nlhr.requestType = pszPostFields?REQUEST_POST:REQUEST_GET;
	nlhr.szUrl = pszURL;
	nlhr.pData = pszPostFields;
	nlhr.dataLength = cbPostFields;
	nlhr.headers = headers;

	// Add headers
	EnterCriticalSection (&m_hNL->cs);
	if (m_hNL->pszCookies && *m_hNL->pszCookies)
	{
		headers[nlhr.headersCount].szName = "Cookie";
		headers[nlhr.headersCount++].szValue = pszCookies = strdup(m_hNL->pszCookies);
	}
	headers[nlhr.headersCount].szName = "User-Agent";
	headers[nlhr.headersCount++].szValue = USER_AGENT;
	headers[nlhr.headersCount].szName = "Accept-Encoding";
	headers[nlhr.headersCount++].szValue = "deflate, gzip";
	headers[nlhr.headersCount].szName = "Content-Type";
	headers[nlhr.headersCount++].szValue = "application/x-www-form-urlencoded; charset=UTF-8";
	headers[nlhr.headersCount].szName = "X-Requested-With";
	headers[nlhr.headersCount++].szValue = "XMLHttpRequest";
	LeaveCriticalSection (&m_hNL->cs);

	// Do the transaction
	nlhr.nlc = hIO->nlc = OpenConnection (&nlhr);
	if (hIO->nlhrReply) CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,(LPARAM)hIO->nlhrReply);
	hIO->nlhrReply = (NETLIBHTTPREQUEST*)CallService (MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNL->hNetlib, (LPARAM)&nlhr);
	hIO->nlc = NULL;

	if (pszCookies) free (pszCookies);
	if (!hIO->nlhrReply)
	{
		FetchLastError (hIO);
		return NULL;
	}

	if (!hIO->nlhrReply || !hIO->nlhrReply->dataLength || !hIO->nlhrReply->pData)
	{
		if (hIO->lpErrorBuf) LocalFree(hIO->lpErrorBuf);
		if (hIO->lpErrorBuf = LocalAlloc (LPTR, 64))
			strcpy (hIO->lpErrorBuf, "Reply contained no data");
		return NULL;
	}

	// Error handling
	if (hIO->nlhrReply->resultCode < 200 || hIO->nlhrReply->resultCode >= 300)
	{
		if (hIO->lpErrorBuf) LocalFree(hIO->lpErrorBuf);
		if (hIO->lpErrorBuf = LocalAlloc (LPTR, 64))
			sprintf (hIO->lpErrorBuf, "HTTP transaction returned status %d", hIO->nlhrReply->resultCode);
		return NULL;
	}

	// Process headers to collect cookies
	EnterCriticalSection (&m_hNL->cs);
	for (i=0; i<hIO->nlhrReply->headersCount; i++)
	{
		if (!strnicmp(hIO->nlhrReply->headers[i].szName, "Set-Cookie", 10))
			add_cookies(hIO->nlhrReply->headers[i].szValue);
	}
	LeaveCriticalSection (&m_hNL->cs);

	// Return reply
	if (pdwLength) *pdwLength = hIO->nlhrReply->dataLength;
	return hIO->nlhrReply->pData;
}

// -----------------------------------------------------------------------------

static char *IoLayer_Get(IOLAYER *hIO, char *pszURL, unsigned int *pdwLength)
{
	return IoLayer_Post (hIO, pszURL, NULL, 0, pdwLength);
}

// -----------------------------------------------------------------------------

static void IoLayer_Cancel(IOLAYER *hPIO)
{
	IOLAYER_INST *hIO = (IOLAYER_INST*)hPIO;

	if (hIO->nlc && Netlib_CloseHandle(hIO->nlc))
		hIO->nlc = NULL;
}

// -----------------------------------------------------------------------------

static char *IoLayer_GetLastError(IOLAYER *hIO)
{
	return (char*)((IOLAYER_INST*)hIO)->lpErrorBuf;
}

// -----------------------------------------------------------------------------

static char *IoLayer_EscapeString(IOLAYER *hPIO, char *pszData)
{
	return (char*)CallService (MS_NETLIB_URLENCODE, 0, (LPARAM)pszData);
}

// -----------------------------------------------------------------------------

static void IoLayer_FreeEscapeString(char *pszData)
{
	HeapFree(GetProcessHeap(), 0, pszData);
}

// -----------------------------------------------------------------------------
// Static
// -----------------------------------------------------------------------------

static void FetchLastError (IOLAYER_INST *hIO)
{
	if (hIO->lpErrorBuf) LocalFree(hIO->lpErrorBuf);
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
		GetLastError(), MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
		(LPTSTR)&hIO->lpErrorBuf, 0, NULL);
}

// -----------------------------------------------------------------------------

static void add_cookies(char *pszCookies)
{
	int i, nCount, iLenCmp, iLenCookie;
	char *p, *pszCookie = NULL;

	if (!(p = strchr(pszCookies, '='))) return;
	iLenCmp=p-pszCookies+1;
	if ((p=strchr (p, ';')) || (p=strchr (p, ';'))) iLenCookie = p-pszCookies;
	else iLenCookie=strlen(pszCookies);

	for (i=0, nCount=List_Count(m_hNL->hCookies); i<nCount; i++)
	{
		pszCookie = (char*)List_ElementAt (m_hNL->hCookies, i);
		if (!strncmp(pszCookie, pszCookies, iLenCmp))
			break;
	}
	
	if (i==nCount) pszCookie = NULL;
	if (pszCookie = realloc (pszCookie, iLenCookie+1))
	{
		strncpy (pszCookie, pszCookies, iLenCookie);
		pszCookie[iLenCookie]=0;
		if (i<nCount)
			List_ReplaceElementAt (m_hNL->hCookies, pszCookie, i);
		else
			List_Push (m_hNL->hCookies, pszCookie);
	}
	refresh_cookies ();
}

// -----------------------------------------------------------------------------

static void refresh_cookies()
{
	int i, nCount;

	Fifo_Reset (m_hNL->hCookieStr);
	for (i=0, nCount=List_Count(m_hNL->hCookies); i<nCount; i++)
	{
		char *pszCookie = (char*)List_ElementAt (m_hNL->hCookies, i);
		Fifo_Add (m_hNL->hCookieStr, pszCookie, strlen(pszCookie));
		Fifo_Add (m_hNL->hCookieStr, "; ", 2);
	}
	Fifo_Add (m_hNL->hCookieStr, "", 1);
	m_hNL->pszCookies = Fifo_Get (m_hNL->hCookieStr, NULL);
}

// -----------------------------------------------------------------------------

static HANDLE *OpenConnection(NETLIBHTTPREQUEST *nlhr)
{
	NETLIBOPENCONNECTION nloc={0};
	BOOL secur = (nlhr->flags & NLHRF_SSL) || _strnicmp(nlhr->szUrl, "https", 5) == 0;
	char* phost = strstr(nlhr->szUrl, "://");
	char *ppath, *pcolon;

	// Poor man's InternetCrackUrl
	nloc.cbSize = sizeof(NETLIBOPENCONNECTION);
	if (phost) phost+=3; else phost=nlhr->szUrl;
	nloc.szHost = _alloca (strlen(phost)+1);
	strcpy ((char*)nloc.szHost, phost);
	if (ppath = strchr(nloc.szHost, '/')) *ppath = '\0';
	if (pcolon = strrchr(nloc.szHost, ':')) 
	{
		*pcolon = '\0';
		nloc.wPort = (WORD)strtol(pcolon+1, NULL, 10);
	}
	else nloc.wPort = secur ? 443 : 80;
	nloc.flags = (secur ? NLOCF_SSL : 0) | NLOCF_HTTP;
	if (secur) nlhr->flags |= NLHRF_SSL; else nlhr->flags &= ~NLHRF_SSL;

	// Open connection
	return (HANDLE)CallService (MS_NETLIB_OPENCONNECTION, (WPARAM)m_hNL->hNetlib, (LPARAM)&nloc);
}