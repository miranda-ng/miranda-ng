/*
Weather Protocol plugin for Miranda IM
Copyright (c) 2012 Miranda NG Team
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

#include "weather.h"

#define MS_TOOLTIP_SHOWTIP		"mToolTip/ShowTip"
#define MS_TOOLTIP_HIDETIP		"mToolTip/HideTip"

static HANDLE hMwinWindowList;
static HANDLE hFontHook;

HGENMENU hMwinMenu;

typedef struct
{
	MCONTACT hContact;
	HWND hAvt;
	BOOL haveAvatar;
} MWinDataType;

#define WM_REDRAWWIN (WM_USER + 17369)

static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MWinDataType *data = (MWinDataType*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch(msg)
	{
	case WM_CREATE:
		data = (MWinDataType*)mir_calloc(sizeof(MWinDataType));
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)data);

		data->hContact = (MCONTACT)((LPCREATESTRUCT)lParam)->lpCreateParams;
		data->hAvt = CreateWindow(AVATAR_CONTROL_CLASS, TEXT(""), WS_CHILD,
			0, 0, opt.AvatarSize, opt.AvatarSize, hwnd, NULL, hInst, 0);
		if (data->hAvt) SendMessage(data->hAvt, AVATAR_SETCONTACT, 0, (LPARAM)data->hContact);
		break;

	case WM_DESTROY:
		mir_free(data);
		break;

	case WM_CONTEXTMENU:
		{
			POINT pt;

			HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)data->hContact, 0);
			GetCursorPos(&pt);
			TrackPopupMenu(hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
			DestroyMenu(hMenu);
		}
		break;

	case WM_MOUSEMOVE:
		{
			TRACKMOUSEEVENT tme = {0};
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.hwndTrack = hwnd;
			tme.dwFlags = TME_QUERY;
			TrackMouseEvent(&tme);

			if (tme.dwFlags == 0)
			{
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
			CLCINFOTIP ti = {0};

			GetCursorPos(&pt);
			GetWindowRect(hwnd, &ti.rcItem);

			ti.cbSize = sizeof(ti);
			ti.hItem = (HANDLE)data->hContact;
			ti.ptCursor = pt;
			ti.isTreeFocused = 1;
			CallService(MS_TOOLTIP_SHOWTIP, 0, (LPARAM)&ti);
		}
		break;

	case WM_LBUTTONDBLCLK:
		BriefInfo((WPARAM)data->hContact, 0);
		break;

	case WM_COMMAND:	 //Needed by the contact's context menu
		if ( CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam),MPCF_CONTACTMENU), (LPARAM)data->hContact))
			break;
		return FALSE;

	case WM_MEASUREITEM:	//Needed by the contact's context menu
		return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);

	case WM_DRAWITEM:	//Needed by the contact's context menu
		return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == NM_AVATAR_CHANGED)
		{
			BOOL newava = CallService(MS_AV_GETAVATARBITMAP, (WPARAM)data->hContact, 0) != 0;
			if (newava != data->haveAvatar)
			{
				LONG_PTR style = GetWindowLongPtr(data->hAvt, GWL_STYLE);
				data->haveAvatar = newava;
				SetWindowLongPtr(data->hAvt, GWL_STYLE, newava ? (style | WS_VISIBLE) : (style & ~WS_VISIBLE));
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
			}
		}
		break;

	case WM_REDRAWWIN:
		if (data->hAvt != NULL) MoveWindow(data->hAvt, 0, 0, opt.AvatarSize, opt.AvatarSize, TRUE);
		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		break;

	case WM_PAINT:
		{
			RECT r, rc;

			if (GetUpdateRect(hwnd, &r, FALSE))
			{
				DBVARIANT dbv = {0};
				PAINTSTRUCT ps;
				LOGFONT lfnt, lfnt1;
				COLORREF fntc, fntc1;
				COLORREF clr;
				int picSize = opt.AvatarSize;
				HICON hIcon = NULL;

				if ( !data->haveAvatar)
				{
					int statusIcon = db_get_w(data->hContact, WEATHERPROTONAME, "Status", 0);

					picSize = GetSystemMetrics(SM_CXICON);
					hIcon = LoadSkinnedProtoIconBig(WEATHERPROTONAME, statusIcon);
					if ((INT_PTR)hIcon == CALLSERVICE_NOTFOUND)
					{
						picSize = GetSystemMetrics(SM_CXSMICON);
						hIcon = LoadSkinnedProtoIcon(WEATHERPROTONAME, statusIcon);
					}
				}

				clr = db_get_dw(NULL, WEATHERPROTONAME, "ColorMwinFrame", GetSysColor(COLOR_3DFACE));

				{
					FontIDT fntid = {0};
					mir_tstrcpy(fntid.group, _T(WEATHERPROTONAME));
					mir_tstrcpy(fntid.name, LPGENT("Frame Font"));
					fntc = CallService(MS_FONT_GETT, (WPARAM)&fntid, (LPARAM)&lfnt);

					mir_tstrcpy(fntid.name, LPGENT("Frame Title Font"));
					fntc1 = CallService(MS_FONT_GETT, (WPARAM)&fntid, (LPARAM)&lfnt1);
				}

				db_get_ts(data->hContact, WEATHERCONDITION, "WeatherInfo", &dbv);

				GetClientRect(hwnd, &rc);

				HDC hdc = BeginPaint(hwnd, &ps);

				if ( ServiceExists(MS_SKIN_DRAWGLYPH)) {
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

				if ( !data->haveAvatar)
					DrawIconEx(hdc, 1, 1, hIcon, 0, 0, 0, NULL, DI_NORMAL);

				SetBkMode(hdc, TRANSPARENT);

				HFONT hfnt = CreateFontIndirect(&lfnt1);
				HFONT hfntold = ( HFONT )SelectObject(hdc, hfnt);
				SIZE fontSize;

				TCHAR *nick = ( TCHAR* )CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)data->hContact, GCDNF_TCHAR);

				GetTextExtentPoint32(hdc, _T("|"), 1, &fontSize);

				rc.top += 1;
				rc.left += picSize + fontSize.cx;

				SetTextColor(hdc, fntc1);
				DrawText(hdc, nick, -1, &rc, DT_LEFT | DT_EXPANDTABS);

				rc.top += fontSize.cy;

				SelectObject(hdc, hfntold);
				DeleteObject(hfnt);

				if (dbv.pszVal)
				{
					HFONT hfnt = CreateFontIndirect(&lfnt);
					HFONT hfntold = ( HFONT )SelectObject(hdc, hfnt);

					SetTextColor(hdc, fntc);
					DrawText(hdc, dbv.ptszVal, -1, &rc, DT_LEFT | DT_EXPANDTABS);

					SelectObject(hdc, hfntold);
					DeleteObject(hfnt);
				}
				EndPaint(hwnd, &ps);
				Skin_ReleaseIcon(hIcon);
				db_free(&dbv);
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
	db_get_ts(hContact, WEATHERPROTONAME, "Nick", &dbv);

	TCHAR winname[512];
	mir_sntprintf(winname, SIZEOF(winname), _T("Weather: %s"), dbv.ptszVal);
	db_free(&dbv);

	HWND hWnd = CreateWindow( _T("WeatherFrame"), _T(""), WS_CHILD | WS_VISIBLE,
		0, 0, 10, 10, (HWND)CallService(MS_CLUI_GETHWND, 0, 0), NULL, hInst, (void*)hContact);
	WindowList_Add(hMwinWindowList, hWnd, hContact);

	CLISTFrame Frame = {0};
	Frame.tname = winname;
	Frame.hIcon = LoadIconEx("main",FALSE);
	Frame.cbSize = sizeof(Frame);
	Frame.hWnd = hWnd;
	Frame.align = alBottom;
	Frame.Flags = F_VISIBLE | F_NOBORDER | F_TCHAR;
	Frame.height = 32;
	DWORD frameID = CallService(MS_CLIST_FRAMES_ADDFRAME, (WPARAM)&Frame, 0);

	db_set_dw(hContact, WEATHERPROTONAME, "mwin", frameID);
	db_set_b(hContact, "CList", "Hidden", TRUE);
}

void removeWindow(MCONTACT hContact)
{
	DWORD frameId = db_get_dw(hContact, WEATHERPROTONAME, "mwin", 0);

	WindowList_Remove(hMwinWindowList, WindowList_Find(hMwinWindowList, hContact));
	CallService(MS_CLIST_FRAMES_REMOVEFRAME, frameId, 0);

	db_set_dw(hContact, WEATHERPROTONAME, "mwin", 0);
	db_unset(hContact, "CList", "Hidden");
}

void UpdateMwinData(MCONTACT hContact)
{
	HWND hwnd = WindowList_Find(hMwinWindowList, hContact);
	if (hwnd != NULL)
		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}


INT_PTR Mwin_MenuClicked(WPARAM wParam,LPARAM lParam)
{
	BOOL addwnd = WindowList_Find(hMwinWindowList, wParam) == NULL;
	if (addwnd)
		addWindow(wParam);
	else
		removeWindow(wParam);
	return 0;
}


int BuildContactMenu(WPARAM wparam,LPARAM lparam)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_FLAGS |
		(db_get_dw((MCONTACT)wparam, WEATHERPROTONAME, "mwin", 0) ? CMIF_CHECKED : 0);
	Menu_ModifyItem(hMwinMenu, &mi);
	return 0;
}


int RedrawFrame(WPARAM wParam, LPARAM lParam)
{
	WindowList_Broadcast(hMwinWindowList, WM_REDRAWWIN, 0, 0);
	return 0;
}


void InitMwin(void)
{
	if ( !ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
		return;

	hMwinWindowList = WindowList_Create();

	WNDCLASS wndclass;
	wndclass.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = wndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInst;
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = 0; //(HBRUSH)(COLOR_3DFACE+1);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = _T("WeatherFrame");
	RegisterClass(&wndclass);

	ColourIDT colourid = {0};
	colourid.cbSize = sizeof(ColourIDT);
	mir_strcpy(colourid.dbSettingsGroup, WEATHERPROTONAME);
	mir_strcpy(colourid.setting, "ColorMwinFrame");
	mir_tstrcpy(colourid.name, LPGENT("Frame Background"));
	mir_tstrcpy(colourid.group, _T(WEATHERPROTONAME));
	colourid.defcolour = GetSysColor(COLOR_3DFACE);
	ColourRegisterT(&colourid);

	FontIDT fontid = {0};
	fontid.cbSize = sizeof(FontIDT);
	fontid.flags = FIDF_ALLOWREREGISTER | FIDF_DEFAULTVALID;
	mir_strcpy(fontid.dbSettingsGroup, WEATHERPROTONAME);
	mir_tstrcpy(fontid.group, _T(WEATHERPROTONAME));
	mir_tstrcpy(fontid.name, LPGENT("Frame Font"));
	mir_strcpy(fontid.prefix, "fnt0");

	HDC hdc = GetDC(NULL);
	fontid.deffontsettings.size = -13;
	ReleaseDC(0, hdc);

	fontid.deffontsettings.charset = DEFAULT_CHARSET;
	mir_tstrcpy(fontid.deffontsettings.szFace, _T("Verdana"));
	mir_tstrcpy(fontid.backgroundGroup, _T(WEATHERPROTONAME));
	mir_tstrcpy(fontid.backgroundName, LPGENT("Frame Background"));
	FontRegisterT(&fontid);

	fontid.deffontsettings.style = DBFONTF_BOLD;
	mir_tstrcpy(fontid.name, LPGENT("Frame Title Font"));
	mir_strcpy(fontid.prefix, "fnt1");
	FontRegisterT(&fontid);

	for (MCONTACT hContact = db_find_first(WEATHERPROTONAME); hContact; hContact = db_find_next(hContact, WEATHERPROTONAME))
		if (db_get_dw(hContact, WEATHERPROTONAME, "mwin", 0))
			addWindow(hContact);

	hFontHook = HookEvent(ME_FONT_RELOAD, RedrawFrame);
}

void DestroyMwin(void)
{
	for (MCONTACT hContact = db_find_first(WEATHERPROTONAME); hContact; hContact = db_find_next(hContact, WEATHERPROTONAME)) {
		DWORD frameId = db_get_dw(hContact, WEATHERPROTONAME, "mwin", 0);
		if (frameId)
			CallService(MS_CLIST_FRAMES_REMOVEFRAME, frameId, 0);
	}
	UnregisterClass( _T("WeatherFrame"), hInst);
	WindowList_Destroy(hMwinWindowList);
	UnhookEvent(hFontHook);
}
