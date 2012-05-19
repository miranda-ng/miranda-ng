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
		TCHAR*  szDescr;
		char*  szName;
		int    defIconID;
// 		TCHAR*  szSection;
		HANDLE hIconLibItem;
	};

	CIconList iconList[] =
	{
		{_T("Protocol icon"),ICON_STR_MAIN,IDI_ICON_MAIN},
		{_T("Quote/Rate up"),ICON_STR_QUOTE_UP,IDI_ICON_UP},
		{_T("Quote/Rate down"),ICON_STR_QUOTE_DOWN,IDI_ICON_DOWN},
		{_T("Quote/Rate not changed"),ICON_STR_QUOTE_NOT_CHANGED,IDI_ICON_NOTCHANGED},		
		{_T("Quote Section"),ICON_STR_SECTION,IDI_ICON_SECTION},
		{_T("Quote"),ICON_STR_QUOTE,IDI_ICON_QUOTE},
		{_T("Currency Converter"),ICON_STR_CURRENCY_CONVERTER,IDI_ICON_CURRENCY_CONVERTER},
		{_T("Refresh"),ICON_STR_REFRESH,IDI_ICON_REFRESH},
		{_T("Export"),ICON_STR_EXPORT,IDI_ICON_EXPORT},
		{_T("Swap button"),ICON_STR_SWAP,IDI_ICON_SWAP},
		{_T("Import"),ICON_STR_IMPORT,IDI_ICON_IMPORT},
	};
}

void Quotes_IconsInit()
{
 	USES_CONVERSION;

	SKINICONDESC sid = {0};
	TCHAR szFile[MAX_PATH];
	::GetModuleFileName(CModuleInfo::GetModuleHandle(), szFile, MAX_PATH);

	sid.cbSize = sizeof(SKINICONDESC);
	sid.ptszDefaultFile = szFile;
	sid.cx = sid.cy = 16;
	sid.flags = SIDF_ALL_TCHAR;
	sid.ptszSection = A2T(QUOTES_PROTOCOL_NAME);

//	TCHAR* szRootSection = TranslateTS(A2T(QUOTES_PROTOCOL_NAME));

	for( int i = 0; i < SIZEOF(iconList); i++ )
	{
//		char szSettingName[100];
// 		TCHAR szSectionName[100];
//		mir_snprintf( szSettingName, sizeof( szSettingName ),"%s_%s",QUOTES_PROTOCOL_NAME, iconList[i].szName );
// 		{
// 			mir_sntprintf( szSectionName, SIZEOF( szSectionName ),_T("%s/%s"), TranslateT("Protocols"), szRootSection);
// 			sid.ptszSection = szSectionName;
// 		}

		std::string sName = Quotes_MakeIconName( iconList[i].szName);
		sid.pszName = const_cast<char*>(sName.c_str());
		sid.ptszDescription = iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].defIconID;
		iconList[i].hIconLibItem = reinterpret_cast<HANDLE>(CallService(MS_SKIN2_ADDICON,0,reinterpret_cast<LPARAM>(&sid)));
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
	return reinterpret_cast<HICON>(CallService( MS_SKIN2_GETICON,((bBig) ? 1 : 0),reinterpret_cast<LPARAM>(sIconName.c_str())));
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

