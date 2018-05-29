#pragma once

// returns true if the API exports were good, otherwise, passed in data is returned
#define CHECKAPI_NONE 	 0
#define CHECKAPI_CLIST   1

// block these plugins
#define DEFMOD_REMOVED_UIPLUGINOPTS     21
#define DEFMOD_REMOVED_PROTOCOLNETLIB   22

// basic export prototypes
typedef int(__cdecl *Miranda_Plugin_Load) (void);
typedef int(__cdecl *Miranda_Plugin_Unload) (void);

// prototype for clists
typedef int(__cdecl *CList_Initialise) (void);

struct pluginEntry
{
	wchar_t pluginname[64];
	struct
	{
		bool bFailed : 1;      // not a valid plugin, or API is invalid, pluginname is valid
		bool bOk : 1;			  // plugin should be loaded, if DB means nothing
		bool bLoaded : 1;      // Load(void) has been called, Unload() should be called.
		bool bStopped : 1;     // wasn't loaded cos plugin name not on white list

		bool bIsCore : 1;		  // a plugin from the /Core directory
		bool bIsService : 1;	  // has Service Mode implementation
		bool bIsLast : 1;		  // this plugin should be unloaded after everything else

		bool bHasBasicApi : 1; // has Load, Unload, MirandaPluginInfo() -> PLUGININFO seems valid, this dll is in memory.
		bool bIsProtocol : 1;  // protocol module
		bool bIsDatabase : 1;  // has MUUID_DATABASE in its interfaces, and PCLASS_BASICAPI has to be set too
		bool bIsClist : 1;	  // a CList implementation
		bool bIsCrypto : 1;	  // crypto provider
	};

	void clear()
	{
		pfnLoad = 0;
		pfnUnload = 0;
		m_pInterfaces = 0;
		m_pPlugin = 0;
	}

	// old stubs for pascal plugins
	Miranda_Plugin_Load   pfnLoad;
	Miranda_Plugin_Unload pfnUnload;

	MUUID* m_pInterfaces;          // array of supported interfaces or NULL
	CMPluginBase* m_pPlugin;      // pointer to a plugin's instance
	CList_Initialise m_pClistlink; // link to a clist plugin

	bool checkAPI(wchar_t *plugin, int checkTypeAPI);

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
void LoadPluginOptions();
void UnloadPluginOptions();

int isPluginOnWhiteList(const wchar_t *pluginname);
void SetPluginOnWhiteList(const wchar_t *pluginname, int allow);

int  getDefaultPluginIdx(const MUUID &muuid);
bool hasMuuid(const MUUID *pFirst, const MUUID&);

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
