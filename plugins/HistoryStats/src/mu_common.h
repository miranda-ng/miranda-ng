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

#define MIRANDA_VER 0x0A00

#include <newpluginapi.h>

#include <m_awaymsg.h>     // not used
#include <m_button.h>      // not used
#include <m_chat.h>        // not used
#include <m_clc.h>
#include <m_clist.h>
#include <m_clistint.h>    // not used
#include <m_clui.h>        // not used
#include <m_contacts.h>
#include <m_database.h>
#include <m_email.h>       // not used
#include <m_file.h>        // not used
#include <m_findadd.h>     // not used
#include <m_fontservice.h> // not used
#include <m_genmenu.h>     // not used
#include <m_history.h>     // not used
#include <m_icolib.h>
#include <m_idle.h>        // not used
#include <m_ignore.h>      // not used
#include <m_langpack.h>
#include <m_message.h>     // not used
#include <m_netlib.h>      // not used
#include <m_options.h>
#include <m_png.h>
#include <m_popup.h>       // not used
#include <m_protocols.h>
#include <m_protomod.h>    // not used
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_system.h>
#include <m_system_cpp.h>  // not used
#include <m_url.h>         // not used
#include <m_userinfo.h>    // not used
#include <m_utils.h>

#include <m_addcontact.h>  // not used, depends on m_protosvc.h
#include <m_icq.h>         // depends on m_protosvc.h

#include <m_metacontacts.h>
#include <m_historystats.h> // our own header

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
		const mu_text* getContactDisplayName(MCONTACT hContact);
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
		int enumSettings(MCONTACT hContact, const mu_ansi* szModule, DBSETTINGENUMPROC pEnumProc, LPARAM lProcParam);
		int getCount();
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
		int getNumContacts(MCONTACT hMetaContact);
		MCONTACT getSubContact(MCONTACT hMetaContact, int iContactNumber);
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
		int enumProtocols(int* numProtocols, PROTOACCOUNT*** ppProtoDescriptors);
		const mu_ansi* getContactBaseProto(MCONTACT hContact);
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
		int terminated();
	}

	/*
	 * updater [external]
	 */

	namespace updater
	{
		bool _available();
		void registerFL(int fileID, const PLUGININFOEX* pluginInfo);
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
	 * core interface functions
	 */

	bool load();
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
