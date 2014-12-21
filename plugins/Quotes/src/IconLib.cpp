#include "StdAfx.h"

static IconItem iconList[] =
{
	{ LPGEN("Protocol icon"), ICON_STR_MAIN, IDI_ICON_MAIN },
	{ LPGEN("Auto Update Disabled"), ICON_STR_AUTO_UPDATE_DISABLED, IDI_ICON_DISABLED },
	{ LPGEN("Quote/Rate up"), ICON_STR_QUOTE_UP, IDI_ICON_UP },
	{ LPGEN("Quote/Rate down"), ICON_STR_QUOTE_DOWN, IDI_ICON_DOWN },
	{ LPGEN("Quote/Rate not changed"), ICON_STR_QUOTE_NOT_CHANGED, IDI_ICON_NOTCHANGED },
	{ LPGEN("Quote Section"), ICON_STR_SECTION, IDI_ICON_SECTION },
	{ LPGEN("Quote"), ICON_STR_QUOTE, IDI_ICON_QUOTE },
	{ LPGEN("Currency Converter"), ICON_STR_CURRENCY_CONVERTER, IDI_ICON_CURRENCY_CONVERTER },
	{ LPGEN("Refresh"), ICON_STR_REFRESH, IDI_ICON_REFRESH },
	{ LPGEN("Export"), ICON_STR_EXPORT, IDI_ICON_EXPORT },
	{ LPGEN("Swap button"), ICON_STR_SWAP, IDI_ICON_SWAP },
	{ LPGEN("Import"), ICON_STR_IMPORT, IDI_ICON_IMPORT }
};

void Quotes_IconsInit()
{
	::Icon_Register(g_hInstance, QUOTES_PROTOCOL_NAME, iconList, SIZEOF(iconList), QUOTES_PROTOCOL_NAME);
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

HICON Quotes_LoadIconEx(const char* name, bool bBig /*= false*/)
{
	std::string sIconName = Quotes_MakeIconName(name);
	return Skin_GetIcon(sIconName.c_str(), bBig);
}

HANDLE Quotes_GetIconHandle(int iconId)
{
	for (int i = 0; i < SIZEOF(iconList); i++)
		if (iconList[i].defIconID == iconId)
			return iconList[i].hIcolib;

	return NULL;
}
