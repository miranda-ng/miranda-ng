#ifndef _LUA_SCRIPT_H_
#define _LUA_SCRIPT_H_

class CMLuaScript
{
private:
	lua_State *L;

	char* moduleName;
	TCHAR* fileName;
	TCHAR filePath[MAX_PATH];
	bool isLoaded;
	int group;
	int unloadRef;

public:
	CMLuaScript(lua_State *L, const TCHAR* path, int iGroup = 0);
	~CMLuaScript();

	const char* GetModuleName() const;

	const TCHAR* GetFilePath() const;
	const TCHAR* GetFileName() const;
	const int GetGroup() const;

	bool Load();
	void Unload();
};

#endif //_LUA_SCRIPT_H_
