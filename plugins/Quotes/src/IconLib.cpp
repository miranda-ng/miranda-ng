#include "StdAfx.h"

static IconItem iconList[] =
{
	{ LPGEN("Protocol icon"),           ICON_STR_MAIN,         IDI_ICON_MAIN               },
	{ LPGEN("Auto Update Disabled"),   "auto_update_disabled", IDI_ICON_DISABLED           },
	{ LPGEN("Quote/Rate up"),          "quote_up",             IDI_ICON_UP                 },
	{ LPGEN("Quote/Rate down"),        "quote_down",           IDI_ICON_DOWN               },
	{ LPGEN("Quote/Rate not changed"), "quote_not_changed",    IDI_ICON_NOTCHANGED         },
	{ LPGEN("Quote Section"),          "quote_section",        IDI_ICON_SECTION            },
	{ LPGEN("Quote"),                   ICON_STR_QUOTE,        IDI_ICON_QUOTE              },
	{ LPGEN("Currency Converter"),     "currency_converter",   IDI_ICON_CURRENCY_CONVERTER },
	{ LPGEN("Refresh"),                "refresh",              IDI_ICON_REFRESH            },
	{ LPGEN("Export"),                 "export",               IDI_ICON_EXPORT             },
	{ LPGEN("Swap button"),            "swap",                 IDI_ICON_SWAP               },
	{ LPGEN("Import"),                 "import",               IDI_ICON_IMPORT             }
};

void Quotes_IconsInit()
{
	::Icon_Register(g_hInstance, QUOTES_PROTOCOL_NAME, iconList, _countof(iconList), QUOTES_PROTOCOL_NAME);
}

HICON Quotes_LoadIconEx(int iconId, bool bBig /*= false*/)
{
	for (int i = 0; i < _countof(iconList); i++)
		if (iconList[i].defIconID == iconId)
			return IcoLib_GetIconByHandle(iconList[i].hIcolib, bBig);

	return NULL;
}

HANDLE Quotes_GetIconHandle(int iconId)
{
	for (int i = 0; i < _countof(iconList); i++)
		if (iconList[i].defIconID == iconId)
			return iconList[i].hIcolib;

	return NULL;
}
