#ifndef _LUA_M_CORE_H_
#define _LUA_M_CORE_H_

#define MLUA_CORE	"m"
LUAMOD_API int (luaopen_m)(lua_State *L);

static LIST<void> Hooks(1, PtrKeySortT);
static LIST<void> Events(1, PtrKeySortT);
static LIST<void> Services(1, PtrKeySortT);

struct HandleRefParam
{
	HANDLE h;
	int ref;
	lua_State *L;
	HandleRefParam(HANDLE h) : L(NULL), h(h), ref(0) { }
	HandleRefParam(lua_State *L, HANDLE h, int ref = 0) : L(L), h(h), ref(ref) { }
};

static LIST<void> HookRefs(1, HandleKeySortT);
static LIST<void> ServiceRefs(1, HandleKeySortT);

void KillModuleEventHooks();
void KillModuleServices();

#endif //_LUA_M_CORE_H_