#include "StdAfx.h"

static HANDLE hExtraIcon;

void CurrencyRates_InitExtraIcons()
{
	hExtraIcon = ExtraIcon_RegisterIcolib(ICON_STR_CURRENCYRATE, MODULENAME, MODULENAME "_" ICON_STR_MAIN);
}

bool SetContactExtraImage(MCONTACT hContact, EImageIndex nIndex)
{
	if (!hExtraIcon)
		return false;

	HANDLE hIcolib;
	switch (nIndex) {
	case eiUp:
		hIcolib = CurrencyRates_GetIconHandle(IDI_ICON_UP);
		break;
	case eiDown:
		hIcolib = CurrencyRates_GetIconHandle(IDI_ICON_DOWN);
		break;
	case eiNotChanged:
		hIcolib = CurrencyRates_GetIconHandle(IDI_ICON_NOTCHANGED);
		break;
	default:
		hIcolib = nullptr;
	}
	return ExtraIcon_SetIcon(hExtraIcon, hContact, hIcolib) == 0;
}
