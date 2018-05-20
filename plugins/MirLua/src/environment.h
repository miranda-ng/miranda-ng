#ifndef _LUA_ENVIRONMENT_H_
#define _LUA_ENVIRONMENT_H_

class CMLuaEnvironment
{
private:
	int m_id;
	std::map<HANDLE, int> m_hookRefs;
	std::map<HANDLE, int> m_serviceRefs;

	void CreateEnvironmentTable();

public:
	lua_State *L;

	CMLuaEnvironment(lua_State *L);
	virtual ~CMLuaEnvironment();

	static CMLuaEnvironment* GetEnvironment(lua_State *L);
	static int GetEnvironmentId(lua_State *L);

	int GetId() const;

	void AddHookRef(HANDLE h, int ref);
	void ReleaseHookRef(HANDLE h);

	void AddServiceRef(HANDLE h, int ref);
	void ReleaseServiceRef(HANDLE h);

	bool Load();
};

#endif //_LUA_ENVIRONMENT_H_
