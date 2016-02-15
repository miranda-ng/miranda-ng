#ifndef _LUA_SCRIPT_H_
#define _LUA_SCRIPT_H_

class CMLuaScript
{
public:
	lua_State *L;

	enum Status
	{
		None,
		Loaded,
		Failed
	};

private:
	int id;
	char *moduleName;
	TCHAR *fileName;
	TCHAR filePath[MAX_PATH];
	Status status;

public:
	CMLuaScript(lua_State *L, const TCHAR *path);
	~CMLuaScript();

	//const int GetId() const;

	static CMLuaScript* GetScriptFromEnviroment(lua_State *L, int n = 1);
	static int GetScriptIdFromEnviroment(lua_State *L, int n = 1);

	const char* GetModuleName() const;

	const TCHAR* GetFilePath() const;
	const TCHAR* GetFileName() const;

	const Status GetStatus() const;

	bool Load();
	void Unload();
};

#endif //_LUA_SCRIPT_H_
