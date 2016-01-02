#include "stdafx.h"

HANDLE hRecvMessage = NULL;

static int lua_GetProtocol(lua_State *L)
{
	const char *name = NULL;

	switch (lua_type(L, 1))
	{
	case LUA_TNUMBER:
		name = GetContactProto(lua_tonumber(L, 1));
		break;
	case LUA_TSTRING:
		name = lua_tostring(L, 1);
		break;
	}

	PROTOCOLDESCRIPTOR *pd = Proto_IsProtocolLoaded(name);
	if (pd)
		MT<PROTOCOLDESCRIPTOR>::Set(L, pd);
	else
		lua_pushnil(L);

	return 1;
}

static int lua_ProtocolIterator(lua_State *L)
{
	int i = lua_tointeger(L, lua_upvalueindex(1));
	int count = lua_tointeger(L, lua_upvalueindex(2));
	PROTOCOLDESCRIPTOR **protos = (PROTOCOLDESCRIPTOR**)lua_touserdata(L, lua_upvalueindex(3));

	if (i < count)
	{
		lua_pushinteger(L, (i + 1));
		lua_replace(L, lua_upvalueindex(1));
		MT<PROTOCOLDESCRIPTOR>::Set(L, protos[i]);
	}
	else
		lua_pushnil(L);

	return 1;
}

static int lua_Protocols(lua_State *L)
{
	int count;
	PROTOCOLDESCRIPTOR** protos;
	Proto_EnumProtocols(&count, &protos);

	lua_pushinteger(L, 0);
	lua_pushinteger(L, count);
	lua_pushlightuserdata(L, protos);
	lua_pushcclosure(L, lua_ProtocolIterator, 3);

	return 1;
}

/***********************************************/

static int lua_GetAccount(lua_State *L)
{
	const char *name = NULL;

	switch (lua_type(L, 1))
	{
	case LUA_TNUMBER:
		name = GetContactProto(lua_tonumber(L, 1));
		break;
	case LUA_TSTRING:
		name = lua_tostring(L, 1);
		break;
	}

	PROTOACCOUNT* pa = Proto_GetAccount(name);
	if (pa)
		MT<PROTOACCOUNT>::Set(L, pa);
	else
		lua_pushnil(L);

	return 1;
}

static int lua_AccountIterator(lua_State *L)
{
	int i = lua_tointeger(L, lua_upvalueindex(1));
	int count = lua_tointeger(L, lua_upvalueindex(2));
	PROTOACCOUNT** accounts = (PROTOACCOUNT**)lua_touserdata(L, lua_upvalueindex(3));

	if (i < count)
	{
		lua_pushinteger(L, (i + 1));
		lua_replace(L, lua_upvalueindex(1));
		MT<PROTOACCOUNT>::Set(L, accounts[i]);
	}
	else
		lua_pushnil(L);

	return 1;
}

static int lua_Accounts(lua_State *L)
{
	int count;
	PROTOACCOUNT** accounts;
	Proto_EnumAccounts(&count, &accounts);

	lua_pushinteger(L, 0);
	lua_pushinteger(L, count);
	lua_pushlightuserdata(L, accounts);
	lua_pushcclosure(L, lua_AccountIterator, 3);

	return 1;
}

/***********************************************/

INT_PTR FilterRecvMessage(WPARAM wParam, LPARAM lParam)
{
	NotifyEventHooks(hRecvMessage, wParam, lParam);

	return Proto_ChainRecv(wParam, (CCSDATA*)lParam);
}

/***********************************************/

static luaL_Reg protocolsApi[] =
{
	{ "GetProto", lua_GetProtocol },
	{ "GetProtocol", lua_GetProtocol },
	{ "AllProtos", lua_Protocols },
	{ "AllProtocols", lua_Protocols },
	{ "Protocols", lua_Protocols },

	{ "GetAccount", lua_GetAccount },
	{ "AllAccounts", lua_Accounts },
	{ "Accounts", lua_Accounts },

	{ NULL, NULL }
};

/***********************************************/

#define MT_CCSDATA "CCSDATA"

static int ccs__init(lua_State *L)
{
	CCSDATA *udata = (CCSDATA*)lua_touserdata(L, 1);
	if (udata == NULL)
	{
		lua_pushnil(L);
		return 1;
	}

	CCSDATA **ccs = (CCSDATA**)lua_newuserdata(L, sizeof(CCSDATA*));
	*ccs = udata;

	luaL_setmetatable(L, MT_CCSDATA);

	return 1;
}

static int ccs__index(lua_State *L)
{
	CCSDATA *ccs = *(CCSDATA**)luaL_checkudata(L, 1, MT_CCSDATA);
	const char *key = lua_tostring(L, 2);

	if (mir_strcmpi(key, "hContact") == 0)
		lua_pushinteger(L, ccs->hContact);
	else if (mir_strcmpi(key, "Message") == 0)
	{
		PROTORECVEVENT *pre = (PROTORECVEVENT*)ccs->lParam;
		lua_pushstring(L, pre->szMessage);
	}
	else
		lua_pushnil(L);

	return 1;
}

static luaL_Reg ccsMeta[] =
{
	{ "__init", ccs__init },
	{ "__index", ccs__index },
	{ NULL, NULL }
};

/***********************************************/

LUAMOD_API int luaopen_m_protocols(lua_State *L)
{
	luaL_newlib(L, protocolsApi);

	MT<PROTOCOLDESCRIPTOR>(L, "PROTOCOLDESCRIPTOR")
		.Field(&PROTOCOLDESCRIPTOR::szName, "Name", LUA_TSTRINGA)
		.Field(&PROTOCOLDESCRIPTOR::type, "Type", LUA_TINTEGER);
	lua_pop(L, 1);

	MT<PROTOACCOUNT>(L, "PROTOACCOUNT")
		.Field(&PROTOACCOUNT::szModuleName, "ModuleName", LUA_TSTRINGA)
		.Field(&PROTOACCOUNT::tszAccountName, "AccountName", LUA_TSTRINGW)
		.Field(&PROTOACCOUNT::szProtoName, "ProtoName", LUA_TSTRINGA)
		.Field(&PROTOACCOUNT::bIsEnabled, "IsEnabled", LUA_TBOOLEAN)
		.Field(&PROTOACCOUNT::bIsVisible, "IsVisible", LUA_TBOOLEAN)
		.Field(&PROTOACCOUNT::bIsVirtual, "IsVirtual", LUA_TBOOLEAN)
		.Field(&PROTOACCOUNT::bOldProto, "IsOldProto", LUA_TBOOLEAN);
	lua_pop(L, 1);

	MT<ACKDATA>(L, "ACKDATA")
		.Field(&ACKDATA::szModule, "Module", LUA_TSTRINGA)
		.Field(&ACKDATA::hContact, "hContact", LUA_TINTEGER)
		.Field(&ACKDATA::type, "Type", LUA_TINTEGER)
		.Field(&ACKDATA::result, "Result", LUA_TINTEGER)
		.Field(&ACKDATA::hProcess, "hProcess", LUA_TLIGHTUSERDATA)
		.Field(&ACKDATA::lParam, "lParam", LUA_TLIGHTUSERDATA);
	lua_pop(L, 1);

	luaL_newmetatable(L, MT_CCSDATA);
	luaL_setfuncs(L, ccsMeta, 0);
	lua_pop(L, 1);

	return 1;
}
