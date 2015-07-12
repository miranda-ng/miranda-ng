#include "stdafx.h"

static int lua_FindFirstContact(lua_State *L)
{
	const char *szProto = lua_tostring(L, 1);

	MCONTACT res = db_find_first(szProto);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_FindNextContact(lua_State *L)
{
	MCONTACT hContact = lua_tointeger(L, 1);
	const char *szProto = lua_tostring(L, 2);

	MCONTACT res = db_find_next(hContact, szProto);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_GetEventCount(lua_State *L)
{
	MCONTACT hContact = lua_tointeger(L, 1);

	int res = ::db_event_count(hContact);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_GetFirstEvent(lua_State *L)
{
	MCONTACT hContact = lua_tointeger(L, 1);

	MEVENT res = ::db_event_first(hContact);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_GetPrevEvent(lua_State *L)
{
	MCONTACT hContact = lua_tointeger(L, 1);
	MEVENT hEvent = lua_tointeger(L, 2);

	MEVENT res = ::db_event_prev(hContact, hEvent);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_GetNextEvent(lua_State *L)
{
	MCONTACT hContact = lua_tointeger(L, 1);
	MEVENT hEvent = lua_tointeger(L, 2);

	MEVENT res = ::db_event_next(hContact, hEvent);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_GetLastEvent(lua_State *L)
{
	MCONTACT hContact = lua_tointeger(L, 1);

	MEVENT res = ::db_event_last(hContact);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_GetEvent(lua_State *L)
{
	MEVENT hEvent = lua_tointeger(L, 1);

	DBEVENTINFO dbei = { sizeof(DBEVENTINFO) };
	dbei.cbBlob = db_event_getBlobSize(hEvent);
	dbei.pBlob = (PBYTE)mir_calloc(dbei.cbBlob);

	int res = ::db_event_get(hEvent, &dbei);
	if (res)
	{
		lua_pushnil(L);
		return 1;
	}

	lua_newtable(L);
	lua_pushstring(L, "Module");
	lua_pushstring(L, ptrA(mir_utf8encode(dbei.szModule)));
	lua_settable(L, -3);
	lua_pushstring(L, "Timestamp");
	lua_pushnumber(L, dbei.timestamp);
	lua_settable(L, -3);
	lua_pushstring(L, "Type");
	lua_pushinteger(L, dbei.eventType);
	lua_settable(L, -3);
	lua_pushstring(L, "Flags");
	lua_pushinteger(L, dbei.flags);
	lua_settable(L, -3);
	lua_pushstring(L, "Length");
	lua_pushnumber(L, dbei.cbBlob);
	lua_settable(L, -3);
	lua_pushstring(L, "Blob");
	lua_newtable(L);
	for (DWORD i = 0; i < dbei.cbBlob; i++)
	{
		lua_pushinteger(L, i + 1);
		lua_pushinteger(L, dbei.pBlob[i]);
		lua_settable(L, -3);
	}
	lua_settable(L, -3);

	mir_free(dbei.pBlob);

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
		lua_pushinteger(L, 1);
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
		lua_pushvalue(L, 4);
		return 1;
	}

	switch (dbv.type)
	{
	case DBVT_BYTE:
		lua_pushinteger(L, dbv.bVal);
		break;
	case DBVT_WORD:
		lua_pushinteger(L, dbv.wVal);
		break;
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
		lua_pushvalue(L, 4);
		return 1;
	}

	db_free(&dbv);

	return 1;
}

typedef struct
{
	int  arrlen;
	char **pszSettingName;
}
enumDBSettingsParam;

static int SettingsEnumProc(const char* szSetting, LPARAM lParam)
{
	if (szSetting)
	{
		enumDBSettingsParam* p = (enumDBSettingsParam*)lParam;

		p->arrlen++;
		p->pszSettingName = (char**)mir_realloc(p->pszSettingName, p->arrlen * sizeof(char*));
		p->pszSettingName[p->arrlen - 1] = mir_strdup(szSetting);
	}
	return 0;
}

static int lua_EnumSettings(lua_State *L)
{
	MCONTACT hContact = lua_tointeger(L, 1);
	LPCSTR szModule = luaL_checkstring(L, 2);

	if (!lua_isfunction(L, 3))
	{
		lua_pushlightuserdata(L, NULL);
		return 1;
	}

	lua_pushvalue(L, 3);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	enumDBSettingsParam param = { 0, NULL };

	DBCONTACTENUMSETTINGS dbces = { 0 };
	dbces.pfnEnumProc = SettingsEnumProc;
	dbces.szModule = szModule;
	dbces.ofsSettings = 0;
	dbces.lParam = (LPARAM)&param;
	INT_PTR res = ::CallService(MS_DB_CONTACT_ENUMSETTINGS, hContact, (LPARAM)&dbces);

	for (int i = 0; i < param.arrlen; i++)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
		lua_pushstring(L, mir_utf8encode(param.pszSettingName[i]));
		if (lua_pcall(L, 1, 0, 0))
			CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)lua_tostring(L, -1));

		mir_free(param.pszSettingName[i]);
	}
	mir_free(param.pszSettingName);

	luaL_unref(L, LUA_REGISTRYINDEX, ref);
	lua_pushinteger(L, res);

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

	INT_PTR res = ::CallService(MS_DB_MODULE_DELETE, hContact, (LPARAM)szModule);
	lua_pushinteger(L, res);

	return 1;
}

static int SettingsChangedHookEventObjParam(void *obj, WPARAM wParam, LPARAM lParam, LPARAM param)
{
	lua_State *L = (lua_State*)obj;

	int ref = param;
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

	lua_pushnumber(L, wParam);

	DBCONTACTWRITESETTING *dbcws = (DBCONTACTWRITESETTING*)lParam;
	

	if (lua_pcall(L, 2, 1, 0))
		printf("%s\n", lua_tostring(L, -1));

	int res = (int)lua_tointeger(L, 1);

	return res;
}

static int lua_OnContactSettingChanged(lua_State *L)
{
	if (!lua_isfunction(L, 1))
	{
		lua_pushlightuserdata(L, NULL);
		return 1;
	}

	lua_pushvalue(L, 1);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	HANDLE res = ::HookEventObjParam(ME_DB_CONTACT_SETTINGCHANGED, SettingsChangedHookEventObjParam, L, ref);
	lua_pushlightuserdata(L, res);

	CMLua::Hooks.insert(res);
	CMLua::HookRefs.insert(new HandleRefParam(L, res, ref));

	return 1;
}

static int lua_DecodeDBCONTACTWRITESETTING(lua_State *L)
{
	DBCONTACTWRITESETTING *pDBCWS = (DBCONTACTWRITESETTING*)lua_tointeger(L, 1);

	lua_newtable(L);
	lua_pushstring(L, "Module");
	lua_pushstring(L, pDBCWS->szModule);
	lua_settable(L, -3);
	lua_pushstring(L, "Setting");
	lua_pushstring(L, pDBCWS->szSetting);
	lua_settable(L, -3);
	lua_pushstring(L, "Value");
	switch (pDBCWS->value.type)
	{
		case DBVT_BYTE:
			lua_pushinteger(L, pDBCWS->value.bVal);
			break;
		case DBVT_WORD:
			lua_pushinteger(L, pDBCWS->value.wVal);
			break;
		case DBVT_DWORD:
			lua_pushnumber(L, pDBCWS->value.dVal);
			break;
		case DBVT_ASCIIZ:
			lua_pushstring(L, ptrA(mir_utf8encode(pDBCWS->value.pszVal)));
			break;
		case DBVT_UTF8:
			lua_pushstring(L, pDBCWS->value.pszVal);
			break;
		case DBVT_WCHAR:
			lua_pushstring(L, ptrA(mir_utf8encodeW(pDBCWS->value.pwszVal)));
			break;
		default:
			lua_pushvalue(L, 4);
			return 1;
	}
	lua_settable(L, -3);

	return 1;
}

static luaL_Reg databaseApi[] =
{
	{ "FindFirstContact", lua_FindFirstContact },
	{ "FindNextContact", lua_FindNextContact },

	{ "GetEventCount", lua_GetEventCount },

	{ "GetFirstEvent", lua_GetFirstEvent },
	{ "GetPrevEvent", lua_GetPrevEvent },
	{ "GetNextEvent", lua_GetNextEvent },
	{ "GetLastEvent", lua_GetLastEvent },

	{ "GetEvent", lua_GetEvent },

	{ "WriteContactSetting", lua_WriteContactSetting },
	
	{ "GetContactSetting", lua_GetContactSetting },
	{ "EnumSettings", lua_EnumSettings },

	{ "DeleteContactSetting", lua_DeleteContactSetting },
	{ "DeleteModule", lua_DeleteModule },

	{ "OnContactSettingChanged", lua_OnContactSettingChanged },
	{ "DecodeDBCONTACTWRITESETTING", lua_DecodeDBCONTACTWRITESETTING },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_database(lua_State *L)
{
	luaL_newlib(L, databaseApi);

	return 1;
}
