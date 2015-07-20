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
	MCONTACT hContact = luaL_checkinteger(L, 1);
	const char *szProto = lua_tostring(L, 2);

	MCONTACT res = db_find_next(hContact, szProto);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_ContactIterator(lua_State *L)
{
	MCONTACT hContact = lua_tointeger(L, lua_upvalueindex(1));
	const char *szProto = lua_tostring(L, lua_upvalueindex(2));

	hContact = hContact == NULL
		? db_find_first(szProto)
		: db_find_next(hContact);

	if (hContact)
	{
		lua_pushinteger(L, hContact);
		lua_pushvalue(L, -1);
		lua_replace(L, lua_upvalueindex(1));
	}
	else
		lua_pushnil(L);

	return 1;
}

static int lua_AllContacts(lua_State *L)
{
	const char *szProto = lua_tostring(L, 1);

	lua_pushinteger(L, NULL);
	lua_pushstring(L, szProto);
	lua_pushcclosure(L, lua_ContactIterator, 2);

	return 1;
}

/***********************************************/

static int lua_GetEventCount(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);

	int res = ::db_event_count(hContact);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_GetFirstEvent(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);

	MEVENT res = ::db_event_first(hContact);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_GetPrevEvent(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);
	MEVENT hEvent = luaL_checkinteger(L, 2);

	MEVENT res = ::db_event_prev(hContact, hEvent);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_GetNextEvent(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);
	MEVENT hEvent = luaL_checkinteger(L, 2);

	MEVENT res = ::db_event_next(hContact, hEvent);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_GetLastEvent(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);

	MEVENT res = ::db_event_last(hContact);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_GetEvent(lua_State *L)
{
	MEVENT hEvent = luaL_checkinteger(L, 1);

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
	lua_pushliteral(L, "Module");
	lua_pushstring(L, ptrA(mir_utf8encode(dbei.szModule)));
	lua_settable(L, -3);
	lua_pushliteral(L, "Timestamp");
	lua_pushnumber(L, dbei.timestamp);
	lua_settable(L, -3);
	lua_pushliteral(L, "Type");
	lua_pushinteger(L, dbei.eventType);
	lua_settable(L, -3);
	lua_pushliteral(L, "Flags");
	lua_pushinteger(L, dbei.flags);
	lua_settable(L, -3);
	lua_pushliteral(L, "Length");
	lua_pushnumber(L, dbei.cbBlob);
	lua_settable(L, -3);
	lua_pushliteral(L, "Blob");
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

static int lua_EventIterator(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, lua_upvalueindex(1));
	MEVENT hEvent = luaL_checkinteger(L, lua_upvalueindex(2));

	hEvent = hEvent == NULL
		? db_event_first(hContact)
		: db_event_next(hContact, hEvent);

	if (hEvent)
	{
		lua_pushinteger(L, hContact);
		lua_pushvalue(L, -1);
		lua_replace(L, lua_upvalueindex(2));
	}
	else
		lua_pushnil(L);

	return 1;
}

static int lua_AllEvents(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);

	lua_pushinteger(L, hContact);
	lua_pushinteger(L, NULL);
	lua_pushcclosure(L, lua_EventIterator, 2);

	return 1;
}

static int lua_EventReverseIterator(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, lua_upvalueindex(1));
	MEVENT hEvent = luaL_checkinteger(L, lua_upvalueindex(2));

	hEvent = hEvent == NULL
		? db_event_last(hContact)
		: db_event_prev(hContact, hEvent);

	if (hEvent)
	{
		lua_pushinteger(L, hContact);
		lua_pushvalue(L, -1);
		lua_replace(L, lua_upvalueindex(2));
	}
	else
		lua_pushnil(L);

	return 1;
}

static int lua_AllEventsFromEnd(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);

	lua_pushinteger(L, hContact);
	lua_pushinteger(L, NULL);
	lua_pushcclosure(L, lua_EventReverseIterator, 2);

	return 1;
}

/***********************************************/

static int lua_WriteSetting(lua_State *L)
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

static int lua_GetSetting(lua_State *L)
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
	int  count;
	char **pszSettingName;
}
enumDBSettingsParam;

static int SettingsEnumProc(const char* szSetting, LPARAM lParam)
{
	if (szSetting)
	{
		enumDBSettingsParam* p = (enumDBSettingsParam*)lParam;

		p->count++;
		p->pszSettingName = (char**)mir_realloc(p->pszSettingName, p->count * sizeof(char*));
		p->pszSettingName[p->count - 1] = mir_strdup(szSetting);
	}
	return 0;
}

static int lua_SettingIterator(lua_State *L)
{
	int i = lua_tointeger(L, lua_upvalueindex(1));
	enumDBSettingsParam* param = (enumDBSettingsParam*)lua_touserdata(L, lua_upvalueindex(2));

	if (i < param->count)
	{
		lua_pushinteger(L, (i + 1));
		lua_replace(L, lua_upvalueindex(1));
		lua_pushstring(L, ptrA(mir_utf8encode(param->pszSettingName[i])));
		mir_free(param->pszSettingName[i]);
	}
	else
	{
		lua_pushnil(L);
		mir_free(param->pszSettingName);
		//mir_free(param);
	}

	return 1;
}

static int lua_AllSettings(lua_State *L)
{
	const char* szModule = luaL_checkstring(L, 1);
	MCONTACT hContact = lua_tointeger(L, 2);

	enumDBSettingsParam* param = (enumDBSettingsParam*)mir_alloc(sizeof(enumDBSettingsParam*));
	param->count = 0;
	param->pszSettingName = NULL;

	DBCONTACTENUMSETTINGS dbces = { 0 };
	dbces.pfnEnumProc = SettingsEnumProc;
	dbces.szModule = szModule;
	dbces.ofsSettings = 0;
	dbces.lParam = (LPARAM)param;
	::CallService(MS_DB_CONTACT_ENUMSETTINGS, hContact, (LPARAM)&dbces);

	lua_pushinteger(L, 0);
	lua_pushlightuserdata(L, param);
	lua_pushcclosure(L, lua_SettingIterator, 2);

	return 1;
}

static int lua_EnumSettings(lua_State *L)
{
	LPCSTR szModule = luaL_checkstring(L, 1);
	MCONTACT hContact = lua_tointeger(L, 2);

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

	for (int i = 0; i < param.count; i++)
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

static int lua_DeleteSetting(lua_State *L)
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
	lua_newtable(L);
	lua_pushliteral(L, "Module");
	lua_pushstring(L, dbcws->szModule);
	lua_settable(L, -3);
	lua_pushliteral(L, "Setting");
	lua_pushstring(L, dbcws->szSetting);
	lua_settable(L, -3);
	lua_pushliteral(L, "Value");
	switch (dbcws->value.type)
	{
	case DBVT_BYTE:
		lua_pushinteger(L, dbcws->value.bVal);
		break;
	case DBVT_WORD:
		lua_pushinteger(L, dbcws->value.wVal);
		break;
	case DBVT_DWORD:
		lua_pushnumber(L, dbcws->value.dVal);
		break;
	case DBVT_ASCIIZ:
		lua_pushstring(L, ptrA(mir_utf8encode(dbcws->value.pszVal)));
		break;
	case DBVT_UTF8:
		lua_pushstring(L, dbcws->value.pszVal);
		break;
	case DBVT_WCHAR:
		lua_pushstring(L, ptrA(mir_utf8encodeW(dbcws->value.pwszVal)));
		break;
	default:
		lua_pushvalue(L, 4);
		return 1;
	}
	lua_settable(L, -3);

	if (lua_pcall(L, 2, 1, 0))
		printf("%s\n", lua_tostring(L, -1));

	int res = (int)lua_tointeger(L, 1);

	return res;
}

static int lua_OnSettingChanged(lua_State *L)
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
	lua_pushliteral(L, "Module");
	lua_pushstring(L, pDBCWS->szModule);
	lua_settable(L, -3);
	lua_pushliteral(L, "Setting");
	lua_pushstring(L, pDBCWS->szSetting);
	lua_settable(L, -3);
	lua_pushliteral(L, "Value");
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
	{ "AllContacts", lua_AllContacts },

	{ "GetEventCount", lua_GetEventCount },

	{ "GetFirstEvent", lua_GetFirstEvent },
	{ "GetPrevEvent", lua_GetPrevEvent },
	{ "GetNextEvent", lua_GetNextEvent },
	{ "GetLastEvent", lua_GetLastEvent },
	{ "AllEvents", lua_AllEvents },
	{ "GetEventsFromEnd", lua_AllEventsFromEnd },

	{ "GetEvent", lua_GetEvent },

	{ "WriteContactSetting", lua_WriteSetting },
	{ "WriteSetting", lua_WriteSetting },
	
	{ "GetContactSetting", lua_GetSetting },
	{ "GetSetting", lua_GetSetting },
	{ "AllSettings", lua_AllSettings },
	{ "EnumSettings", lua_EnumSettings },

	{ "DeleteContactSetting", lua_DeleteSetting },
	{ "DeleteSetting", lua_DeleteSetting },
	{ "DeleteModule", lua_DeleteModule },

	{ "OnContactSettingChanged", lua_OnSettingChanged },
	{ "OnSettingChanged", lua_OnSettingChanged },
	{ "DecodeDBCONTACTWRITESETTING", lua_DecodeDBCONTACTWRITESETTING },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_database(lua_State *L)
{
	luaL_newlib(L, databaseApi);

	return 1;
}
