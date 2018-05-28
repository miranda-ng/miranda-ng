#pragma once

class CMLuaScript : public CMLuaEnvironment
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

	const wchar_t *fileName;
	wchar_t filePath[MAX_PATH];

public:
	CMLuaScript(lua_State *L, const wchar_t *path);
	CMLuaScript(const CMLuaScript &script);
	~CMLuaScript();

	const wchar_t* GetFilePath() const;
	const wchar_t* GetFileName() const;

	bool IsEnabled();
	void Enable();
	void Disable();

	Status GetStatus() const;

	int Load() override;
	int Unload() override;

	bool Reload();
};
