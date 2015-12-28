#include "stdafx.h"

HANDLE hRecvMessage = NULL;

static void MapToTable(lua_State *L, const PROTOCOLDESCRIPTOR* pd)
{
	lua_newtable(L);
	lua_pushliteral(L, "Name");
	lua_pushstring(L, ptrA(mir_utf8encode(pd->szName)));
	lua_settable(L, -3);
	lua_pushliteral(L, "Type");
	lua_pushinteger(L, pd->type);
	lua_settable(L, -3);
}

static int lua_GetProtocol(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);

	PROTOCOLDESCRIPTOR *pd = ::Proto_IsProtocolLoaded(ptrA(mir_utf8decodeA(name)));
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

static int lua_AllProtocols(lua_State *L)
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

static int lua_EnumProtos(lua_State *L)
{
	ObsoleteMethod(L, "Use \"for x in protos.AllProtos \" instead");

	if (!lua_isfunction(L, 1))
	{
		lua_pushlightuserdata(L, NULL);
		return 1;
	}

	lua_pushvalue(L, 1);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	int count;
	PROTOCOLDESCRIPTOR** protos;
	Proto_EnumProtocols(&count, &protos);

	for (int i = 0; i < count; i++)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
		MapToTable(L, protos[i]);
		luaM_pcall(L, 1, 0);
	}

	luaL_unref(L, LUA_REGISTRYINDEX, ref);
	lua_pushinteger(L, count);

	return 1;
}

static void MapToTable(lua_State *L, const PROTOACCOUNT* pa)
{
	lua_newtable(L);
	lua_pushliteral(L, "ModuleName");
	lua_pushstring(L, ptrA(mir_utf8encode(pa->szModuleName)));
	lua_settable(L, -3);
	lua_pushliteral(L, "AccountName");
	lua_pushstring(L, ptrA(mir_utf8encodeT(pa->tszAccountName)));
	lua_settable(L, -3);
	lua_pushliteral(L, "ProtoName");
	lua_pushstring(L, ptrA(mir_utf8encode(pa->szProtoName)));
	lua_settable(L, -3);
	lua_pushliteral(L, "IsEnabled");
	lua_pushboolean(L, pa->bIsEnabled);
	lua_settable(L, -3);
	lua_pushliteral(L, "IsVisible");
	lua_pushboolean(L, pa->bIsVisible);
	lua_settable(L, -3);
	lua_pushliteral(L, "IsVirtual");
	lua_pushboolean(L, pa->bIsVirtual);
	lua_settable(L, -3);
	lua_pushliteral(L, "IsOldProto");
	lua_pushboolean(L, pa->bOldProto);
	lua_settable(L, -3);
}

static int lua_GetAccount(lua_State *L)
{
	ptrA moduleName(mir_utf8decodeA(luaL_checkstring(L, 1)));

	PROTOACCOUNT* pa = ::Proto_GetAccount(moduleName);
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

static int lua_AllAccounts(lua_State *L)
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

static int lua_EnumAccounts(lua_State *L)
{
	ObsoleteMethod(L, "Use \"for x in protos.AllAccounts \" instead");

	if (!lua_isfunction(L, 1))
	{
		lua_pushlightuserdata(L, NULL);
		return 1;
	}

	lua_pushvalue(L, 1);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	int count;
	PROTOACCOUNT** accounts;
	Proto_EnumAccounts(&count, &accounts);

	for (int i = 0; i < count; i++)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
		MapToTable(L, accounts[i]);
		luaM_pcall(L, 1, 0);
	}

	luaL_unref(L, LUA_REGISTRYINDEX, ref);
	lua_pushinteger(L, count);

	return 1;
}

int ProtoAckHookEventObjParam(void *obj, WPARAM wParam, LPARAM lParam, LPARAM param)
{
	lua_State *L = (lua_State*)obj;

	int ref = param;
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

	lua_pushnumber(L, wParam);

	ACKDATA *ack = (ACKDATA*)lParam;

	lua_newtable(L);
	lua_pushliteral(L, "Module");
	lua_pushstring(L, ptrA(mir_utf8encode(ack->szModule)));
	lua_settable(L, -3);
	lua_pushliteral(L, "hContact");
	lua_pushinteger(L, ack->hContact);
	lua_settable(L, -3);
	lua_pushliteral(L, "Type");
	lua_pushinteger(L, ack->type);
	lua_settable(L, -3);
	lua_pushliteral(L, "Result");
	lua_pushinteger(L, ack->result);
	lua_settable(L, -3);
	lua_pushliteral(L, "hProcess");
	lua_pushlightuserdata(L, ack->hProcess);
	lua_settable(L, -3);
	lua_pushliteral(L, "lParam");
	lua_pushnumber(L, ack->lParam);
	lua_settable(L, -3);

	luaM_pcall(L, 2, 1);

	int res = (int)lua_tointeger(L, 1);

	return res;
}

static int lua_OnProtoAck(lua_State *L)
{
	ObsoleteMethod(L, "Use m.HookEvent instead");

	if (!lua_isfunction(L, 1))
	{
		lua_pushlightuserdata(L, NULL);
		return 1;
	}

	lua_pushvalue(L, 1);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	HANDLE res = ::HookEventObjParam(ME_PROTO_ACK, ProtoAckHookEventObjParam, L, ref);
	lua_pushlightuserdata(L, res);

	CMLua::HookRefs.insert(new HandleRefParam(L, res, ref));

	return 1;
}

int RecvMessageHookEventObjParam(void *obj, WPARAM wParam, LPARAM lParam, LPARAM param)
{
	lua_State *L = (lua_State*)obj;

	int ref = param;
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

	lua_pushnumber(L, wParam);

	CCSDATA *ccs = (CCSDATA*)lParam;
	PROTORECVEVENT *pre = (PROTORECVEVENT*)ccs->lParam;

	lua_newtable(L);
	lua_pushliteral(L, "hContact");
	lua_pushinteger(L, ccs->hContact);
	lua_settable(L, -3);
	lua_pushliteral(L, "Message");
	lua_pushstring(L, pre->szMessage);
	lua_settable(L, -3);

	luaM_pcall(L, 2, 1);

	int res = (int)lua_tointeger(L, 1);

	return res;
}

static int lua_OnReceiveMessage(lua_State *L)
{
	ObsoleteMethod(L, "Use m.HookEvent instead");

	if (!lua_isfunction(L, 1))
	{
		lua_pushlightuserdata(L, NULL);
		return 1;
	}

	lua_pushvalue(L, 1);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	HANDLE res = ::HookEventObjParam(MODULE PSR_MESSAGE, RecvMessageHookEventObjParam, L, ref);
	lua_pushlightuserdata(L, res);

	CMLua::HookRefs.insert(new HandleRefParam(L, res, ref));

	return 1;
}

INT_PTR FilterRecvMessage(WPARAM wParam, LPARAM lParam)
{
	NotifyEventHooks(hRecvMessage, wParam, lParam);

	return Proto_ChainRecv(wParam, (CCSDATA*)lParam);
}

static luaL_Reg protocolsApi[] =
{
	{ "GetProto", lua_GetProtocol },
	{ "GetProtocol", lua_GetProtocol },
	{ "AllProtos", lua_AllProtocols },
	{ "AllProtocols", lua_AllProtocols },
	{ "EnumProtos", lua_EnumProtos },

	{ "GetAccount", lua_GetAccount },
	{ "AllAccounts", lua_AllAccounts },
	{ "EnumAccounts", lua_EnumAccounts },

	{ "OnProtoAck", lua_OnProtoAck },
	{ "OnReceiveMessage", lua_OnReceiveMessage },

	{ NULL, NULL }
};

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
	{ MT_CCSDATA, ccs__init },
	{ "__index", ccs__index },
	{ NULL, NULL }
};

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
