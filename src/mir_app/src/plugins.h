#pragma once

// block these plugins
#define DEFMOD_REMOVED_UIPLUGINOPTS     21
#define DEFMOD_REMOVED_PROTOCOLNETLIB   22

// basic export prototypes
typedef int(MIR_CDECL *Miranda_Plugin_Load) (void);
typedef int(MIR_CDECL *Miranda_Plugin_Unload) (void);

struct pluginEntry
{
	char pluginname[64];
	struct
	{
		bool bFailed : 1;      // not a valid plugin, or API is invalid, pluginname is valid
		bool bLoaded : 1;      // Load(void) has been called, Unload() should be called.

		bool bIsCore : 1;		  // a plugin from the /Core directory
		bool bIsService : 1;	  // has Service Mode implementation
		bool bIsLast : 1;		  // this plugin should be unloaded after everything else

		bool bHasBasicApi : 1; // has Load, Unload, MirandaPluginInfo() -> PLUGININFO seems valid, this dll is in memory.
		bool bIsProtocol : 1;  // protocol module
		bool bIsClist : 1;	  // has MIID_CLIST in its interfaces
		bool bIsCrypto : 1;	  // has MIID_CRYPTO in its interfaces
		bool bIsDatabase : 1;  // has MUUID_DATABASE in its interfaces
	};

	void clear()
	{
		pfnLoad = 0;
		pfnUnload = 0;
		m_pInterfaces = 0;
		m_pPlugin = 0;
	}

	// old stubs for pascal plugins
	Miranda_Plugin_Load pfnLoad;
	Miranda_Plugin_Unload pfnUnload;

	MUUID* m_pInterfaces;          // array of supported interfaces or NULL
	CMPluginBase* m_pPlugin;      // pointer to a plugin's instance

	bool checkAPI(wchar_t *plugin);

	int load()
	{	return (pfnLoad == nullptr) ? m_pPlugin->Load() : pfnLoad();
	}

	int unload()
	{	return (pfnUnload == nullptr) ? m_pPlugin->Unload() : pfnUnload();
	}
};

extern LIST<pluginEntry> pluginList, servicePlugins, clistPlugins;
extern MUUID miid_last;

int PluginOptionsInit(WPARAM, LPARAM);

int  getDefaultPluginIdx(const MUUID &muuid);
bool hasMuuid(const MUUID *pFirst, const MUUID&);
bool isPluginBanned(const MUUID &u1);

pluginEntry* OpenPlugin(wchar_t *tszFileName, wchar_t *dir, wchar_t *path);

bool TryLoadPlugin(pluginEntry *p, bool bDynamic);
void Plugin_Uninit(pluginEntry *p);
bool Plugin_UnloadDyn(pluginEntry *p);

typedef BOOL (*SCAN_PLUGINS_CALLBACK) (WIN32_FIND_DATA *fd, wchar_t *path, WPARAM wParam, LPARAM lParam);
void enumPlugins(SCAN_PLUGINS_CALLBACK cb, WPARAM wParam, LPARAM lParam);

struct MuuidReplacement
{
	MUUID uuid;  // default interface plugin
	wchar_t* stdplugname;
	pluginEntry* pImpl; // replacement plugin
};

bool LoadCorePlugin(MuuidReplacement&);

MUUID* GetPluginInterfaces(const wchar_t *ptszFileName, bool &bIsPlugin);
