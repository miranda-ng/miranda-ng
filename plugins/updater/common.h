#ifndef _COMMON_INC
#define _COMMON_INC

#define _CRT_SECURE_NO_DEPRECATE

#define MIRANDA_VER 0x0800

#include <m_stdhdr.h>

#include <windows.h>
#include <shlobj.h>
#include <commctrl.h>

#include <process.h>
#include <io.h>

#include "ezxml.h"

#include <newpluginapi.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_idle.h>
#include <m_clui.h>
#include <m_netlib.h>
#include <m_utils.h>
#include <m_clist.h>
#include <m_icoLib.h>

#include <win2k.h>

#include "m_updater.h"
#include "m_trigger.h"
#include "m_folders.h"

//#define TESTING		// causes version to be 0.0.0.1
//#define USE_MY_SERVER	// uses URLS for scottellis.com.au test site for non-beta
#define REGISTER_BETA	// register beta urls for beta site
//#define DEBUG_POPUPS	// define to show some popups (restoring status, etc)
//#define _UD_LOGGING		// define to log some stuff, from inside the external process, overwriting the data file
//#define DEBUG_HTTP_POPUPS	// define to show popups re http
#define REGISTER_AUTO	// get updater to automatically collect file listing URL's from backend xml data

#define BETA_HOST_URL_PREFIX	"http://www.scottellis.com.au/miranda_plugins" //(thanks Omniwolf for old twosx webspace, thx Koobs for hosting my domain)

typedef enum Category {MC_PLUGINS, MC_LOCALIZATION, MC_UNKNOWN, NUM_CATEGORIES};

typedef struct UpdateOptions_tag {
	bool enabled;			// user has enabled updates for this plugin
	bool use_beta;			// use the beta update data
	bool fixed;				// use_beta flag is fixed - cannot be changed by user (e.g. szUpdateURL is null, szBetaUpdateURL is not)
} UpdateOptions;

typedef struct UpdateInternal_tag {
	Update update;
	UpdateOptions update_options;
	int file_id;			// miranda file listing id
	bool auto_register;
	char *newVersion;

	Category cat;

	char *shortName;
} UpdateInternal;

struct UpdateList : OBJLIST<UpdateInternal> 
{
	UpdateList() : OBJLIST<UpdateInternal>(10) {}
	UpdateList(const UpdateList& upd) : OBJLIST<UpdateInternal>(10, compareUrl)
	{
		for (int i=0; i<upd.getCount(); ++i)
			insert(new UpdateInternal(upd[i]));
	}

	static int compareUrl(const UpdateInternal* p1, const UpdateInternal* p2)
	{ 
		if (!p1->update.szBetaVersionURL && !p2->update.szBetaVersionURL)
			return 0;
		else if (!p1->update.szBetaVersionURL && p2->update.szBetaVersionURL)
			return -1;
		else if (p1->update.szBetaVersionURL && !p2->update.szBetaVersionURL)
			return 1;
		else
			return _stricmp(p1->update.szBetaVersionURL, p2->update.szBetaVersionURL); 
	}
};

struct StrList : LIST<TCHAR>
{
    StrList() : LIST<TCHAR>(5) {}
    ~StrList() 
    { 
        for (int i=0; i<getCount(); ++i) mir_free(getArray()[i]); 
        destroy();
    }
};

struct FileNameStruct
{
    int file_id;
    StrList list;

    FileNameStruct(int id) : file_id(id) {}
};

typedef OBJLIST<FileNameStruct> FilenameMap;

#include "resource.h"

#define MODULE			"Updater"

extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;
extern PLUGININFOEX pluginInfo;

extern bool is_idle;

extern HANDLE hNetlibUser, hNetlibHttp;

int FindFileInList(const char *name);

void* memmem (const void *buf1, size_t size1, const void *buf2,  size_t size2);

void NLog(char *msg);
void NLogF(const char *fmt, ...);

#ifdef _UNICODE
void NLog(wchar_t *msg);
#endif

// clist 'exit' menu item command id
#define ID_ICQ_EXIT		40001

// maximum number of redirects when getting xml data or download files via HTTP
#define MAX_REDIRECT_RECURSE		4

#ifndef MIID_UPDATER
#define MIID_UPDATER	{0x4a47b19b, 0xde5a, 0x4436, { 0xab, 0x4b, 0xe1, 0xf3, 0xa0, 0x22, 0x5d, 0xe7}}
#endif

#endif


