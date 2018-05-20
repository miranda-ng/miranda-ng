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
	static int GetEnvironmentId(lua_State *L);

	void AddHookRef(HANDLE h, int ref);
	void ReleaseHookRef(HANDLE h);

	void AddServiceRef(HANDLE h, int ref);
	void ReleaseServiceRef(HANDLE h);

	bool Load();
};
