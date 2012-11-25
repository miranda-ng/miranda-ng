//fürs varaibles händling - dufte
#include "stdafx.h"

#include "variables.h"
#include <string>

using namespace std;

char* Varxfiregame(ARGUMENTSINFO *ai)
{
	if (ai->cbSize < sizeof(ARGUMENTSINFO)) 
		return NULL;

	if(ai->fi->hContact==NULL)
	{
		//
	}
	else
	{
		char temp[256];
		DBVARIANT dbv3;
		if(!DBGetContactSetting(ai->fi->hContact,protocolname, "RGame",&dbv3))
		{
			strncpy(temp,dbv3.pszVal,255);
			DBFreeVariant(&dbv3);
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
	if(!DBGetContactSetting(NULL,protocolname, "currentgamename",&dbv3))
	{
		char* ret=mir_strdup(dbv3.pszVal);
		DBFreeVariant(&dbv3);
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
	if(!DBGetContactSetting(ai->fi->hContact,protocolname, "RVoice",&dbv3))
	{
		char* ret=mir_strdup(dbv3.pszVal);
		DBFreeVariant(&dbv3);
		return ret;
	}

	ai->flags = AIF_FALSE;
	return mir_strdup("");
}


char* Varmyxfirevoiceip(ARGUMENTSINFO *ai) {
	if (ai->cbSize < sizeof(ARGUMENTSINFO)) 
		return NULL;

	DBVARIANT dbv3;
	if(!DBGetContactSetting(NULL,protocolname, "VServerIP",&dbv3))
	{
		char* ret=mir_strdup(dbv3.pszVal);
		DBFreeVariant(&dbv3);
		return ret;
	}
	
	ai->flags = AIF_FALSE;
	return mir_strdup("");
}

char* Varmyxfireserverip(ARGUMENTSINFO *ai) {
	if (ai->cbSize < sizeof(ARGUMENTSINFO)) 
		return NULL;

	DBVARIANT dbv3;
	if(!DBGetContactSetting(NULL,protocolname, "ServerIP",&dbv3))
	{
		char* ret=mir_strdup(dbv3.pszVal);
		DBFreeVariant(&dbv3);
		return ret;
	}
	
	ai->flags = AIF_FALSE;
	return mir_strdup("");
}

char* Varxfireserverip(ARGUMENTSINFO *ai) {
	if (ai->cbSize < sizeof(ARGUMENTSINFO)) 
		return NULL;

	if(ai->fi->hContact==NULL)
	{
		ai->flags = AIF_FALSE;
		return mir_strdup("");
	}
	else
	{
		char temp[24];
		DBVARIANT dbv3;
		if(!DBGetContactSetting(ai->fi->hContact,protocolname, "ServerIP",&dbv3))
		{
			sprintf(temp,"%s:%d",dbv3.pszVal,DBGetContactSettingWord(ai->fi->hContact,protocolname, "Port",0));
			DBFreeVariant(&dbv3);
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

	if(ai->fi->hContact==NULL)
	{
		ai->flags = AIF_FALSE;
		return mir_strdup("");
	}
	else
	{
		char temp[24];
		DBVARIANT dbv3;
		if(!DBGetContactSetting(ai->fi->hContact,protocolname, "VServerIP",&dbv3))
		{
			sprintf(temp,"%s:%d",dbv3.pszVal,DBGetContactSettingWord(ai->fi->hContact,protocolname, "VPort",0));
			DBFreeVariant(&dbv3);
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
	if(!DBGetContactSetting(NULL,protocolname, "currentvoicename",&dbv3))
	{
		char* ret=mir_strdup(dbv3.pszVal);
		DBFreeVariant(&dbv3);
		return ret;
	}

	return mir_strdup("");
}