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

#include <assert.h>
#include "luainc.h"
#include "luascript.h"

#define LUA_CODE_CHUNK_ENTER(luaBridge) lua_State *state = (lua_State*)luaBridge;

#define LUA_CODE_CHUNK_LEAVE

CLuaScript::CLuaScript(CLuaBridge & luaBridge) : m_luaBridge(luaBridge), m_nMethods(DEFAULT_INDEX_INITIALIZER), m_nThisRef(DEFAULT_INDEX_INITIALIZER), m_nArgs(DEFAULT_INDEX_INITIALIZER)
{
LUA_CODE_CHUNK_ENTER(luaBridge)

	lua_newtable(state);
	m_nThisRef = luaL_ref(state, LUA_REGISTRYINDEX);

	CLuaStack luaStack(luaBridge);
	lua_rawgeti(state, LUA_REGISTRYINDEX, m_nThisRef);
	lua_pushlightuserdata(state, reinterpret_cast<void*>(this));
	lua_rawseti(state, -2, 0);

LUA_CODE_CHUNK_LEAVE
}

CLuaScript::~CLuaScript(void)
{
	CLuaStack luaStack(m_luaBridge);

LUA_CODE_CHUNK_ENTER(m_luaBridge)
	
	lua_rawgeti(state, LUA_REGISTRYINDEX, m_nThisRef);
	lua_pushnil(state);
	lua_rawseti(state, -2, 0);

LUA_CODE_CHUNK_LEAVE
}

int CLuaScript::LuaCallback(lua_State *lua)
{
	int iNumberIdx	 = lua_upvalueindex(1);
	int nRetsOnStack = 0;

	bool bRetVal = false;

	if (lua_istable(lua, 1))
	{
		lua_rawgeti(lua, 1, 0);

		if (lua_islightuserdata(lua, -1))
		{
			CLuaScript *pThis	= reinterpret_cast<CLuaScript*>(lua_touserdata(lua, -1));
			int			nMethod = static_cast<int>(lua_tonumber(lua, iNumberIdx));

			assert(!(nMethod > pThis->MethodsCount()));

			lua_remove(lua, 1); lua_remove(lua, -1);

			nRetsOnStack = pThis->ScriptCalling(pThis->Bridge(), nMethod);

			bRetVal = true;
		}
	}

	if (bRetVal == false)
	{
		lua_error(lua);
	}

	return nRetsOnStack;
}

bool CLuaScript::CompileScript(const char *szScript, size_t nLength)
{  
	CLuaTableThis luaThisTable(m_luaBridge, m_nThisRef);

	return m_luaBridge.RunScript(szScript, nLength);
}

int CLuaScript::RegisterFunction(const char *szFuncName)
{
	int nMethod = -1;

	CLuaStack luaStack(m_luaBridge);

LUA_CODE_CHUNK_ENTER(m_luaBridge)

	nMethod = ++m_nMethods;
	
	lua_rawgeti(state, LUA_REGISTRYINDEX, m_nThisRef);
	lua_pushstring(state, szFuncName);
	lua_pushnumber(state, (lua_Number)nMethod);
	lua_pushcclosure(state, CLuaScript::LuaCallback, 1);
	lua_settable(state, -3);

LUA_CODE_CHUNK_LEAVE

	return nMethod;
}

bool CLuaScript::SelectScriptFunction(const char *szFunction)
{
	bool bRetVal = true;

LUA_CODE_CHUNK_ENTER(m_luaBridge)

	lua_rawgeti(state, LUA_REGISTRYINDEX, m_nThisRef);
	lua_pushstring(state, szFunction);
	lua_rawget(state, -2);
	lua_remove(state, -2);

	lua_rawgeti(state, LUA_REGISTRYINDEX, m_nThisRef);

	if (!lua_isfunction(state, -2))
	{
		bRetVal = false;
		lua_pop(state, 2);
	}
	else
	{
		m_nArgs = 0;
		m_szFunction = szFunction;
	}

LUA_CODE_CHUNK_LEAVE
   
	return bRetVal;
}

bool CLuaScript::ScriptHasFunction(const char * szFunction)
{
	CLuaStack luaStack(m_luaBridge);

	bool bRetVal = false;

LUA_CODE_CHUNK_ENTER(m_luaBridge)

	lua_rawgeti(state, LUA_REGISTRYINDEX, m_nThisRef);
	lua_pushstring(state, szFunction);
	lua_rawget(state, -2);
	lua_remove(state, -2);

	bRetVal = (lua_isfunction(state, -1) != FALSE);

LUA_CODE_CHUNK_LEAVE

	return bRetVal;
}

void CLuaScript::AddParam(char * szValue)
{
LUA_CODE_CHUNK_ENTER(m_luaBridge)
	lua_pushstring(state, szValue); ++m_nArgs;
LUA_CODE_CHUNK_LEAVE
}

void CLuaScript::AddParam(int nValue)
{
LUA_CODE_CHUNK_ENTER(m_luaBridge)
	lua_pushnumber(state, (lua_Number)nValue); ++m_nArgs;
LUA_CODE_CHUNK_LEAVE
}

void CLuaScript::AddParam(float fValue)
{
LUA_CODE_CHUNK_ENTER(m_luaBridge)
	lua_pushnumber(state, (lua_Number)fValue); ++m_nArgs;
LUA_CODE_CHUNK_LEAVE
}

bool CLuaScript::Run(int nRetValues)
{
	bool bRetVal = m_luaBridge.ExecuteFunction((m_nArgs + 1), nRetValues);

	if (bRetVal == true && nRetValues > 0)
	{
		HandleReturns(m_luaBridge, m_szFunction);
		lua_pop(static_cast<lua_State*>(m_luaBridge), nRetValues);
	}

	return bRetVal;
}

CLuaBridge & CLuaScript::Bridge()
{ 
	return m_luaBridge; 
}

int CLuaScript::MethodsCount()
{
	return m_nMethods;
}