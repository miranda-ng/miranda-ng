#pragma once

enum class ScriptStatus
{
	None,
	Loaded,
	Failed,
};

class CMLuaScript : public CMLuaEnvironment
{
private:
	wchar_t *scriptName;
	wchar_t filePath[MAX_PATH];
	bool isBinary;
	ScriptStatus status;
	int unloadRef;

public:
	CMLuaScript(lua_State *L, const wchar_t *path);
	CMLuaScript(const CMLuaScript &script);
	~CMLuaScript();

	const wchar_t* GetFilePath() const;
	const wchar_t* GetName() const;

	bool IsBinary() const;

	bool IsEnabled() const;
	void Enable();
	void Disable();

	ScriptStatus GetStatus() const;

	int Load() override;
	int Unload() override;

	bool Reload();

	bool Compile();
};
