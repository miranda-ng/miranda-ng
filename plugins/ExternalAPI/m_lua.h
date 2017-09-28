#ifndef _M_LUA_H_
#define _M_LUA_H_

// Call lua function from module
// wParam = module name (or NULL for global function)
// lParam = function name
// Returns a pointer to the resolved string
#define MS_LUA_CALL "Lua/Call"

__inline static wchar_t *lua_call(const wchar_t *module, const wchar_t *function) {
	return (wchar_t*)CallService(MS_LUA_CALL, (WPARAM)module, (LPARAM)function);
}

// Execute lua script from file
// wParam = NULL
// lParam = file path
// Returns a pointer to the resolved string
#define MS_LUA_EXEC "Lua/Exec"

__inline static wchar_t *lua_exec(const wchar_t *path) {
	return (wchar_t*)CallService(MS_LUA_EXEC, NULL, (LPARAM)path);
}

// Evaluate lua script from string
// wParam = NULL
// lParam = lua script
// Returns a pointer to the resolved string
#define MS_LUA_EVAL "Lua/Eval"

__inline static wchar_t *lua_eval(const wchar_t *script) {
	return (wchar_t*)CallService(MS_LUA_EVAL, NULL, (LPARAM)script);
}

#endif //_M_LUA_H_
