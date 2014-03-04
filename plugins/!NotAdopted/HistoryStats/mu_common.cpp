#include "_globals.h"
#include "mu_common.h"

#include <map>
#include <set>

namespace mu
{
	/*
	 * clist
	 */

	namespace clist
	{
		HANDLE addMainMenuItem(const mu_text* pszName, DWORD flags, int position, HICON hIcon, const mu_ansi* pszService, const mu_text* pszPopupName /* = NULL */, int popupPosition /* = 0 */, DWORD hotKey /* = 0 */)
		{
			// TODO: support for unicode-core with unicode-aware CList
			CLISTMENUITEM mi;

			ZeroMemory(&mi, sizeof(mi));

			mi.cbSize = sizeof(mi);
			mi.pszName = MU_DO_BOTH(const_cast<mu_ansi*>(pszName), wideToAnsiDup(pszName));
			mi.flags = flags;
			mi.position = position;
			mi.hIcon = hIcon;
			mi.pszService = const_cast<mu_ansi*>(pszService);
			mi.pszPopupName = MU_DO_BOTH(const_cast<mu_ansi*>(pszPopupName), wideToAnsiDup(pszPopupName));
			mi.popupPosition = popupPosition;
			mi.hotKey = hotKey;

			HANDLE res = reinterpret_cast<HANDLE>(_link::callService(MS_CLIST_ADDMAINMENUITEM, 0, reinterpret_cast<LPARAM>(&mi)));

			MU_DO_WIDE(freeAnsi(mi.pszName));
			MU_DO_WIDE(freeAnsi(mi.pszPopupName));

			return res;
		}

		HANDLE addContactMenuItem(const mu_text* pszName, DWORD flags, int position, HICON hIcon, const mu_ansi* pszService, DWORD hotKey /* = 0 */, const mu_ansi* pszContactOwner /* = NULL */)
		{
			// TODO: support for unicode-core with unicode-aware CList
			CLISTMENUITEM mi;

			ZeroMemory(&mi, sizeof(mi));

			mi.cbSize = sizeof(mi);
			mi.pszName = MU_DO_BOTH(const_cast<mu_ansi*>(pszName), wideToAnsiDup(pszName));
			mi.flags = flags;
			mi.position = position;
			mi.hIcon = hIcon;
			mi.pszService = const_cast<mu_ansi*>(pszService);
			mi.hotKey = hotKey;
			mi.pszContactOwner = const_cast<mu_ansi*>(pszContactOwner);

			HANDLE res = reinterpret_cast<HANDLE>(_link::callService(MS_CLIST_ADDCONTACTMENUITEM, 0, reinterpret_cast<LPARAM>(&mi)));

			MU_DO_WIDE(freeAnsi(mi.pszName));

			return res;
		}

		int modifyMenuItem(HANDLE hMenuItem, DWORD toModify, const mu_text* pszName /* = NULL */, DWORD flags /* = 0 */, HICON hIcon /* = NULL */, DWORD hotKey /* = 0 */)
		{
			// TODO: support for unicode-core with unicode-aware CList
			CLISTMENUITEM mi;

			ZeroMemory(&mi, sizeof(mi));

			mi.cbSize = sizeof(mi);
			mi.pszName = MU_DO_BOTH(const_cast<mu_ansi*>(pszName), wideToAnsiDup(pszName));
			mi.flags = toModify | flags;
			mi.hIcon = hIcon;
			mi.hotKey = hotKey;

			int res = _link::callService(MS_CLIST_MODIFYMENUITEM, reinterpret_cast<WPARAM>(hMenuItem), reinterpret_cast<LPARAM>(&mi));

			MU_DO_WIDE(freeAnsi(mi.pszName));

			return res;
		}

		const mu_text* getContactDisplayName(HANDLE hContact)
		{
			return reinterpret_cast<const mu_text*>(_link::callService(MS_CLIST_GETCONTACTDISPLAYNAME, reinterpret_cast<WPARAM>(hContact), MU_DO_BOTH(0, GCDNF_UNICODE)));
		}

		const mu_text* getStatusModeDescription(int nStatusMode)
		{
			return reinterpret_cast<const mu_text*>(_link::callService(MS_CLIST_GETSTATUSMODEDESCRIPTION, static_cast<WPARAM>(nStatusMode), MU_DO_BOTH(0, GCMDF_UNICODE)));
		}
	}

	/*
	 * db
	 */

	namespace db
	{
		int getProfilePath(int cbName, mu_text* pszName)
		{
			// TODO: support for unicode core (if supported)
#if defined(MU_WIDE)
			mu_ansi* pszNameAnsi = new mu_ansi[cbName];

			int ret = _link::callService(MS_DB_GETPROFILEPATH, cbName, reinterpret_cast<LPARAM>(pszNameAnsi));

			if (ret == 0)
			{
				ansiToWide(pszNameAnsi, pszName, cbName);
			}

			delete pszNameAnsi;

			return ret;
#else // MU_WIDE
			return _link::callService(MS_DB_GETPROFILEPATH, cbName, reinterpret_cast<LPARAM>(pszName));
#endif // MU_WIDE
		}

		int getProfileName(int cbName, mu_text* pszName)
		{
			// TODO: support for unicode core (if supported)
#if defined(MU_WIDE)
			mu_ansi* pszNameAnsi = new mu_ansi[cbName];

			int ret = _link::callService(MS_DB_GETPROFILENAME, cbName, reinterpret_cast<LPARAM>(pszNameAnsi));

			if (ret == 0)
			{
				ansiToWide(pszNameAnsi, pszName, cbName);
			}

			delete pszNameAnsi;

			return ret;
#else // MU_WIDE
			return _link::callService(MS_DB_GETPROFILENAME, cbName, reinterpret_cast<LPARAM>(pszName));
#endif // MU_WIDE
		}

		void setSafetyMode(bool safetyMode)
		{
			mu::_link::callService(MS_DB_SETSAFETYMODE, BOOL_(safetyMode), 0);
		}
	}

	/*
	 * db_contact
	 */

	namespace db_contact
	{
		int getSetting(HANDLE hContact, const char* szModule, const char* szSetting, DBVARIANT* dbv)
		{
			DBCONTACTGETSETTING dbcgs;

			dbcgs.szModule = szModule;
			dbcgs.szSetting = szSetting;
			dbcgs.pValue = dbv;

			return _link::callService(MS_DB_CONTACT_GETSETTING, reinterpret_cast<WPARAM>(hContact), reinterpret_cast<LPARAM>(&dbcgs));
		}

		int getSettingStr(HANDLE hContact, const mu_ansi* szModule, const mu_ansi* szSetting, DBVARIANT* dbv)
		{
			DBCONTACTGETSETTING dbcgs;

			dbcgs.szModule = szModule;
			dbcgs.szSetting = szSetting;
			dbcgs.pValue = dbv;

			return _link::callService(MS_DB_CONTACT_GETSETTING_STR, reinterpret_cast<WPARAM>(hContact), reinterpret_cast<LPARAM>(&dbcgs));
		}

		int freeVariant(DBVARIANT* dbv)
		{
			return _link::callService(MS_DB_CONTACT_FREEVARIANT, 0, reinterpret_cast<WPARAM>(dbv));
		}

		int writeSetting(HANDLE hContact, DBCONTACTWRITESETTING* dbcws)
		{
			return _link::callService(MS_DB_CONTACT_WRITESETTING, reinterpret_cast<WPARAM>(hContact), reinterpret_cast<LPARAM>(dbcws));
		}

		int deleteSetting(HANDLE hContact, const char* szModule, const char* szSetting)
		{
			DBCONTACTGETSETTING dbcgs;

			dbcgs.szModule = szModule;
			dbcgs.szSetting = szSetting;

			return _link::callService(MS_DB_CONTACT_DELETESETTING, reinterpret_cast<WPARAM>(hContact), reinterpret_cast<LPARAM>(&dbcgs));
		}

		int enumSettings(HANDLE hContact, const char* szModule, DBSETTINGENUMPROC pEnumProc, LPARAM lProcParam)
		{
			DBCONTACTENUMSETTINGS dbces;
			
			dbces.pfnEnumProc = pEnumProc;
			dbces.lParam = lProcParam;
			dbces.szModule = szModule;
			dbces.ofsSettings = 0;

			return _link::callService(MS_DB_CONTACT_ENUMSETTINGS, reinterpret_cast<WPARAM>(hContact), reinterpret_cast<LPARAM>(&dbces));
		}

		int getCount()
		{
			return _link::callService(MS_DB_CONTACT_GETCOUNT, 0, 0);
		}

		HANDLE findFirst()
		{
			return reinterpret_cast<HANDLE>(_link::callService(MS_DB_CONTACT_FINDFIRST, 0, 0));
		}

		HANDLE findNext(HANDLE hContact)
		{
			return reinterpret_cast<HANDLE>(_link::callService(MS_DB_CONTACT_FINDNEXT, reinterpret_cast<WPARAM>(hContact), 0));
		}
	}

	/*
	 * db_event
	 */

	namespace db_event
	{
		HANDLE add(HANDLE hContact, DBEVENTINFO* dbe)
		{
			return reinterpret_cast<HANDLE>(_link::callService(MS_DB_EVENT_ADD, reinterpret_cast<WPARAM>(hContact), reinterpret_cast<LPARAM>(dbe)));
		}

		int getBlobSize(HANDLE hDbEvent)
		{
			return _link::callService(MS_DB_EVENT_GETBLOBSIZE, reinterpret_cast<WPARAM>(hDbEvent), 0);
		}

		int get(HANDLE hDbEvent, DBEVENTINFO* dbe)
		{
			return _link::callService(MS_DB_EVENT_GET, reinterpret_cast<WPARAM>(hDbEvent), reinterpret_cast<LPARAM>(dbe));
		}

		HANDLE findFirst(HANDLE hContact)
		{
			return reinterpret_cast<HANDLE>(_link::callService(MS_DB_EVENT_FINDFIRST, reinterpret_cast<WPARAM>(hContact), 0));
		}

		HANDLE findNext(HANDLE hDbEvent)
		{
			return reinterpret_cast<HANDLE>(_link::callService(MS_DB_EVENT_FINDNEXT, reinterpret_cast<WPARAM>(hDbEvent), 0));
		}
	}

	/*
	 * db_time
	 */

	namespace db_time
	{
		DWORD timestampToLocal(DWORD timestamp)
		{
			return static_cast<DWORD>(_link::callService(MS_DB_TIME_TIMESTAMPTOLOCAL, static_cast<WPARAM>(timestamp), 0));
		}
	}

	/*
	 * icolib
	 */

	namespace icolib
	{
		bool _available()
		{
			return
				true &&
				_link::serviceExists(MS_SKIN2_ADDICON) &&
				_link::serviceExists(MS_SKIN2_GETICON);
		}

		void addIcon(const mu_text* szSection, const mu_text* szDescription, const mu_ansi* szIconName, const mu_ansi* szDefaultFile, int iDefaultIndex, int cx /* = 16 */, int cy /* = 16 */)
		{
			SKINICONDESC sid;

			sid.cbSize = sizeof(sid);
			sid.ptszSection = const_cast<mu_text*>(szSection);
			sid.ptszDescription = const_cast<mu_text*>(szDescription);
			sid.pszName = const_cast<mu_ansi*>(szIconName);
			sid.pszDefaultFile = const_cast<mu_ansi*>(szDefaultFile);
			sid.iDefaultIndex = iDefaultIndex;
			sid.hDefaultIcon = NULL;
			sid.cx = cx;
			sid.cy = cy;
			sid.flags = MU_DO_BOTH(0, SIDF_UNICODE);

			_link::callService(MS_SKIN2_ADDICON, 0, reinterpret_cast<LPARAM>(&sid));
		}

		void addIcon(const mu_text* szSection, const mu_text* szDescription, const mu_ansi* szIconName, HICON hDefaultIcon, int cx /* = 16 */, int cy /* = 16 */)
		{
			SKINICONDESC sid;

			sid.cbSize = sizeof(sid);
			sid.ptszSection = const_cast<mu_text*>(szSection);
			sid.ptszDescription = const_cast<mu_text*>(szDescription);
			sid.pszName = const_cast<mu_ansi*>(szIconName);
			sid.pszDefaultFile = NULL;
			sid.iDefaultIndex = 0;
			sid.hDefaultIcon = hDefaultIcon;
			sid.cx = cx;
			sid.cy = cy;
			sid.flags = MU_DO_BOTH(0, SIDF_UNICODE);

			_link::callService(MS_SKIN2_ADDICON, 0, reinterpret_cast<LPARAM>(&sid));
		}

		HICON getIcon(const mu_ansi* szIconName)
		{
			return reinterpret_cast<HICON>(_link::callService(MS_SKIN2_GETICON, 0, reinterpret_cast<LPARAM>(szIconName)));
		}
	}

	/*
	 * langpack
	 */

	namespace langpack
	{
		const mu_text* translateString(const mu_text* szEnglish)
		{
			return reinterpret_cast<const mu_text*>(_link::callService(MS_LANGPACK_TRANSLATESTRING, MU_DO_BOTH(0, LANG_UNICODE), reinterpret_cast<LPARAM>(szEnglish)));
		}

		int translateDialog(HWND hwndDlg, DWORD flags /* = 0 */, const int* ignoreControls /* = NULL */)
		{
			LANGPACKTRANSLATEDIALOG lptd;

			lptd.cbSize = sizeof(lptd);
			lptd.flags = flags;
			lptd.hwndDlg = hwndDlg;
			lptd.ignoreControls = ignoreControls;

			return _link::callService(MS_LANGPACK_TRANSLATEDIALOG, 0, reinterpret_cast<LPARAM>(&lptd));
		}

		UINT getCodePage()
		{
			static UINT CodePage = -1;

			if (CodePage == -1)
			{
				CodePage = _link::serviceExists(MS_LANGPACK_GETCODEPAGE) ? _link::callService(MS_LANGPACK_GETCODEPAGE, 0, 0) : CP_ACP;
			}

			return CodePage;
		}
	}

	/*
	 * metacontacts [external]
	 */

	namespace metacontacts
	{
		bool _available()
		{
			return
				true &&
				_link::serviceExists(MS_MC_GETMETACONTACT) &&
				_link::serviceExists(MS_MC_GETNUMCONTACTS) &&
				_link::serviceExists(MS_MC_GETSUBCONTACT) &&
				_link::serviceExists(MS_MC_GETPROTOCOLNAME);
		}

		HANDLE getMetaContact(HANDLE hSubContact)
		{
			return reinterpret_cast<HANDLE>(_link::callService(MS_MC_GETMETACONTACT, reinterpret_cast<WPARAM>(hSubContact), 0));
		}

		int getNumContacts(HANDLE hMetaContact)
		{
			return _link::callService(MS_MC_GETNUMCONTACTS, reinterpret_cast<WPARAM>(hMetaContact), 0);
		}

		HANDLE getSubContact(HANDLE hMetaContact, int iContactNumber)
		{
			return reinterpret_cast<HANDLE>(_link::callService(MS_MC_GETSUBCONTACT, reinterpret_cast<WPARAM>(hMetaContact), iContactNumber));
		}

		const mu_ansi* getProtocolName()
		{
			return reinterpret_cast<const mu_ansi*>(_link::callService(MS_MC_GETPROTOCOLNAME, 0, 0));
		}
	}

	/*
	 * opt
	 */

	namespace opt
	{
		void addPage(WPARAM addInfo, const mu_text* pszGroup, const mu_text* pszTitle, const mu_text* pszTab, DLGPROC pfnDlgProc, const mu_ansi* pszTemplate, HINSTANCE hInstance, DWORD flags /* = ODPF_BOLDGROUPS */)
		{
			OPTIONSDIALOGPAGE odp;

			ZeroMemory(&odp, sizeof(odp));

			odp.cbSize = sizeof(odp);
			odp.ptszTitle = const_cast<mu_text*>(pszTitle);
			odp.pfnDlgProc = pfnDlgProc;
			odp.pszTemplate = const_cast<mu_ansi*>(pszTemplate);
			odp.hInstance = hInstance;
			odp.ptszGroup = const_cast<mu_text*>(pszGroup);
			odp.flags = flags | MU_DO_BOTH(0, ODPF_UNICODE);
			odp.ptszTab = const_cast<mu_text*>(pszTab);
			
			_link::callService(MS_OPT_ADDPAGE, addInfo, reinterpret_cast<LPARAM>(&odp));
		}
	}

	/*
	 * png
	 */

	namespace png
	{
		bool _available()
		{
			return
				true &&
				_link::serviceExists(MS_DIB2PNG);
		}

		bool dibToPng(const BITMAPINFOHEADER* pBMIH, const BYTE* pDIData, BYTE* pImageData, long* pImageLen)
		{
			DIB2PNG info;

			info.pbmi = const_cast<BITMAPINFO*>(reinterpret_cast<const BITMAPINFO*>(pBMIH));
			info.pDiData = const_cast<BYTE*>(pDIData);
			info.pResult = pImageData;
			info.pResultLen = pImageLen;

			return bool_(_link::callService(MS_DIB2PNG, 0, reinterpret_cast<LPARAM>(&info)));
		}
	}

	/*
	 * proto
	 */

	namespace proto
	{
		int enumProtocols(int* numProtocols, PROTOCOLDESCRIPTOR*** ppProtoDescriptors)
		{
			return _link::callService(MS_PROTO_ENUMPROTOCOLS, reinterpret_cast<WPARAM>(numProtocols), reinterpret_cast<LPARAM>(ppProtoDescriptors));
		}

		const mu_ansi* getContactBaseProto(HANDLE hContact)
		{
			return reinterpret_cast<const mu_ansi*>(_link::callService(MS_PROTO_GETCONTACTBASEPROTO, reinterpret_cast<WPARAM>(hContact), 0));
		}
	}

	/*
	 * protosvc
	 */

	namespace protosvc
	{
		DWORD getCaps(const mu_ansi* szProto, int flagNum)
		{
			return static_cast<DWORD>(_link::callProtoService(szProto, PS_GETCAPS, static_cast<WPARAM>(flagNum), 0));
		}

		int getName(const mu_ansi* szProto, int cchName, mu_text* szName)
		{
			// TODO: support for unicode core (if supported)
#if defined(MU_WIDE)
			mu_ansi* szNameANSI = reinterpret_cast<mu_ansi*>(malloc(sizeof(mu_ansi) * cchName));
			
			int retVal = _link::callProtoService(szProto, PS_GETNAME, static_cast<WPARAM>(cchName), reinterpret_cast<LPARAM>(szNameANSI));

			if (retVal == 0)
			{
				ansiToWide(szNameANSI, szName, cchName, langpack::getCodePage());
			}
			
			free(szNameANSI);

			return retVal;
#else // MU_WIDE
			return _link::callProtoService(szProto, PS_GETNAME, static_cast<WPARAM>(cchName), reinterpret_cast<LPARAM>(szName));
#endif // MU_WIDE
		}

		HICON loadIcon(const mu_ansi* szProto, int whichIcon)
		{
			return reinterpret_cast<HICON>(_link::callProtoService(szProto, PS_LOADICON, static_cast<WPARAM>(whichIcon), 0));
		}
	}

	/*
	 * sha1
	 */

	namespace sha1
	{
		bool _available()
		{
			return
				true &&
				_link::serviceExists(MS_SYSTEM_GET_SHA1I);
		}

		SHA1_INTERFACE getInterface()
		{
			SHA1_INTERFACE sha1i;
			
			sha1i.cbSize = sizeof(SHA1_INTERFACE);
			_link::callService(MS_SYSTEM_GET_SHA1I, 0, reinterpret_cast<LPARAM>(&sha1i));

			return sha1i;
		}
	}

	/*
	 * skin
	 */

	namespace skin
	{
		HICON loadIcon(int id)
		{
			return reinterpret_cast<HICON>(_link::callService(MS_SKIN_LOADICON, id, 0));
		}
	}

	/*
	 * system
	 */

	namespace system
	{
		DWORD getVersion()
		{
			return static_cast<DWORD>(_link::callService(MS_SYSTEM_GETVERSION, 0, 0));
		}

		int getVersionText(int cchVersion, mu_ansi* szVersion)
		{
			return _link::callService(MS_SYSTEM_GETVERSIONTEXT, cchVersion, reinterpret_cast<LPARAM>(szVersion));
		}

		void threadPush()
		{
			_link::callService(MS_SYSTEM_THREAD_PUSH, 0, 0);
		}

		void threadPop()
		{
			_link::callService(MS_SYSTEM_THREAD_POP, 0, 0);
		}

		int terminated()
		{
			return _link::callService(MS_SYSTEM_TERMINATED, 0, 0);
		}
	}

	/*
	 * updater [external]
	 */

	namespace updater
	{
		bool _available()
		{
			return
				true &&
				_link::serviceExists(MS_UPDATE_REGISTERFL);
		}

		void registerFL(int fileID, const PLUGININFO* pluginInfo)
		{
			_link::callService(MS_UPDATE_REGISTERFL, fileID, reinterpret_cast<LPARAM>(pluginInfo));
		}
	}

	/*
	 * utils
	 */

	namespace utils
	{
		int pathToRelative(const mu_text* pszPath, mu_text* pszNewPath)
		{
			return _link::callService(MU_DO_BOTH(MS_UTILS_PATHTORELATIVE, MS_UTILS_PATHTORELATIVEW), reinterpret_cast<WPARAM>(pszPath), reinterpret_cast<LPARAM>(pszNewPath));
		}

		int pathToAbsolute(const mu_text* pszPath, mu_text* pszNewPath)
		{
			return _link::callService(MU_DO_BOTH(MS_UTILS_PATHTOABSOLUTE, MS_UTILS_PATHTOABSOLUTEW), reinterpret_cast<WPARAM>(pszPath), reinterpret_cast<LPARAM>(pszNewPath));
		}
	}

	/*
	 * _link
	 */

	namespace _link
	{
		PLUGINLINK* g_pLink = NULL;

		int callProtoService(const char* szModule, const char* szService, WPARAM wParam, LPARAM lParam)
		{
			mu_ansi str[MAXMODULELABELLENGTH];
			
			strcpy(str, szModule);
			strcat(str, szService);
			
			return mu::_link::callService(str, wParam, lParam);
		}
	}

	/*
	 * memory handling
	 */

	namespace _mem_internal
	{
		static MM_INTERFACE g_mmInterface = {
			sizeof(MM_INTERFACE),
			NULL,
			NULL,
			NULL
		};

		static void init()
		{
			_link::callService(MS_SYSTEM_GET_MMI, 0, reinterpret_cast<LPARAM>(&g_mmInterface));
		}
	}

	void* mirAlloc(size_t size)
	{
		return _mem_internal::g_mmInterface.mmi_malloc(size);
	}

	void* mirRealloc(void* memblock, size_t size)
	{
		return _mem_internal::g_mmInterface.mmi_realloc(memblock, size);
	}

	void mirFree(void* memblock)
	{
		_mem_internal::g_mmInterface.mmi_free(memblock);
	}

	/*
	 * core interface functions
	 */

	bool load(PLUGINLINK* pLink)
	{
		if (!pLink)
		{
			return false;
		}

		_link::g_pLink = pLink;

		// check for version
		if (!isMirandaVersionOk(system::getVersion()))
		{
			return false;
		}

		// additionaly check for unicode, if requested
#if defined(MU_WIDE)
		// MEMO: version dependency check
		if (!isMirandaUnicode())
		{
			return false;
		}
#endif // MU_WIDE

		// init
		_mem_internal::init();

		return true;
	}

	void unload()
	{
	}

	DWORD getMinimalMirandaVersion()
	{
		// MEMO: version dependency check
		return PLUGIN_MAKE_VERSION(0, 6, 7, 0);
	}

	bool isMirandaVersionOk(DWORD version)
	{
		return (version >= getMinimalMirandaVersion());
	}

	bool isMirandaUnicode()
	{
		if (system::getVersion() < PLUGIN_MAKE_VERSION(0, 4, 3, 33))
		{
			return false;
		}

		mu_ansi szVersion[256] = { 0 };
		
		if (system::getVersionText(256, szVersion) != 0)
		{
			return false;
		}

		if (!strstr(szVersion, muA("Unicode")))
		{
			return false;
		}

		return true;
	}

	/*
	 * string handling
	 */

	mu_ansi* wideToAnsiDup(const mu_wide* pszWide, UINT uCP /* = CP_ACP */)
	{
		if (!pszWide)
		{
			return NULL;
		}

		int len = WideCharToMultiByte(uCP, 0, pszWide, -1, NULL, 0, NULL, NULL);
		mu_ansi* result = reinterpret_cast<mu_ansi*>(malloc(sizeof(mu_ansi) * len));

		if (!result)
		{
			return NULL;
		}

		WideCharToMultiByte(uCP, 0, pszWide, -1, result, len, NULL, NULL);
		result[len - 1] = 0;

		return result;
	}

	mu_wide* ansiToWideDup(const mu_ansi* pszAnsi, UINT uCP /* = CP_ACP */)
	{
		if (!pszAnsi)
		{
			return NULL;
		}

		int len = MultiByteToWideChar(uCP, 0, pszAnsi, -1, NULL, 0);
		mu_wide* result = reinterpret_cast<mu_wide*>(malloc(sizeof(mu_wide) * len));
		
		if (!result)
		{
			return NULL;
		}

		MultiByteToWideChar(uCP, 0, pszAnsi, -1, result, len);
		result[len - 1] = 0;

		return result;
	}

	mu_ansi* wideToAnsi(const mu_wide* pszWide, mu_ansi* pszRes, int maxLen, UINT uCP /* = CP_ACP */)
	{
		if (!pszWide)
		{
			return NULL;
		}

		WideCharToMultiByte(uCP, 0, pszWide, -1, pszRes, maxLen, NULL, NULL);

		return pszRes;
	}

	mu_wide* ansiToWide(const mu_ansi* pszAnsi, mu_wide* pszRes, int maxLen, UINT uCP /* = CP_ACP */)
	{
		if (!pszAnsi)
		{
			return NULL;
		}

		MultiByteToWideChar(uCP, 0, pszAnsi, -1, pszRes, maxLen);

		return pszRes;
	}
}
