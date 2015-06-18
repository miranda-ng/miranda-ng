#ifndef _LUA_LOADER_H_
#define _LUA_LOADER_H_

class CLuaLoader
{
private:
	CMLua *mLua;

	void LoadScripts(const TCHAR *scriptDir);

public:
	CLuaLoader(CMLua *mLua);

	void LoadScripts();
};

#endif //_LUA_LOADER_H_