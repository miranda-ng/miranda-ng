#include "StdAfx.h"

static IconItem iconList[] =
{
	{ LPGEN("Protocol icon"),             "main",                    IDI_ICON_MAIN               },
	{ LPGEN("Auto Update Disabled"),      "auto_update_disabled",    IDI_ICON_DISABLED           },
	{ LPGEN("Currency Rate up"),          "currencyrate_up",         IDI_ICON_UP                 },
	{ LPGEN("Currency Rate down"),        "currencyrate_down",       IDI_ICON_DOWN               },
	{ LPGEN("Currency Rate not changed"), "currencyrate_not_changed",IDI_ICON_NOTCHANGED         },
	{ LPGEN("Currency Rate Section"),     "currencyrate_section",    IDI_ICON_SECTION            },
	{ LPGEN("Currency Rate"),             "currencyrate",            IDI_ICON_CURRENCYRATE       },
	{ LPGEN("Currency Converter"),        "currency_converter",      IDI_ICON_CURRENCY_CONVERTER },
	{ LPGEN("Refresh"),                   "refresh",                 IDI_ICON_REFRESH            },
	{ LPGEN("Export"),                    "export",                  IDI_ICON_EXPORT             },
	{ LPGEN("Swap button"),               "swap",                    IDI_ICON_SWAP               },
	{ LPGEN("Import"),                    "import",                  IDI_ICON_IMPORT             }
};

void CurrencyRates_IconsInit()
{
	::g_plugin.registerIcon(MODULENAME, iconList, MODULENAME);
}
