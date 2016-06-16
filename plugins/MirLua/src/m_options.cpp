#include "stdafx.h"

class CMLuaScriptOptionPage : public CDlgBase
{
private:
	CMLuaScript *script;
	int onInitDialogRef;
	int onApplyRef;

public:
	CMLuaScriptOptionPage(CMLuaScript *script, int onInitDialogRef, int onApplyRef)
		: CDlgBase(g_hInstance, IDD_SCRIPTOPTIONSPAGE), script(script),
		onInitDialogRef(onInitDialogRef), onApplyRef(onApplyRef)
	{
	}

protected:
	void OnInitDialog()
	{
		if (onInitDialogRef)
		{
			lua_rawgeti(script->L, LUA_REGISTRYINDEX, onInitDialogRef);

			lua_pushlightuserdata(script->L, (void*)this->GetHwnd());
			luaM_pcall(script->L, 1, 0);
		}
	}

	void OnApply()
	{
		if (onApplyRef)
		{
			lua_rawgeti(script->L, LUA_REGISTRYINDEX, onApplyRef);

			lua_pushlightuserdata(script->L, (void*)this->GetHwnd());
			luaM_pcall(script->L, 1, 0);
		}
	}
};

void MakeOptionDialogPage(lua_State *L, OPTIONSDIALOGPAGE &odp)
{
	CMLuaScript *script = CMLuaScript::GetScriptFromEnviroment(L);

	odp.hInstance = g_hInstance;
	odp.hLangpack = script->GetId();

	lua_getfield(L, -1, "Flags");
	odp.flags = luaL_optinteger(L, -1, ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_DONTTRANSLATE);
	lua_pop(L, 1);

	if (!(odp.flags & ODPF_TCHAR))
		odp.flags |= ODPF_TCHAR;

	lua_getfield(L, -1, "Group");
	odp.ptszGroup = mir_utf8decodeT((char*)lua_tostring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "Title");
	odp.ptszTitle = mir_utf8decodeT((char*)luaL_checkstring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "Tab");
	odp.ptszTab = mir_utf8decodeT((char*)lua_tostring(L, -1));
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

	odp.pDialog = new CMLuaScriptOptionPage(script, onInitDialogRef, onApplyRef);
}

int opt_AddPage(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
	WPARAM wParam = (WPARAM)lua_touserdata(L, 1);

	OPTIONSDIALOGPAGE odp = { 0 };
	MakeOptionDialogPage(L, odp);

	INT_PTR res = Options_AddPage(wParam, &odp);
	lua_pushboolean(L, !res);

	mir_free(odp.ptszGroup);
	mir_free(odp.ptszTitle);
	mir_free(odp.ptszTab);

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