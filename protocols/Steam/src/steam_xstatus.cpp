#include "stdafx.h"

#define STATUS_TITLE_MAX 64
#define STATUS_DESC_MAX  250

static std::vector<int> xstatusIconsValid;
static std::map<int, int> xstatusIcons;

int OnReloadIcons(WPARAM, LPARAM)
{
	xstatusIconsValid.clear();
	return 0;
}

int CSteamProto::GetContactXStatus(MCONTACT hContact)
{
	return getDword(hContact, "XStatusId", 0) ? 1 : 0;
}

void SetContactExtraIcon(MCONTACT hContact, int status)
{
	ExtraIcon_SetIcon(hExtraXStatus, hContact, (status > 0) ? g_plugin.getIconHandle(IDI_GAMING) : nullptr);
}

INT_PTR CSteamProto::OnGetXStatusEx(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = (MCONTACT)wParam;

	CUSTOM_STATUS *pData = (CUSTOM_STATUS *)lParam;
	if (pData->cbSize < sizeof(CUSTOM_STATUS))
		return 1;

	// fill status member
	if (pData->flags & CSSF_MASK_STATUS)
		*pData->status = GetContactXStatus(hContact);

	// fill status name member
	if (pData->flags & CSSF_MASK_NAME) {
		int status = (pData->wParam == nullptr) ? GetContactXStatus(hContact) : *pData->wParam;
		if (status < 1)
			return 1;

		ptrW title;
		if (pData->flags & CSSF_DEFAULT_NAME)
			title = mir_wstrdup(TranslateT("Playing"));
		else
			title = getWStringA(hContact, "XStatusName");

		if (pData->flags & CSSF_UNICODE)
			mir_wstrncpy(pData->ptszName, title, STATUS_TITLE_MAX);
		else
			mir_strncpy(pData->pszName, _T2A(title), STATUS_TITLE_MAX);
	}

	// fill status message member
	if (pData->flags & CSSF_MASK_MESSAGE) {
		ptrW message(getWStringA(hContact, "XStatusMsg"));

		if (pData->flags & CSSF_UNICODE)
			mir_wstrncpy(pData->ptszMessage, message, STATUS_DESC_MAX);
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
		if (pData->wParam)
			*pData->wParam = STATUS_TITLE_MAX;
		if (pData->lParam)
			*pData->lParam = STATUS_DESC_MAX;
	}

	return 0;
}

HICON CSteamProto::GetXStatusIcon(int status, UINT flags)
{
	if (status < 1)
		return nullptr;

	HICON icon = g_plugin.getIcon(IDI_GAMING, (flags & LR_BIGICON) != 0);
	return (flags & LR_SHARED) ? icon : CopyIcon(icon);
}

INT_PTR CSteamProto::OnGetXStatusIcon(WPARAM wParam, LPARAM lParam)
{
	if (!wParam)
		wParam = GetContactXStatus(NULL);

	if (wParam < 1)
		return 0;

	return (INT_PTR)GetXStatusIcon(wParam, lParam);
}

INT_PTR CSteamProto::OnRequestAdvStatusIconIdx(WPARAM wParam, LPARAM)
{
	int status = GetContactXStatus(wParam);
	if (status) {
		if (std::find(xstatusIconsValid.begin(), xstatusIconsValid.end(), status) == xstatusIconsValid.end()) {
			// adding/updating icon
			HIMAGELIST clistImageList = Clist_GetImageList();
			if (clistImageList) {
				HICON hXStatusIcon = GetXStatusIcon(status, LR_SHARED);

				std::map<int, int>::iterator it = xstatusIcons.find(status);
				if (it != xstatusIcons.end() && it->second > 0)
					ImageList_ReplaceIcon(clistImageList, it->first, hXStatusIcon);
				else
					xstatusIcons.insert(std::make_pair(status, ImageList_AddIcon(clistImageList, hXStatusIcon)));

				// mark icon index in the array as valid
				xstatusIconsValid.push_back(status);

				IcoLib_ReleaseIcon(hXStatusIcon);
			}
		}

		if (std::find(xstatusIconsValid.begin(), xstatusIconsValid.end(), status) != xstatusIconsValid.end()) {
			std::map<int, int>::iterator it = xstatusIcons.find(status);
			if (it != xstatusIcons.end())
				return ((INT_PTR)it->second & 0xFFFF) << 16;
		}
	}

	return -1;
}
