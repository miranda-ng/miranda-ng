#include "stdafx.h"

static int db_FindFirstContact(lua_State *L)
{
	const char *szProto = lua_tostring(L, 1);

	MCONTACT res = db_find_first(szProto);
	lua_pushinteger(L, res);

	return 1;
}

static int db_FindNextContact(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);
	const char *szProto = lua_tostring(L, 2);

	MCONTACT res = db_find_next(hContact, szProto);
	lua_pushinteger(L, res);

	return 1;
}

static int db_ContactIterator(lua_State *L)
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

static int db_Contacts(lua_State *L)
{
	const char *szProto = lua_tostring(L, 1);

	lua_pushinteger(L, 0);
	lua_pushstring(L, szProto);
	lua_pushcclosure(L, db_ContactIterator, 2);

	return 1;
}

/***********************************************/

static int db_GetEventCount(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);

	int res = db_event_count(hContact);
	lua_pushinteger(L, res);

	return 1;
}

static int db_GetFirstEvent(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);

	MEVENT res = db_event_first(hContact);
	lua_pushinteger(L, res);

	return 1;
}

static int db_GetPrevEvent(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);
	MEVENT hDbEvent = luaL_checkinteger(L, 2);

	MEVENT res = db_event_prev(hContact, hDbEvent);
	lua_pushinteger(L, res);

	return 1;
}

static int db_GetNextEvent(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);
	MEVENT hDbEvent = luaL_checkinteger(L, 2);

	MEVENT res = db_event_next(hContact, hDbEvent);
	lua_pushinteger(L, res);

	return 1;
}

static int db_GetLastEvent(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);

	MEVENT res = db_event_last(hContact);
	lua_pushinteger(L, res);

	return 1;
}

static int db_EventIterator(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, lua_upvalueindex(1));
	MEVENT hDbEvent = luaL_checkinteger(L, lua_upvalueindex(2));

	hDbEvent = hDbEvent == NULL
		? db_event_first(hContact)
		: db_event_next(hContact, hDbEvent);

	if (hDbEvent)
	{
		lua_pushinteger(L, hDbEvent);
		lua_pushvalue(L, -1);
		lua_replace(L, lua_upvalueindex(2));
	}
	else
		lua_pushnil(L);

	return 1;
}

static int db_Events(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);

	lua_pushinteger(L, hContact);
	lua_pushinteger(L, NULL);
	lua_pushcclosure(L, db_EventIterator, 2);

	return 1;
}

static int db_EventReverseIterator(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, lua_upvalueindex(1));
	MEVENT hDbEvent = luaL_checkinteger(L, lua_upvalueindex(2));

	hDbEvent = hDbEvent == NULL
		? db_event_last(hContact)
		: db_event_prev(hContact, hDbEvent);

	if (hDbEvent)
	{
		lua_pushinteger(L, hDbEvent);
		lua_pushvalue(L, -1);
		lua_replace(L, lua_upvalueindex(2));
	}
	else
		lua_pushnil(L);

	return 1;
}

static int db_EventsFromEnd(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);

	lua_pushinteger(L, hContact);
	lua_pushinteger(L, NULL);
	lua_pushcclosure(L, db_EventReverseIterator, 2);

	return 1;
}

/***********************************************/

#define MT_BLOB "BLOB"

static int array_create(lua_State *L)
{
	BYTE *data = (BYTE*)lua_touserdata(L, 1);
	size_t size = luaL_checkinteger(L, 2);

	BLOB *blob = (BLOB*)lua_newuserdata(L, sizeof(BLOB));
	blob->cbSize = size;
	blob->pBlobData = (BYTE*)mir_calloc(size);
	memcpy(blob->pBlobData, data, size);
	luaL_setmetatable(L, MT_BLOB);

	return 1;
}

static int array__index(lua_State *L)
{
	BLOB *blob = (BLOB*)luaL_checkudata(L, 1, MT_BLOB);
	int i = luaL_checkinteger(L, 2);

	lua_pushinteger(L, (uint8_t)blob->pBlobData[i - 1]);

	return 1;
}

static int array__newindex(lua_State *L)
{
	BLOB *blob = (BLOB*)luaL_checkudata(L, 1, MT_BLOB);
	int i = luaL_checkinteger(L, 2);

	blob->pBlobData[i - 1] = (BYTE)luaL_checkinteger(L, 3);

	return 0;
}

static int array__len(lua_State *L)
{
	BLOB *blob = (BLOB*)luaL_checkudata(L, 1, MT_BLOB);

	lua_pushinteger(L, blob->cbSize);

	return 1;
}

static int array__tostring(lua_State *L)
{
	BLOB *blob = (BLOB*)luaL_checkudata(L, 1, MT_BLOB);

	char *res = (char*)alloca(blob->cbSize * 2 + 1);
	bin2hex(blob->pBlobData, blob->cbSize, res);

	lua_pushstring(L, res);

	return 1;
}

static int array__gc(lua_State *L)
{
	BLOB *blob = (BLOB*)luaL_checkudata(L, 1, MT_BLOB);

	mir_free(blob->pBlobData);

	return 0;
}

static const struct luaL_Reg blobApi[] =
{
	{ "__index", array__index },
	{ "__newindex", array__newindex },
	{ "__len", array__len },
	{ "__tostring", array__tostring },
	{ "__gc", array__gc },

	(NULL, NULL)
};

/***********************************************/

static int db_GetSetting(lua_State *L)
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
	case DBVT_BLOB:
		{
			lua_pushcfunction(L, array_create);
			lua_pushlightuserdata(L, dbv.pbVal);
			lua_pushnumber(L, dbv.cpbVal);
			luaM_pcall(L, 2, 1);
		}
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

static int db_SettingIterator(lua_State *L)
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

static int db_Settings(lua_State *L)
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
	lua_pushcclosure(L, db_SettingIterator, 2);

	return 1;
}

static int db_WriteSetting(lua_State *L)
{
	MCONTACT hContact = lua_tointeger(L, 1);
	LPCSTR szModule = luaL_checkstring(L, 2);
	LPCSTR szSetting = luaL_checkstring(L, 3);

	DBVARIANT dbv = { 0 };
	if (lua_isnoneornil(L, 5))
	{
		int type = lua_type(L, 4);
		switch (type)
		{
		case LUA_TBOOLEAN:
			dbv.type = DBVT_BYTE;
			break;
		case LUA_TNUMBER:
			dbv.type = DBVT_DWORD;
			break;
		case LUA_TSTRING:
			dbv.type = DBVT_UTF8;
			break;
		case LUA_TUSERDATA:
			dbv.type = DBVT_BLOB;
			break;
		default:
			lua_pushboolean(L, false);
			return 1;
		}
	}
	else
		dbv.type = luaL_checkinteger(L, 5);

	switch (dbv.type)
	{
	case DBVT_BYTE:
		dbv.bVal = lua_isboolean(L, 4)
			? lua_toboolean(L, 4)
			: luaL_checknumber(L, 4);
		break;
	case DBVT_WORD:
		dbv.wVal = luaL_checknumber(L, 4);
		break;
	case DBVT_DWORD:
		dbv.dVal = luaL_checknumber(L, 4);
		break;
	case DBVT_UTF8:
		dbv.pszVal = mir_strdup(luaL_checkstring(L, 4));
		break;
	case DBVT_ASCIIZ:
		dbv.pszVal = mir_utf8decodeA(luaL_checkstring(L, 4));
		break;
	case DBVT_WCHAR:
		dbv.pwszVal = mir_utf8decodeW(luaL_checkstring(L, 4));
		break;
	case DBVT_BLOB:
	{
		BLOB *blob = (BLOB*)luaL_checkudata(L, 4, MT_BLOB);
		dbv.cpbVal = blob->cbSize;
		dbv.pbVal = blob->pBlobData;
	}
		break;
	default:
		lua_pushboolean(L, false);
		return 1;
	}

	INT_PTR res = db_set(hContact, szModule, szSetting, &dbv);
	lua_pushboolean(L, !res);

	return 1;
}

static int db_DeleteSetting(lua_State *L)
{
	MCONTACT hContact = lua_tointeger(L, 1);
	LPCSTR szModule = luaL_checkstring(L, 2);
	LPCSTR szSetting = luaL_checkstring(L, 3);

	INT_PTR res = db_unset(hContact, szModule, szSetting);
	lua_pushboolean(L, !res);

	return 1;
}

static int db_DeleteModule(lua_State *L)
{
	MCONTACT hContact = lua_tointeger(L, 1);
	LPCSTR szModule = luaL_checkstring(L, 2);

	INT_PTR res = ::CallService(MS_DB_MODULE_DELETE, hContact, (LPARAM)szModule);
	lua_pushboolean(L, !res);

	return 1;
}

/***********************************************/

static luaL_Reg databaseApi[] =
{
	{ "FindFirstContact", db_FindFirstContact },
	{ "FindNextContact", db_FindNextContact },
	{ "Contacts", db_Contacts },

	{ "GetEventCount", db_GetEventCount },

	{ "GetFirstEvent", db_GetFirstEvent },
	{ "GetPrevEvent", db_GetPrevEvent },
	{ "GetNextEvent", db_GetNextEvent },
	{ "GetLastEvent", db_GetLastEvent },
	{ "Events", db_Events },
	{ "EventsFromEnd", db_EventsFromEnd },

	{ "WriteSetting", db_WriteSetting },
	{ "SetSetting", db_WriteSetting },

	{ "GetSetting", db_GetSetting },
	{ "Settings", db_Settings },

	{ "DeleteSetting", db_DeleteSetting },
	{ "DeleteModule", db_DeleteModule },

	{ "DBVT_BYTE", NULL },
	{ "DBVT_WORD", NULL },
	{ "DBVT_DWORD", NULL },
	{ "DBVT_ASCIIZ", NULL },
	{ "DBVT_UTF8", NULL },
	{ "DBVT_WCHAR", NULL },

	{ NULL, NULL }
};

/***********************************************/

#define MT_DBCONTACTWRITESETTING "DBCONTACTWRITESETTING"

int MT<DBCONTACTWRITESETTING>::Index(lua_State *L, DBCONTACTWRITESETTING *dbcw)
{
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
		case DBVT_BLOB:
		{
			lua_pushcfunction(L, array_create);
			lua_pushlightuserdata(L, dbcw->value.pbVal);
			lua_pushnumber(L, dbcw->value.cpbVal);
			luaM_pcall(L, 2, 1);
		}
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

void MT<DBEVENTINFO>::Free(lua_State*, DBEVENTINFO **dbei)
{
	mir_free((*dbei)->pBlob);
}

/***********************************************/

#define MT_CONTACTINFO "CONTACTINFO"

void MT<CONTACTINFO>::Init(lua_State *L, CONTACTINFO **ci)
{
	if (!lua_isinteger(L, 1))
	{
		const char *msg = lua_pushfstring(L, "hContact expected, got %s", luaL_typename(L, 1));
		luaL_argerror(L, 1, msg);
	}
	MCONTACT hContact = luaL_checkinteger(L, 1);

	(*ci) = (CONTACTINFO*)mir_calloc(sizeof(CONTACTINFO));
	(*ci)->cbSize = sizeof(CONTACTINFO);
	(*ci)->hContact = hContact;
}

int MT<CONTACTINFO>::Index(lua_State *L, CONTACTINFO *ci)
{
	if (lua_isnumber(L, 2))
		ci->dwFlag = lua_tointeger(L, 2);
	else if (lua_isstring(L, 2))
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
		else if (mir_strcmpi(key, "CustomNick") == 0)
			ci->dwFlag = CNF_CUSTOMNICK;
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

void MT<CONTACTINFO>::Free(lua_State* /*L*/, CONTACTINFO **ci)
{
	mir_free((*ci));
}

/***********************************************/

LUAMOD_API int luaopen_m_database(lua_State *L)
{
	luaL_newlib(L, databaseApi);

	lua_pushnumber(L, DBVT_BYTE);
	lua_setfield(L, -2, "DBVT_BYTE");
	lua_pushnumber(L, DBVT_WORD);
	lua_setfield(L, -2, "DBVT_WORD");
	lua_pushnumber(L, DBVT_DWORD);
	lua_setfield(L, -2, "DBVT_DWORD");
	lua_pushnumber(L, DBVT_ASCIIZ);
	lua_setfield(L, -2, "DBVT_ASCIIZ");
	lua_pushnumber(L, DBVT_UTF8);
	lua_setfield(L, -2, "DBVT_UTF8");
	lua_pushnumber(L, DBVT_WCHAR);
	lua_setfield(L, -2, "DBVT_WCHAR");

	luaL_newmetatable(L, MT_BLOB);
	luaL_setfuncs(L, blobApi, 0);
	lua_pop(L, 1);

	MT<DBCONTACTWRITESETTING>(L, MT_DBCONTACTWRITESETTING)
		.Field(&DBCONTACTWRITESETTING::szModule, "Module", LUA_TSTRINGA)
		.Field(&DBCONTACTWRITESETTING::szSetting, "Setting", LUA_TSTRINGA);
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
		.Field(&CONTACTINFO::hContact, "hContact", LUA_TINTEGER);
	lua_pop(L, 1);

	return 1;
}
