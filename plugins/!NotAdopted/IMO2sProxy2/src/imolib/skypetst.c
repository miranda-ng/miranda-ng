/* Module:  skypetst.c
   Purpose: Very simple and straight forward testing application for Skype imo.im service
   Author:  leecher
   Date:    30.08.2009
*/

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif
#include "imo_skype.h"

static IMOSKYPE *m_hInst = NULL;

// -----------------------------------------------------------------------------
// Static
// -----------------------------------------------------------------------------
static int StatusCallback (cJSON *pMsg, void *pUser)
{
	char *pszName;
	cJSON *pContent = cJSON_GetArrayItem(pMsg, 0);

	if (!pContent) return;
	pszName = cJSON_GetObjectItem(pContent,"name")->valuestring;

	if (!strcmp(pszName, "recv_im"))
	{
		// I got a message!
		cJSON *pEdata = cJSON_GetObjectItem(pContent,"edata");

		if (pEdata)
		{
			time_t timestamp = (time_t)cJSON_GetObjectItem(pEdata, "timestamp")->valueint;
			char szTimestamp[32];

			strftime (szTimestamp, sizeof(szTimestamp), "%d.%m.%Y %H.%M.%S", localtime(&timestamp));
			printf ("[%s] %s: %s\n", szTimestamp, 
				cJSON_GetObjectItem(pEdata, "alias")->valuestring,
				cJSON_GetObjectItem(pEdata, "msg")->valuestring);
		}
	}
	else if (!strcmp(pszName, "signed_on"))
	{
		// I just signed on. 
		cJSON *pEdata = cJSON_GetObjectItem(pContent,"edata");
		char *pszAlias;

		if (pEdata && (pszAlias = cJSON_GetObjectItem(pEdata, "alias")->valuestring))
		{
			printf ("My alias is: %s\n", pszAlias);
		}
	}
	else if (!strcmp(pszName, "buddy_added") || !strcmp(pszName, "buddy_status"))
	{
		// Here comes the contact list
		cJSON *pArray = cJSON_GetObjectItem(pContent,"edata"), *pItem;
		int i, iCount;

		if (pArray)
		{
			for (i=0, iCount = cJSON_GetArraySize(pArray); i<iCount; i++)
			{
				if (pItem = cJSON_GetArrayItem(pArray, i))
				{
					printf ("%s (%s)\t-\t%s\n",
						cJSON_GetObjectItem(pItem, "alias")->valuestring,
						cJSON_GetObjectItem(pItem, "buid")->valuestring,
						cJSON_GetObjectItem(pItem, "primitive")->valuestring);
				}
			}
		}
	}
	else
	{
		char *pszMsg = cJSON_Print(pMsg);
		fprintf (stderr, "%s\n\n", pszMsg);
		free (pszMsg);
	}

	return 0;
}

// -----------------------------------------------------------------------------

static void DispatcherThread(void *pDummy)
{
	while (1)
		ImoSkype_Poll(m_hInst);
}

// -----------------------------------------------------------------------------

#ifdef WIN32
static HANDLE m_hThread;

static int Dispatcher_Start(void)
{
	DWORD ThreadID;

    return (m_hThread=CreateThread(NULL, 0, 
		(LPTHREAD_START_ROUTINE)DispatcherThread, NULL, 0, &ThreadID))!=0; 
	
}

static int Dispatcher_Stop(void)
{
	return TerminateThread (m_hThread, 0);
}

#else
static pthread_t m_hThread;

static int Dispatcher_Start(void)
{
	return pthread_create(&m_hThread, NULL, DispatcherThread, NULL)==0;
}

static int Dispatcher_Stop(void)
{
	if (pthread_cancel(m_hThread))
	{
		m_hThread=0;
		return 1;
	}
	return 0;
}
#endif

// -----------------------------------------------------------------------------
// EIP
// -----------------------------------------------------------------------------

int main(int argc, char **argv)
{
	int iRet = -1;

	if (argc<3)
	{
		printf ("Usage: %s [username] [password]\n", argv[0]);
		return -1;
	}

	/* Logon */
	if (!(m_hInst = ImoSkype_Init(StatusCallback, NULL)))
	{
		fprintf (stderr, "Initializing failed.\n");
		return -1;
	}
	if (ImoSkype_Login(m_hInst, argv[1], argv[2]) == 1)
	{
		/* Dispatch loop */
		char szLine[4096]={0};

		Dispatcher_Start();
		printf ("> ");
		fflush(stdout);
		while (gets(szLine))
		{
			if (strcmp(szLine, "exit") == 0) break;
			if (strcmp(szLine, "help") == 0)
			{
				printf ("msg [buddy] [message]\nstatus [available|away|busy|invisible] <Statusmessage>\nexit\n");
			} else
			if (strncmp(szLine, "msg ", 4) == 0)
			{
				char *pszBuddy;

				if (pszBuddy = strtok (szLine+4, " "))
				{
					if (ImoSkype_SendMessage(m_hInst, pszBuddy, pszBuddy+strlen(pszBuddy)+1))
						printf ("Sent.\n");
					else
						printf ("Sending failed: %s.\n", ImoSkype_GetLastError(m_hInst));
				}
			} else
			if (strncmp(szLine, "status ", 7) == 0)
			{
				char *pszStatus;

				if (pszStatus = strtok (szLine+7, " "))
				{
					if (ImoSkype_SetStatus(m_hInst, pszStatus, pszStatus+strlen(pszStatus)+1))
						printf ("Status set.\n");
					else
						printf ("Setting status failed: %s.\n", ImoSkype_GetLastError(m_hInst));
				}
			}
			memset (szLine, 0, sizeof(szLine));
			printf ("> ");
			fflush(stdout);
		}
		Dispatcher_Stop();
		ImoSkype_Logout(m_hInst);
	}
	else
	{
		fprintf (stderr, "Login failed: %s\n", ImoSkype_GetLastError(m_hInst));
		return -1;
	}

	ImoSkype_Exit(m_hInst);
	return iRet;
}
