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
#include "LuaBridge.h"

CLuaBridge::CLuaBridge(void) : m_pLuaContext(NULL)
{
	m_pLuaContext = lua_open();

	if (m_pLuaContext != NULL)
	{
		lua_pushcclosure(m_pLuaContext, LuaDebugMessage, 0);
		lua_setglobal(m_pLuaContext, "trace");

		lua_atpanic(m_pLuaContext, (lua_CFunction)HandleLuaAsssert);

		luaopen_io(m_pLuaContext);
		luaopen_math(m_pLuaContext);
		luaopen_base(m_pLuaContext);
		luaopen_loadlib(m_pLuaContext);
		luaopen_table(m_pLuaContext);
		luaopen_string(m_pLuaContext);
		luaopen_debug(m_pLuaContext);		
	}
}

CLuaBridge::~CLuaBridge(void)
{
	if (m_pLuaContext != NULL)
	{
		lua_close(m_pLuaContext);
	}
}

int CLuaBridge::LuaDebugMessage(lua_State *pContext)
{
	const char *szFunction = NULL;
	const char *szMessage  = NULL;
	lua_Debug	LuaDebug   = {0};

	int n = lua_tonumber(pContext, 1);
	szMessage = lua_tostring(pContext, 1);

	if (szMessage == NULL)
	{
		return FALSE;
	}
	
	lua_getstack(pContext, 1, &LuaDebug);
	lua_getinfo (pContext, "Snl", &LuaDebug);
	
	szFunction = LuaDebug.source;

	if (szFunction == NULL)
	{
		return FALSE;
	}

	OutputDebugStringA("Smart Auto Replier : ");
	OutputDebugStringA("(");
	OutputDebugStringA(szFunction);
	OutputDebugStringA("): ");
	OutputDebugStringA(szMessage);	
	OutputDebugStringA("\r\n");

	return TRUE;
}

void CLuaBridge::HandleLuaAsssert(lua_State * pContext)
{
	DebugBreak();
}

bool CLuaBridge::RunScript(const char *szScript, size_t nLength, const char *szFuncName)
{
	bool bRetVal = false;

	if (szFuncName == NULL)
	{
		szFuncName = "SarFakeProcedure";
	}

	if (luaL_loadbuffer(m_pLuaContext, szScript, nLength, szFuncName) == 0)
	{
		bRetVal = (lua_pcall(m_pLuaContext, 0, LUA_MULTRET, 0) == 0);
	}

	return bRetVal;
}

bool CLuaBridge::ExecuteFunction(int nArgs, int nReturns)
{
	bool bRetVal = false;

	if (lua_isfunction(m_pLuaContext, -nArgs - 1))
	{
		bRetVal = (lua_pcall(m_pLuaContext, nArgs, nReturns, 0) == FALSE);
	}

	return bRetVal;
}

CLuaBridge::operator lua_State *()
{ 
	return m_pLuaContext; 
}