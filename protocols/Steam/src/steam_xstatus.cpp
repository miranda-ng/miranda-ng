#include "common.h"

#define STATUS_TITLE_MAX 64
#define STATUS_DESC_MAX  255

int CSteamProto::GetContactXStatus(MCONTACT hContact)
{
	return getDword(hContact, "XStatusId", 0) ? 1 : 0;
}

INT_PTR CSteamProto::GetXStatusEx(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = (MCONTACT)wParam;

	CUSTOM_STATUS *pData = (CUSTOM_STATUS*)lParam;
	if (pData->cbSize < sizeof(CUSTOM_STATUS))
		return 1;

	// fill status member
	if (pData->flags & CSSF_MASK_STATUS)
		*pData->status = GetContactXStatus(hContact);

	// fill status name member
	if (pData->flags & CSSF_MASK_NAME)
	{
		int status = (pData->wParam == NULL) ? GetContactXStatus(hContact) : *pData->wParam;
		if (status < 1)
			return 1;

		ptrT title;
		if (pData->flags & CSSF_DEFAULT_NAME)
			title = mir_tstrdup(TranslateT("Playing"));
		else
			title = getTStringA(hContact, "XStatusName");

		if (pData->flags & CSSF_UNICODE)
			mir_tstrncpy(pData->ptszName, title, STATUS_TITLE_MAX);
		else
			mir_strncpy(pData->pszName, _T2A(title), STATUS_TITLE_MAX);
	}

	// fill status message member
	if (pData->flags & CSSF_MASK_MESSAGE) {
		ptrT message(getTStringA(hContact, "XStatusMsg"));
		
		if (pData->flags & CSSF_UNICODE)
			mir_tstrncpy(pData->ptszMessage, message, STATUS_DESC_MAX);
		else
			mir_strncpy(pData->pszMessage, _T2A(message), STATUS_DESC_MAX);
	}

	// disable menu
	if (pData->flags & CSSF_DISABLE_MENU)
		if (pData->wParam)
			*pData->wParam = true;

	// disable ui
	if (pData->flags & CSSF_DISABLE_UI)
		if (pData->wParam)
			*pData->wParam = true;

	// number of xstatuses
	if (pData->flags & CSSF_STATUSES_COUNT)
		if (pData->wParam)
			*pData->wParam = 1; // TODO: how to solve unknown count of games?

	// data sizes
	if (pData->flags & CSSF_STR_SIZES) {
		if (pData->wParam) *pData->wParam = STATUS_TITLE_MAX;
		if (pData->lParam) *pData->lParam = STATUS_DESC_MAX;
	}

	return 0;
}

INT_PTR CSteamProto::GetXStatusIcon(WPARAM wParam, LPARAM lParam)
{
	if (!wParam)
		wParam = GetContactXStatus(NULL);

	if (wParam < 1)
		return 0;

	char iconName[100];
	mir_snprintf(iconName, SIZEOF(iconName), "%s_%s", MODULE, "gaming");

	HICON icon = Skin_GetIcon(iconName, (lParam & LR_BIGICON) ? 32 : 16);
	return (lParam & LR_SHARED) ? (INT_PTR)icon : (INT_PTR)CopyIcon(icon);
}

INT_PTR CSteamProto::RequestAdvStatusIconIdx(WPARAM wParam, LPARAM lParam)
{
	int status = GetContactXStatus(wParam);
	if (status < 1)
		return -1;

	return ((status & 0xFFFF) << 16);
}