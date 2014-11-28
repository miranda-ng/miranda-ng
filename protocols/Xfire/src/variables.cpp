//fürs varaibles händling - dufte
#include "stdafx.h"

#include "variables.h"
#include <string>

extern HANDLE XFireWorkingFolder, XFireIconFolder, XFireAvatarFolder;

using namespace std;

char* Varxfiregame(ARGUMENTSINFO *ai)
{
	if (ai->cbSize < sizeof(ARGUMENTSINFO))
		return NULL;

	if (ai->fi->hContact == NULL)
	{
		//
	}
	else
	{
		char temp[256];
		DBVARIANT dbv3;
		if (!db_get(ai->fi->hContact, protocolname, "RGame", &dbv3))
		{
			strncpy(temp, dbv3.pszVal, 255);
			db_free(&dbv3);
			return mir_strdup(temp);
		}
	}

	ai->flags = AIF_FALSE;
	return mir_strdup("");
}

char* Varmyxfiregame(ARGUMENTSINFO *ai)
{
	if (ai->cbSize < sizeof(ARGUMENTSINFO))
		return NULL;

	DBVARIANT dbv3;
	if (!db_get(NULL, protocolname, "currentgamename", &dbv3))
	{
		char* ret = mir_strdup(dbv3.pszVal);
		db_free(&dbv3);
		return ret;
	}
	ai->flags = AIF_FALSE;
	return mir_strdup("");
}

char* Varxfirevoice(ARGUMENTSINFO *ai)
{
	if (ai->cbSize < sizeof(ARGUMENTSINFO))
		return NULL;

	DBVARIANT dbv3;
	if (!db_get(ai->fi->hContact, protocolname, "RVoice", &dbv3))
	{
		char* ret = mir_strdup(dbv3.pszVal);
		db_free(&dbv3);
		return ret;
	}

	ai->flags = AIF_FALSE;
	return mir_strdup("");
}


char* Varmyxfirevoiceip(ARGUMENTSINFO *ai) {
	if (ai->cbSize < sizeof(ARGUMENTSINFO))
		return NULL;

	DBVARIANT dbv3;
	if (!db_get(NULL, protocolname, "VServerIP", &dbv3))
	{
		char* ret = mir_strdup(dbv3.pszVal);
		db_free(&dbv3);
		return ret;
	}

	ai->flags = AIF_FALSE;
	return mir_strdup("");
}

char* Varmyxfireserverip(ARGUMENTSINFO *ai) {
	if (ai->cbSize < sizeof(ARGUMENTSINFO))
		return NULL;

	DBVARIANT dbv3;
	if (!db_get(NULL, protocolname, "ServerIP", &dbv3))
	{
		char* ret = mir_strdup(dbv3.pszVal);
		db_free(&dbv3);
		return ret;
	}

	ai->flags = AIF_FALSE;
	return mir_strdup("");
}

char* Varxfireserverip(ARGUMENTSINFO *ai) {
	if (ai->cbSize < sizeof(ARGUMENTSINFO))
		return NULL;

	if (ai->fi->hContact == NULL)
	{
		ai->flags = AIF_FALSE;
		return mir_strdup("");
	}
	else
	{
		char temp[24];
		DBVARIANT dbv3;
		if (!db_get(ai->fi->hContact, protocolname, "ServerIP", &dbv3))
		{
			mir_snprintf(temp, SIZEOF(temp), "%s:%d", dbv3.pszVal, db_get_w(ai->fi->hContact, protocolname, "Port", 0));
			db_free(&dbv3);
			return mir_strdup(temp);
		}
		ai->flags = AIF_FALSE;
		return mir_strdup("");
	}

	ai->flags = AIF_FALSE;
	return mir_strdup("");
}

char* Varxfirevoiceip(ARGUMENTSINFO *ai) {
	if (ai->cbSize < sizeof(ARGUMENTSINFO))
		return NULL;

	if (ai->fi->hContact == NULL)
	{
		ai->flags = AIF_FALSE;
		return mir_strdup("");
	}
	else
	{
		char temp[24];
		DBVARIANT dbv3;
		if (!db_get(ai->fi->hContact, protocolname, "VServerIP", &dbv3))
		{
			mir_snprintf(temp, SIZEOF(temp), "%s:%d", dbv3.pszVal, db_get_w(ai->fi->hContact, protocolname, "VPort", 0));
			db_free(&dbv3);
			return mir_strdup(temp);
		}
		ai->flags = AIF_FALSE;
		return mir_strdup("");
	}

	ai->flags = AIF_FALSE;
	return mir_strdup("");
}

char* Varmyxfirevoice(ARGUMENTSINFO *ai)
{
	if (ai->cbSize < sizeof(ARGUMENTSINFO))
		return NULL;

	DBVARIANT dbv3;
	if (!db_get(NULL, protocolname, "currentvoicename", &dbv3))
	{
		char* ret = mir_strdup(dbv3.pszVal);
		db_free(&dbv3);
		return ret;
	}

	return mir_strdup("");
}

char* XFireGetFoldersPath(char * pathtype)
{// Get XFire folder path
	static char path[1024]; path[0] = 0;
	if (ServiceExists(MS_FOLDERS_REGISTER_PATH)){
		if (!strcmp(pathtype, "Avatar")){
			FoldersGetCustomPath(XFireAvatarFolder, path, 1024, "");
		}
		if (!strcmp(pathtype, "IniFile")){
			FoldersGetCustomPath(XFireWorkingFolder, path, 1024, "");
		}
		if (!strcmp(pathtype, "IconsFile")){
			FoldersGetCustomPath(XFireIconFolder, path, 1024, "");
		}
		strcat(path, "\\");
		return path;
	}
	else {
		/******BASE********/
		char BaseFolder[MAX_PATH] = "";
		char CurProfileF[MAX_PATH] = "";
		char CurProfile[MAX_PATH] = "";
		CallService(MS_DB_GETPROFILEPATH, (WPARAM)MAX_PATH, (LPARAM)BaseFolder);
		strcat(BaseFolder, "\\");
		CallService(MS_DB_GETPROFILENAME, (WPARAM)MAX_PATH, (LPARAM)CurProfileF);
		int i;
		for (i = MAX_PATH; 5; i--){
			if (CurProfileF[i] == 't' && CurProfileF[i - 3] == '.'){
				i = i - 3;
				break;
			}
		}
		memcpy(CurProfile, CurProfileF, i);
		strcat(BaseFolder, CurProfile);
		strcat(BaseFolder, "\\");
		strcat(BaseFolder, "XFire");
		strcat(BaseFolder, "\\");
		/*******BASE********/
		if (!strcmp(pathtype, "Avatar")){
			strcat(BaseFolder, "Avatars");
			strcat(BaseFolder, "\\");
		}
		strcat(path, BaseFolder);
	}
	return path;
}
