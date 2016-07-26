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
	int unloadRef;
	char *moduleName;
	wchar_t *fileName;
	wchar_t filePath[MAX_PATH];
	Status status;


public:
	CMLuaScript(lua_State *L, const wchar_t *path);
	~CMLuaScript();

	static CMLuaScript* GetScriptFromEnviroment(lua_State *L);
	static int GetScriptIdFromEnviroment(lua_State *L);
	static bool GetEnviroment(lua_State *L);

	int GetId() const;

	const char* GetModuleName() const;

	const wchar_t* GetFilePath() const;
	const wchar_t* GetFileName() const;

	const Status GetStatus() const;

	bool Load();
	void Unload();
};

#endif //_LUA_SCRIPT_H_
