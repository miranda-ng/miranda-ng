/*
Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

void PaintClc(HWND hwnd, ClcData *dat, HDC hdc, RECT *rcPaint);

/////////////////////////////////////////////////////////////////////////////////////////

wchar_t status_name[128];
static wchar_t* GetStatusName(struct ClcContact *item)
{
	status_name[0] = '\0';
	if (item->hContact == NULL || item->pce->szProto == nullptr)
		return status_name;

	// Get XStatusName
	MyDBGetContactSettingTString(item->hContact, item->pce->szProto, "XStatusName", status_name, _countof(status_name), nullptr);
	if (status_name[0] != '\0')
		return status_name;

	// Get status name
	int status = db_get_w(item->hContact, item->pce->szProto, "Status", ID_STATUS_OFFLINE);
	mir_wstrncpy(status_name, Clist_GetStatusModeDescription(status, 0), _countof(status_name));
	return status_name;
}

wchar_t status_message[256];
static wchar_t* GetStatusMessage(struct ClcContact *item)
{
	status_message[0] = '\0';
	if (item->hContact == NULL || item->pce->szProto == nullptr)
		return status_message;

	// Get XStatusMsg
	MyDBGetContactSettingTString(item->hContact, item->pce->szProto, "XStatusMsg", status_message, _countof(status_message), nullptr);
	if (status_message[0] != '\0')
		return status_message;

	// Get status message
	MyDBGetContactSettingTString(item->hContact, "CList", "StatusMsg", status_message, _countof(status_message), nullptr);
	return status_message;
}

wchar_t proto_name[128];
static wchar_t* GetProtoName(struct ClcContact *item)
{
	proto_name[0] = '\0';
	if (item->hContact == NULL || item->pce->szProto == nullptr) {
		mir_wstrncpy(proto_name, TranslateT("Unknown protocol"), _countof(proto_name));
		return proto_name;
	}

	PROTOACCOUNT *acc = Proto_GetAccount(item->pce->szProto);
	if (acc == nullptr) {
		char description[128];
		CallProtoService(item->pce->szProto, PS_GETNAME, sizeof(description), (LPARAM)description);
		mir_snwprintf(proto_name, L"%S", description);
		return proto_name;
	}

	mir_wstrncpy(proto_name, acc->tszAccountName, _countof(proto_name));
	return proto_name;
}

static void RebuildEntireListInternal(HWND hwnd, ClcData *tmp_dat, BOOL call_orig)
{
	ClcData *dat = (ClcData *)tmp_dat;
	wchar_t tmp[1024];
	wchar_t template_contact[1024];
	wchar_t template_group[1024];
	wchar_t template_divider[1024];
	wchar_t template_info[1024];
	int selection = dat->selection;
	BOOL has_focus = (GetFocus() == dat->hwnd_list || GetFocus() == hwnd);

	if (call_orig)
		coreCli.pfnRebuildEntireList(hwnd, (ClcData *)dat);

	MyDBGetContactSettingTString(NULL, "CLC", "TemplateContact", template_contact, 1024, TranslateT("%name% [%status% %protocol%] %status_message%"));
	MyDBGetContactSettingTString(NULL, "CLC", "TemplateGroup", template_group, 1024, TranslateT("Group: %name% %count% [%mode%]"));
	MyDBGetContactSettingTString(NULL, "CLC", "TemplateDivider", template_divider, 1024, TranslateT("Divider: %s"));
	MyDBGetContactSettingTString(NULL, "CLC", "TemplateInfo", template_info, 1024, TranslateT("Info: %s"));

	SendMessage(dat->hwnd_list, WM_SETREDRAW, FALSE, 0);

	// Reset content
	SendMessage(dat->hwnd_list, LB_RESETCONTENT, 0, 0);

	// Set font
	SendMessage(dat->hwnd_list, WM_SETFONT, (WPARAM)dat->fontInfo[FONTID_CONTACTS].hFont, 0);

	// Add all items to the list
	ClcGroup *group = &dat->list;
	group->scanIndex = 0;

	wchar_t *text = tmp;
	size_t size = _countof(tmp);
	while (true) {
		if (group->scanIndex == group->cl.getCount()) {
			if ((group = group->parent) == nullptr)
				break;
			text -= 2;
			size += 2;
			group->scanIndex++;
			continue;
		}

		ClcContact *item = group->cl[group->scanIndex];
		text[0] = '\0';
		switch (item->type) {
		case CLCIT_GROUP:
			{
				CMStringW wszText(template_group);
				wszText.Replace(L"%name%", item->szText);
				wszText.Replace(L"%mode%", item->group->expanded ? TranslateT("Expanded") : TranslateT("Collapsed"));

				wchar_t *szCounts = Clist_GetGroupCountsText(dat, item);
				wchar_t count[128];
				if (szCounts[0] != '\0')
					mir_snwprintf(count, L"%s ", szCounts);
				else
					count[0] = '\0';
				wszText.Replace(L"%count%", count);

				if (!wszText.IsEmpty())
					mir_wstrncpy(text, wszText, size);
			}
			break;

		case CLCIT_CONTACT:
			{
				CMStringW wszText(template_contact);
				wszText.Replace(L"%name%", item->szText);
				wszText.Replace(L"%status%", GetStatusName(item));
				wszText.Replace(L"%protocol%", GetProtoName(item));
				wszText.Replace(L"%status_message%", GetStatusMessage(item));

				if (!wszText.IsEmpty())
					mir_wstrncpy(text, wszText, size);
			}
			break;

		case CLCIT_DIVIDER:
			mir_snwprintf(text, size, template_divider, item->szText);
			break;

		case CLCIT_INFO:
			mir_snwprintf(text, size, template_info, item->szText);
			break;
		}

		SendMessage(dat->hwnd_list, LB_ADDSTRING, 0, (LPARAM)tmp);

		if (item->type == CLCIT_GROUP && item->group->expanded) {
			group = item->group;
			text[0] = ' ';
			text[1] = ' ';
			text += 2;
			size -= 2;
			group->scanIndex = 0;
			continue;
		}
		group->scanIndex++;
	}

	SendMessage(dat->hwnd_list, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(dat->hwnd_list, nullptr, TRUE);

	dat->selection = selection;
	SendMessage(dat->hwnd_list, LB_SETCURSEL, dat->selection, 0);
	if (has_focus)
		SetFocus(dat->hwnd_list);

	dat->need_rebuild = FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK ContactListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_NCCREATE:
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_CREATE:
		break;

	case WM_DRAWITEM:
		return Menu_DrawItem(lParam);
	}
	return coreCli.pfnContactListWndProc(hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK ContactListControlSubclass(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_CHAR) {
		coreCli.pfnContactListControlWndProc(GetParent(hwnd), msg, wParam, lParam);
		return 0;
	}

	return mir_callNextSubclass(hwnd, ContactListControlSubclass, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK ContactListControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ClcData *dat = (ClcData *)GetWindowLongPtr(hwnd, 0);
	RECT r;

	switch (msg) {
	case WM_CREATE:
		dat = new ClcData();
		SetWindowLongPtr(hwnd, 0, (LONG_PTR)dat);

		dat->hwnd_list = CreateWindowW(L"LISTBOX", L"",
			(WS_VISIBLE | WS_CHILD | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY | LBS_WANTKEYBOARDINPUT | WS_VSCROLL),
			0, 0, 0, 0, hwnd, nullptr, g_plugin.getInst(), nullptr);
		dat->need_rebuild = FALSE;
		mir_subclassWindow(dat->hwnd_list, ContactListControlSubclass);

		GetClientRect(hwnd, &r);
		SetWindowPos(dat->hwnd_list, nullptr, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOACTIVATE);
		break;

	case WM_SIZE:
		GetClientRect(hwnd, &r);
		SetWindowPos(dat->hwnd_list, nullptr, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOACTIVATE);
		break;

	case WM_PRINTCLIENT:
	case WM_PAINT:
		if (dat->need_rebuild)
			RebuildEntireListInternal(hwnd, (ClcData *)dat, FALSE);
		__fallthrough;

	case WM_VSCROLL:
	case WM_MOUSEWHEEL:
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case INTM_SCROLLBARCHANGED:
		return TRUE;

	case WM_VKEYTOITEM:
		{
			WORD key = LOWORD(wParam);
			if (key == VK_LEFT || key == VK_RIGHT || key == VK_RETURN || key == VK_DELETE || key == VK_F2) {
				coreCli.pfnContactListControlWndProc(hwnd, WM_KEYDOWN, key, 0);
				return dat->selection;
			}

			NMKEY nmkey;
			nmkey.hdr.hwndFrom = hwnd;
			nmkey.hdr.idFrom = GetDlgCtrlID(hwnd);
			nmkey.hdr.code = NM_KEYDOWN;
			nmkey.nVKey = key;
			nmkey.uFlags = 0;
			if (SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)&nmkey))
				return -2;
		}
		return -1;

	case WM_COMMAND:
		if ((HANDLE)lParam != dat->hwnd_list || HIWORD(wParam) != LBN_SELCHANGE)
			break;

		dat->selection = SendMessage(dat->hwnd_list, LB_GETCURSEL, 0, 0);

		KillTimer(hwnd, TIMERID_INFOTIP);
		KillTimer(hwnd, TIMERID_RENAME);
		dat->szQuickSearch[0] = 0;
		g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
		Clist_EnsureVisible(hwnd, (ClcData *)dat, dat->selection, 0);
		UpdateWindow(hwnd);
		break;

	case WM_SETFOCUS:
	case WM_ENABLE:
		SetFocus(dat->hwnd_list);
		break;
	}

	return coreCli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
}

static void RebuildEntireList(HWND hwnd, ClcData *dat)
{
	RebuildEntireListInternal(hwnd, dat, TRUE);
}

static void SetGroupExpand(HWND hwnd, ClcData *dat, struct ClcGroup *group, int newState)
{
	coreCli.pfnSetGroupExpand(hwnd, dat, group, newState);
	dat->need_rebuild = TRUE;
}

static void ScrollTo(HWND, ClcData *, int, int)
{
}

static void RecalcScrollBar(HWND, ClcData *)
{
}

static void LoadClcOptions(HWND hwnd, ClcData *dat, BOOL bFirst)
{
	coreCli.pfnLoadClcOptions(hwnd, dat, bFirst);

	dat->bFilterSearch = false;
	dat->rowHeight = SendMessage(dat->hwnd_list, LB_GETITEMHEIGHT, 0, 0);
}

static int GetRowHeight(ClcData *dat, int)
{
	dat->rowHeight = SendMessage(dat->hwnd_list, LB_GETITEMHEIGHT, 0, 0);
	return dat->rowHeight;
}

static void SortCLC(HWND hwnd, ClcData *dat, int useInsertionSort)
{
	if (dat->bNeedsResort) {
		coreCli.pfnSortCLC(hwnd, dat, useInsertionSort);
		dat->need_rebuild = TRUE;
	}
}

void InitClc()
{
	Clist_GetInterface();
	coreCli = g_clistApi;
	g_clistApi.bOwnerDrawMenu = false;
	g_clistApi.hInst = g_plugin.getInst();
	g_clistApi.pfnPaintClc = PaintClc;
	g_clistApi.pfnContactListWndProc = ContactListWndProc;
	g_clistApi.pfnContactListControlWndProc = ContactListControlWndProc;
	g_clistApi.pfnRebuildEntireList = RebuildEntireList;
	g_clistApi.pfnSetGroupExpand = SetGroupExpand;
	g_clistApi.pfnRecalcScrollBar = RecalcScrollBar;
	g_clistApi.pfnScrollTo = ScrollTo;
	g_clistApi.pfnLoadClcOptions = LoadClcOptions;
	g_clistApi.pfnGetRowHeight = GetRowHeight;
	g_clistApi.pfnSortCLC = SortCLC;
	g_clistApi.pfnCompareContacts = CompareContacts;
}
