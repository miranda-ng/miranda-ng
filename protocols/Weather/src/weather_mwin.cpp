/*
Weather Protocol plugin for Miranda IM
Copyright (c) 2012 Miranda NG team
Copyright (c) 2006-2009 Boris Krasnovskiy All Rights Reserved
Copyright (c) 2002-2006 Calvin Che

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

static MWindowList hMwinWindowList;
static HANDLE hFontHook;

HGENMENU hMwinMenu;

struct MWinDataType
{
	MCONTACT hContact;
	HWND hAvt;
	BOOL haveAvatar;
};

#define WM_REDRAWWIN (WM_USER + 17369)

static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MWinDataType *data = (MWinDataType*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (msg) {
	case WM_CREATE:
		data = (MWinDataType*)mir_calloc(sizeof(MWinDataType));
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)data);

		data->hContact = (DWORD_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams;
		data->hAvt = CreateWindow(AVATAR_CONTROL_CLASS, TEXT(""), WS_CHILD, 0, 0, opt.AvatarSize, opt.AvatarSize, hwnd, 0, g_plugin.getInst(), 0);
		if (data->hAvt)
			SendMessage(data->hAvt, AVATAR_SETCONTACT, 0, (LPARAM)data->hContact);
		break;

	case WM_DESTROY:
		mir_free(data);
		break;

	case WM_CONTEXTMENU:
		{
			POINT pt;
			GetCursorPos(&pt);
			HMENU hMenu = Menu_BuildContactMenu(data->hContact);
			TrackPopupMenu(hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, nullptr);
			DestroyMenu(hMenu);
		}
		break;

	case WM_MOUSEMOVE:
		{
			TRACKMOUSEEVENT tme = {};
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.hwndTrack = hwnd;
			tme.dwFlags = TME_QUERY;
			TrackMouseEvent(&tme);

			if (tme.dwFlags == 0) {
				tme.dwFlags = TME_HOVER | TME_LEAVE;
				tme.hwndTrack = hwnd;
				tme.dwHoverTime = CallService(MS_CLC_GETINFOTIPHOVERTIME, 0, 0);
				TrackMouseEvent(&tme);
			}
		}
		break;

	case WM_MOUSEHOVER:
		{
			POINT pt;
			CLCINFOTIP ti = {};

			GetCursorPos(&pt);
			GetWindowRect(hwnd, &ti.rcItem);

			ti.cbSize = sizeof(ti);
			ti.hItem = (HANDLE)data->hContact;
			ti.ptCursor = pt;
			ti.isTreeFocused = 1;
			CallService(MS_TIPPER_SHOWTIP, 0, (LPARAM)&ti);
		}
		break;

	case WM_LBUTTONDBLCLK:
		BriefInfo(data->hContact, 0);
		break;

	case WM_COMMAND:	 //Needed by the contact's context menu
		if (Clist_MenuProcessCommand(LOWORD(wParam), MPCF_CONTACTMENU, data->hContact))
			break;
		return FALSE;

	case WM_MEASUREITEM:	//Needed by the contact's context menu
		return Menu_MeasureItem(lParam);

	case WM_DRAWITEM:	//Needed by the contact's context menu
		return Menu_DrawItem(lParam);

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == NM_AVATAR_CHANGED) {
			BOOL newava = CallService(MS_AV_GETAVATARBITMAP, data->hContact, 0) != 0;
			if (newava != data->haveAvatar) {
				LONG_PTR style = GetWindowLongPtr(data->hAvt, GWL_STYLE);
				data->haveAvatar = newava;
				SetWindowLongPtr(data->hAvt, GWL_STYLE, newava ? (style | WS_VISIBLE) : (style & ~WS_VISIBLE));
				RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE);
			}
		}
		break;

	case WM_REDRAWWIN:
		if (data->hAvt != nullptr) MoveWindow(data->hAvt, 0, 0, opt.AvatarSize, opt.AvatarSize, TRUE);
		RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
		break;

	case WM_PAINT:
		{
			RECT r, rc;

			if (GetUpdateRect(hwnd, &r, FALSE)) {
				int picSize = opt.AvatarSize;
				HICON hIcon = nullptr;

				if (!data->haveAvatar) {
					picSize = GetSystemMetrics(SM_CXICON);
					hIcon = GetStatusIconBig(data->hContact);
				}

				LOGFONT lfnt, lfnt1;
				COLORREF clr = g_plugin.getDword("ColorMwinFrame", GetSysColor(COLOR_3DFACE));
				COLORREF fntc = Font_GetW(_A2W(MODULENAME), LPGENW("Frame Font"), &lfnt);
				COLORREF fntc1 = Font_GetW(_A2W(MODULENAME), LPGENW("Frame Title Font"), &lfnt1);

				ptrW tszInfo(db_get_wsa(data->hContact, WEATHERCONDITION, "WeatherInfo"));

				GetClientRect(hwnd, &rc);

				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hwnd, &ps);

				if (ServiceExists(MS_SKIN_DRAWGLYPH)) {
					SKINDRAWREQUEST rq;
					memset(&rq, 0, sizeof(rq));
					rq.hDC = hdc;
					rq.rcDestRect = rc;
					rq.rcClipRect = rc;

					mir_strcpy(rq.szObjectID, "Main,ID=WeatherFrame");
					CallService(MS_SKIN_DRAWGLYPH, (WPARAM)&rq, 0);
				}

				if (clr != 0xFFFFFFFF) {
					HBRUSH hBkgBrush = CreateSolidBrush(clr);
					FillRect(hdc, &rc, hBkgBrush);
					DeleteObject(hBkgBrush);
				}

				if (!data->haveAvatar)
					DrawIconEx(hdc, 1, 1, hIcon, 0, 0, 0, nullptr, DI_NORMAL);

				SetBkMode(hdc, TRANSPARENT);

				HFONT hfnt = CreateFontIndirect(&lfnt1);
				HFONT hfntold = (HFONT)SelectObject(hdc, hfnt);

				wchar_t *nick = Clist_GetContactDisplayName(data->hContact);

				SIZE fontSize;
				GetTextExtentPoint32(hdc, L"|", 1, &fontSize);

				rc.top += 1;
				rc.left += picSize + fontSize.cx;

				SetTextColor(hdc, fntc1);
				DrawText(hdc, nick, -1, &rc, DT_LEFT | DT_EXPANDTABS);

				rc.top += fontSize.cy;

				SelectObject(hdc, hfntold);
				DeleteObject(hfnt);

				if (tszInfo) {
					HFONT hFont = CreateFontIndirect(&lfnt);
					HFONT hFontOld = (HFONT)SelectObject(hdc, hFont);

					SetTextColor(hdc, fntc);
					DrawText(hdc, tszInfo, -1, &rc, DT_LEFT | DT_EXPANDTABS);

					SelectObject(hdc, hFontOld);
					DeleteObject(hFont);
				}
				EndPaint(hwnd, &ps);

				if (hIcon)
					DestroyIcon(hIcon);
			}
			break;
		}

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return(TRUE);
}

static void addWindow(MCONTACT hContact)
{
	DBVARIANT dbv;
	if (g_plugin.getWString(hContact, "Nick", &dbv))
		return;

	wchar_t winname[512];
	mir_snwprintf(winname, L"Weather: %s", dbv.pwszVal);
	db_free(&dbv);

	HWND hWnd = CreateWindow(L"WeatherFrame", L"", WS_CHILD | WS_VISIBLE,
		0, 0, 10, 10, g_clistApi.hwndContactList, nullptr, g_plugin.getInst(), (void*)hContact);
	WindowList_Add(hMwinWindowList, hWnd, hContact);

	CLISTFrame Frame = {};
	Frame.szName.w = winname;
	Frame.hIcon = g_plugin.getIcon(IDI_ICON);
	Frame.cbSize = sizeof(Frame);
	Frame.hWnd = hWnd;
	Frame.align = alBottom;
	Frame.Flags = F_VISIBLE | F_NOBORDER | F_UNICODE;
	Frame.height = 32;
	int frameID = g_plugin.addFrame(&Frame);

	g_plugin.setDword(hContact, "mwin", frameID);
	Contact::Hide(hContact);
}

void removeWindow(MCONTACT hContact)
{
	uint32_t frameId = g_plugin.getDword(hContact, "mwin");

	WindowList_Remove(hMwinWindowList, WindowList_Find(hMwinWindowList, hContact));
	CallService(MS_CLIST_FRAMES_REMOVEFRAME, frameId, 0);

	g_plugin.setDword(hContact, "mwin", 0);
	Contact::Hide(hContact, false);
}

void UpdateMwinData(MCONTACT hContact)
{
	HWND hwnd = WindowList_Find(hMwinWindowList, hContact);
	if (hwnd != nullptr)
		RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
}

INT_PTR Mwin_MenuClicked(WPARAM wParam, LPARAM)
{
	BOOL addwnd = WindowList_Find(hMwinWindowList, wParam) == nullptr;
	if (addwnd)
		addWindow(wParam);
	else
		removeWindow(wParam);
	return 0;
}

int BuildContactMenu(WPARAM wparam, LPARAM)
{
	int flags = g_plugin.getDword(wparam, "mwin") ? CMIF_CHECKED : 0;
	Menu_ModifyItem(hMwinMenu, nullptr, INVALID_HANDLE_VALUE, flags);
	return 0;
}

int RedrawFrame(WPARAM, LPARAM)
{
	WindowList_Broadcast(hMwinWindowList, WM_REDRAWWIN, 0, 0);
	return 0;
}

void InitMwin(void)
{
	if (!ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
		return;

	hMwinWindowList = WindowList_Create();

	WNDCLASS wndclass;
	wndclass.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = wndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = g_plugin.getInst();
	wndclass.hIcon = nullptr;
	wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndclass.hbrBackground = nullptr; //(HBRUSH)(COLOR_3DFACE+1);
	wndclass.lpszMenuName = nullptr;
	wndclass.lpszClassName = L"WeatherFrame";
	RegisterClass(&wndclass);

	ColourIDW colourid = {};
	mir_strcpy(colourid.dbSettingsGroup, MODULENAME);
	mir_strcpy(colourid.setting, "ColorMwinFrame");
	mir_wstrcpy(colourid.name, LPGENW("Frame Background"));
	mir_wstrcpy(colourid.group, _A2W(MODULENAME));
	colourid.defcolour = GetSysColor(COLOR_3DFACE);
	g_plugin.addColor(&colourid);

	FontIDW fontid = {};
	fontid.flags = FIDF_ALLOWREREGISTER | FIDF_DEFAULTVALID;
	mir_strcpy(fontid.dbSettingsGroup, MODULENAME);
	mir_wstrcpy(fontid.group, _A2W(MODULENAME));
	mir_wstrcpy(fontid.name, LPGENW("Frame Font"));
	mir_strcpy(fontid.setting, "fnt0");

	HDC hdc = GetDC(nullptr);
	fontid.deffontsettings.size = -13;
	ReleaseDC(nullptr, hdc);

	fontid.deffontsettings.charset = DEFAULT_CHARSET;
	mir_wstrcpy(fontid.deffontsettings.szFace, L"Verdana");
	mir_wstrcpy(fontid.backgroundGroup, _A2W(MODULENAME));
	mir_wstrcpy(fontid.backgroundName, LPGENW("Frame Background"));
	g_plugin.addFont(&fontid);

	fontid.deffontsettings.style = DBFONTF_BOLD;
	mir_wstrcpy(fontid.name, LPGENW("Frame Title Font"));
	mir_strcpy(fontid.setting, "fnt1");
	g_plugin.addFont(&fontid);

	for (auto &hContact : Contacts(MODULENAME))
		if (g_plugin.getDword(hContact, "mwin"))
			addWindow(hContact);

	hFontHook = HookEvent(ME_FONT_RELOAD, RedrawFrame);
}

void DestroyMwin(void)
{
	for (auto &hContact : Contacts(MODULENAME)) {
		uint32_t frameId = g_plugin.getDword(hContact, "mwin");
		if (frameId)
			CallService(MS_CLIST_FRAMES_REMOVEFRAME, frameId, 0);
	}
	UnregisterClass(L"WeatherFrame", g_plugin.getInst());
	WindowList_Destroy(hMwinWindowList);
	UnhookEvent(hFontHook);
}
