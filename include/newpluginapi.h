/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef M_NEWPLUGINAPI_H__
#define M_NEWPLUGINAPI_H__

#if !defined(HIMAGELIST)
typedef struct _IMAGELIST* HIMAGELIST;
#endif

#include <m_gui.h>
#include <m_database.h>
#include <m_protocols.h>

#define PLUGIN_MAKE_VERSION(a, b, c, d)   (((((DWORD)(a))&0xFF)<<24)|((((DWORD)(b))&0xFF)<<16)|((((DWORD)(c))&0xFF)<<8)|(((DWORD)(d))&0xFF))
#define MAXMODULELABELLENGTH 64

#define UNICODE_AWARE 0x0001
#define STATIC_PLUGIN 0x0002

MIR_APP_DLL(int)  GetPluginLangId(const MUUID &uuid, int langId);
MIR_APP_DLL(int)  IsPluginLoaded(const MUUID &uuid);
MIR_APP_DLL(int)  SetServiceModePlugin(const char *szPluginName, WPARAM = 0, LPARAM = 0);

// manually get/set flag specified at Options - Plugins - Enabled
MIR_APP_DLL(bool) IsPluginOnWhiteList(const char *szPluginName);
MIR_APP_DLL(void) SetPluginOnWhiteList(const char *szPluginName, bool bAllow);

/////////////////////////////////////////////////////////////////////////////////////////
// Used to define the end of the MirandaPluginInterface list

#define MIID_LAST  {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}}

/////////////////////////////////////////////////////////////////////////////////////////
// Replaceable internal modules interface ids

#define MIID_HISTORY        {0x5ca0cbc1, 0x999a, 0x4ea2, {0x8b, 0x44, 0xf8, 0xf6, 0x7d, 0x7f, 0x8e, 0xbe}}
#define MIID_UIUSERINFO     {0x570b931c, 0x9af8, 0x48f1, {0xad, 0x9f, 0xc4, 0x49, 0x8c, 0x61, 0x8a, 0x77}}
#define MIID_SRAWAY         {0x5ab54c76, 0x1b4c, 0x4a00, {0xb4, 0x04, 0x48, 0xcb, 0xea, 0x5f, 0xef, 0xe7}}
#define MIID_SREMAIL        {0xd005b5a6, 0x1b66, 0x445a, {0xb6, 0x03, 0x74, 0xd4, 0xd4, 0x55, 0x2d, 0xe2}}
#define MIID_SRFILE         {0x989d104d, 0xacb7, 0x4ee0, {0xb9, 0x6d, 0x67, 0xce, 0x46, 0x53, 0xb6, 0x95}}
#define MIID_UIHISTORY      {0x7f7e3d98, 0xce1f, 0x4962, {0x82, 0x84, 0x96, 0x85, 0x50, 0xf1, 0xd3, 0xd9}}
#define MIID_AUTOAWAY       {0x9c87f7dc, 0x3bd7, 0x4983, {0xb7, 0xfb, 0xb8, 0x48, 0xfd, 0xbc, 0x91, 0xf0}}
#define MIID_USERONLINE     {0x130829e0, 0x2463, 0x4ff8, {0xbb, 0xc8, 0xce, 0x73, 0xc0, 0x18, 0x84, 0x42}}
#define MIID_CRYPTO         {0x415ca6e1, 0x895f, 0x40e6, {0x87, 0xbd, 0x9b, 0x39, 0x60, 0x16, 0xd0, 0xe5}}
#define MIID_POPUP          {0xb275f4a4, 0xe347, 0x4515, {0xaf, 0x71, 0x77, 0xd0, 0x1e, 0xef, 0x54, 0x41}}

/////////////////////////////////////////////////////////////////////////////////////////
// Common plugin interfaces (core plugins)

#define MIID_DATABASE       {0xae77fd33, 0xe484, 0x4dc7, {0x8c, 0xbc, 0x09, 0x9f, 0xed, 0xcc, 0xcf, 0xdd}}
#define MIID_CLIST          {0x9d8da8bf, 0x665b, 0x4908, {0x9e, 0x61, 0x9f, 0x75, 0x98, 0xae, 0x33, 0x0e}}
#define MIID_SRMM           {0x58c7eea6, 0xf9db, 0x4dd9, {0x80, 0x36, 0xae, 0x80, 0x2b, 0xc0, 0x41, 0x4c}}
#define MIID_TESTPLUGIN     {0x53b974f4, 0x3c74, 0x4dba, {0x8f, 0xc2, 0x6f, 0x92, 0xfe, 0x01, 0x3b, 0x8c}}

/////////////////////////////////////////////////////////////////////////////////////////
// Special exception interface for protocols.
// This interface allows more than one plugin to implement it at the same time

#define MIID_PROTOCOL    {0x2a3c815e, 0xa7d9, 0x424b, {0xba, 0x30, 0x2, 0xd0, 0x83, 0x22, 0x90, 0x85}}

#define MIID_SERVICEMODE    {0x8a92c026, 0x953a, 0x4f5f, { 0x99, 0x21, 0xf2, 0xc2, 0xdc, 0x19, 0x5e, 0xc5}}

/////////////////////////////////////////////////////////////////////////////////////////
// Each service mode plugin must implement MS_SERVICEMODE_LAUNCH
// This service might return one of the following values:
// SERVICE_CONTINUE - load Miranda normally, like there's no service plugins at all
// SERVICE_ONLYDB - load database and then execute service plugin only
// SERVICE_MONOPOLY - execute only service plugin, even without database
// SERVICE_FAILED - terminate Miranda execution

#define SERVICE_CONTINUE    0
#define SERVICE_ONLYDB      1
#define SERVICE_MONOPOLY    2
#define SERVICE_FAILED      (-1)

#define MS_SERVICEMODE_LAUNCH "ServiceMode/Launch"

struct PLUGININFOEX
{
	int cbSize;
	const char *shortName;
	uint32_t version;
	const char *description;
	const char *author;
	const char *copyright;
	const char *homepage;
	uint8_t flags;	   // right now the only flag, UNICODE_AWARE, is recognized here
	MUUID uuid;    // plugin's unique identifier
};

/////////////////////////////////////////////////////////////////////////////////////////
// Miranda/System/LoadModule event
// called when a plugin is being loaded dynamically
// wParam = CMPluginBase*
// lParam = HINSTANCE of the loaded plugin

#define ME_SYSTEM_MODULELOAD "Miranda/System/LoadModule"

/////////////////////////////////////////////////////////////////////////////////////////
// Miranda/System/UnloadModule event
// called when a plugin is being unloaded dynamically
// wParam = CMPluginBase*
// lParam = HINSTANCE of the plugin to be unloaded

#define ME_SYSTEM_MODULEUNLOAD "Miranda/System/UnloadModule"

/////////////////////////////////////////////////////////////////////////////////////////
// plugin's class

// initializes an empty account
typedef struct PROTO_INTERFACE* (*pfnInitProto)(const char* szModuleName, const wchar_t* szUserName);

// deallocates an account instance
typedef int(*pfnUninitProto)(PROTO_INTERFACE*);

#pragma warning(push)
#pragma warning(disable:4275)

struct IcolibItem;

class MIR_APP_EXPORT CMPluginBase : public MNonCopyable
{
	void tryOpenLog();

protected:
	HINSTANCE m_hInst;
	const char *m_szModuleName;
	const PLUGININFOEX &m_pInfo;
	HANDLE m_hLogger = nullptr;
	LIST<IcolibItem> m_arIcons;

	CMPluginBase(const char *moduleName, const PLUGININFOEX &pInfo);
	~CMPluginBase();

	// pass one of PROTOTYPE_* constants as type
	void RegisterProtocol(int type, pfnInitProto = nullptr, pfnUninitProto = nullptr);
	void SetUniqueId(const char *pszUniqueId);

public:
	void debugLogA(LPCSTR szFormat, ...);
	void debugLogW(LPCWSTR wszFormat, ...);

	__forceinline void addIcolib(HANDLE hIcolib) { m_arIcons.insert((IcolibItem*)hIcolib); }
	int    addImgListIcon(HIMAGELIST himl, int iconId);
	HICON  getIcon(int iconId, bool big = false);
	HANDLE getIconHandle(int iconId);
	void   releaseIcon(int iconId, bool big = false);

	__forceinline const PLUGININFOEX& getInfo() const { return m_pInfo; }
	__forceinline const char* getModule() const { return m_szModuleName; }

	__forceinline HINSTANCE getInst() const { return m_hInst; }
	__forceinline void setInst(HINSTANCE hInst) { m_hInst = hInst; }

	virtual int Load();
	virtual int Unload();

	////////////////////////////////////////////////////////////////////////////////////////
	// registering module's resources

	template <size_t _Size>
	__forceinline void registerIcon(const char *szSection, IconItem(&pIcons)[_Size], const char *prefix = nullptr)
	{
		Icon_Register(m_hInst, szSection, pIcons, _Size, prefix, this);
	}

	template <size_t _Size>
	__forceinline void registerIconW(const wchar_t *szSection, IconItemT(&pIcons)[_Size], const char *prefix = nullptr)
	{
		Icon_RegisterT(m_hInst, szSection, pIcons, _Size, prefix, this);
	}

	int addOptions(WPARAM wParam, struct OPTIONSDIALOGPAGE *odp);
	void openOptions(const wchar_t *pszGroup, const wchar_t *pszPage = 0, const wchar_t *pszTab = 0);
	void openOptionsPage(const wchar_t *pszGroup, const wchar_t *pszPage = 0, const wchar_t *pszTab = 0);

	HANDLE addIcon(const struct SKINICONDESC*);
	HANDLE addTTB(const struct TTBButton*);

	HGENMENU addRootMenu(int hMenuObject, LPCWSTR ptszName, int position, HANDLE hIcoLib = nullptr);

	int addFont(struct FontID *pFont);
	int addFont(struct FontIDW *pFont);

	int addColor(struct ColourID *pColor);
	int addColor(struct ColourIDW *pColor);

	int addEffect(struct EffectID *pEffect);
	int addEffect(struct EffectIDW *pEffect);

	int addPopupOption(const char *pszDescr, CMOption<bool> &pVar);
	int addPopupOption(const wchar_t *pwszDescr, CMOption<bool> &pVal);

	int addFrame(const struct CLISTFrame*);
	int addHotkey(const struct HOTKEYDESC*);
	int addSound(const char *name, const wchar_t *section, const wchar_t *description, const wchar_t *defaultFile = nullptr);
	int addUserInfo(WPARAM wParam, struct USERINFOPAGE *odp);

	////////////////////////////////////////////////////////////////////////////////////////

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

	__forceinline int getByte(const char *name, int defaultValue = 0)
	{
		return db_get_b(0, m_szModuleName, name, defaultValue);
	}
	__forceinline int getByte(MCONTACT hContact, const char *name, int defaultValue = 0)
	{
		return db_get_b(hContact, m_szModuleName, name, defaultValue);
	}

	__forceinline int getWord(const char *name, int defaultValue = 0)
	{
		return db_get_w(0, m_szModuleName, name, defaultValue);
	}
	__forceinline int getWord(MCONTACT hContact, const char *name, int defaultValue = 0)
	{
		return db_get_w(hContact, m_szModuleName, name, defaultValue);
	}

	__forceinline uint32_t getDword(const char *name, int defaultValue = 0)
	{
		return db_get_dw(0, m_szModuleName, name, defaultValue);
	}
	__forceinline uint32_t getDword(MCONTACT hContact, const char *name, int defaultValue = 0)
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

	__forceinline INT_PTR getUString(const char *name, DBVARIANT *result)
	{
		return db_get_utf(0, m_szModuleName, name, result);
	}
	__forceinline INT_PTR getUString(MCONTACT hContact, const char *name, DBVARIANT *result)
	{
		return db_get_utf(hContact, m_szModuleName, name, result);
	}

	__forceinline INT_PTR getWString(const char *name, DBVARIANT *result)
	{
		return db_get_ws(0, m_szModuleName, name, result);
	}
	__forceinline INT_PTR getWString(MCONTACT hContact, const char *name, DBVARIANT *result)
	{
		return db_get_ws(hContact, m_szModuleName, name, result);
	}

	__forceinline CMStringA getMStringA(const char *name, const char *szValue = nullptr)
	{
		return db_get_sm(0, m_szModuleName, name, szValue);
	}
	__forceinline CMStringA getMStringA(MCONTACT hContact, const char *name, const char *szValue = nullptr)
	{
		return db_get_sm(hContact, m_szModuleName, name, szValue);
	}

	__forceinline char* getStringA(const char *name, const char *szValue = nullptr)
	{
		return db_get_sa(0, m_szModuleName, name, szValue);
	}
	__forceinline char* getStringA(MCONTACT hContact, const char *name, const char *szValue = nullptr)
	{
		return db_get_sa(hContact, m_szModuleName, name, szValue);
	}

	__forceinline char* getUStringA(const char *name, const char *szValue = nullptr)
	{
		return db_get_utfa(0, m_szModuleName, name, szValue);
	}
	__forceinline char* getUStringA(MCONTACT hContact, const char *name, const char *szValue = nullptr)
	{
		return db_get_utfa(hContact, m_szModuleName, name, szValue);
	}

	__forceinline wchar_t* getWStringA(const char *name, const wchar_t *szValue = nullptr)
	{
		return db_get_wsa(0, m_szModuleName, name, szValue);
	}
	__forceinline wchar_t* getWStringA(MCONTACT hContact, const char *name, const wchar_t *szValue = nullptr)
	{
		return db_get_wsa(hContact, m_szModuleName, name, szValue);
	}

	__forceinline CMStringW getMStringW(const char *name, const wchar_t *szValue = nullptr)
	{
		return db_get_wsm(0, m_szModuleName, name, szValue);
	}
	__forceinline CMStringW getMStringW(MCONTACT hContact, const char *name, const wchar_t *szValue = nullptr)
	{
		return db_get_wsm(hContact, m_szModuleName, name, szValue);
	}

	__forceinline void setByte(const char *name, uint8_t value)
	{
		db_set_b(0, m_szModuleName, name, value);
	}
	__forceinline void setByte(MCONTACT hContact, const char *name, uint8_t value)
	{
		db_set_b(hContact, m_szModuleName, name, value);
	}

	__forceinline void setWord(const char *name, uint16_t value)
	{
		db_set_w(0, m_szModuleName, name, value);
	}
	__forceinline void setWord(MCONTACT hContact, const char *name, uint16_t value)
	{
		db_set_w(hContact, m_szModuleName, name, value);
	}

	__forceinline void setDword(const char *name, uint32_t value)
	{
		db_set_dw(0, m_szModuleName, name, value);
	}
	__forceinline void setDword(MCONTACT hContact, const char *name, uint32_t value)
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

	__forceinline void setUString(const char *name, const char* value)
	{
		db_set_utf(0, m_szModuleName, name, value);
	}
	__forceinline void setUString(MCONTACT hContact, const char *name, const char* value)
	{
		db_set_utf(hContact, m_szModuleName, name, value);
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

#pragma warning(pop)

extern struct CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////
// Basic class for plugins (not protocols) written in C++

typedef BOOL(MIR_SYSCALL* const _pfnCrtInit)(HINSTANCE, uint32_t, void*);

template<class T> class PLUGIN : public CMPluginBase
{
	typedef CMPluginBase CSuper;

protected:
	__forceinline PLUGIN(const char *moduleName, const PLUGININFOEX &pInfo)
		: CSuper(moduleName, pInfo)
	{}

	__forceinline HANDLE CreatePluginEvent(const char *name)
	{
		CMStringA str(FORMAT, "%s\\%s", m_szModuleName, name);
		return CreateHookableEvent(str);
	}

	typedef int(MIR_CDECL T::*MyEventFunc)(WPARAM, LPARAM);
	__forceinline void HookPluginEvent(const char *name, MyEventFunc pFunc)
	{
		HookEventObj(name, (MIRANDAHOOKOBJ)*(void**)&pFunc, this);
	}

	typedef INT_PTR(MIR_CDECL T::*MyServiceFunc)(WPARAM, LPARAM);
	__forceinline void CreatePluginService(const char *name, MyServiceFunc pFunc)
	{
		CMStringA str(FORMAT, "%s\\%s", m_szModuleName, name);
		CreateServiceFunctionObj(str, (MIRANDASERVICEOBJ)*(void**)&pFunc, this);
	}

	typedef INT_PTR(MIR_CDECL T::*MyServiceFuncParam)(WPARAM, LPARAM, LPARAM);
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
	ACCPROTOPLUGIN(const char *moduleName, const PLUGININFOEX &pInfo) :
		CSuper(moduleName, pInfo)
	{
		CMPluginBase::RegisterProtocol(1002, &fnInit, &fnUninit);
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
		return getInstance(::Proto_GetBaseAccountName(hContact));
	}
};

template<class P>
OBJLIST<P> ACCPROTOPLUGIN<P>::g_arInstances(1, PtrKeySortT);

#ifndef __NO_CMPLUGIN_NEEDED
#ifdef _DEBUG
#pragma comment(lib, "cmstubd.lib")
#else
#pragma comment(lib, "cmstub.lib")
#endif
#endif

EXTERN_C MIR_APP_DLL(HINSTANCE) GetInstByAddress(void* codePtr);
EXTERN_C MIR_APP_DLL(CMPluginBase&) GetPluginByInstance(HINSTANCE hInst);

#endif // M_NEWPLUGINAPI_H__
