#ifndef _LUA_SCRIPT_H_
#define _LUA_SCRIPT_H_

class CMLuaScript
{
public:
	enum Status
	{
		None,
		Loaded,
		Failed
	};

private:
	lua_State *L;

	char *moduleName;
	TCHAR *fileName;
	TCHAR filePath[MAX_PATH];
	Status status;
	int unloadRef;

public:
	CMLuaScript(lua_State *L, const TCHAR *path);
	~CMLuaScript();

	const char* GetModuleName() const;

	const TCHAR* GetFilePath() const;
	const TCHAR* GetFileName() const;

	const Status GetStatus() const;

	bool Load();
	void Unload();
};

#endif //_LUA_SCRIPT_H_
