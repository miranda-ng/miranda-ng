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
		HGENMENU addMainMenuItem(const wchar_t* pszName, DWORD flags, int position, HICON hIcon, const char* pszService, HGENMENU hRoot = 0);
		HGENMENU addContactMenuItem(const wchar_t* pszName, DWORD flags, int position, HICON hIcon, const char* pszService);
	}

	/*
	 * icolib
	 */

	namespace icolib
	{
		bool _available();
		void addIcon(const wchar_t* szSection, const wchar_t* szDescription, const char* szIconName, const char* szDefaultFile, int iDefaultIndex);
		HICON getIcon(const char* szIconName);
	}

	/*
	 * opt
	 */

	namespace opt
	{
		void addPage(WPARAM addInfo, const wchar_t* pszGroup, const wchar_t* pszTitle, const wchar_t* pszTab, DLGPROC pfnDlgProc, const char* pszTemplate, HINSTANCE hInstance, DWORD flags = ODPF_BOLDGROUPS);
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
	 * updater [external]
	 */

	namespace updater
	{
		bool _available();
		void registerFL(int fileID, const PLUGININFOEX* pluginInfo);
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
