#include "StdAfx.h"

static IconItem iconList[] =
{
	{ LPGEN("Protocol icon"),           ICON_STR_MAIN,         IDI_ICON_MAIN               },
	{ LPGEN("Auto Update Disabled"),   "auto_update_disabled", IDI_ICON_DISABLED           },
	{ LPGEN("Currency Rate up"),          "currencyrate_up",             IDI_ICON_UP                 },
	{ LPGEN("Currency Rate down"),        "currencyrate_down",           IDI_ICON_DOWN               },
	{ LPGEN("Currency Rate not changed"), "currencyrate_not_changed",    IDI_ICON_NOTCHANGED         },
	{ LPGEN("Currency Rate Section"),          "currencyrate_section",        IDI_ICON_SECTION            },
	{ LPGEN("Currency Rate"),                   ICON_STR_CURRENCYRATE,        IDI_ICON_CURRENCYRATE              },
	{ LPGEN("Currency Converter"),     "currency_converter",   IDI_ICON_CURRENCY_CONVERTER },
	{ LPGEN("Refresh"),                "refresh",              IDI_ICON_REFRESH            },
	{ LPGEN("Export"),                 "export",               IDI_ICON_EXPORT             },
	{ LPGEN("Swap button"),            "swap",                 IDI_ICON_SWAP               },
	{ LPGEN("Import"),                 "import",               IDI_ICON_IMPORT             }
};

void CurrencyRates_IconsInit()
{
	::g_plugin.registerIcon(MODULENAME, iconList, MODULENAME);
}

HICON CurrencyRates_LoadIconEx(int iconId, bool bBig /*= false*/)
{
	for (int i = 0; i < _countof(iconList); i++)
		if (iconList[i].defIconID == iconId)
			return IcoLib_GetIconByHandle(iconList[i].hIcolib, bBig);

	return nullptr;
}

HANDLE CurrencyRates_GetIconHandle(int iconId)
{
	for (int i = 0; i < _countof(iconList); i++)
		if (iconList[i].defIconID == iconId)
			return iconList[i].hIcolib;

	return nullptr;
}
