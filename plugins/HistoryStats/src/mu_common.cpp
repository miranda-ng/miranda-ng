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

			HANDLE res = Menu_AddMainMenuItem(&mi);

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

			HANDLE res = Menu_AddContactMenuItem(&mi);

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

			int res = CallService(MS_CLIST_MODIFYMENUITEM, reinterpret_cast<WPARAM>(hMenuItem), reinterpret_cast<LPARAM>(&mi));

			MU_DO_WIDE(freeAnsi(mi.pszName));

			return res;
		}

		const mu_text* getContactDisplayName(MCONTACT hContact)
		{
			return reinterpret_cast<const mu_text*>(CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_UNICODE));
		}

		const mu_text* getStatusModeDescription(int nStatusMode)
		{
			return reinterpret_cast<const mu_text*>(CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, static_cast<WPARAM>(nStatusMode), GSMDF_UNICODE));
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

			int ret = CallService(MS_DB_GETPROFILEPATH, cbName, reinterpret_cast<LPARAM>(pszNameAnsi));

			if (ret == 0)
			{
				ansiToWide(pszNameAnsi, pszName, cbName);
			}

			delete pszNameAnsi;

			return ret;
#else // MU_WIDE
			return CallService(MS_DB_GETPROFILEPATH, cbName, reinterpret_cast<LPARAM>(pszName));
#endif // MU_WIDE
		}

		int getProfileName(int cbName, mu_text* pszName)
		{
			// TODO: support for unicode core (if supported)
#if defined(MU_WIDE)
			mu_ansi* pszNameAnsi = new mu_ansi[cbName];

			int ret = CallService(MS_DB_GETPROFILENAME, cbName, reinterpret_cast<LPARAM>(pszNameAnsi));

			if (ret == 0)
			{
				ansiToWide(pszNameAnsi, pszName, cbName);
			}

			delete pszNameAnsi;

			return ret;
#else // MU_WIDE
			return CallService(MS_DB_GETPROFILENAME, cbName, reinterpret_cast<LPARAM>(pszName));
#endif // MU_WIDE
		}

		void setSafetyMode(bool safetyMode)
		{
			CallService(MS_DB_SETSAFETYMODE, BOOL_(safetyMode), 0);
		}
	}

	/*
	 * db_time
	 */

	namespace db_contact
	{
		int enumSettings(MCONTACT hContact, const mu_ansi* szModule, DBSETTINGENUMPROC pEnumProc, LPARAM lProcParam)
		{
			DBCONTACTENUMSETTINGS dbces;
			
			dbces.pfnEnumProc = pEnumProc;
			dbces.lParam = lProcParam;
			dbces.szModule = szModule;
			dbces.ofsSettings = 0;

			return CallService(MS_DB_CONTACT_ENUMSETTINGS, hContact, reinterpret_cast<LPARAM>(&dbces));
		}
	}

	namespace db_time
	{
		DWORD timestampToLocal(DWORD timestamp)
		{
			return static_cast<DWORD>(CallService(MS_DB_TIME_TIMESTAMPTOLOCAL, static_cast<WPARAM>(timestamp), 0));
		}
	}

	/*
	 * icolib
	 */

	namespace icolib
	{
		bool _available()
		{
			return true;
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
			Skin_AddIcon(&sid);
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
			Skin_AddIcon(&sid);
		}

		HICON getIcon(const mu_ansi* szIconName)
		{
			return reinterpret_cast<HICON>(CallService(MS_SKIN2_GETICON, 0, reinterpret_cast<LPARAM>(szIconName)));
		}
	}

	/*
	 * langpack
	 */

	namespace langpack
	{
		const mu_text* translateString(const mu_text* szEnglish)
		{
			return reinterpret_cast<const mu_text*>(CallService(MS_LANGPACK_TRANSLATESTRING, MU_DO_BOTH(0, LANG_UNICODE), reinterpret_cast<LPARAM>(szEnglish)));
		}

		UINT getCodePage()
		{
			static UINT CodePage = -1;

			if (CodePage == -1)
			{
				CodePage = ServiceExists(MS_LANGPACK_GETCODEPAGE) ? CallService(MS_LANGPACK_GETCODEPAGE, 0, 0) : CP_ACP;
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
			return true;
		}

		int getNumContacts(MCONTACT hMetaContact)
		{
			return CallService(MS_MC_GETNUMCONTACTS, hMetaContact, 0);
		}

		MCONTACT getSubContact(MCONTACT hMetaContact, int iContactNumber)
		{
			return CallService(MS_MC_GETSUBCONTACT, hMetaContact, iContactNumber);
		}
	}

	/*
	 * opt
	 */

	namespace opt
	{
		void addPage(WPARAM addInfo, const mu_text* pszGroup, const mu_text* pszTitle, const mu_text* pszTab, DLGPROC pfnDlgProc, const mu_ansi* pszTemplate, HINSTANCE hInstance, DWORD flags /* = ODPF_BOLDGROUPS */)
		{
			OPTIONSDIALOGPAGE odp = { sizeof(odp) };
			odp.ptszTitle = const_cast<mu_text*>(pszTitle);
			odp.pfnDlgProc = pfnDlgProc;
			odp.pszTemplate = const_cast<mu_ansi*>(pszTemplate);
			odp.hInstance = hInstance;
			odp.ptszGroup = const_cast<mu_text*>(pszGroup);
			odp.flags = flags | MU_DO_BOTH(0, ODPF_UNICODE);
			odp.ptszTab = const_cast<mu_text*>(pszTab);
			Options_AddPage(addInfo, &odp);
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
				ServiceExists(MS_DIB2PNG);
		}

		bool dibToPng(const BITMAPINFOHEADER* pBMIH, const BYTE* pDIData, BYTE* pImageData, long* pImageLen)
		{
			DIB2PNG info;

			info.pbmi = const_cast<BITMAPINFO*>(reinterpret_cast<const BITMAPINFO*>(pBMIH));
			info.pDiData = const_cast<BYTE*>(pDIData);
			info.pResult = pImageData;
			info.pResultLen = pImageLen;

			return bool_(CallService(MS_DIB2PNG, 0, reinterpret_cast<LPARAM>(&info)));
		}
	}

	/*
	 * proto
	 */

	namespace proto
	{
		int enumProtocols(int* numProtocols, PROTOACCOUNT*** ppProtoDescriptors)
		{
			return ProtoEnumAccounts(numProtocols, ppProtoDescriptors);
		}

		const mu_ansi* getContactBaseProto(MCONTACT hContact)
		{
			return reinterpret_cast<const mu_ansi*>(CallService(MS_PROTO_GETCONTACTBASEPROTO, hContact, 0));
		}
	}

	/*
	 * protosvc
	 */

	namespace protosvc
	{
		DWORD getCaps(const mu_ansi* szProto, int flagNum)
		{
			return (DWORD)CallProtoService(szProto, PS_GETCAPS, static_cast<WPARAM>(flagNum), 0);
		}

		int getName(const mu_ansi* szProto, int cchName, mu_text* szName)
		{
			return CallProtoService(szProto, PS_GETNAME, static_cast<WPARAM>(cchName), reinterpret_cast<LPARAM>(szName));
		}

		HICON loadIcon(const mu_ansi* szProto, int whichIcon)
		{
			return reinterpret_cast<HICON>(CallProtoService(szProto, PS_LOADICON, static_cast<WPARAM>(whichIcon), 0));
		}
	}

	/*
	 * skin
	 */

	namespace skin
	{
		HICON loadIcon(int id)
		{
			return reinterpret_cast<HICON>(CallService(MS_SKIN_LOADICON, id, 0));
		}
	}

	/*
	 * system
	 */

	namespace system
	{
		DWORD getVersion()
		{
			return static_cast<DWORD>(CallService(MS_SYSTEM_GETVERSION, 0, 0));
		}

		int getVersionText(int cchVersion, mu_ansi* szVersion)
		{
			return CallService(MS_SYSTEM_GETVERSIONTEXT, cchVersion, reinterpret_cast<LPARAM>(szVersion));
		}

		int terminated()
		{
			return CallService(MS_SYSTEM_TERMINATED, 0, 0);
		}
	}

	/*
	 * utils
	 */

	namespace utils
	{
		int pathToRelative(const mu_text* pszPath, mu_text* pszNewPath)
		{
			return CallService(MU_DO_BOTH(MS_UTILS_PATHTORELATIVE, MS_UTILS_PATHTORELATIVEW), reinterpret_cast<WPARAM>(pszPath), reinterpret_cast<LPARAM>(pszNewPath));
		}

		int pathToAbsolute(const mu_text* pszPath, mu_text* pszNewPath)
		{
			return CallService(MU_DO_BOTH(MS_UTILS_PATHTOABSOLUTE, MS_UTILS_PATHTOABSOLUTEW), reinterpret_cast<WPARAM>(pszPath), reinterpret_cast<LPARAM>(pszNewPath));
		}
	}

	/*
	 * core interface functions
	 */

	bool load()
	{
		// check for version
		if (!isMirandaVersionOk(system::getVersion()))
			return false;

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
