#include "stdafx.h"

class CMLuaScriptOptionPage : public CDlgBase
{
private:
	int m_onInitDialogRef;
	int m_onApplyRef;
	lua_State *L;

public:
	CMLuaScriptOptionPage(lua_State *_L, int onInitDialogRef, int onApplyRef)
		: CDlgBase(g_hInstance, IDD_SCRIPTOPTIONSPAGE), L(_L),
		m_onInitDialogRef(onInitDialogRef), m_onApplyRef(onApplyRef)
	{
	}

	void OnInitDialog() override 
	{
		if (m_onInitDialogRef)
		{
			lua_rawgeti(L, LUA_REGISTRYINDEX, m_onInitDialogRef);
			lua_pushlightuserdata(L, m_hwnd);
			luaM_pcall(L, 1, 0);
		}
	}

	void OnApply() override
	{
		if (m_onApplyRef)
		{
			lua_rawgeti(L, LUA_REGISTRYINDEX, m_onApplyRef);
			lua_pushlightuserdata(L, m_hwnd);
			luaM_pcall(L, 1, 0);
		}
	}

	void OnDestroy() override
	{
		lua_pushnil(L);
		lua_rawsetp(L, LUA_REGISTRYINDEX, this);
	}
};

void MakeOptionDialogPage(lua_State *L, OPTIONSDIALOGPAGE &odp)
{
	odp.hInstance = g_hInstance;
	odp.hLangpack = CMLuaEnviroment::GetEnviromentId(L);

	lua_getfield(L, -1, "Flags");
	odp.flags = luaL_optinteger(L, -1, ODPF_BOLDGROUPS | ODPF_UNICODE | ODPF_DONTTRANSLATE);
	lua_pop(L, 1);

	if (!(odp.flags & ODPF_UNICODE))
		odp.flags |= ODPF_UNICODE;

	lua_getfield(L, -1, "Group");
	odp.szGroup.w = mir_utf8decodeW(lua_tostring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "Title");
	odp.szTitle.w = mir_utf8decodeW(luaL_checkstring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "Tab");
	odp.szTab.w = mir_utf8decodeW(lua_tostring(L, -1));
	lua_pop(L, 1);

	int onInitDialogRef = LUA_NOREF;
	lua_getfield(L, -1, "OnInitDialog");
	if (lua_isfunction(L, -1))
		onInitDialogRef = luaL_ref(L, LUA_REGISTRYINDEX);
	else
		lua_pop(L, 1);

	int onApplyRef = LUA_NOREF;
	lua_getfield(L, -1, "OnApply");
	if (lua_isfunction(L, -1))
		onApplyRef = luaL_ref(L, LUA_REGISTRYINDEX);
	else
		lua_pop(L, 1);
	
	lua_State *T = lua_newthread(L);
	lua_rawsetp(L, LUA_REGISTRYINDEX, T);
	odp.pDialog = new CMLuaScriptOptionPage(T, onInitDialogRef, onApplyRef);
}

int opt_AddPage(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
	WPARAM wParam = (WPARAM)lua_touserdata(L, 1);

	OPTIONSDIALOGPAGE odp = { 0 };
	MakeOptionDialogPage(L, odp);

	INT_PTR res = Options_AddPage(wParam, &odp);
	lua_pushboolean(L, !res);

	mir_free(odp.szGroup.w);
	mir_free(odp.szTitle.w);
	mir_free(odp.szTab.w);

	return 1;
}

static luaL_Reg optionsApi[] =
{
	{ "AddPage", opt_AddPage },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_options(lua_State *L)
{
	luaL_newlib(L, optionsApi);

	return 1;
}