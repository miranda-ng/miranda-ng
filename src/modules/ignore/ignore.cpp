/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "..\..\core\commonheaders.h"

#define IGNOREEVENT_MAX  7

static const DWORD ignoreIdToPf1[IGNOREEVENT_MAX] = {PF1_IMRECV, PF1_URLRECV, PF1_FILERECV, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
static const DWORD ignoreIdToPf4[IGNOREEVENT_MAX] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, PF4_SUPPORTTYPING};

static DWORD GetMask(MCONTACT hContact)
{
	DWORD mask = db_get_dw(hContact, "Ignore", "Mask1", (DWORD)(-1));
	if (mask == (DWORD)(-1)) {
		if (hContact == NULL) mask = 0;
		else {
			if (db_get_b(hContact, "CList", "Hidden", 0) || db_get_b(hContact, "CList", "NotOnList", 0))
				mask = db_get_dw(NULL, "Ignore", "Mask1", 0);
			else
				mask = db_get_dw(NULL, "Ignore", "Default1", 0);
		}
	}
	return mask;
}

static void SetListGroupIcons(HWND hwndList, HANDLE hFirstItem, HANDLE hParentItem, int *groupChildCount)
{
	int iconOn[IGNOREEVENT_MAX] = {1, 1, 1, 1, 1, 1, 1};
	int childCount[IGNOREEVENT_MAX] = {0, 0, 0, 0, 0, 0, 0}, i;
	int iImage;
	HANDLE hItem, hChildItem;

	int typeOfFirst = SendMessage(hwndList, CLM_GETITEMTYPE, (WPARAM)hFirstItem, 0);
	//check groups
	if (typeOfFirst == CLCIT_GROUP) hItem = hFirstItem;
	else hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hFirstItem);
	while (hItem) {
		hChildItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
		if (hChildItem) SetListGroupIcons(hwndList, hChildItem, hItem, childCount);
		for (i=0; i < SIZEOF(iconOn); i++)
			if (iconOn[i] && SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, i) == 0) iconOn[i] = 0;
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hItem);
	}
	//check contacts
	if (typeOfFirst == CLCIT_CONTACT) hItem = hFirstItem;
	else hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, (LPARAM)hFirstItem);
	while (hItem) {
		for (i=0; i < SIZEOF(iconOn); i++) {
			iImage = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, i);
			if (iconOn[i] && iImage == 0) iconOn[i] = 0;
			if (iImage != EMPTY_EXTRA_ICON)
				childCount[i]++;
		}
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, (LPARAM)hItem);
	}
	//set icons
	for (i=0; i < SIZEOF(iconOn); i++) {
		SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hParentItem, MAKELPARAM(i, childCount[i]?(iconOn[i]?i+3:0) : EMPTY_EXTRA_ICON));
		if (groupChildCount) groupChildCount[i]+=childCount[i];
	}
	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hParentItem, MAKELPARAM(IGNOREEVENT_MAX, 1));
	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hParentItem, MAKELPARAM(IGNOREEVENT_MAX+1, 2));
}

static void SetAllChildIcons(HWND hwndList, HANDLE hFirstItem, int iColumn, int iImage)
{
	HANDLE hItem;

	int typeOfFirst = SendMessage(hwndList, CLM_GETITEMTYPE, (WPARAM)hFirstItem, 0);
	//check groups
	if (typeOfFirst == CLCIT_GROUP) hItem = hFirstItem;
	else hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hFirstItem);
	while (hItem) {
		HANDLE hChildItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
		if (hChildItem)
			SetAllChildIcons(hwndList, hChildItem, iColumn, iImage);
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hItem);
	}
	//check contacts
	if (typeOfFirst == CLCIT_CONTACT) hItem = hFirstItem;
	else hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, (LPARAM)hFirstItem);
	while (hItem) {
		int iOldIcon = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, iColumn);
		if (iOldIcon != EMPTY_EXTRA_ICON && iOldIcon != iImage)
			SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(iColumn, iImage));
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, (LPARAM)hItem);
	}
}

static void ResetListOptions(HWND hwndList)
{
	SendMessage(hwndList, CLM_SETHIDEEMPTYGROUPS, 1, 0);
}

static void SetIconsForColumn(HWND hwndList, HANDLE hItem, HANDLE hItemAll, int iColumn, int iImage)
{
	switch ( SendMessage(hwndList, CLM_GETITEMTYPE, (WPARAM)hItem, 0)) {
	case CLCIT_CONTACT:
		{
			int oldiImage = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, iColumn);
			if (oldiImage != EMPTY_EXTRA_ICON && oldiImage != iImage)
				SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(iColumn, iImage));
		}
		break;
	case CLCIT_INFO:
		if (hItem == hItemAll)
			SetAllChildIcons(hwndList, hItem, iColumn, iImage);
		else
			SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(iColumn, iImage)); //hItemUnknown
		break;

	case CLCIT_GROUP:
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
		if (hItem)
			SetAllChildIcons(hwndList, hItem, iColumn, iImage);
	}
}

static void InitialiseItem(HWND hwndList, MCONTACT hContact, HANDLE hItem, DWORD proto1Caps, DWORD proto4Caps)
{
	DWORD mask = GetMask(hContact);
	for (int i=0; i < IGNOREEVENT_MAX; i++)
		if ((ignoreIdToPf1[i] == 0xFFFFFFFF && ignoreIdToPf4[i] == 0xFFFFFFFF) || (proto1Caps&ignoreIdToPf1[i] || proto4Caps&ignoreIdToPf4[i]))
			SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(i, mask&(1<<i)?i+3:0));

	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(IGNOREEVENT_MAX, 1));
	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(IGNOREEVENT_MAX+1, 2));
}

static void SaveItemMask(HWND hwndList, MCONTACT hContact, HANDLE hItem, const char *pszSetting)
{
	DWORD mask = 0;
	for (int i=0; i < IGNOREEVENT_MAX; i++) {
		int iImage = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(i, 0));
		if (iImage && iImage != EMPTY_EXTRA_ICON)
			mask |= 1 << i;
	}
	db_set_dw(hContact, "Ignore", pszSetting, mask);
}

static void SetAllContactIcons(HWND hwndList)
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0);
		if (hItem && SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(IGNOREEVENT_MAX, 0)) == EMPTY_EXTRA_ICON) {
			DWORD proto1Caps, proto4Caps;
			char *szProto = GetContactProto(hContact);
			if (szProto) {
				proto1Caps = CallProtoServiceInt(NULL,szProto, PS_GETCAPS, PFLAGNUM_1, 0);
				proto4Caps = CallProtoServiceInt(NULL,szProto, PS_GETCAPS, PFLAGNUM_4, 0);
			}
			else proto1Caps = proto4Caps = 0;
			InitialiseItem(hwndList, hContact, hItem, proto1Caps, proto4Caps);
			if (!db_get_b(hContact, "CList", "Hidden", 0))
				SendMessage(hwndList, CLM_SETCHECKMARK, (WPARAM)hItem, 1);
		}
	}
}

static INT_PTR CALLBACK DlgProcIgnoreOpts(HWND hwndDlg, UINT msg, WPARAM, LPARAM lParam)
{
	static HICON hIcons[IGNOREEVENT_MAX+2];
	static HANDLE hItemAll, hItemUnknown;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			HIMAGELIST hIml = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 3 + IGNOREEVENT_MAX, 3 + IGNOREEVENT_MAX);
			ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_SMALLDOT);
			ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_FILLEDBLOB);
			ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_EMPTYBLOB);
			ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_EVENT_MESSAGE);
			ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_EVENT_URL);
			ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_EVENT_FILE);
			ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_USERONLINE);
			ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_AUTH_REQUEST);
			ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_AUTH_ADD);
			ImageList_AddIcon_IconLibLoaded(hIml, SKINICON_OTHER_TYPING);

			SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_SETEXTRAIMAGELIST, 0, (LPARAM)hIml);
			for (int i=0; i < SIZEOF(hIcons); i++)
				hIcons[i] = ImageList_GetIcon(hIml, 1+i, ILD_NORMAL);
		}

		SendDlgItemMessage(hwndDlg, IDC_ALLICON, STM_SETICON, (WPARAM)hIcons[0], 0);
		SendDlgItemMessage(hwndDlg, IDC_NONEICON, STM_SETICON, (WPARAM)hIcons[1], 0);
		SendDlgItemMessage(hwndDlg, IDC_MSGICON, STM_SETICON, (WPARAM)hIcons[2], 0);
		SendDlgItemMessage(hwndDlg, IDC_URLICON, STM_SETICON, (WPARAM)hIcons[3], 0);
		SendDlgItemMessage(hwndDlg, IDC_FILEICON, STM_SETICON, (WPARAM)hIcons[4], 0);
		SendDlgItemMessage(hwndDlg, IDC_ONLINEICON, STM_SETICON, (WPARAM)hIcons[5], 0);
		SendDlgItemMessage(hwndDlg, IDC_AUTHICON, STM_SETICON, (WPARAM)hIcons[6], 0);
		SendDlgItemMessage(hwndDlg, IDC_ADDED, STM_SETICON, (WPARAM)hIcons[7], 0);
		SendDlgItemMessage(hwndDlg, IDC_TYPINGICON, STM_SETICON, (WPARAM)hIcons[8], 0);

		ResetListOptions(GetDlgItem(hwndDlg, IDC_LIST));
		SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_SETEXTRACOLUMNS, IGNOREEVENT_MAX+2, 0);
		{
			CLCINFOITEM cii = { sizeof(cii) };
			cii.flags = CLCIIF_GROUPFONT;
			cii.pszText = TranslateT("** All contacts **");
			hItemAll = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);

			cii.pszText = TranslateT("** Unknown contacts **");
			hItemUnknown = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
			InitialiseItem( GetDlgItem(hwndDlg, IDC_LIST), NULL, hItemUnknown, 0xFFFFFFFF, 0xFFFFFFFF);
		}

		SetAllContactIcons( GetDlgItem(hwndDlg, IDC_LIST));
		SetListGroupIcons( GetDlgItem(hwndDlg, IDC_LIST), (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETNEXTITEM, CLGN_ROOT, 0), hItemAll, NULL);
		return TRUE;

	case WM_SETFOCUS:
		SetFocus( GetDlgItem(hwndDlg, IDC_LIST));
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case IDC_LIST:
			switch (((LPNMHDR)lParam)->code) {
			case CLN_NEWCONTACT:
			case CLN_LISTREBUILT:
				SetAllContactIcons( GetDlgItem(hwndDlg, IDC_LIST));
				//fall through
			case CLN_CONTACTMOVED:
				SetListGroupIcons( GetDlgItem(hwndDlg, IDC_LIST), (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETNEXTITEM, CLGN_ROOT, 0), hItemAll, NULL);
				break;
			case CLN_OPTIONSCHANGED:
				ResetListOptions(GetDlgItem(hwndDlg, IDC_LIST));
				break;
			case CLN_CHECKCHANGED:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case NM_CLICK:
				{
					NMCLISTCONTROL *nm = (NMCLISTCONTROL*)lParam;
					if (nm->iColumn == -1)
						break;

					DWORD hitFlags;
					HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_HITTEST, (WPARAM)&hitFlags, MAKELPARAM(nm->pt.x, nm->pt.y));
					if (hItem == NULL || !(hitFlags & CLCHT_ONITEMEXTRA))
						break;

					if (nm->iColumn == IGNOREEVENT_MAX) { // ignore all
						for (int iImage = 0;iImage<IGNOREEVENT_MAX;iImage++)
							SetIconsForColumn( GetDlgItem(hwndDlg, IDC_LIST), hItem, hItemAll, iImage, iImage+3);
					}
					else if (nm->iColumn == IGNOREEVENT_MAX+1) {	// ignore none
						for (int iImage = 0;iImage<IGNOREEVENT_MAX;iImage++)
							SetIconsForColumn( GetDlgItem(hwndDlg, IDC_LIST), hItem, hItemAll, iImage, 0);
					}
					else {
						int iImage = SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn, 0));
						if (iImage == 0)
							iImage = nm->iColumn+3;
						else if (iImage != EMPTY_EXTRA_ICON)
							iImage = 0;
						SetIconsForColumn( GetDlgItem(hwndDlg, IDC_LIST), hItem, hItemAll, nm->iColumn, iImage);
					}
					SetListGroupIcons( GetDlgItem(hwndDlg, IDC_LIST), (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETNEXTITEM, CLGN_ROOT, 0), hItemAll, NULL);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
			}
			break;

		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
					HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_FINDCONTACT, hContact, 0);
					if (hItem) SaveItemMask( GetDlgItem(hwndDlg, IDC_LIST), hContact, hItem, "Mask1");
					if (SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETCHECKMARK, (WPARAM)hItem, 0))
						db_unset(hContact, "CList", "Hidden");
					else
						db_set_b(hContact, "CList", "Hidden", 1);
				}

				SaveItemMask( GetDlgItem(hwndDlg, IDC_LIST), NULL, hItemAll, "Default1");
				SaveItemMask( GetDlgItem(hwndDlg, IDC_LIST), NULL, hItemUnknown, "Mask1");
				return TRUE;
			}
		}
		break;

	case WM_DESTROY:
		for (int i=0; i < SIZEOF(hIcons); i++)
			DestroyIcon(hIcons[i]);
		HIMAGELIST hIml = (HIMAGELIST)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETEXTRAIMAGELIST, 0, 0);
		ImageList_Destroy(hIml);
		break;
	}
	return FALSE;
}

static int IgnoreOptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = 900000000;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_IGNORE);
	odp.pszTitle = LPGEN("Ignore");
	odp.pszGroup = LPGEN("Contacts");
	odp.pfnDlgProc = DlgProcIgnoreOpts;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}

static INT_PTR IsIgnored(WPARAM wParam, LPARAM lParam)
{
	DWORD mask = GetMask(wParam);
	if (lParam < 1 || lParam > IGNOREEVENT_MAX)
		return 1;
	return (mask >> (lParam-1))&1;
}

static INT_PTR Ignore(WPARAM wParam, LPARAM lParam)
{
	DWORD mask = GetMask(wParam);
	if ((lParam < 1 || lParam > IGNOREEVENT_MAX) && lParam != IGNOREEVENT_ALL)
		return 1;
	if (lParam == IGNOREEVENT_ALL)
		mask = (1 << IGNOREEVENT_MAX)-1;
	else
		mask |= 1 << (lParam-1);
	db_set_dw(wParam, "Ignore", "Mask1", mask);
	return 0;
}

static INT_PTR Unignore(WPARAM wParam, LPARAM lParam)
{
	DWORD mask = GetMask(wParam);
	if ((lParam < 1 || lParam > IGNOREEVENT_MAX) && lParam != IGNOREEVENT_ALL)
		return 1;

	if (lParam == IGNOREEVENT_ALL)
		mask = 0;
	else
		mask &= ~(1 << (lParam-1));
	db_set_dw(wParam, "Ignore", "Mask1", mask);
	return 0;
}

static INT_PTR IgnoreRecvMessage(WPARAM wParam, LPARAM lParam)
{
	if (IsIgnored((WPARAM)((CCSDATA*)lParam)->hContact, IGNOREEVENT_MESSAGE))
		return 1;
	return CallService(MS_PROTO_CHAINRECV, wParam, lParam);
}

static INT_PTR IgnoreRecvUrl(WPARAM wParam, LPARAM lParam)
{
	if ( IsIgnored((WPARAM)((CCSDATA*)lParam)->hContact, IGNOREEVENT_URL))
		return 1;
	return CallService(MS_PROTO_CHAINRECV, wParam, lParam);
}

static INT_PTR IgnoreRecvFile(WPARAM wParam, LPARAM lParam)
{
	if ( IsIgnored((WPARAM)((CCSDATA*)lParam)->hContact, IGNOREEVENT_FILE))
		return 1;
	return CallService(MS_PROTO_CHAINRECV, wParam, lParam);
}

static INT_PTR IgnoreRecvAuth(WPARAM wParam, LPARAM lParam)
{
	if ( IsIgnored((WPARAM)((CCSDATA*)lParam)->hContact, IGNOREEVENT_AUTHORIZATION))
		return 1;
	return CallService(MS_PROTO_CHAINRECV, wParam, lParam);
}

static int IgnoreAddedNotify(WPARAM, LPARAM lParam)
{
	DBEVENTINFO *dbei = (DBEVENTINFO*)lParam;
	if (dbei && dbei->eventType == EVENTTYPE_ADDED && dbei->pBlob != NULL) {
		MCONTACT hContact = DbGetAuthEventContact(dbei);
		if (CallService(MS_DB_CONTACT_IS, hContact, 0) && IsIgnored(hContact, IGNOREEVENT_YOUWEREADDED))
			return 1;
	}
	return 0;
}

static int iBoldControls[] = { IDC_TXT_TITLE1, IDC_TXT_TITLE2, IDC_TXT_TITLE3, MODERNOPT_CTRL_LAST };

static int IgnoreModernOptInit(WPARAM wParam, LPARAM)
{
	MODERNOPTOBJECT obj = { sizeof(obj) };
	obj.hInstance = hInst;
	obj.dwFlags = MODEROPT_FLG_TCHAR;
	obj.iSection = MODERNOPT_PAGE_IGNORE;
	obj.iType = MODERNOPT_TYPE_SECTIONPAGE;
	obj.iBoldControls = iBoldControls;
	obj.lpzTemplate = MAKEINTRESOURCEA(IDD_MODERNOPT_IGNORE);
	obj.pfnDlgProc = DlgProcIgnoreOpts;
	CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);
	return 0;
}

int LoadIgnoreModule(void)
{
	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = "Ignore";
	pd.type = PROTOTYPE_IGNORE;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	CreateServiceFunction("Ignore"PSR_MESSAGE, IgnoreRecvMessage);
	CreateServiceFunction("Ignore"PSR_URL, IgnoreRecvUrl);
	CreateServiceFunction("Ignore"PSR_FILE, IgnoreRecvFile);
	CreateServiceFunction("Ignore"PSR_AUTH, IgnoreRecvAuth);
	CreateServiceFunction(MS_IGNORE_ISIGNORED, IsIgnored);
	CreateServiceFunction(MS_IGNORE_IGNORE, Ignore);
	CreateServiceFunction(MS_IGNORE_UNIGNORE, Unignore);

	HookEvent(ME_DB_EVENT_FILTER_ADD, IgnoreAddedNotify);
	HookEvent(ME_MODERNOPT_INITIALIZE, IgnoreModernOptInit);
	HookEvent(ME_OPT_INITIALISE, IgnoreOptInitialise);
	return 0;
}
