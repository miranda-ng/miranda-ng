#pragma once

class CMLuaEnvironment : public CMPluginBase
{
private:
	std::map<HANDLE, int> m_hookRefs;
	std::map<HANDLE, int> m_serviceRefs;

	void CreateEnvironmentTable();

public:
	lua_State *L;

	CMLuaEnvironment(lua_State *L);
	virtual ~CMLuaEnvironment();

	static CMLuaEnvironment* GetEnvironment(lua_State *L);
	static HPLUGIN GetEnvironmentId(lua_State *L);

	HANDLE HookEvent(const char *name, int ref);
	int UnhookEvent(HANDLE hHook);

	HANDLE CreateServiceFunction(const char *name, int ref);
	void DestroyServiceFunction(HANDLE hService);

	int Load() override;
};
