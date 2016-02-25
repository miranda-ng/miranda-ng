#include "StdAfx.h"

static HANDLE hExtraIcon;

void Quotes_InitExtraIcons()
{
	hExtraIcon = ExtraIcon_RegisterIcolib(ICON_STR_QUOTE, QUOTES_PROTOCOL_NAME, QUOTES_PROTOCOL_NAME "_" ICON_STR_MAIN);
}

bool SetContactExtraImage(MCONTACT hContact, EImageIndex nIndex)
{
	if (!hExtraIcon)
		return false;

	HANDLE hIcolib;
	switch (nIndex) {
	case eiUp:
		hIcolib = Quotes_GetIconHandle(IDI_ICON_UP);
		break;
	case eiDown:
		hIcolib = Quotes_GetIconHandle(IDI_ICON_DOWN);
		break;
	case eiNotChanged:
		hIcolib = Quotes_GetIconHandle(IDI_ICON_NOTCHANGED);
		break;
	default:
		hIcolib = NULL;
	}
	return ExtraIcon_SetIcon(hExtraIcon, hContact, hIcolib) == 0;
}
