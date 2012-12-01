#include "StdAfx.h"
#include "IconLib.h"
#include <m_icolib.h>
#include "resource.h"
#include "EconomicRateInfo.h"
// #include <newpluginapi.h>
#include <m_langpack.h>
#include <sstream>
#pragma warning (disable:4996)
#include <m_utils.h>
#pragma warning (default:4996)
#include "ModuleInfo.h"

// extern HMODULE g_hInstance;

namespace
{
	struct CIconList
	{
		char*  szDescr;
		char*  szName;
		int    defIconID;
		HANDLE hIconLibItem;
	};

	CIconList iconList[] =
	{
		{ LPGEN("Protocol icon"),          ICON_STR_MAIN,               IDI_ICON_MAIN },
		{ LPGEN("Quote/Rate up"),          ICON_STR_QUOTE_UP,           IDI_ICON_UP },
		{ LPGEN("Quote/Rate down"),        ICON_STR_QUOTE_DOWN,         IDI_ICON_DOWN },
		{ LPGEN("Quote/Rate not changed"), ICON_STR_QUOTE_NOT_CHANGED,  IDI_ICON_NOTCHANGED },
		{ LPGEN("Quote Section"),          ICON_STR_SECTION,            IDI_ICON_SECTION },
		{ LPGEN("Quote"),                  ICON_STR_QUOTE,              IDI_ICON_QUOTE },
		{ LPGEN("Currency Converter"),     ICON_STR_CURRENCY_CONVERTER, IDI_ICON_CURRENCY_CONVERTER },
		{ LPGEN("Refresh"),                ICON_STR_REFRESH,            IDI_ICON_REFRESH },
		{ LPGEN("Export"),                 ICON_STR_EXPORT,             IDI_ICON_EXPORT },
		{ LPGEN("Swap button"),            ICON_STR_SWAP,               IDI_ICON_SWAP },
		{ LPGEN("Import"),                 ICON_STR_IMPORT,             IDI_ICON_IMPORT }
	};
}

void Quotes_IconsInit()
{
	TCHAR szFile[MAX_PATH];
	::GetModuleFileName(g_hInstance, szFile, MAX_PATH);

	SKINICONDESC sid = { sizeof(sid) };
	sid.ptszDefaultFile = szFile;
	sid.cx = sid.cy = 16;
	sid.flags = SIDF_PATH_TCHAR;
	sid.pszSection = QUOTES_PROTOCOL_NAME;

	for (int i = 0; i < SIZEOF(iconList); i++) {
		std::string sName = Quotes_MakeIconName( iconList[i].szName);
		sid.pszName = const_cast<char*>(sName.c_str());
		sid.pszDescription = iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].defIconID;
		iconList[i].hIconLibItem = Skin_AddIcon(&sid);
	}	
}

std::string Quotes_MakeIconName(const char* name)
{
	assert(name);
	//char szSettingName[100];
	//mir_snprintf(szSettingName,SIZEOF(szSettingName),"%s_%s",QUOTES_PROTOCOL_NAME,name);
	std::string sName(QUOTES_PROTOCOL_NAME);
	sName += "_";
	sName += name;
	return sName;
}

HICON Quotes_LoadIconEx(const char* name,bool bBig /*= false*/)
{
	std::string sIconName = Quotes_MakeIconName(name);
	return Skin_GetIcon(sIconName.c_str(), bBig);
}

HANDLE Quotes_GetIconHandle(int iconId)
{
	for(int i=0;i < SIZEOF(iconList);i++)
	{
		if(iconList[i].defIconID == iconId)
		{
			return iconList[i].hIconLibItem;
		}
	}

	return NULL;
}

