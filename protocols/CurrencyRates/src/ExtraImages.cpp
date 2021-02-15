#include "StdAfx.h"

static HANDLE hExtraIcon;

void CurrencyRates_InitExtraIcons()
{
	hExtraIcon = ExtraIcon_RegisterIcolib("currencyrate", MODULENAME, MODULENAME "_main");
}

bool SetContactExtraImage(MCONTACT hContact, EImageIndex nIndex)
{
	if (!hExtraIcon)
		return false;

	HANDLE hIcolib;
	switch (nIndex) {
	case eiUp:
		hIcolib = g_plugin.getIconHandle(IDI_ICON_UP);
		break;
	case eiDown:
		hIcolib = g_plugin.getIconHandle(IDI_ICON_DOWN);
		break;
	case eiNotChanged:
		hIcolib = g_plugin.getIconHandle(IDI_ICON_NOTCHANGED);
		break;
	default:
		hIcolib = nullptr;
	}
	return ExtraIcon_SetIcon(hExtraIcon, hContact, hIcolib) == 0;
}
