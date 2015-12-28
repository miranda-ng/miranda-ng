#include "stdafx.h"

int GCHookEventObjParam(void *obj, WPARAM wParam, LPARAM lParam, LPARAM param)
{
	lua_State *L = (lua_State*)obj;

	int ref = param;
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

	WPARAM* wParam_ptr = (WPARAM*)lua_newuserdata(L, sizeof(WPARAM));
	*wParam_ptr = wParam;

	LPARAM* lParam_ptr = (LPARAM*)lua_newuserdata(L, sizeof(LPARAM));
	*lParam_ptr = lParam;

	GCEVENT *gce = (GCEVENT*)lParam;

	lua_newtable(L);
	lua_pushliteral(L, "Module");
	lua_pushstring(L, gce->pDest->pszModule);
	lua_settable(L, -3);
	lua_pushliteral(L, "Id");
	lua_pushstring(L, ptrA(mir_utf8encodeT(gce->pDest->ptszID)));
	lua_settable(L, -3);
	lua_pushliteral(L, "Type");
	lua_pushinteger(L, gce->pDest->iType);
	lua_settable(L, -3);
	lua_pushliteral(L, "Timestamp");
	lua_pushnumber(L, gce->time);
	lua_settable(L, -3);
	lua_pushliteral(L, "IsMe");
	lua_pushboolean(L, gce->bIsMe);
	lua_settable(L, -3);
	lua_pushliteral(L, "Flags");
	lua_pushinteger(L, gce->dwFlags);
	lua_settable(L, -3);
	lua_pushliteral(L, "Uid");
	lua_pushstring(L, ptrA(mir_utf8encodeT(gce->pDest->ptszID)));
	lua_settable(L, -3);
	lua_pushliteral(L, "Nick");
	lua_pushstring(L, ptrA(mir_utf8encodeT(gce->pDest->ptszID)));
	lua_settable(L, -3);
	lua_pushliteral(L, "Status");
	lua_pushstring(L, ptrA(mir_utf8encodeT(gce->pDest->ptszID)));
	lua_settable(L, -3);
	lua_pushliteral(L, "Text");
	lua_pushstring(L, ptrA(mir_utf8encodeT(gce->pDest->ptszID)));
	lua_settable(L, -3);

	luaM_pcall(L, 2, 1);

	int res = (int)lua_tointeger(L, 1);

	return res;
}

static int lua_OnReceiveEvent(lua_State *L)
{
	ObsoleteMethod(L, "Use m.HookEvent instead");

	if (!lua_isfunction(L, 1))
	{
		lua_pushlightuserdata(L, NULL);
		return 1;
	}

	lua_pushvalue(L, 1);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	HANDLE res = ::HookEventObjParam(ME_GC_HOOK_EVENT, GCHookEventObjParam, L, ref);
	lua_pushlightuserdata(L, res);

	CMLua::HookRefs.insert(new HandleRefParam(L, res, ref));

	return 1;
}

static luaL_Reg chatApi[] =
{
	{ "OnReceiveEvent", lua_OnReceiveEvent },

	{ NULL, NULL }
};

#define MT_GCEVENT "GCEVENT"

static int gce__init(lua_State *L)
{
	GCEVENT *udata = (GCEVENT*)lua_touserdata(L, 1);
	if (udata == NULL)
	{
		lua_pushnil(L);
		return 1;
	}

	GCEVENT **gce = (GCEVENT**)lua_newuserdata(L, sizeof(GCEVENT*));
	*gce = udata;

	luaL_setmetatable(L, MT_GCEVENT);

	return 1;
}

static int gce__index(lua_State *L)
{
	GCEVENT *gce = (GCEVENT*)luaL_checkudata(L, 1, MT_GCEVENT);
	const char *key = luaL_checkstring(L, 2);

	if (mir_strcmpi(key, "Module") == 0)
		lua_pushstring(L, gce->pDest->pszModule);
	else if (mir_strcmpi(key, "Id") == 0)
		lua_pushstring(L, ptrA(mir_utf8encodeT(gce->pDest->ptszID)));
	else if (mir_strcmpi(key, "Type") == 0)
		lua_pushinteger(L, gce->pDest->iType);
	else if (mir_strcmpi(key, "Timestamp") == 0)
		lua_pushnumber(L, gce->time);
	else if (mir_strcmpi(key, "IsMe") == 0)
		lua_pushboolean(L, gce->bIsMe);
	else if (mir_strcmpi(key, "Flags") == 0)
		lua_pushinteger(L, gce->dwFlags);
	else if (mir_strcmpi(key, "Uid") == 0)
		lua_pushstring(L, ptrA(mir_utf8encodeT(gce->pDest->ptszID)));
	else if (mir_strcmpi(key, "Nick") == 0)
		lua_pushstring(L, ptrA(mir_utf8encodeT(gce->ptszNick)));
	else if (mir_strcmpi(key, "Status") == 0)
		lua_pushstring(L, ptrA(mir_utf8encodeT(gce->ptszStatus)));
	else if (mir_strcmpi(key, "Text") == 0)
		lua_pushstring(L, ptrA(mir_utf8encodeT(gce->ptszText)));
	else
		lua_pushnil(L);

	return 1;
}

static const luaL_Reg gceMeta[] =
{
	{ MT_GCEVENT, gce__init },
	{ "__index", gce__index },
	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_chat(lua_State *L)
{
	luaL_newlib(L, chatApi);

	luaL_newmetatable(L, MT_GCEVENT);
	luaL_setfuncs(L, gceMeta, 0);
	lua_pop(L, 1);

	return 1;
}
