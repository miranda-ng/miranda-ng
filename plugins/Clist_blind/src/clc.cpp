/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

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

static void ScrollTo(HWND, ClcData *dat, int, int)
{
	ListView_SetSelectionMark(dat->hwnd_list, dat->selection);
	ListView_SetItemState(dat->hwnd_list, dat->selection, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
}

/////////////////////////////////////////////////////////////////////////////////////////

wchar_t status_name[128];
static wchar_t* GetStatusName(ClcContact *item)
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
static wchar_t* GetStatusMessage(ClcContact *item)
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
static wchar_t* GetProtoName(ClcContact *item)
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

static void RebuildEntireListInternal(HWND hwnd, ClcData *tmp_dat, bool call_orig)
{
	ClcData *dat = tmp_dat;
	int level = 0, iItem = 0;

	wchar_t tmp[1024];
	wchar_t template_contact[1024];
	wchar_t template_group[1024];
	wchar_t template_divider[1024];
	wchar_t template_info[1024];
	int selection = dat->selection;
	BOOL has_focus = (GetFocus() == dat->hwnd_list || GetFocus() == hwnd);

	if (call_orig)
		coreCli.pfnRebuildEntireList(hwnd, dat);

	MyDBGetContactSettingTString(NULL, "CLC", "TemplateContact", template_contact, 1024, TranslateT("%name% [%status% %protocol%] %status_message%"));
	MyDBGetContactSettingTString(NULL, "CLC", "TemplateGroup", template_group, 1024, TranslateT("Group: %name% %count% [%mode%]"));
	MyDBGetContactSettingTString(NULL, "CLC", "TemplateDivider", template_divider, 1024, TranslateT("Divider: %s"));
	MyDBGetContactSettingTString(NULL, "CLC", "TemplateInfo", template_info, 1024, TranslateT("Info: %s"));

	SendMessage(dat->hwnd_list, WM_SETREDRAW, FALSE, 0);

	// Reset content
	ListView_DeleteAllItems(dat->hwnd_list);

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
			
			level--;
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

		LVITEMW lvi = {};
		lvi.iItem = iItem++;
		lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_INDENT;
		lvi.pszText = tmp;
		lvi.iIndent = level;
		lvi.lParam = LPARAM(item);
		ListView_InsertItem(dat->hwnd_list, &lvi);

		if (item->type == CLCIT_GROUP && item->group->expanded) {
			group = item->group;
			level++;
			group->scanIndex = 0;
			continue;
		}
		group->scanIndex++;
	}

	SendMessage(dat->hwnd_list, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(dat->hwnd_list, nullptr, TRUE);

	if (dat->list.cl.getCount()) {
		RECT rc;
		ListView_GetItemRect(dat->hwnd_list, 0, &rc, LVIR_SELECTBOUNDS);
		if (rc.bottom > rc.top)
			dat->rowHeight = rc.bottom - rc.top;
	}

	dat->selection = selection;
	ScrollTo(dat->hwnd_list, dat, 0, 0);
	if (has_focus)
		SetFocus(dat->hwnd_list);

	dat->bNeedsRebuild = false;
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
	switch (msg) {
	case WM_KEYDOWN:
		if (wParam == VK_LEFT || wParam == VK_RIGHT || wParam == VK_RETURN || wParam == VK_DELETE || wParam == VK_F2)
			coreCli.pfnContactListControlWndProc(GetParent(hwnd), WM_KEYDOWN, wParam, 0);
		break;

	case WM_CHAR:
		HWND hwndParent = GetParent(hwnd);
		ClcData *dat = (ClcData *)GetWindowLongPtr(hwndParent, 0);
		int iSaveSelection = dat->selection;

		coreCli.pfnContactListControlWndProc(hwndParent, msg, wParam, lParam);

		if (iSaveSelection != dat->selection)
			ScrollTo(dat->hwnd_list, dat, 0, 0);

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

		dat->hwnd_list = CreateWindowW(WC_LISTVIEWW, L"", 
			WS_VISIBLE | WS_CHILD | WS_VSCROLL | LVS_NOCOLUMNHEADER | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_REPORT,
			0, 0, 0, 0, hwnd, nullptr, g_plugin.getInst(), nullptr);
		dat->bNeedsRebuild = false;
		mir_subclassWindow(dat->hwnd_list, ContactListControlSubclass);

		ListView_SetExtendedListViewStyle(dat->hwnd_list, LVS_EX_FULLROWSELECT);

		GetClientRect(hwnd, &r);
		SetWindowPos(dat->hwnd_list, nullptr, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOACTIVATE);
		{
			LVCOLUMN lvc = {};
			lvc.mask = LVCF_FMT | LVCF_WIDTH;
			lvc.fmt = LVCFMT_LEFT;
			lvc.cx = r.right - r.left;
			ListView_InsertColumn(dat->hwnd_list, 0, &lvc);

			HIMAGELIST hImgList = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 10, 1);
			ListView_SetImageList(dat->hwnd_list, hImgList, LVSIL_SMALL);
		}
		break;

	case WM_SIZE:
		GetClientRect(hwnd, &r);
		SetWindowPos(dat->hwnd_list, nullptr, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOACTIVATE);
		ListView_SetColumnWidth(dat->hwnd_list, 0, r.right - r.left);
		break;

	case WM_PRINTCLIENT:
	case WM_PAINT:
		if (dat->bNeedsRebuild)
			RebuildEntireListInternal(hwnd, dat, false);
		__fallthrough;

	case WM_VSCROLL:
	case WM_MOUSEWHEEL:
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case INTM_SCROLLBARCHANGED:
		return TRUE;

	case WM_NOTIFY:
		if (LPNMHDR pnmh = (LPNMHDR)lParam)
			if (pnmh->code == LVN_ITEMCHANGED || pnmh->code == LVN_ITEMACTIVATE)
				dat->selection = ListView_GetSelectionMark(dat->hwnd_list);
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
	RebuildEntireListInternal(hwnd, dat, true);
}

static void SetGroupExpand(HWND hwnd, ClcData *dat, ClcGroup *group, int newState)
{
	coreCli.pfnSetGroupExpand(hwnd, dat, group, newState);
	dat->bNeedsRebuild = true;
}

static void RecalcScrollBar(HWND, ClcData *)
{
}

static int GetRowHeight(ClcData *dat, int)
{
	return dat->rowHeight;
}

static void LoadClcOptions(HWND hwnd, ClcData *dat, BOOL bFirst)
{
	coreCli.pfnLoadClcOptions(hwnd, dat, bFirst);

	dat->rowHeight = 16;
}

static void SortCLC(HWND hwnd, ClcData *dat, int useInsertionSort)
{
	if (dat->bNeedsResort) {
		coreCli.pfnSortCLC(hwnd, dat, useInsertionSort);
		dat->bNeedsRebuild = true;
	}
}

void InitClc()
{
	Clist_GetInterface();
	coreCli = g_clistApi;
	g_clistApi.bOwnerDrawMenu = false;
	g_clistApi.hInst = g_plugin.getInst();
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
