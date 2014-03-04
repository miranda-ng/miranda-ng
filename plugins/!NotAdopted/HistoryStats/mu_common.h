#if !defined(HISTORYSTATS_GUARD_MU_COMMON_H)
#define HISTORYSTATS_GUARD_MU_COMMON_H

/*
 * mu = miranda unified services
 */

#define _WIN32_WINDOWS 0x0500 // for WM_MOUSEWHEEL
#define _WIN32_WINNT   0x0501 // for WM_THEMECHANGED

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

/*
 * include miranda headers
 */

#define MIRANDA_VER 0x0600

#include <miranda/newpluginapi.h>

#include <miranda/m_awaymsg.h>     // not used
#include <miranda/m_button.h>      // not used
#include <miranda/m_chat.h>        // not used
#include <miranda/m_clc.h>
#include <miranda/m_clist.h>
#include <miranda/m_clistint.h>    // not used
#include <miranda/m_clui.h>        // not used
#include <miranda/m_contactdir.h>  // not used
#include <miranda/m_contacts.h>
#include <miranda/m_database.h>
#include <miranda/m_email.h>       // not used
#include <miranda/m_file.h>        // not used
#include <miranda/m_findadd.h>     // not used
#include <miranda/m_fontservice.h> // not used
#include <miranda/m_fuse.h>        // not used
#include <miranda/m_genmenu.h>     // not used
#include <miranda/m_history.h>     // not used
#include <miranda/m_icolib.h>
#include <miranda/m_idle.h>        // not used
#include <miranda/m_ignore.h>      // not used
#include <miranda/m_langpack.h>
#include <miranda/m_message.h>     // not used
#include <miranda/m_netlib.h>      // not used
#include <miranda/m_options.h>
#include <miranda/m_plugins.h>     // not used
#include <miranda/m_png.h>
#include <miranda/m_popup.h>       // not used
#include <miranda/m_protocols.h>
#include <miranda/m_protomod.h>    // not used
#include <miranda/m_protosvc.h>
#include <miranda/m_skin.h>
#include <miranda/m_system.h>
#include <miranda/m_system_cpp.h>  // not used
#include <miranda/m_url.h>         // not used
#include <miranda/m_userinfo.h>    // not used
#include <miranda/m_utils.h>

#include <miranda/m_addcontact.h>  // not used, depends on m_protosvc.h
#include <miranda/m_icq.h>         // depends on m_protosvc.h

#include "include/m_updater.h"
#include "include/m_metacontacts.h"

#include "include/m_historystats.h" // our own header

/*
 * undefine macros that pollute our namespace
 */

// from above
#undef MIRANDA_VER

// from newpluginapi.h
#undef CreateHookableEvent
#undef DestroyHookableEvent
#undef NotifyEventHooks
#undef HookEventMessage
#undef HookEvent
#undef UnhookEvent
#undef CreateServiceFunction
#undef CreateTransientServiceFunction
#undef DestroyServiceFunction
#undef CallService
#undef ServiceExists
#undef CallServiceSync
#undef CallFunctionAsync
#undef SetHookDefaultForHookableEvent
#undef CreateServiceFunctionParam
#undef NotifyEventHooksDirect

// from m_database.h
#undef db_byte_get
#undef db_word_get
#undef db_dword_get
#undef db_get
#undef db_byte_set
#undef db_word_set
#undef db_dword_set
#undef db_string_set
#undef db_unset
#undef DBGetContactSettingByte
#undef DBGetContactSettingWord
#undef DBGetContactSettingDword
#undef DBGetContactSetting
#undef DBGetContactSettingString
#undef DBGetContactSettingWString
#undef DBGetContactSettingUTF8String
#undef DBGetContactSettingTString
#undef db_msg_dbg
#undef DBGetContactSettingStringUtf
#undef DBWriteContactSettingStringUtf
#undef DBGetContactSettingW

// from m_langpack.h
#undef Translate
#undef TranslateW
#undef TranslateT
#undef TranslateTS

// from m_popup.h
#undef PUAddPopUpT
#undef PUChangeTextT
#undef PUChangeT
#undef ShowClassPopupT

// from m_system.h
#undef mir_alloc
#undef mir_free
#undef mir_realloc
#undef mir_calloc
#undef mir_strdup
#undef mir_wstrdup
#undef mir_snprintf
#undef mir_sntprintf
#undef mir_vsnprintf
#undef mir_vsntprintf
#undef mir_a2u_cp
#undef mir_a2u
#undef mir_u2a_cp
#undef mir_u2a
#undef mir_tstrdup
#undef miranda_sys_free
#undef memoryManagerInterface
#undef mir_utf8decode
#undef mir_utf8decodecp
#undef mir_utf8decodeW
#undef mir_utf8encode
#undef mir_utf8encodecp
#undef mir_utf8encodeW
#undef mir_utf8encodeT

// from m_utils.h
#undef mir_md5_init
#undef mir_md5_append
#undef mir_md5_finish
#undef mir_md5_hash
#undef mir_sha1_init
#undef mir_sha1_append
#undef mir_sha1_finish
#undef mir_sha1_hash
#undef TCHAR_STR_PARAM
#undef mir_t2a
#undef mir_a2t
#undef mir_t2u
#undef mir_u2t
#undef mir_t2a_cp
#undef mir_a2t_cp
#undef mir_t2u_cp
#undef mir_u2t_cp

/*
 * basic defines
 */

#if defined(_UNICODE)
	#undef MU_ANSI
	#define MU_WIDE
#else
	#define MU_ANSI
	#undef MU_WIDE
#endif

/*
 * helper macros to avoid many "#if defined(MU_WIDE) ... #else ... #endif" constructs
 */

#if defined(MU_WIDE)
	#define MU_DO_BOTH(ansi, wide) wide
	#define MU_DO_WIDE(wide) wide
	#define MU_DO_ANSI(ansi) (void) 0
#else
	#define MU_DO_BOTH(ansi, wide) ansi
	#define MU_DO_WIDE(wide) (void) 0
	#define MU_DO_ANSI(ansi) ansi
#endif

/*
 * common types mu_wide/mu_ansi/mu_text
 */

typedef wchar_t                   mu_wide;
typedef char                      mu_ansi;
typedef MU_DO_BOTH(char, wchar_t) mu_text;

/*
 * common macros for wrapping text
 */

#define muC(x) x
#define muW(x) L##x
#define muA(x) x
#define muT(x) MU_DO_BOTH(muA(x), muW(x))

/*
 * helper functions
 */

namespace mu
{
	/*
	 * clist
	 */

	namespace clist
	{
		HANDLE addMainMenuItem(const mu_text* pszName, DWORD flags, int position, HICON hIcon, const mu_ansi* pszService, const mu_text* pszPopupName = NULL, int popupPosition = 0, DWORD hotKey = 0);
		HANDLE addContactMenuItem(const mu_text* pszName, DWORD flags, int position, HICON hIcon, const mu_ansi* pszService, DWORD hotKey = 0, const mu_ansi* pszContactOwner = NULL);
		int modifyMenuItem(HANDLE hMenuItem, DWORD toModify, const mu_text* pszName = NULL, DWORD flags = 0, HICON hIcon = NULL, DWORD hotKey = 0);
		const mu_text* getContactDisplayName(HANDLE hContact);
		const mu_text* getStatusModeDescription(int nStatusMode);
	}

	/*
	 * db
	 */

	namespace db
	{
		int getProfilePath(int cbName, mu_text* pszName);
		int getProfileName(int cbName, mu_text* pszName);
		void setSafetyMode(bool safetyMode);
	}

	/*
	 * db_contact
	 */

	namespace db_contact
	{
		int getSetting(HANDLE hContact, const mu_ansi* szModule, const mu_ansi* szSetting, DBVARIANT* dbv);
		int getSettingStr(HANDLE hContact, const mu_ansi* szModule, const mu_ansi* szSetting, DBVARIANT* dbv);
		int freeVariant(DBVARIANT* dbv);
		int writeSetting(HANDLE hContact, DBCONTACTWRITESETTING* dbcws);
		int deleteSetting(HANDLE hContact, const mu_ansi* szModule, const mu_ansi* szSetting);
		int enumSettings(HANDLE hContact, const mu_ansi* szModule, DBSETTINGENUMPROC pEnumProc, LPARAM lProcParam);
		int getCount();
		HANDLE findFirst();
		HANDLE findNext(HANDLE hContact);
	}

	/*
	 * db_event
	 */

	namespace db_event
	{
		HANDLE add(HANDLE hContact, DBEVENTINFO* dbe);
		int getBlobSize(HANDLE hDbEvent);
		int get(HANDLE hDbEvent, DBEVENTINFO* dbe);
		HANDLE findFirst(HANDLE hContact);
		HANDLE findNext(HANDLE hDbEvent);
	}

	/*
	 * db_time
	 */

	namespace db_time
	{
		DWORD timestampToLocal(DWORD timestamp);
	}

	/*
	 * icolib
	 */

	namespace icolib
	{
		bool _available();
		void addIcon(const mu_text* szSection, const mu_text* szDescription, const mu_ansi* szIconName, const mu_ansi* szDefaultFile, int iDefaultIndex, int cx = 16, int cy = 16);
		void addIcon(const mu_text* szSection, const mu_text* szDescription, const mu_ansi* szIconName, HICON hDefaultIcon, int cx = 16, int cy = 16);
		HICON getIcon(const mu_ansi* szIconName);
	}

	/*
	 * langpack
	 */

	namespace langpack
	{
		int translateDialog(HWND hwndDlg, DWORD flags = 0, const int* ignoreControls = NULL);
		const mu_text* translateString(const mu_text* szEnglish);
		UINT getCodePage();
	}

	/*
	 * metacontacts [external]
	 */

	namespace metacontacts
	{
		bool _available();
		HANDLE getMetaContact(HANDLE hSubContact);
		int getNumContacts(HANDLE hMetaContact);
		HANDLE getSubContact(HANDLE hMetaContact, int iContactNumber);
		const mu_ansi* getProtocolName();
	}

	/*
	 * opt
	 */

	namespace opt
	{
		void addPage(WPARAM addInfo, const mu_text* pszGroup, const mu_text* pszTitle, const mu_text* pszTab, DLGPROC pfnDlgProc, const mu_ansi* pszTemplate, HINSTANCE hInstance, DWORD flags = ODPF_BOLDGROUPS);
	}

	/*
	 * png
	 */

	namespace png
	{
		bool _available();
		bool dibToPng(const BITMAPINFOHEADER* pBMIH, const BYTE* pDIData, BYTE* pImageData, long* pImageLen);
	}

	/*
	 * proto
	 */

	namespace proto
	{
		int enumProtocols(int* numProtocols, PROTOCOLDESCRIPTOR*** ppProtoDescriptors);
		const mu_ansi* getContactBaseProto(HANDLE hContact);
	}

	/*
	 * protosvc
	 */

	namespace protosvc
	{
		DWORD getCaps(const mu_ansi* szProto, int flagNum);
		int getName(const mu_ansi* szProto, int cchName, mu_text* szName);
		HICON loadIcon(const mu_ansi* szProto, int whichIcon);
	}

	/*
	 * sha1
	 */

	namespace sha1
	{
		bool _available();
		SHA1_INTERFACE getInterface();
	}

	/*
	 * skin
	 */

	namespace skin
	{
		HICON loadIcon(int id);
	}

	/*
	 * system
	 */

	namespace system
	{
		DWORD getVersion();
		int getVersionText(int cchVersion, mu_ansi* szVersion);
		void threadPush();
		void threadPop();
		int terminated();
	}

	/*
	 * updater [external]
	 */

	namespace updater
	{
		bool _available();
		void registerFL(int fileID, const PLUGININFO* pluginInfo);
	}

	/*
	 * utils
	 */

	namespace utils
	{
		int pathToRelative(const mu_text* pszPath, mu_text* pszNewPath);
		int pathToAbsolute(const mu_text* pszPath, mu_text* pszNewPath);
	}

	/*
	 * _link
	 */

	namespace _link
	{
		extern PLUGINLINK* g_pLink;

		inline HANDLE createHookableEvent(const char* szEvent) { return g_pLink->CreateHookableEvent(szEvent); }
		inline int destroyHookableEvent(HANDLE hEvent) { return g_pLink->DestroyHookableEvent(hEvent); }
		inline int notifyEventHooks(HANDLE hEvent, WPARAM wParam, LPARAM lParam) { return g_pLink->NotifyEventHooks(hEvent, wParam, lParam); }
		inline HANDLE hookEvent(const char* szEvent, MIRANDAHOOK pHookFunc) { return g_pLink->HookEvent(szEvent, pHookFunc); }
		inline HANDLE hookEventMessage(const char* szEvent, HWND hWnd, UINT uData) { return g_pLink->HookEventMessage(szEvent, hWnd, uData); }
		inline int unhookEvent(HANDLE hEvent) { return (hEvent ? g_pLink->UnhookEvent(hEvent) : 0); }
		inline HANDLE createServiceFunction(const char* szService, MIRANDASERVICE pServiceFunc) { return g_pLink->CreateServiceFunction(szService, pServiceFunc); }
		inline HANDLE createTransientServiceFunction(const char* szService, MIRANDASERVICE pServiceFunc) { return g_pLink->CreateTransientServiceFunction(szService, pServiceFunc); }
		inline int destroyServiceFunction(HANDLE hService) { return g_pLink->DestroyServiceFunction(hService); }
		inline int callService(const char* szService, WPARAM wParam, LPARAM lParam) { return g_pLink->CallService(szService, wParam, lParam); }
		inline int serviceExists(const char* szService) { return g_pLink->ServiceExists(szService); }
		inline int callServiceSync(const char* szService, WPARAM wParam, LPARAM lParam) { return g_pLink->CallServiceSync(szService, wParam, lParam); }
		inline int callFunctionAsync(void (__stdcall* pFunc)(void*), void* pData) { return g_pLink->CallFunctionAsync(pFunc, pData); }
		inline int setHookDefaultForHookableEvent(HANDLE hEvent, MIRANDAHOOK pHookFunc) { return g_pLink->SetHookDefaultForHookableEvent(hEvent, pHookFunc); }
		inline HANDLE createServiceFunctionParam(const char* szService, MIRANDASERVICEPARAM pServiceFunc, LPARAM lParam) { return g_pLink->CreateServiceFunctionParam(szService, pServiceFunc, lParam); }
		inline int notifyEventHooksDirect(HANDLE hEvent, WPARAM wParam, LPARAM lParam) { return g_pLink->NotifyEventHooksDirect(hEvent, wParam, lParam); }

		int callProtoService(const char* szModule, const char* szService, WPARAM wParam, LPARAM lParam);
	}

	/*
	 * memory handling
	 */

	void* mirAlloc(size_t size);
	void* mirRealloc(void* memblock, size_t size);
	void mirFree(void* memblock);

	/*
	 * core interface functions
	 */

	bool load(PLUGINLINK* pLink);
	void unload();
	DWORD getMinimalMirandaVersion();
	bool isMirandaVersionOk(DWORD version);
	bool isMirandaUnicode();

	/*
	 * string handling
	 */

	mu_ansi* wideToAnsiDup(const mu_wide* pszWide, UINT uCP = CP_ACP);
	mu_wide* ansiToWideDup(const mu_ansi* pszAnsi, UINT uCP = CP_ACP);
	mu_ansi* wideToAnsi(const mu_wide* pszWide, mu_ansi* pszRes, int maxLen, UINT uCP = CP_ACP);
	mu_wide* ansiToWide(const mu_ansi* pszAnsi, mu_wide* pszRes, int maxLen, UINT uCP = CP_ACP);
	inline void freeWide(mu_wide* pszWide) { free(pszWide); }
	inline void freeAnsi(mu_ansi* pszAnsi) { free(pszAnsi); }
}

#endif // HISTORYSTATS_GUARD_MU_COMMON_H
