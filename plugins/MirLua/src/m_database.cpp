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
		: db_find_next(hContact, szProto);

	if (hContact)
	{
		lua_pushinteger(L, hContact);
		lua_pushvalue(L, -1);
		lua_replace(L, lua_upvalueindex(2));
	}
	else
		lua_pushnil(L);

	return 1;
}

static int lua_Contacts(lua_State *L)
{
	const char *szProto = lua_tostring(L, 1);

	lua_pushinteger(L, 0);
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

static int lua_EventIterator(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, lua_upvalueindex(1));
	MEVENT hEvent = luaL_checkinteger(L, lua_upvalueindex(2));

	hEvent = hEvent == NULL
		? db_event_first(hContact)
		: db_event_next(hContact, hEvent);

	if (hEvent)
	{
		lua_pushinteger(L, hEvent);
		lua_pushvalue(L, -1);
		lua_replace(L, lua_upvalueindex(2));
	}
	else
		lua_pushnil(L);

	return 1;
}

static int lua_Events(lua_State *L)
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

static int lua_EventsFromEnd(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);

	lua_pushinteger(L, hContact);
	lua_pushinteger(L, NULL);
	lua_pushcclosure(L, lua_EventReverseIterator, 2);

	return 1;
}

/***********************************************/

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
		mir_free(param);
	}

	return 1;
}

static int lua_Settings(lua_State *L)
{
	MCONTACT hContact = lua_tointeger(L, 1);
	const char* szModule = luaL_checkstring(L, 2);

	enumDBSettingsParam* param = (enumDBSettingsParam*)mir_alloc(sizeof(enumDBSettingsParam));
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
	lua_pushboolean(L, !res);

	return 1;
}

static int lua_DeleteSetting(lua_State *L)
{
	MCONTACT hContact = lua_tointeger(L, 1);
	LPCSTR szModule = luaL_checkstring(L, 2);
	LPCSTR szSetting = luaL_checkstring(L, 3);

	INT_PTR res = db_unset(hContact, szModule, szSetting);
	lua_pushboolean(L, !res);

	return 1;
}

static int lua_DeleteModule(lua_State *L)
{
	MCONTACT hContact = lua_tointeger(L, 1);
	LPCSTR szModule = luaL_checkstring(L, 2);

	INT_PTR res = ::CallService(MS_DB_MODULE_DELETE, hContact, (LPARAM)szModule);
	lua_pushboolean(L, !res);

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

	luaM_pcall(L, 2, 1);

	int res = (int)lua_tointeger(L, 1);

	return res;
}

static luaL_Reg databaseApi[] =
{
	{ "FindFirstContact", lua_FindFirstContact },
	{ "FindNextContact", lua_FindNextContact },
	{ "Contacts", lua_Contacts },

	{ "GetEventCount", lua_GetEventCount },

	{ "GetFirstEvent", lua_GetFirstEvent },
	{ "GetPrevEvent", lua_GetPrevEvent },
	{ "GetNextEvent", lua_GetNextEvent },
	{ "GetLastEvent", lua_GetLastEvent },
	{ "Events", lua_Events },
	{ "EventsFromEnd", lua_EventsFromEnd },

	{ "WriteSetting", lua_WriteSetting },

	{ "GetSetting", lua_GetSetting },
	{ "Settings", lua_Settings },

	{ "DeleteSetting", lua_DeleteSetting },
	{ "DeleteModule", lua_DeleteModule },

	{ NULL, NULL }
};

/***********************************************/

#define MT_DBCONTACTWRITESETTING "DBCONTACTWRITESETTING"

static int dbcw__index(lua_State *L)
{
	DBCONTACTWRITESETTING *dbcw = (DBCONTACTWRITESETTING*)luaL_checkudata(L, 1, MT_DBCONTACTWRITESETTING);
	const char *key = luaL_checkstring(L, 2);

	if (mir_strcmpi(key, "Module") == 0)
		lua_pushstring(L, dbcw->szModule);
	else if (mir_strcmpi(key, "Setting") == 0)
		lua_pushstring(L, dbcw->szSetting);
	else if (mir_strcmpi(key, "Value") == 0)
	{
		switch (dbcw->value.type)
		{
		case DBVT_BYTE:
			lua_pushinteger(L, dbcw->value.bVal);
			break;
		case DBVT_WORD:
			lua_pushinteger(L, dbcw->value.wVal);
			break;
		case DBVT_DWORD:
			lua_pushnumber(L, dbcw->value.dVal);
			break;
		case DBVT_ASCIIZ:
			lua_pushstring(L, ptrA(mir_utf8encode(dbcw->value.pszVal)));
			break;
		case DBVT_UTF8:
			lua_pushstring(L, dbcw->value.pszVal);
			break;
		case DBVT_WCHAR:
			lua_pushstring(L, ptrA(mir_utf8encodeW(dbcw->value.pwszVal)));
			break;
		default:
			lua_pushnil(L);
		}
	}
	else
		lua_pushnil(L);

	return 1;
}

/***********************************************/

#define MT_DBEVENTINFO "DBEVENTINFO"

void MT<DBEVENTINFO>::Init(lua_State *L, DBEVENTINFO **dbei)
{
	MEVENT hDbEvent = luaL_checkinteger(L, 1);

	(*dbei)->cbSize = sizeof(DBEVENTINFO);
	(*dbei)->cbBlob = db_event_getBlobSize((MEVENT)hDbEvent);
	(*dbei)->pBlob = (PBYTE)mir_calloc((*dbei)->cbBlob);
	db_event_get((MEVENT)hDbEvent, (*dbei));
}

void MT<DBEVENTINFO>::Free(DBEVENTINFO **dbei)
{
	mir_free((*dbei)->pBlob);
}

/***********************************************/

#define MT_CONTACTINFO "CONTACTINFO"

void MT<CONTACTINFO>::Init(lua_State *L, CONTACTINFO **ci)
{
	MCONTACT hContact = 0;
	switch (lua_type(L, 1))
	{
	case LUA_TNUMBER:
		hContact = lua_tointeger(L, 1);
		break;
	case LUA_TLIGHTUSERDATA:
		hContact = (MCONTACT)lua_touserdata(L, 1);
		break;
	default:
		const char *msg = lua_pushfstring(L, "hContact expected, got %s", lua_typename(L, lua_type(L, 1)));
		luaL_argerror(L, 1, msg);
	}

	(*ci)->cbSize = sizeof(CONTACTINFO);
	(*ci)->hContact = hContact;
}

static int ci__index(lua_State *L)
{
	CONTACTINFO *ci = (CONTACTINFO*)luaL_checkudata(L, 1, MT_CONTACTINFO);

	if (lua_type(L, 2) == LUA_TNUMBER)
		ci->dwFlag = lua_tointeger(L, 2);
	else if (lua_type(L, 2) == LUA_TSTRING)
	{
		const char *key = luaL_checkstring(L, 2);

		if (mir_strcmpi(key, "Handle") == 0)
		{
			lua_pushinteger(L, ci->hContact);
			return 1;
		}

		if (mir_strcmpi(key, "FirstName") == 0)
			ci->dwFlag = CNF_FIRSTNAME;
		else if (mir_strcmpi(key, "LastName") == 0)
			ci->dwFlag = CNF_LASTNAME;
		else if (mir_strcmpi(key, "Nick") == 0)
			ci->dwFlag = CNF_NICK;
		else if (mir_strcmpi(key, "FullName") == 0)
			ci->dwFlag = CNF_FIRSTLAST;
		else if (mir_strcmpi(key, "DisplayName") == 0)
			ci->dwFlag = CNF_DISPLAY;
		else if (mir_strcmpi(key, "Uid") == 0)
			ci->dwFlag = CNF_UNIQUEID;
		else if (mir_strcmpi(key, "Email") == 0)
			ci->dwFlag = CNF_EMAIL;
		else if (mir_strcmpi(key, "City") == 0)
			ci->dwFlag = CNF_CITY;
		else if (mir_strcmpi(key, "State") == 0)
			ci->dwFlag = CNF_STATE;
		else if (mir_strcmpi(key, "Country") == 0)
			ci->dwFlag = CNF_COUNTRY;
		else if (mir_strcmpi(key, "Phone") == 0)
			ci->dwFlag = CNF_PHONE;
		else if (mir_strcmpi(key, "Homepage") == 0)
			ci->dwFlag = CNF_HOMEPAGE;
		else if (mir_strcmpi(key, "About") == 0)
			ci->dwFlag = CNF_ABOUT;
		else if (mir_strcmpi(key, "Age") == 0)
			ci->dwFlag = CNF_AGE;
		else if (mir_strcmpi(key, "Gender") == 0)
			ci->dwFlag = CNF_GENDER;
		else
		{
			lua_pushnil(L);
			return 1;
		}
	}
	else
	{
		lua_pushnil(L);
		return 1;
	}

	ci->dwFlag |= CNF_TCHAR;
	if (CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)(CONTACTINFO*)ci))
	{
		lua_pushnil(L);
		return 1;
	}

	switch (ci->type)
	{
	case CNFT_BYTE:
		lua_pushinteger(L, ci->bVal);
		break;
	case CNFT_WORD:
		lua_pushinteger(L, ci->wVal);
		break;
	case CNFT_DWORD:
		lua_pushnumber(L, ci->dVal);
		break;
	case CNFT_ASCIIZ:
		lua_pushstring(L, ptrA(mir_utf8encodeT(ci->pszVal)));
		break;
	default:
		lua_pushnil(L);
	}

	return 1;
}

/***********************************************/

static int dbei__test(lua_State *L)
{
	int type = lua_type(L, 1);

	return 0;
}

LUAMOD_API int luaopen_m_database(lua_State *L)
{
	luaL_newlib(L, databaseApi);

	MT<DBCONTACTWRITESETTING>(L, MT_DBCONTACTWRITESETTING)
		.Method(dbcw__index, "__index");
	lua_pop(L, 1);

	MT<DBEVENTINFO>(L, MT_DBEVENTINFO)
		.Field(&DBEVENTINFO::szModule, "Module", LUA_TSTRINGA)
		.Field(&DBEVENTINFO::timestamp, "Timestamp", LUA_TINTEGER)
		.Field(&DBEVENTINFO::eventType, "Type", LUA_TINTEGER)
		.Field(&DBEVENTINFO::flags, "Flags", LUA_TINTEGER)
		.Field(&DBEVENTINFO::cbBlob, "Length", LUA_TINTEGER)
		.Field(&DBEVENTINFO::pBlob, "Blob", LUA_TLIGHTUSERDATA);
	lua_pop(L, 1);

	MT<CONTACTINFO>(L, "CONTACTINFO")
		.Field(&CONTACTINFO::hContact, "hContact", LUA_TINTEGER)
		.Method(ci__index, "__index");
	lua_pop(L, 1);

	return 1;
}
