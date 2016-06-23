#ifndef _LUA_CORE_H_
#define _LUA_CORE_H_

struct HandleRefParam
{
	HANDLE h;
	int ref;
	lua_State *L;
};

class CMLua
{
private:
	lua_State *L;

	void SetPaths();

public:
	LIST<CMLuaScript> Scripts;

	CMLua();
	~CMLua();

	operator lua_State*() const
	{
		return L;
	}

	void Load();
	void Unload();
};

#endif //_LUA_CORE_H_
