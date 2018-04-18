#pragma once

#include <m_core.h>
#include <m_database.h>
#include <m_protocols.h>

extern HINSTANCE g_hInstance;

class MIR_APP_EXPORT CMPluginBase
{
	void tryOpenLog();

protected:
	const char *m_szModuleName;
	HANDLE m_hLogger = nullptr;
	HINSTANCE m_hInst;

	CMPluginBase(HINSTANCE, const char *moduleName);
	~CMPluginBase();

	// pass one of PROTOTYPE_* constants as type
	void RegisterProtocol(int type, pfnInitProto = nullptr, pfnUninitProto = nullptr);
	__forceinline void SetUniqueId(const char *pszUniqueId)
	{
		::Proto_SetUniqueId(m_szModuleName, pszUniqueId);
	}

public:
	void debugLogA(LPCSTR szFormat, ...);
	void debugLogW(LPCWSTR wszFormat, ...);

	__forceinline HINSTANCE getInst() const { return m_hInst; }

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

extern struct CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////
// Basic class for plugins (not protocols) written in C++

typedef BOOL(WINAPI * const _pfnCrtInit)(HINSTANCE, DWORD, LPVOID);

template<class T> class PLUGIN : public CMPluginBase
{
	typedef CMPluginBase CSuper;

public:
	static BOOL WINAPI RawDllMain(HINSTANCE hInstance, DWORD, LPVOID)
	{
		g_hInstance = hInstance;
		return TRUE;
	}

protected:
	PLUGIN(const char *moduleName)
		: CSuper(g_hInstance, moduleName)
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

/////////////////////////////////////////////////////////////////////////////////////////
// Basic class for protocols with accounts

struct CMPlugin;

template<class P> class ACCPROTOPLUGIN : public PLUGIN<CMPlugin>
{
	typedef PLUGIN<CMPlugin> CSuper;

protected:
	ACCPROTOPLUGIN(const char *moduleName) :
		CSuper(moduleName)
	{
		CMPluginBase::RegisterProtocol(PROTOTYPE_PROTOCOL, &fnInit, &fnUninit);
	}

	static PROTO_INTERFACE* fnInit(const char *szModuleName, const wchar_t *wszAccountName)
	{
		P *ppro = new P(szModuleName, wszAccountName);
		g_arInstances.insert(ppro);
		return ppro;
	}

	static int fnUninit(PROTO_INTERFACE *ppro)
	{
		g_arInstances.remove((P*)ppro);
		return 0;
	}

public:
	static OBJLIST<P> g_arInstances;

	static P* getInstance(const char *szProto)
	{
		for (auto &it : g_arInstances)
			if (mir_strcmp(szProto, it->m_szModuleName) == 0)
				return it;

		return nullptr;
	}

	static P* getInstance(MCONTACT hContact)
	{
		return getInstance(::GetContactProto(hContact));
	}
};

template<class P>
OBJLIST<P> ACCPROTOPLUGIN<P>::g_arInstances(1, PtrKeySortT);
