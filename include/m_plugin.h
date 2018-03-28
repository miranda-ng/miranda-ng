#pragma once

#include <m_core.h>
#include <m_database.h>

class MIR_APP_EXPORT CMPlugin
{
	void tryOpenLog();

protected:
	const char *m_szModuleName;
	HANDLE m_hLogger = nullptr;

	CMPlugin(const char *moduleName);
	~CMPlugin();

public:
	void debugLogA(LPCSTR szFormat, ...);
	void debugLogW(LPCWSTR wszFormat, ...);

	__forceinline INT_PTR delSetting(const char *name)
	{
		return db_unset(0, m_szModuleName, name);
	}
	__forceinline INT_PTR delSetting(MCONTACT hContact, const char *name)
	{
		return db_unset(hContact, m_szModuleName, name);
	}

	__forceinline bool getBool(const char *name, bool defaultValue = false)
	{
		return db_get_b(0, m_szModuleName, name, defaultValue) != 0;
	}
	__forceinline bool getBool(MCONTACT hContact, const char *name, bool defaultValue = false)
	{
		return db_get_b(hContact, m_szModuleName, name, defaultValue) != 0;
	}

	__forceinline int getByte(const char *name, BYTE defaultValue = 0)
	{
		return db_get_b(0, m_szModuleName, name, defaultValue);
	}
	__forceinline int getByte(MCONTACT hContact, const char *name, BYTE defaultValue = 0)
	{
		return db_get_b(hContact, m_szModuleName, name, defaultValue);
	}

	__forceinline int getWord(const char *name, WORD defaultValue = 0)
	{
		return db_get_w(0, m_szModuleName, name, defaultValue);
	}
	__forceinline int getWord(MCONTACT hContact, const char *name, WORD defaultValue = 0)
	{
		return db_get_w(hContact, m_szModuleName, name, defaultValue);
	}

	__forceinline DWORD getDword(const char *name, DWORD defaultValue = 0)
	{
		return db_get_dw(0, m_szModuleName, name, defaultValue);
	}
	__forceinline DWORD getDword(MCONTACT hContact, const char *name, DWORD defaultValue = 0)
	{
		return db_get_dw(hContact, m_szModuleName, name, defaultValue);
	}

	__forceinline INT_PTR getString(const char *name, DBVARIANT *result)
	{
		return db_get_s(0, m_szModuleName, name, result);
	}
	__forceinline INT_PTR getString(MCONTACT hContact, const char *name, DBVARIANT *result)
	{
		return db_get_s(hContact, m_szModuleName, name, result);
	}

	__forceinline INT_PTR getWString(const char *name, DBVARIANT *result)
	{
		return db_get_ws(0, m_szModuleName, name, result);
	}
	__forceinline INT_PTR getWString(MCONTACT hContact, const char *name, DBVARIANT *result)
	{
		return db_get_ws(hContact, m_szModuleName, name, result);
	}

	__forceinline char* getStringA(const char *name)
	{
		return db_get_sa(0, m_szModuleName, name);
	}
	__forceinline char* getStringA(MCONTACT hContact, const char *name)
	{
		return db_get_sa(hContact, m_szModuleName, name);
	}

	__forceinline wchar_t* getWStringA(const char *name)
	{
		return db_get_wsa(0, m_szModuleName, name);
	}
	__forceinline wchar_t* getWStringA(MCONTACT hContact, const char *name)
	{
		return db_get_wsa(hContact, m_szModuleName, name);
	}

	__forceinline void setByte(const char *name, BYTE value)
	{
		db_set_b(0, m_szModuleName, name, value);
	}
	__forceinline void setByte(MCONTACT hContact, const char *name, BYTE value)
	{
		db_set_b(hContact, m_szModuleName, name, value);
	}

	__forceinline void setWord(const char *name, WORD value)
	{
		db_set_w(0, m_szModuleName, name, value);
	}
	__forceinline void setWord(MCONTACT hContact, const char *name, WORD value)
	{
		db_set_w(hContact, m_szModuleName, name, value);
	}

	__forceinline void setDword(const char *name, DWORD value)
	{
		db_set_dw(0, m_szModuleName, name, value);
	}
	__forceinline void setDword(MCONTACT hContact, const char *name, DWORD value)
	{
		db_set_dw(hContact, m_szModuleName, name, value);
	}

	__forceinline void setString(const char *name, const char* value)
	{
		db_set_s(0, m_szModuleName, name, value);
	}
	__forceinline void setString(MCONTACT hContact, const char *name, const char* value)
	{
		db_set_s(hContact, m_szModuleName, name, value);
	}

	__forceinline void setWString(const char *name, const wchar_t* value)
	{
		db_set_ws(0, m_szModuleName, name, value);
	}
	__forceinline void setWString(MCONTACT hContact, const char *name, const wchar_t* value)
	{
		db_set_ws(hContact, m_szModuleName, name, value);
	}
};

extern CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////
// Basic class for plugins (not protocols) written in C++

template<class T> class PLUGIN : public CMPlugin
{
	typedef CMPlugin CSuper;

protected:
	PLUGIN(const char *moduleName)
		: CSuper(moduleName)
	{}

	__forceinline HANDLE CreatePluginEvent(const char *name)
	{
		CMStringA str(FORMAT, "%s\\%s", m_szModuleName, name);
		return CreateHookableEvent(str);
	}

	typedef int(__cdecl T::*MyEventFunc)(WPARAM, LPARAM);
	__forceinline void HookPluginEvent(const char *name, MyEventFunc pFunc)
	{
		HookEventObj(name, (MIRANDAHOOKOBJ)*(void**)&pFunc, this);
	}

	typedef INT_PTR(__cdecl T::*MyServiceFunc)(WPARAM, LPARAM);
	__forceinline void CreatePluginService(const char *name, MyServiceFunc pFunc)
	{
		CMStringA str(FORMAT, "%s\\%s", m_szModuleName, name);
		CreateServiceFunctionObj(str, (MIRANDASERVICEOBJ)*(void**)&pFunc, this);
	}

	typedef INT_PTR(__cdecl T::*MyServiceFuncParam)(WPARAM, LPARAM, LPARAM);
	__forceinline void CreatePluginServiceParam(const char *name, MyServiceFuncParam pFunc, LPARAM param)
	{
		CMStringA str(FORMAT, "%s\\%s", m_szModuleName, name);
		CreateServiceFunctionObjParam(str, (MIRANDASERVICEOBJPARAM)*(void**)&pFunc, this, param);
	}
};
