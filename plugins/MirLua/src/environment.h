#pragma once

class CMLuaEnvironment : public CMPluginBase, public MZeroedObject
{
private:
	CMPluginBase *m_plugin;

	std::map<HANDLE, int> m_hookRefs;
	std::map<HANDLE, int> m_serviceRefs;

protected:
	void CreateEnvironmentTable();

	wchar_t* Error();

public:
	lua_State *L;

	CMLuaEnvironment(lua_State *L);

	int Unload() override;

	static CMLuaEnvironment* GetEnvironment(lua_State *L);
	static HPLUGIN GetEnvironmentId(lua_State *L);

	HANDLE HookEvent(const char *name, int ref);
	int UnhookEvent(HANDLE hHook);

	HANDLE CreateServiceFunction(const char *name, int ref);
	void DestroyServiceFunction(HANDLE hService);

	wchar_t* Call();
	wchar_t* Eval(const wchar_t *script);
	wchar_t* Exec(const wchar_t *path);
};
