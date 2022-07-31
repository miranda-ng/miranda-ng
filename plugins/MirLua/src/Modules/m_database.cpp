#include "../stdafx.h"

void luaM_pushdbvt(lua_State *L, const DBVARIANT &value)
{
	switch (value.type) {
	case DBVT_BYTE:
		lua_pushinteger(L, value.bVal);
		break;
	case DBVT_WORD:
		lua_pushinteger(L,value.wVal);
		break;
	case DBVT_DWORD:
		lua_pushnumber(L, value.dVal);
		break;
	case DBVT_ASCIIZ:
		lua_pushstring(L, ptrA(mir_utf8encode(value.pszVal)));
		break;
	case DBVT_UTF8:
		lua_pushstring(L, value.pszVal);
		break;
	case DBVT_WCHAR:
		lua_pushstring(L, T2Utf(value.pwszVal));
		break;
	case DBVT_BLOB:
		lua_createtable(L, value.cpbVal, 0);
		for (int i = 0; i < value.cpbVal; i++) {
			lua_pushinteger(L, value.pbVal[i]);
			lua_rawseti(L, -2, i + 1);
		}
		break;
	default:
		lua_pushnil(L);
	}
}

/***********************************************/

static int db_ContactIterator(lua_State *L)
{
	MCONTACT hContact = lua_tointeger(L, lua_upvalueindex(1));
	const char *szModule = lua_tostring(L, lua_upvalueindex(2));

	hContact = hContact == NULL
		? db_find_first(szModule)
		: db_find_next(hContact, szModule);

	if (hContact) {
		lua_pushinteger(L, hContact);
		lua_pushvalue(L, -1);
		lua_replace(L, lua_upvalueindex(1));
	}
	else
		lua_pushnil(L);

	return 1;
}

static int db_Contacts(lua_State *L)
{
	const char *szModule = nullptr;

	switch (lua_type(L, 1)) {
	case LUA_TNONE:
		break;
	case LUA_TSTRING:
		szModule = lua_tostring(L, 1);
		break;
	case LUA_TUSERDATA:
	{
		PROTOACCOUNT **pa = (PROTOACCOUNT**)luaL_checkudata(L, 1, MT_PROTOACCOUNT);
		szModule = (*pa)->szModuleName;
		break;
	}
	default:
		luaL_argerror(L, 1, luaL_typename(L, 1));
	}

	lua_pushinteger(L, 0);
	lua_pushstring(L, szModule);
	lua_pushcclosure(L, db_ContactIterator, 2);

	return 1;
}

static const char *mods[] =
{
	"FirstName",
	"LastName",
	"Nick",
	"CustomNick",
	"Email",
	"City",
	"State",
	"Country",
	"Phone",
	"Homepage",
	"About",
	"Gender",
	"Age",
	"FullName",
	"Uid",
	"DisplayName",
	nullptr
};

static int db_GetContactInfo(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, 1);

	int type = 0;
	switch (lua_type(L, 2)) {
	case LUA_TNUMBER:
		type = luaL_checkinteger(L, 2);
		break;
	case LUA_TSTRING:
		type = luaL_checkoption(L, 2, nullptr, mods) + 1;
		break;
	default:
		luaL_argerror(L, 2, luaL_typename(L, 2));
	}

	ptrW value(Contact::GetInfo(type, hContact));
	if (value)
		lua_pushstring(L, ptrA(mir_utf8encodeW(value)));
	else
		lua_pushnil(L);

	return 1;
}

/***********************************************/

static int db_GetEventCount(lua_State *L)
{
	MCONTACT hContact = luaL_optinteger(L, 1, 0);

	int res = db_event_count(hContact);
	lua_pushinteger(L, res);

	return 1;
}

static int db_GetFirstEvent(lua_State *L)
{
	MCONTACT hContact = luaL_optinteger(L, 1, 0);

	MEVENT res = db_event_first(hContact);
	lua_pushinteger(L, res);

	return 1;
}

static int db_GetPrevEvent(lua_State *L)
{
	MCONTACT hContact = luaL_optinteger(L, 1, 0);
	MEVENT hDbEvent = luaL_checkinteger(L, 2);

	MEVENT res = db_event_prev(hContact, hDbEvent);
	lua_pushinteger(L, res);

	return 1;
}

static int db_GetNextEvent(lua_State *L)
{
	MCONTACT hContact = luaL_optinteger(L, 1, 0);
	MEVENT hDbEvent = luaL_checkinteger(L, 2);

	MEVENT res = db_event_next(hContact, hDbEvent);
	lua_pushinteger(L, res);

	return 1;
}

static int db_GetLastEvent(lua_State *L)
{
	MCONTACT hContact = luaL_optinteger(L, 1, 0);

	MEVENT res = db_event_last(hContact);
	lua_pushinteger(L, res);

	return 1;
}

static int db_GetFirstUnreadEvent(lua_State *L)
{
	MCONTACT hContact = luaL_optinteger(L, 1, 0);

	MEVENT res = db_event_firstUnread(hContact);
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

	if (hDbEvent) {
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
	MCONTACT hContact = luaL_optinteger(L, 1, 0);

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

	if (hDbEvent) {
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
	MCONTACT hContact = luaL_optinteger(L, 1, 0);

	lua_pushinteger(L, hContact);
	lua_pushinteger(L, NULL);
	lua_pushcclosure(L, db_EventReverseIterator, 2);

	return 1;
}

static int db_UnreadEventIterator(lua_State *L)
{
	MCONTACT hContact = luaL_checkinteger(L, lua_upvalueindex(1));
	MEVENT hDbEvent = luaL_checkinteger(L, lua_upvalueindex(2));

	hDbEvent = db_event_firstUnread(hContact);

	if (hDbEvent) {
		lua_pushinteger(L, hDbEvent);
		lua_pushvalue(L, -1);
		lua_replace(L, lua_upvalueindex(2));
	}
	else
		lua_pushnil(L);

	return 1;
}

static int db_UnreadEvents(lua_State *L)
{
	MCONTACT hContact = luaL_optinteger(L, 1, 0);

	lua_pushinteger(L, hContact);
	lua_pushinteger(L, NULL);
	lua_pushcclosure(L, db_UnreadEventIterator, 2);

	return 1;
}

void MakeDbEvent(lua_State *L, DBEVENTINFO &dbei)
{
	lua_getfield(L, -1, "Module");
	dbei.szModule = mir_strdup(lua_tostring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "Type");
	dbei.eventType = lua_tointeger(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Timestamp");
	dbei.timestamp = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Flags");
	dbei.flags = lua_tointeger(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Blob");
	switch (lua_type(L, -1)) {
	case LUA_TTABLE:
		dbei.cbBlob = (uint32_t)lua_rawlen(L, 4);
		dbei.pBlob = (uint8_t*)mir_calloc(dbei.cbBlob);
		for (uint32_t i = 0; i < dbei.cbBlob; i++) {
			lua_geti(L, 4, i + 1);
			dbei.pBlob[i] = lua_tointeger(L, -1);
			lua_pop(L, 1);
		}
		break;
	case LUA_TSTRING:
		size_t nLen;
		const char *str = lua_tolstring(L, -1, &nLen);
		dbei.cbBlob = (uint32_t)nLen;
		dbei.pBlob = (uint8_t*)mir_alloc(nLen);
		memcpy(dbei.pBlob, str, nLen);
		break;
	}
	lua_pop(L, 1);
}

static int db_AddEvent(lua_State *L)
{
	MCONTACT hContact = luaL_optinteger(L, 1, 0);

	DBEVENTINFO dbei = {};
	MakeDbEvent(L, dbei);

	MEVENT hDbEvent = db_event_add(hContact, &dbei);
	if (hDbEvent)
		lua_pushnumber(L, hDbEvent);
	else
		lua_pushnil(L);

	return 1;
}

static int db_DeleteEvent(lua_State *L)
{
	MCONTACT hDbEvent = luaL_optinteger(L, 1, 0);

	int res = db_event_delete(hDbEvent);
	lua_pushboolean(L, res == 0);

	return 1;
}

static int db_MarkReadEvent(lua_State *L)
{
	MCONTACT hContact = luaL_optinteger(L, 1, 0);
	MEVENT hDbEvent = luaL_checkinteger(L, 2);

	int res = db_event_markRead(hContact, hDbEvent);
	lua_pushnumber(L, res);

	return 1;
}

/***********************************************/

static int ModulesEnumProc(const char *szModuleName, void *lParam)
{
	if (szModuleName) {
		LIST<char>* p = (LIST<char>*)lParam;
		p->insert(mir_strdup(szModuleName));
	}

	return 0;
}

static int db_ModulesIterator(lua_State *L)
{
	int i = lua_tointeger(L, lua_upvalueindex(1));
	LIST<char> &param = *(LIST<char>*)lua_touserdata(L, lua_upvalueindex(2));

	if (i < param.getCount()) {
		lua_pushinteger(L, i + 1);
		lua_replace(L, lua_upvalueindex(1));
		lua_pushstring(L, ptrA(mir_utf8encode(param[i])));
		mir_free(param[i]);
	}
	else {
		lua_pushnil(L);
		delete &param;
	}

	return 1;
}

static int db_Modules(lua_State *L)
{
	LIST<char> *param = new LIST<char>(5, PtrKeySortT);

	db_enum_modules(ModulesEnumProc, param);

	lua_pushinteger(L, 0);
	lua_pushlightuserdata(L, param);
	lua_pushcclosure(L, db_ModulesIterator, 2);

	return 1;
}

static int db_DeleteModule(lua_State *L)
{
	MCONTACT hContact = luaL_optinteger(L, 1, 0);
	const char *szModule = luaL_checkstring(L, 2);

	INT_PTR res = db_delete_module(hContact, szModule);
	lua_pushboolean(L, !res);

	return 1;
}

static int SettingsEnumProc(const char *szSetting, void *lParam)
{
	if (szSetting ) {
		LIST<char>* p = (LIST<char>*)lParam;
		p->insert(mir_strdup(szSetting));
	}
	return 0;
}

static int db_SettingIterator(lua_State *L)
{
	int i = lua_tointeger(L, lua_upvalueindex(1));
	LIST<char> &param = *(LIST<char>*)lua_touserdata(L, lua_upvalueindex(2));

	if (i < param.getCount()) {
		lua_pushinteger(L, (i + 1));
		lua_replace(L, lua_upvalueindex(1));
		lua_pushstring(L, ptrA(mir_utf8encode(param[i])));
		mir_free(param[i]);
	}
	else {
		lua_pushnil(L);
		delete &param;
	}

	return 1;
}

static int db_Settings(lua_State *L)
{
	MCONTACT hContact = luaL_optinteger(L, 1, 0);
	const char* szModule = luaL_checkstring(L, 2);

	LIST<char> *param = new LIST<char>(5, PtrKeySortT);
	db_enum_settings(hContact, SettingsEnumProc, szModule, param);

	lua_pushinteger(L, 0);
	lua_pushlightuserdata(L, param);
	lua_pushcclosure(L, db_SettingIterator, 2);

	return 1;
}

static int db_GetSetting(lua_State *L)
{
	MCONTACT hContact = luaL_optinteger(L, 1, 0);
	const char *szModule = luaL_checkstring(L, 2);
	const char *szSetting = luaL_checkstring(L, 3);

	DBVARIANT dbv;
	if (db_get(hContact, szModule, szSetting, &dbv)) {
		lua_pushvalue(L, 4);
		return 1;
	}

	luaM_pushdbvt(L, dbv);
	db_free(&dbv);

	if (lua_isnil(L, -1) && !lua_isnoneornil(L, 4)) {
		lua_pop(L, 1);
		lua_pushvalue(L, 4);
	}

	return 1;
}

static int db_WriteSetting(lua_State *L)
{
	MCONTACT hContact = luaL_optinteger(L, 1, 0);
	const char *szModule = luaL_checkstring(L, 2);
	const char *szSetting = luaL_checkstring(L, 3);
	luaL_checkany(L, 4);

	DBVARIANT dbv;
	if (lua_isnoneornil(L, 5)) {
		int type = lua_type(L, 4);
		switch (type) {
		case LUA_TBOOLEAN:
			dbv.type = DBVT_BYTE;
			break;
		case LUA_TINTEGER:
			dbv.type = DBVT_WORD;
			break;
		case LUA_TNUMBER:
			dbv.type = DBVT_DWORD;
			break;
		case LUA_TSTRING:
			dbv.type = DBVT_UTF8;
			break;
		case LUA_TTABLE:
			dbv.type = DBVT_BLOB;
			break;
		default:
			lua_pushboolean(L, false);
			return 1;
		}
	}
	else
		dbv.type = luaL_checkinteger(L, 5);

	switch (dbv.type) {
	case DBVT_BYTE:
		dbv.bVal = lua_isboolean(L, 4)
			? lua_toboolean(L, 4)
			: luaL_checkinteger(L, 4);
		break;
	case DBVT_WORD:
		dbv.wVal = luaL_checkinteger(L, 4);
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
		dbv.cpbVal = (uint16_t)lua_rawlen(L, 4);
		dbv.pbVal = (uint8_t*)mir_calloc(dbv.cpbVal);
		for (int i = 0; i < dbv.cpbVal; i++) {
			lua_geti(L, 4, i + 1);
			dbv.pbVal[i] = lua_tointeger(L, -1);
			lua_pop(L, 1);
		}
		break;
	}
	default:
		luaL_argerror(L, 4, luaL_typename(L, 4));
	}

	INT_PTR res = db_set(hContact, szModule, szSetting, &dbv);
	lua_pushboolean(L, !res);

	return 1;
}

static int db_DeleteSetting(lua_State *L)
{
	MCONTACT hContact = luaL_optinteger(L, 1, 0);
	LPCSTR szModule = luaL_checkstring(L, 2);
	LPCSTR szSetting = luaL_checkstring(L, 3);

	INT_PTR res = db_unset(hContact, szModule, szSetting);
	lua_pushboolean(L, !res);

	return 1;
}

/***********************************************/

static luaL_Reg databaseApi[] =
{
	{ "Contacts", db_Contacts },
	{ "GetContactInfo", db_GetContactInfo },

	{ "GetEventCount", db_GetEventCount },

	{ "GetFirstEvent", db_GetFirstEvent },
	{ "GetPrevEvent", db_GetPrevEvent },
	{ "GetNextEvent", db_GetNextEvent },
	{ "GetLastEvent", db_GetLastEvent },
	{ "GetFirstUnreadEvent", db_GetFirstUnreadEvent },
	{ "Events", db_Events },
	{ "EventsFromEnd", db_EventsFromEnd },
	{ "UnreadEvents", db_UnreadEvents },
	{ "AddEvent", db_AddEvent },
	{ "DeleteEvent", db_DeleteEvent },
	{ "MarkReadEvent", db_MarkReadEvent },

	{ "Settings", db_Settings },
	{ "Modules", db_Modules },

	{ "DeleteModule", db_DeleteModule },

	{ "GetSetting", db_GetSetting },
	{ "WriteSetting", db_WriteSetting },
	{ "SetSetting", db_WriteSetting },
	{ "DeleteSetting", db_DeleteSetting },

	{ "DBVT_BYTE", nullptr },
	{ "DBVT_WORD", nullptr },
	{ "DBVT_DWORD", nullptr },
	{ "DBVT_ASCIIZ", nullptr },
	{ "DBVT_UTF8", nullptr },
	{ "DBVT_WCHAR", nullptr },

	{ nullptr, nullptr }
};

/***********************************************/

#define MT_DBCONTACTWRITESETTING "DBCONTACTWRITESETTING"

template <>
int MT<DBCONTACTWRITESETTING>::Get(lua_State *L, DBCONTACTWRITESETTING *dbcw)
{
	const char *key = luaL_checkstring(L, 2);

	if (mir_strcmpi(key, "Value") == 0)
		luaM_pushdbvt(L, dbcw->value);
	else
		lua_pushnil(L);

	return 1;
}

/***********************************************/

#define MT_DBEVENTINFO "DBEVENTINFO"

template <>
DBEVENTINFO* MT<DBEVENTINFO>::Init(lua_State *L)
{
	MEVENT hDbEvent = luaL_checkinteger(L, 1);

	DBEVENTINFO *dbei = (DBEVENTINFO *)mir_calloc(sizeof(DBEVENTINFO));
	dbei->cbBlob = -1;
	db_event_get((MEVENT)hDbEvent, dbei);
	return dbei;
}

template <>
int MT<DBEVENTINFO>::Get(lua_State *L, DBEVENTINFO *dbei)
{
	const char *key = luaL_checkstring(L, 2);

	if (mir_strcmpi(key, "Blob") == 0) {
		lua_createtable(L, dbei->cbBlob, 0);
		for (uint32_t i = 0; i < dbei->cbBlob; i++) {
			lua_pushinteger(L, dbei->pBlob[i]);
			lua_rawseti(L, -2, i + 1);
		}
	}
	else
		lua_pushnil(L);

	return 1;
}

template <>
void MT<DBEVENTINFO>::Free(lua_State*, DBEVENTINFO **dbei)
{
	mir_free((*dbei)->pBlob);
	mir_free(*dbei);
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

	MT<DBCONTACTWRITESETTING>(L, MT_DBCONTACTWRITESETTING)
		.Field(&DBCONTACTWRITESETTING::szModule, "Module", LUA_TSTRINGA)
		.Field(&DBCONTACTWRITESETTING::szSetting, "Setting", LUA_TSTRINGA);

	MT<DBEVENTINFO>(L, MT_DBEVENTINFO)
		.Field(&DBEVENTINFO::szModule, "Module", LUA_TSTRINGA)
		.Field(&DBEVENTINFO::timestamp, "Timestamp", LUA_TINTEGER)
		.Field(&DBEVENTINFO::eventType, "Type", LUA_TINTEGER)
		.Field(&DBEVENTINFO::flags, "Flags", LUA_TINTEGER);

	return 1;
}
