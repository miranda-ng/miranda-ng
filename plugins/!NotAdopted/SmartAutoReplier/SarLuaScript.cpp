/*
 *  Smart Auto Replier (SAR) - auto replier plugin for Miranda IM
 *
 *  Copyright (C) 2004 - 2012 by Volodymyr M. Shcherbyna <volodymyr@shcherbyna.com>
 *
 *  This code is inspired by article of RichardS at http://www.codeproject.com/Articles/11508/Integrating-Lua-into-C
 *
 *      This file is part of SAR.
 *
 *  SAR is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SAR is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SAR.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "SarLuaScript.h"

#include <m_protomod.h>
#include <m_protosvc.h>

extern INT g_nCurrentMode;
extern CMessagesHandler *g_pMessHandler;

wchar_t* Utf8toUtf16(CHAR * szStrIn, UINT & nSize);
char* Utf16toUtf8(LPCWSTR lpwszStrIn, UINT & nSize);

CSarLuaScript::CSarLuaScript(CLuaBridge & luaBridge) : CLuaScript(luaBridge)
{
	m_nFuncBaseIndex = RegisterFunction("SendMessage");

	RegisterFunction("GetMyStatus");
	RegisterFunction("SetMyStatus");
	RegisterFunction("Wait");
	RegisterFunction("FindUser");
	RegisterFunction("SetVariable");
	RegisterFunction("GetVariable");
}

CSarLuaScript::~CSarLuaScript(void)
{
}

int CSarLuaScript::ScriptCalling(CLuaBridge & luaBridge, int nFncNumber)
{
	switch (nFncNumber - m_nFuncBaseIndex)
	{
	case 0:
		return SendMessage(luaBridge);

	case 1:
		return GetMyStatus(luaBridge);

	case 2:
		return SetMyStatus(luaBridge);

	case 3:
		return Wait(luaBridge);

	case 4:
		return FindUser(luaBridge);

	case 5:
		return SetVariable(luaBridge);

	case 6:
		return GetVariable(luaBridge);
	}

	return FALSE;
}

int CSarLuaScript::SendMessage(CLuaBridge & luaBridge)
{
    lua_State *pFunctionContext = (lua_State*)luaBridge;

	UINT nMessageLen		= lua_strlen(pFunctionContext, -1);
	const char * szMessage	= lua_tostring(pFunctionContext, -1);
	int hUserToken			= (int)lua_tonumber(pFunctionContext, -2);

	if (szMessage == NULL)
	{
		return FALSE;
	}

	int nRetVal = 0;

#ifdef _UNICODE
	/*/// as I am in unicode, I have to convert all strings from utf8 to utf16 :)
	wchar_t * szwMessage = Utf8toUtf16((char*)szMessage, nMessageLen);*/

	nRetVal = CallContactService((HANDLE)hUserToken, PSS_MESSAGE, PREF_UTF, reinterpret_cast<LPARAM>(szMessage));

	/*free(szwMessage);*/
#else
	nRetVal = CallContactService((HANDLE)hUserToken, PSS_MESSAGE, 0, reinterpret_cast<LPARAM>(szMessage));
#endif	
				
    return FALSE;
}

int CSarLuaScript::GetMyStatus(CLuaBridge & luaBridge)
{
	lua_State *pFunctionContext = (lua_State*)luaBridge;
	int nStatusMode = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);

	nStatusMode = g_nCurrentMode;

	switch (nStatusMode)
	{
	case ID_STATUS_ONLINE:
		AddParam("Online");
		break;

	case ID_STATUS_OFFLINE:
		AddParam("Offline");
		break;

	case ID_STATUS_AWAY:
		AddParam("Away");
		break;

	case ID_STATUS_DND:
		AddParam("DND");
		break;

	case ID_STATUS_NA:
		AddParam("NA");
		break;

	case ID_STATUS_OCCUPIED:
		AddParam("Occupied");
		break;

	case ID_STATUS_FREECHAT:
		AddParam("FreeChat");
		break;

	case ID_STATUS_INVISIBLE:
		AddParam("Invisible");
		break;

	case ID_STATUS_ONTHEPHONE:
		AddParam("OnThePhone");
		break;

	case ID_STATUS_OUTTOLUNCH:
		AddParam("OutToLunch");
		break;

	default:
		AddParam("Unknown");
		break;
	}

	return TRUE;
}

int CSarLuaScript::SetMyStatus(CLuaBridge & luaBridge)
{
	int nNewStatus = 0;
	lua_State *pFunctionContext = (lua_State*)luaBridge;

	const char * szStatus = lua_tostring(pFunctionContext, -1);

	if (szStatus == NULL)
	{
		return FALSE;
	}

	if (strcmp(szStatus, "Online") == 0)
	{
		nNewStatus = ID_STATUS_ONLINE;
	}
	else if (strcmp(szStatus, "Offline") == 0)
	{
		nNewStatus = ID_STATUS_OFFLINE;
	}
	else if (strcmp(szStatus, "Away") == 0)
	{
		nNewStatus = ID_STATUS_AWAY;
	}
	else if (strcmp(szStatus, "DND") == 0)
	{
		nNewStatus = ID_STATUS_DND;
	}
	else if (strcmp(szStatus, "NA") == 0)
	{
		nNewStatus = ID_STATUS_NA;
	}
	else if (strcmp(szStatus, "Occupied") == 0)
	{
		nNewStatus = ID_STATUS_OCCUPIED;
	}
	else if (strcmp(szStatus, "FreeChat") == 0)
	{
		nNewStatus = ID_STATUS_FREECHAT;
	}
	else if (strcmp(szStatus, "Invisible") == 0)
	{
		nNewStatus = ID_STATUS_INVISIBLE;
	}
	else if (strcmp(szStatus, "OnThePhone") == 0)
	{
		nNewStatus = ID_STATUS_ONTHEPHONE;
	}
	else if (strcmp(szStatus, "OutToLunch") == 0)
	{
		nNewStatus = ID_STATUS_OUTTOLUNCH;
	}

	CallService(MS_CLIST_SETSTATUSMODE, (WPARAM)nNewStatus, 0);

	return FALSE;
}

typedef map<wstring, wstring> MapOfStrings;

MapOfStrings g_MapOfVariables;

int CSarLuaScript::SetVariable(CLuaBridge & luaBridge)
{
	lua_State *pFunctionContext = (lua_State*)luaBridge;

	UINT nNameLength		= lua_strlen(pFunctionContext, -2);
	const char * szName		= lua_tostring(pFunctionContext, -2);

	UINT nVariableLength	= lua_strlen(pFunctionContext, -1);
	const char * szVariable	= lua_tostring(pFunctionContext, -1);

	if (szName == NULL || szVariable == NULL)
	{
		return FALSE;
	}
	
	wchar_t * szwName		= Utf8toUtf16((char*)szName, nNameLength);
	wchar_t * szwVariable	= Utf8toUtf16((char*)szVariable, nVariableLength);

	if (szwName && szwVariable)
	{
		g_MapOfVariables[szwName] = szwVariable;
	}

	if (szwName)
	{
		free(szwName);
	}

	if (szwVariable)
	{
		free(szwVariable);
	}

	return FALSE;
}

int CSarLuaScript::GetVariable(CLuaBridge & luaBridge)
{
	lua_State *pFunctionContext = (lua_State*)luaBridge;

	UINT nNameLength		= lua_strlen(pFunctionContext, -1);
	const char * szName		= lua_tostring(pFunctionContext, -1);

	if (szName == NULL)
	{
		return FALSE;
	}
	
	wchar_t * szwName		= Utf8toUtf16((char*)szName, nNameLength);

	if (szwName)
	{
		MapOfStrings::iterator it = g_MapOfVariables.find(szwName);

		if (it != g_MapOfVariables.end())
		{
			UINT nSize	 = 0;
			char * szUtf = Utf16toUtf8(it->second.c_str(), nSize);

			if (szUtf != NULL)
			{
				AddParam(szUtf);
				free(szUtf);
				free(szwName);

				return TRUE;
			}
		}
	}

	if (szwName)
	{
		free(szwName);
	}	

	return FALSE;
}

int CSarLuaScript::Wait(CLuaBridge & luaBridge)
{
	lua_State *pFunctionContext = (lua_State*)luaBridge;

	int nSleepInterval = (int)lua_tonumber(pFunctionContext, -1);

	Sleep(1000 * nSleepInterval);

	return FALSE;
}

int CSarLuaScript::FindUser(CLuaBridge & luaBridge)
{
	lua_State *pFunctionContext = (lua_State*)luaBridge;

	const CHAR * szUser		= (CHAR*)lua_tostring(pFunctionContext, -2);
	const CHAR * szProtocol = (CHAR*)lua_tostring(pFunctionContext, -1);	

	if (szProtocol == NULL || szUser == NULL)
	{
		return FALSE;
	}

	HANDLE	hContact		= db_find_first();
	CHAR*	szContactName	= NULL;
	CHAR*	szProto			= NULL;
	DWORD	wId				= 0;

	while (hContact != NULL)
	{
		szContactName = reinterpret_cast<CHAR*>(CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_NOMYHANDLE));
		szProto		  = (CHAR*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

		if (szProto && szContactName)
		{
			if ( (strcmp(szUser, szContactName)) == 0 && (strcmp(szProto, szProtocol) == 0))
			{
				AddParam((int)hContact);
				return TRUE;
			}
		}

		hContact = db_find_next(hContact);
	}

	return FALSE;
}

void CSarLuaScript::HandleReturns(CLuaBridge & luaBridge, const char *szFunc)
{

}