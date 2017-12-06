/*  
 * Skype Login
 * 
 * Based on:
 *   FakeSkype : Skype reverse engineering proof-of-concept client
 *               Ouanilo MEDEGAN (c) 2006   http://www.oklabs.net
 *   pyskype   : Skype login Python script by uunicorn
 *
 * Written by:   leecher@dose.0wnz.at (c) 2015 
 *
 * Module:       Main module which contains public functions of library
 *
 */
#ifdef _WIN32
#define EXPORT
#define strcasecmp stricmp
#endif
#include <time.h>
#include "common.h"
#include "login.h"
#include "platform.h"
#include "uic.h"
#include "objects.h"
#include "credentials.h"
#include "skylogin.h"

EXPORT SkyLogin SkyLogin_Init()
{
	Skype_Inst *pInst = calloc(1, sizeof(Skype_Inst));

#ifdef _WIN32
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD( 2, 2 );
	WSAStartup( wVersionRequested, &wsaData);
#endif

	if (pInst) InitInstance(pInst);
	return (SkyLogin*)pInst;
}

EXPORT void SkyLogin_Exit(SkyLogin pPInst)
{
	Skype_Inst *pInst = (Skype_Inst*)pPInst;
	if (pInst->LoginD.User) free(pInst->LoginD.User);
	if (pInst->LoginD.RSAKeys) RSA_free(pInst->LoginD.RSAKeys);
	if (pInst->LoginD.SignedCredentials.Memory) free(pInst->LoginD.SignedCredentials.Memory);
	free(pInst);
}

EXPORT void SkyLogin_SetLogFunction(SkyLogin pPInst, int (__cdecl *pfLog)(void *stream, const char *format, ...), void *pLogStream)
{
	Skype_Inst *pInst = (Skype_Inst*)pPInst;

	pInst->pfLog = pfLog;
	pInst->pLogStream = pLogStream;
}

EXPORT int SkyLogin_LoadCredentials(SkyLogin pPInst, char *User)
{
	Skype_Inst *pInst = (Skype_Inst*)pPInst;

	Memory_U creds = Credentials_Load(User);
	int ret = 0;

	if (creds.Memory)
	{
		// Credentials were found and loaded, now let's parse them.
		SResponse LoginDatas;

		if (Credentials_Read(pInst, creds, &LoginDatas) == 0)
		{
			// Credentials were successfully read :)
			// Now verify if they are still valid
			uint Idx;
			time_t t;

			for (Idx = 0, ret = 1; ret && Idx < LoginDatas.NbObj; Idx++)
			{
				switch (LoginDatas.Objs[Idx].Id)
				{
				case OBJ_ID_LDUSER:
					// Credentials for wrong user?
					ret = !strcasecmp((char*)LoginDatas.Objs[Idx].Value.Memory.Memory, User);
					if (pInst->LoginD.User) free(pInst->LoginD.User);
					pInst->LoginD.User = strdup((char*)LoginDatas.Objs[Idx].Value.Memory.Memory);
					break;
				case OBJ_ID_LDEXPIRY:
					// Credentials expired?
					ret = (int)LoginDatas.Objs[Idx].Value.Nbr * 60 > time(&t)-60;
					break;
				}
			}
			FreeResponse(&LoginDatas);
		}
		free(creds.Memory);
	}
	return ret; 
}

EXPORT int SkyLogin_PerformLogin(SkyLogin pPInst, char *User, char *Pass)
{
	int ret;
	Skype_Inst *pInst = (Skype_Inst*)pPInst;

	if ((ret = PerformLogin(pInst, User, Pass)) > 0)
	{
		// On successful login, save login datas
		Memory_U creds = Credentials_Write(pInst);
		if (creds.Memory)
		{
			Credentials_Save(creds, User);
			free (creds.Memory);
		}

		if (pInst->LoginD.User) free(pInst->LoginD.User);
		pInst->LoginD.User = strdup(User);
	}
	return ret;
}

EXPORT int SkyLogin_PerformLoginOAuth(SkyLogin pPInst, const char *OAuth)
{
	int ret;
	Skype_Inst *pInst = (Skype_Inst*)pPInst;

	if ((ret = PerformLogin(pInst, OAuth, NULL)) > 0)
	{
		// On successful login, save login datas
		Memory_U creds = Credentials_Write(pInst);
		if (creds.Memory)
		{
			SResponse LoginDatas;

			// We don't know user name, so read it from Credentials
			if (Credentials_Parse(pInst, pInst->LoginD.SignedCredentials, &LoginDatas) == 0)
			{
				uint Idx;

				for (Idx = 0; Idx < LoginDatas.NbObj; Idx++)
				{ 
					if (LoginDatas.Objs[Idx].Id == OBJ_ID_LDUSER)
					{
						Credentials_Save(creds, (char*)LoginDatas.Objs[Idx].Value.Memory.Memory);
						if (pInst->LoginD.User) free(pInst->LoginD.User);
						pInst->LoginD.User = (uchar*)strdup((char*)LoginDatas.Objs[Idx].Value.Memory.Memory);
						break;
					}
				}
				FreeResponse(&LoginDatas);
			}
			free (creds.Memory);
		}
	}
	return ret;
}


EXPORT int SkyLogin_CreateUICString(SkyLogin pInst, const char *pszNonce, char *pszOutUIC)
{
	return CreateUICString((Skype_Inst*)pInst, pszNonce, "WS-SecureConversationSESSION KEY TOKEN", pszOutUIC);
}

EXPORT int SkyLogin_GetCredentialsUIC(SkyLogin pInst, char *pszOutUIC)
{
	return GetCredentialsUIC((Skype_Inst*)pInst, pszOutUIC);
}

EXPORT char *SkyLogin_GetUser(SkyLogin pInst)
{
	return (char*)((Skype_Inst*)pInst)->LoginD.User;
}
