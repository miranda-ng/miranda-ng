#ifndef _LUA_SCRIPT_H_
#define _LUA_SCRIPT_H_

class CMLuaScript : public CMLuaEnviroment
{
public:
	enum Status
	{
		None,
		Loaded,
		Failed
	};

private:
	Status status;
	int unloadRef;

	char *moduleName;
	wchar_t *fileName;
	wchar_t filePath[MAX_PATH];

public:
	CMLuaScript(lua_State *L, const wchar_t *path);
	CMLuaScript(const CMLuaScript &script);
	~CMLuaScript();

	const char* GetModuleName() const;

	const wchar_t* GetFilePath() const;
	const wchar_t* GetFileName() const;

	Status GetStatus() const;

	bool Load();
};

#endif //_LUA_SCRIPT_H_
