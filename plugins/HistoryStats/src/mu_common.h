#if !defined(HISTORYSTATS_GUARD_MU_COMMON_H)
#define HISTORYSTATS_GUARD_MU_COMMON_H

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
		HANDLE addMainMenuItem(const TCHAR* pszName, DWORD flags, int position, HICON hIcon, const char* pszService, const TCHAR* pszPopupName = NULL, int popupPosition = 0, DWORD hotKey = 0);
		HANDLE addContactMenuItem(const TCHAR* pszName, DWORD flags, int position, HICON hIcon, const char* pszService, DWORD hotKey = 0, const char* pszContactOwner = NULL);
		int modifyMenuItem(HANDLE hMenuItem, DWORD toModify, const TCHAR* pszName = NULL, DWORD flags = 0, HICON hIcon = NULL, DWORD hotKey = 0);
		const TCHAR* getContactDisplayName(MCONTACT hContact);
		const TCHAR* getStatusModeDescription(int nStatusMode);
	}

	/*
	 * db
	 */

	namespace db
	{
		int getProfilePath(int cbName, TCHAR* pszName);
		int getProfileName(int cbName, TCHAR* pszName);
		void setSafetyMode(bool safetyMode);
	}

	/*
	 * db_contact
	 */

	namespace db_contact
	{
		int enumSettings(MCONTACT hContact, const char* szModule, DBSETTINGENUMPROC pEnumProc, LPARAM lProcParam);
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
		void addIcon(const TCHAR* szSection, const TCHAR* szDescription, const char* szIconName, const char* szDefaultFile, int iDefaultIndex, int cx = 16, int cy = 16);
		void addIcon(const TCHAR* szSection, const TCHAR* szDescription, const char* szIconName, HICON hDefaultIcon, int cx = 16, int cy = 16);
		HICON getIcon(const char* szIconName);
	}

	/*
	 * opt
	 */

	namespace opt
	{
		void addPage(WPARAM addInfo, const TCHAR* pszGroup, const TCHAR* pszTitle, const TCHAR* pszTab, DLGPROC pfnDlgProc, const char* pszTemplate, HINSTANCE hInstance, DWORD flags = ODPF_BOLDGROUPS);
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
		const char* getContactBaseProto(MCONTACT hContact);
	}

	/*
	 * protosvc
	 */

	namespace protosvc
	{
		DWORD getCaps(const char* szProto, int flagNum);
		//int getName(const char* szProto, int cchName, char* szName);
		HICON loadIcon(const char* szProto, int whichIcon);
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
		int getVersionText(int cchVersion, char* szVersion);
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
		int pathToRelative(const TCHAR* pszPath, TCHAR* pszNewPath);
		int pathToAbsolute(const TCHAR* pszPath, TCHAR* pszNewPath);
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

	char* wideToAnsiDup(const WCHAR* pszWide, UINT uCP = CP_ACP);
	WCHAR* ansiToWideDup(const char* pszAnsi, UINT uCP = CP_ACP);
	char* wideToAnsi(const WCHAR* pszWide, char* pszRes, int maxLen, UINT uCP = CP_ACP);
	WCHAR* ansiToWide(const char* pszAnsi, WCHAR* pszRes, int maxLen, UINT uCP = CP_ACP);
	inline void freeWide(WCHAR* pszWide) { free(pszWide); }
	inline void freeAnsi(char* pszAnsi) { free(pszAnsi); }
}

#endif // HISTORYSTATS_GUARD_MU_COMMON_H
