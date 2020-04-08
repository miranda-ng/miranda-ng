/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-20 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-05 Miranda ICQ/IM project,
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

#include "stdafx.h"

CMPlugin g_plugin;
CLIST_INTERFACE coreCli;

HIMAGELIST himlCListClc = nullptr;

LRESULT CALLBACK ContactListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ContactListControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void RebuildEntireList(HWND hwnd, ClcData *dat);
void RebuildEntireListInternal(HWND hwnd, ClcData *dat, BOOL call_orig);
void SetGroupExpand(HWND hwnd, ClcData *dat, struct ClcGroup *group, int newState);
void ScrollTo(HWND hwnd, ClcData *dat, int desty, int noSmooth);
void RecalcScrollBar(HWND hwnd, ClcData *dat);
void LoadClcOptions(HWND hwnd, ClcData *dat, BOOL);
int GetRowHeight(ClcData *dat, int item);
void SortCLC(HWND hwnd, ClcData *dat, int useInsertionSort);

/////////////////////////////////////////////////////////////////////////////////////////
// external functions

void InitCustomMenus(void);
void PaintClc(HWND hwnd, ClcData *dat, HDC hdc, RECT *rcPaint);

int ClcOptInit(WPARAM wParam, LPARAM lParam);
int CluiOptInit(WPARAM wParam, LPARAM lParam);
int CListOptInit(WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////////////////
// returns the plugin information

PLUGININFOEX pluginInfoEx = {
	sizeof(pluginInfoEx),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {53E095A3-2695-490A-9DAD-D20479093831}
	{0x53e095a3, 0x2695, 0x490a, {0x9d, 0xad, 0xd2, 0x4, 0x79, 0x9, 0x38, 0x31}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>("CList", pluginInfoEx)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// returns plugin's interfaces information

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_CLIST, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////
// called when number of accounts has been changed

static int OnAccountsChanged(WPARAM, LPARAM)
{
	himlCListClc = Clist_GetImageList();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// called when all modules got loaded

static int OnModulesLoaded(WPARAM, LPARAM)
{
	himlCListClc = Clist_GetImageList();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// options iniatialization

static int OnOptsInit(WPARAM wParam, LPARAM lParam)
{
	ClcOptInit(wParam, lParam);
	CluiOptInit(wParam, lParam);
	CListOptInit(wParam, lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// menu status services

static INT_PTR GetStatusMode(WPARAM, LPARAM)
{
	return g_clistApi.currentDesiredStatusMode;
}

/////////////////////////////////////////////////////////////////////////////////////////
// main clist initialization routine

int CMPlugin::Load()
{
	g_bSortByStatus = g_plugin.getByte("SortByStatus", SETTING_SORTBYSTATUS_DEFAULT);
	g_bSortByProto = g_plugin.getByte("SortByProto", SETTING_SORTBYPROTO_DEFAULT);

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

	CreateServiceFunction(MS_CLIST_GETSTATUSMODE, GetStatusMode);

	HookEvent(ME_OPT_INITIALISE, OnOptsInit);
	HookEvent(ME_PROTO_ACCLISTCHANGED, OnAccountsChanged);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	InitCustomMenus();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

wchar_t* MyDBGetContactSettingTString(MCONTACT hContact, char* module, char* setting, wchar_t* out, size_t len, wchar_t *def)
{
	DBVARIANT dbv;

	out[0] = '\0';

	if (!db_get_ws(hContact, module, setting, &dbv)) {
		if (dbv.type == DBVT_ASCIIZ)
			MultiByteToWideChar(CP_ACP, 0, dbv.pszVal, -1, out, (int)len);
		else if (dbv.type == DBVT_UTF8)
			MultiByteToWideChar(CP_UTF8, 0, dbv.pszVal, -1, out, (int)len);
		else if (dbv.type == DBVT_WCHAR)
			mir_wstrncpy(out, dbv.pwszVal, (int)len);
		else if (def != nullptr)
			mir_wstrncpy(out, def, (int)len);

		db_free(&dbv);
	}
	else {
		if (def != nullptr)
			mir_wstrncpy(out, def, (int)len);
	}

	return out;
}

#define DATA_BLOCK 128

typedef struct
{
	wchar_t *text;
	size_t allocated;
	size_t used;

} StringHelper;

int CopyData(StringHelper *str, const wchar_t *text, size_t len)
{
	size_t totalSize;

	if (len == 0)
		return 0;

	if (text == nullptr)
		return 0;

	totalSize = str->used + len + 1;

	if (totalSize > str->allocated) {
		totalSize += DATA_BLOCK - (totalSize % DATA_BLOCK);

		if (str->text != nullptr) {
			wchar_t *tmp = (wchar_t *)mir_realloc(str->text, sizeof(wchar_t) * totalSize);

			if (tmp == nullptr) {
				mir_free(str->text);
				return -1;
			}

			str->text = tmp;
		}
		else {
			str->text = (wchar_t *)mir_alloc(sizeof(wchar_t) * totalSize);

			if (str->text == nullptr) {
				return -2;
			}
		}

		str->allocated = totalSize;
	}

	memmove(&str->text[str->used], text, sizeof(wchar_t) * len);
	str->used += len;
	str->text[str->used] = '\0';

	return 0;
}


wchar_t *ParseText(const wchar_t *text,
	const wchar_t **variables, size_t variablesSize,
	const wchar_t **data, size_t dataSize)
{
	size_t length = mir_wstrlen(text);
	size_t nextPos = 0;
	StringHelper ret = {};
	size_t i;

	// length - 1 because a % in last char will be a % and point
	for (i = 0; i < length - 1; i++) {
		if (text[i] == '%') {
			BOOL found = FALSE;

			if (CopyData(&ret, &text[nextPos], i - nextPos))
				return nullptr;

			if (text[i + 1] == '%') {
				if (CopyData(&ret, L"%", 1))
					return nullptr;

				i++;

				found = TRUE;
			}
			else {
				size_t size = min(variablesSize, dataSize);
				size_t j;

				// See if can find it
				for (j = 0; j < size; j++) {
					size_t vlen = mir_wstrlen(variables[j]);

					if (wcsnicmp(&text[i], variables[j], vlen) == 0) {
						if (CopyData(&ret, data[j], mir_wstrlen(data[j])))
							return nullptr;

						i += vlen - 1;

						found = TRUE;

						break;
					}
				}
			}

			if (found)
				nextPos = i + 1;
			else
				nextPos = i;
		}
	}

	if (nextPos < length)
		if (CopyData(&ret, &text[nextPos], length - nextPos))
			return nullptr;

	return ret.text;
}

LRESULT CALLBACK ContactListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

/////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK ContactListControlSubclass(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_CHAR) {
		coreCli.pfnContactListControlWndProc(GetParent(hwnd), msg, wParam, lParam);
		return 0;
	}

	return mir_callNextSubclass(hwnd, ContactListControlSubclass, msg, wParam, lParam);
}

LRESULT CALLBACK ContactListControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ClcData *dat = (ClcData*)GetWindowLongPtr(hwnd, 0);
	RECT r;

	switch (msg) {
	case WM_CREATE:
		dat = new ClcData();
		SetWindowLongPtr(hwnd, 0, (LONG_PTR)dat);

		dat->hwnd_list = CreateWindow(L"LISTBOX", L"",
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
		Clist_EnsureVisible(hwnd, (ClcData*)dat, dat->selection, 0);
		UpdateWindow(hwnd);
		break;

	case WM_SETFOCUS:
	case WM_ENABLE:
		SetFocus(dat->hwnd_list);
		break;
	}

	return coreCli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

wchar_t status_name[128];
wchar_t* GetStatusName(struct ClcContact *item)
{
	int status;

	status_name[0] = '\0';
	if (item->hContact == NULL || item->pce->szProto == nullptr)
		return status_name;

	// Get XStatusName
	MyDBGetContactSettingTString(item->hContact, item->pce->szProto, "XStatusName", status_name, _countof(status_name), nullptr);
	if (status_name[0] != '\0')
		return status_name;

	// Get status name
	status = db_get_w(item->hContact, item->pce->szProto, "Status", ID_STATUS_OFFLINE);
	mir_wstrncpy(status_name, Clist_GetStatusModeDescription(status, 0), _countof(status_name));

	return status_name;
}

wchar_t status_message[256];
wchar_t* GetStatusMessage(struct ClcContact *item)
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
wchar_t* GetProtoName(struct ClcContact *item)
{
	char description[128];

	proto_name[0] = '\0';
	if (item->hContact == NULL || item->pce->szProto == nullptr) {
		mir_wstrncpy(proto_name, TranslateT("Unknown protocol"), _countof(proto_name));
		return proto_name;
	}

	PROTOACCOUNT *acc = Proto_GetAccount(item->pce->szProto);
	if (acc == nullptr) {
		CallProtoService(item->pce->szProto, PS_GETNAME, sizeof(description), (LPARAM)description);
		mir_snwprintf(proto_name, L"%S", description);
		return proto_name;
	}

	mir_wstrncpy(proto_name, acc->tszAccountName, _countof(proto_name));

	return proto_name;
}

void RebuildEntireListInternal(HWND hwnd, ClcData *tmp_dat, BOOL call_orig)
{
	ClcData *dat = (ClcData*)tmp_dat;
	wchar_t tmp[1024];
	wchar_t count[128];
	wchar_t template_contact[1024];
	wchar_t template_group[1024];
	wchar_t template_divider[1024];
	wchar_t template_info[1024];
	int selection = dat->selection;
	BOOL has_focus = (GetFocus() == dat->hwnd_list || GetFocus() == hwnd);

	if (call_orig)
		coreCli.pfnRebuildEntireList(hwnd, (ClcData*)dat);

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
				wchar_t *szCounts = Clist_GetGroupCountsText(dat, item);
				const wchar_t *t[] = {
					L"%name%",
					L"%count%",
					L"%mode%"
				};
				const wchar_t *v[] = {
					item->szText,
					count,
					item->group->expanded ? TranslateT("Expanded") : TranslateT("Collapsed")
				};

				if (szCounts[0] != '\0')
					mir_snwprintf(count, L"%s ", szCounts);
				else
					count[0] = '\0';

				wchar_t *txt = ParseText(template_group, t, _countof(t), v, _countof(v));
				if (txt != nullptr)
					mir_wstrncpy(text, txt, size);
				mir_free(txt);
			}
			break;

		case CLCIT_CONTACT:
			{
				const wchar_t *t[] = {
					L"%name%",
					L"%status%",
					L"%protocol%",
					L"%status_message%"
				};
				const wchar_t *v[] = {
					item->szText,
					GetStatusName(item),
					GetProtoName(item),
					GetStatusMessage(item)
				};

				wchar_t *txt = ParseText(template_contact, t, _countof(t), v, _countof(v));
				if (txt != nullptr)
					mir_wstrncpy(text, txt, size);
				mir_free(txt);
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

void RebuildEntireList(HWND hwnd, ClcData *dat)
{
	RebuildEntireListInternal(hwnd, dat, TRUE);
}

void SetGroupExpand(HWND hwnd, ClcData *dat, struct ClcGroup *group, int newState)
{
	coreCli.pfnSetGroupExpand(hwnd, dat, group, newState);
	dat->need_rebuild = TRUE;
}

void ScrollTo(HWND, ClcData*, int, int)
{
}

void RecalcScrollBar(HWND, ClcData*)
{
}

void LoadClcOptions(HWND hwnd, ClcData *dat, BOOL bFirst)
{
	coreCli.pfnLoadClcOptions(hwnd, dat, bFirst);

	dat->bFilterSearch = false;
	dat->rowHeight = SendMessage(dat->hwnd_list, LB_GETITEMHEIGHT, 0, 0);
}

int GetRowHeight(ClcData *dat, int)
{
	dat->rowHeight = SendMessage(dat->hwnd_list, LB_GETITEMHEIGHT, 0, 0);
	return dat->rowHeight;
}

void SortCLC(HWND hwnd, ClcData *dat, int useInsertionSort)
{
	if (dat->bNeedsResort) {
		coreCli.pfnSortCLC(hwnd, dat, useInsertionSort);
		dat->need_rebuild = TRUE;
	}
}
