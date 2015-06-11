#include "stdafx.h"

static int lua_FindFirstContact(lua_State *L)
{
	MCONTACT hContact = db_find_first(lua_tostring(L, 1));
	lua_pushinteger(L, hContact);

	return 1;
}

static int lua_FindNextContact(lua_State *L)
{
	MCONTACT hContact = db_find_next(luaL_checkinteger(L, 1), lua_tostring(L, 2));
	lua_pushinteger(L, hContact);

	return 1;
}

static int lua_WriteContactSetting(lua_State *L)
{
	MCONTACT hContact = lua_tointeger(L, 1);
	LPCSTR szModule = luaL_checkstring(L, 2);
	LPCSTR szSetting = luaL_checkstring(L, 3);

	DBVARIANT dbv = { 0 };
	int type = lua_type(L, 4);
	switch (type)
	{
	case LUA_TBOOLEAN:
		dbv.bVal = lua_toboolean(L, 4);
		dbv.type = DBVT_BYTE;
		break;
	case LUA_TNUMBER:
		dbv.dVal = lua_tonumber(L, 4);
		dbv.type = DBVT_DWORD;
		break;
	case LUA_TSTRING:
		dbv.pszVal = (char*)lua_tostring(L, 4);
		dbv.type = DBVT_UTF8;
		break;
	
	default:
		lua_pushinteger(L, hContact);
		return 1;
	}

	INT_PTR res = db_set(hContact, szModule, szSetting, &dbv);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_GetContactSetting(lua_State *L)
{
	MCONTACT hContact = lua_tointeger(L, 1);
	LPCSTR szModule = luaL_checkstring(L, 2);
	LPCSTR szSetting = luaL_checkstring(L, 3);

	DBVARIANT dbv;
	if (db_get(hContact, szModule, szSetting, &dbv))
	{
		lua_pushnil(L);
		return 1;
	}
	

	switch (dbv.type)
	{
	case DBVT_BYTE:
		lua_pushboolean(L, dbv.bVal);
		break;
	case DBVT_WORD:
		lua_pushinteger(L, dbv.wVal);
	case DBVT_DWORD:
		lua_pushnumber(L, dbv.dVal);
		break;
	case DBVT_ASCIIZ:
		lua_pushstring(L, ptrA(mir_utf8encode(dbv.pszVal)));
		break;
	case DBVT_UTF8:
		lua_pushstring(L, dbv.pszVal);
		break;
	case DBVT_WCHAR:
		lua_pushstring(L, ptrA(mir_utf8encodeW(dbv.pwszVal)));
		break;

	default:
		db_free(&dbv);
		lua_pushnil(L);
		return 1;
	}

	db_free(&dbv);

	return 1;
}

static int lua_DeleteContactSetting(lua_State *L)
{
	MCONTACT hContact = lua_tointeger(L, 1);
	LPCSTR szModule = luaL_checkstring(L, 2);
	LPCSTR szSetting = luaL_checkstring(L, 3);

	INT_PTR res = db_unset(hContact, szModule, szSetting);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_DeleteModule(lua_State *L)
{
	MCONTACT hContact = lua_tointeger(L, 1);
	LPCSTR szModule = luaL_checkstring(L, 2);

	INT_PTR res = ::CallService(MS_DB_MODULE_DELETE, 0, (LPARAM)szModule);
	lua_pushinteger(L, res);

	return 1;
}

static luaL_Reg databaseApi[] =
{
	{ "FindFirstContact", lua_FindFirstContact },
	{ "FindNextContact", lua_FindNextContact },

	{ "WriteContactSetting", lua_WriteContactSetting },
	{ "GetContactSetting", lua_GetContactSetting },
	{ "DeleteContactSetting", lua_DeleteContactSetting },

	{ "DeleteModule", lua_DeleteModule },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_database(lua_State *L)
{
	luaL_newlib(L, databaseApi);

	return 1;
}
