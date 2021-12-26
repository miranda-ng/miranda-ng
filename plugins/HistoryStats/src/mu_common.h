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
		HGENMENU addMainMenuItem(const wchar_t* pszName, uint32_t flags, int position, HICON hIcon, const char* pszService, HGENMENU hRoot = nullptr);
		HGENMENU addContactMenuItem(const wchar_t* pszName, uint32_t flags, int position, HICON hIcon, const char* pszService);
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
	 * protosvc
	 */

	namespace protosvc
	{
		uint32_t getCaps(const char* szProto, int flagNum);
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
	uint32_t getMinimalMirandaVersion();
	bool isMirandaVersionOk(uint32_t version);
	bool isMirandaUnicode();

	/*
	 * string handling
	 */

	char* wideToAnsiDup(const wchar_t* pszWide, UINT uCP = CP_ACP);
	wchar_t* ansiToWideDup(const char* pszAnsi, UINT uCP = CP_ACP);
	char* wideToAnsi(const wchar_t* pszWide, char* pszRes, int maxLen, UINT uCP = CP_ACP);
	wchar_t* ansiToWide(const char* pszAnsi, wchar_t* pszRes, int maxLen, UINT uCP = CP_ACP);
	inline void freeWide(wchar_t* pszWide) { free(pszWide); }
	inline void freeAnsi(char* pszAnsi) { free(pszAnsi); }
}

#endif // HISTORYSTATS_GUARD_MU_COMMON_H
