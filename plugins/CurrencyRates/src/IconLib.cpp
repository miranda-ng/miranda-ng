#include "StdAfx.h"

static IconItem iconList[] =
{
	{ LPGEN("Protocol icon"),           ICON_STR_MAIN,         IDI_ICON_MAIN               },
	{ LPGEN("Auto Update Disabled"),   "auto_update_disabled", IDI_ICON_DISABLED           },
	{ LPGEN("CurrencyRate/Rate up"),          "currencyrate_up",             IDI_ICON_UP                 },
	{ LPGEN("CurrencyRate/Rate down"),        "currencyrate_down",           IDI_ICON_DOWN               },
	{ LPGEN("CurrencyRate/Rate not changed"), "currencyrate_not_changed",    IDI_ICON_NOTCHANGED         },
	{ LPGEN("CurrencyRate Section"),          "currencyrate_section",        IDI_ICON_SECTION            },
	{ LPGEN("CurrencyRate"),                   ICON_STR_CURRENCYRATE,        IDI_ICON_CURRENCYRATE              },
	{ LPGEN("Currency Converter"),     "currency_converter",   IDI_ICON_CURRENCY_CONVERTER },
	{ LPGEN("Refresh"),                "refresh",              IDI_ICON_REFRESH            },
	{ LPGEN("Export"),                 "export",               IDI_ICON_EXPORT             },
	{ LPGEN("Swap button"),            "swap",                 IDI_ICON_SWAP               },
	{ LPGEN("Import"),                 "import",               IDI_ICON_IMPORT             }
};

void CurrencyRates_IconsInit()
{
	::g_plugin.registerIcon(CURRENCYRATES_PROTOCOL_NAME, iconList, CURRENCYRATES_PROTOCOL_NAME);
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
