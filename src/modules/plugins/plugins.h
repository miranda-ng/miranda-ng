
// returns true if the API exports were good, otherwise, passed in data is returned
#define CHECKAPI_NONE 	 0
#define CHECKAPI_CLIST   1

// block these plugins
#define DEFMOD_REMOVED_UIPLUGINOPTS     21
#define DEFMOD_REMOVED_PROTOCOLNETLIB   22

// basic export prototypes
typedef int (__cdecl * Miranda_Plugin_Load) (void);
typedef int (__cdecl * Miranda_Plugin_Unload) (void);
// version control
typedef PLUGININFOEX * (__cdecl * Miranda_Plugin_InfoEx) (DWORD mirandaVersion);
// prototype for clists
typedef int (__cdecl * CList_Initialise) (void);

// can all be NULL
struct BASIC_PLUGIN_INFO
{
	HINSTANCE hInst;
	Miranda_Plugin_Load Load;
	Miranda_Plugin_Unload Unload;
	Miranda_Plugin_InfoEx InfoEx;
	CList_Initialise clistlink;
	PLUGININFOEX * pluginInfo;	 // must be freed if hInst = = NULL then its a copy
	MUUID *Interfaces;          // array of supported interfaces
};

#define PCLASS_FAILED     0x1   // not a valid plugin, or API is invalid, pluginname is valid
#define PCLASS_BASICAPI   0x2   // has Load, Unload, MirandaPluginInfo() -> PLUGININFO seems valid, this dll is in memory.
#define PCLASS_DB         0x4   // has DatabasePluginInfo() and is valid as can be, and PCLASS_BASICAPI has to be set too
#define PCLASS_LAST       0x8   // this plugin should be unloaded after everything else
#define PCLASS_OK        0x10   // plugin should be loaded, if DB means nothing
#define PCLASS_LOADED    0x20   // Load(void) has been called, Unload() should be called.
#define PCLASS_STOPPED   0x40   // wasn't loaded cos plugin name not on white list
#define PCLASS_CLIST     0x80   // a CList implementation
#define PCLASS_SERVICE  0x100   // has Service Mode implementation
#define PCLASS_CORE     0x200   // a plugin from the /Core directory
#define PCLASS_CRYPT    0x400   // crypto provider

struct pluginEntry
{
	TCHAR pluginname[64];
	unsigned int pclass; // PCLASS_*
	int hLangpack;
	BASIC_PLUGIN_INFO bpi;
};

extern LIST<pluginEntry> pluginList, servicePlugins, clistPlugins;
extern MUUID miid_last;

int PluginOptionsInit(WPARAM, LPARAM);
void LoadPluginOptions();
void UnloadPluginOptions();

int isPluginOnWhiteList(const TCHAR* pluginname);
void SetPluginOnWhiteList(const TCHAR* pluginname, int allow);

int getDefaultPluginIdx(const MUUID& muuid);
bool hasMuuid(const BASIC_PLUGIN_INFO&, const MUUID&);
bool hasMuuid(const MUUID* pFirst, const MUUID&);
int equalUUID(const MUUID& u1, const MUUID& u2);
int checkAPI(TCHAR* plugin, BASIC_PLUGIN_INFO* bpi, DWORD mirandaVersion, int checkTypeAPI);

pluginEntry* OpenPlugin(TCHAR *tszFileName, TCHAR *dir, TCHAR *path);

bool TryLoadPlugin(pluginEntry *p, bool bDynamic);
void Plugin_Uninit(pluginEntry *p);
int  Plugin_UnloadDyn(pluginEntry *p);

typedef BOOL (*SCAN_PLUGINS_CALLBACK) (WIN32_FIND_DATA * fd, TCHAR *path, WPARAM wParam, LPARAM lParam);
void enumPlugins(SCAN_PLUGINS_CALLBACK cb, WPARAM wParam, LPARAM lParam);

struct MuuidReplacement
{
	MUUID uuid;  // default interface plugin
	TCHAR* stdplugname;
	pluginEntry* pImpl; // replacement plugin
};

bool LoadCorePlugin( MuuidReplacement& );

MUUID* GetPluginInterfaces(const TCHAR* ptszFileName, bool& bIsPlugin);