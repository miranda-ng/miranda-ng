/* Module:  imo2stest.c
   Purpose: Simple test application for imo2skypeapi
   Author:  leecher
   Date:    31.08.2009
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "imo2skypeapi.h"

// -----------------------------------------------------------------------------
// Static
// -----------------------------------------------------------------------------
static void Recv(char *pszMsg, void *pUser)
{
	printf ("< %s\n", pszMsg);
}

// -----------------------------------------------------------------------------
// EIP
// -----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	int iRet = -1;
	IMOSAPI *pInst;
	char *pszError = NULL;

	if (argc<3)
	{
		printf ("Usage: %s [username] [password]\n", argv[0]);
		return -1;
	}

	if (!(pInst = Imo2S_Init(Recv, NULL,0)))
	{
		fprintf (stderr, "Failed initialization.\n");
		return -1;
	}

	/* Logon */
	if (Imo2S_Login(pInst, argv[1], argv[2], &pszError) == 1)
	{
		/* Dispatch loop */
		char szLine[4096]={0};

		while (gets(szLine))
		{
			Imo2S_Send (pInst, szLine);
			memset (szLine, 0, sizeof(szLine));
		}
	}
	else
	{
		fprintf (stderr, "Login failed: %s\n", pszError);
		return -1;
	}

	Imo2S_Exit (pInst);
	return iRet;
}
