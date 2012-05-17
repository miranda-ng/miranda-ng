/*
Weather Protocol plugin for Miranda IM
Copyright (C) 2006-2009 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2002-2006 Calvin Che

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
#include "m_acc.h"

/* DRAWGLYPH Request structure */
typedef struct s_SKINDRAWREQUEST
{
	char szObjectID[255];      // Unic Object ID (path) to paint
	RECT rcDestRect;           // Rectangle to fit
	RECT rcClipRect;           // Rectangle to paint in.
	HDC hDC;                   // Handler to device context to paint in. 
} SKINDRAWREQUEST,*LPSKINDRAWREQUEST;


// Request painting glyph object 
// wParam = pointer to SKINDRAWREQUEST structure
// lParam = 0
#define MS_SKIN_DRAWGLYPH "ModernList/DrawGlyph"


#define MS_TOOLTIP_SHOWTIP		"mToolTip/ShowTip"
#define MS_TOOLTIP_HIDETIP		"mToolTip/HideTip"

typedef BOOL (WINAPI *ft_TrackMouseEvent) (LPTRACKMOUSEEVENT lpEventTrack);

static ft_TrackMouseEvent f_TrackMouseEvent = NULL;
static HANDLE hMwinWindowList;
static HANDLE hFontHook;

HANDLE hMwinMenu;

typedef struct
{
	HANDLE hContact;
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

		data->hContact = (HANDLE)((LPCREATESTRUCT)lParam)->lpCreateParams;
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
			HMENU hMenu;

			hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)data->hContact, 0);
			GetCursorPos(&pt);
			TrackPopupMenu(hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
			DestroyMenu(hMenu);
		}
		break;

	case WM_MOUSEMOVE:
		if (f_TrackMouseEvent)
		{
			TRACKMOUSEEVENT tme = {0};
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.hwndTrack = hwnd;
			tme.dwFlags = TME_QUERY;
			f_TrackMouseEvent(&tme);

			if (tme.dwFlags == 0)
			{
				tme.dwFlags = TME_HOVER | TME_LEAVE;
				tme.hwndTrack = hwnd;
				tme.dwHoverTime = CallService(MS_CLC_GETINFOTIPHOVERTIME, 0, 0);
				f_TrackMouseEvent(&tme);
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
			ti.hItem = data->hContact;
			ti.ptCursor = pt;
			ti.isTreeFocused = 1;
			CallService(MS_TOOLTIP_SHOWTIP, 0, (LPARAM)&ti);
		}
		break;

	case WM_LBUTTONDBLCLK:
		BriefInfo((WPARAM)data->hContact, 0);
		break;

	case WM_COMMAND:	 //Needed by the contact's context menu
		if (CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam),MPCF_CONTACTMENU), (LPARAM)data->hContact))
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

			if(GetUpdateRect(hwnd, &r, FALSE)) 
			{
				DBVARIANT dbv = {0};
				PAINTSTRUCT ps;
				HDC hdc;
				LOGFONT lfnt, lfnt1;
				COLORREF fntc, fntc1;
				COLORREF clr;
				int picSize = opt.AvatarSize;
				HICON hIcon = NULL;

				if (!data->haveAvatar)
				{
					int statusIcon = DBGetContactSettingWord(data->hContact, WEATHERPROTONAME, "Status", 0);

					picSize = GetSystemMetrics(SM_CXICON);
					hIcon = LoadSkinnedProtoIconBig(WEATHERPROTONAME, statusIcon);
					if ((INT_PTR)hIcon == CALLSERVICE_NOTFOUND) 
					{
						picSize = GetSystemMetrics(SM_CXSMICON);
						hIcon = LoadSkinnedProtoIcon(WEATHERPROTONAME, statusIcon);
					}
				}

				clr = DBGetContactSettingDword(NULL, WEATHERPROTONAME, "ColorMwinFrame", GetSysColor(COLOR_3DFACE));

				{
					FontID fntid = {0};
					strcpy(fntid.group, WEATHERPROTONAME);
					strcpy(fntid.name, "Frame Font");
					fntc = CallService(MS_FONT_GET, (WPARAM)&fntid, (LPARAM)&lfnt);

					strcpy(fntid.name, "Frame Title Font");
					fntc1 = CallService(MS_FONT_GET, (WPARAM)&fntid, (LPARAM)&lfnt1);
				}

				DBGetContactSettingString(data->hContact, WEATHERCONDITION, "WeatherInfo", &dbv);

				GetClientRect(hwnd, &rc);

				hdc = BeginPaint(hwnd, &ps);

				if (ServiceExists(MS_SKIN_DRAWGLYPH))
				{
					SKINDRAWREQUEST rq;
					memset(&rq, 0, sizeof(rq));
					rq.hDC = hdc;
					rq.rcDestRect = rc;
					rq.rcClipRect = rc;

					strcpy(rq.szObjectID, "Main,ID=WeatherFrame");
					CallService(MS_SKIN_DRAWGLYPH, (WPARAM)&rq, 0);
				}

				if (clr != 0xFFFFFFFF)
				{
					HBRUSH hBkgBrush = CreateSolidBrush(clr);
					FillRect(hdc, &rc, hBkgBrush);
					DeleteObject(hBkgBrush);
				}

				if (!data->haveAvatar)
					DrawIconEx(hdc, 1, 1, hIcon, 0, 0, 0, NULL, DI_NORMAL);

				SetBkMode(hdc, TRANSPARENT);

				{
					HFONT hfnt = CreateFontIndirect(&lfnt1);
					HFONT hfntold = SelectObject(hdc, hfnt);
					SIZE fontSize;

					char *nick = (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)data->hContact, 0);

					GetTextExtentPoint32(hdc, _T("|"), 1, &fontSize);

					rc.top += 1;
					rc.left += picSize + fontSize.cx;

					SetTextColor(hdc, fntc1);
					DrawText(hdc, nick, -1, &rc, DT_LEFT | DT_EXPANDTABS);

					rc.top += fontSize.cy;

					SelectObject(hdc, hfntold);
					DeleteObject(hfnt);
				}

				if (dbv.pszVal) 
				{
					HFONT hfnt = CreateFontIndirect(&lfnt);
					HFONT hfntold = SelectObject(hdc, hfnt);

					SetTextColor(hdc, fntc);
					DrawText(hdc, dbv.pszVal, -1, &rc, DT_LEFT | DT_EXPANDTABS);

					SelectObject(hdc, hfntold);
					DeleteObject(hfnt);
				}
				EndPaint(hwnd, &ps);
				CallService(MS_SKIN2_RELEASEICON, (WPARAM)hIcon, 0);
				DBFreeVariant(&dbv);
			}
			break;
		}

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return(TRUE);
}

static void addWindow(HANDLE hContact) 
{
	CLISTFrame Frame = {0};
	HWND hWnd;
	DBVARIANT dbv;
	char winname[512];
	DWORD frameID;

	DBGetContactSettingString(hContact, WEATHERPROTONAME, "Nick", &dbv);
	mir_snprintf(winname, sizeof(winname), "Weather: %s", dbv.pszVal);
	DBFreeVariant(&dbv);

	hWnd = CreateWindow("WeatherFrame", "", WS_CHILD | WS_VISIBLE, 
		0, 0, 10, 10, (HWND)CallService(MS_CLUI_GETHWND, 0, 0), NULL, hInst, hContact);

	Frame.name = winname;
	Frame.cbSize = sizeof(Frame);
	Frame.hWnd = hWnd;
	Frame.align = alBottom;
	Frame.Flags = F_VISIBLE|F_NOBORDER;
	Frame.height = 32;

	WindowList_Add(hMwinWindowList, hWnd, hContact);

	frameID = CallService(MS_CLIST_FRAMES_ADDFRAME, (WPARAM)&Frame, 0);
	DBWriteContactSettingDword(hContact, WEATHERPROTONAME, "mwin", frameID);
	DBWriteContactSettingByte(hContact, "CList", "Hidden", TRUE);
}

void removeWindow(HANDLE hContact) 
{
	DWORD frameId = DBGetContactSettingDword(hContact, WEATHERPROTONAME, "mwin", 0);

	WindowList_Remove(hMwinWindowList, WindowList_Find(hMwinWindowList, hContact));
	CallService(MS_CLIST_FRAMES_REMOVEFRAME, frameId, 0);

	DBWriteContactSettingDword(hContact, WEATHERPROTONAME, "mwin", 0);
	DBDeleteContactSetting(hContact, "CList", "Hidden");
}

void UpdateMwinData(HANDLE hContact) 
{
	HWND hwnd = WindowList_Find(hMwinWindowList, hContact);
	if (hwnd != NULL)
		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}


INT_PTR Mwin_MenuClicked(WPARAM wParam,LPARAM lParam) 
{
	BOOL addwnd = WindowList_Find(hMwinWindowList, (HANDLE)wParam) == NULL;
	if (addwnd)
		addWindow((HANDLE)wParam);
	else
		removeWindow((HANDLE)wParam);
	return 0;
}


int BuildContactMenu(WPARAM wparam,LPARAM lparam) 
{
	CLISTMENUITEM mi = {0};

	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | 
		(DBGetContactSettingDword((HANDLE)wparam, WEATHERPROTONAME, "mwin", 0) ? CMIF_CHECKED : 0);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMwinMenu, (LPARAM)&mi);
	return 0;
}


int RedrawFrame(WPARAM wParam, LPARAM lParam)
{
	WindowList_Broadcast(hMwinWindowList, WM_REDRAWWIN, 0, 0);
	return 0;
}


void InitMwin(void) 
{
	HANDLE hContact;
	HMODULE hUser = GetModuleHandle("user32.dll");

	if (!ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) return;

	f_TrackMouseEvent = (ft_TrackMouseEvent)GetProcAddress(hUser, "TrackMouseEvent");


	hMwinWindowList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST,0,0);

	{
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
		wndclass.lpszClassName = "WeatherFrame";
		RegisterClass(&wndclass);
	}

	{
		FontID fontid = {0};
		ColourID colourid = {0};
		HDC hdc;

		colourid.cbSize = sizeof(ColourID);
		strcpy(colourid.dbSettingsGroup, WEATHERPROTONAME);
		strcpy(colourid.setting, "ColorMwinFrame");
		strcpy(colourid.name, "Frame Background");
		strcpy(colourid.group, WEATHERPROTONAME);
		colourid.defcolour = GetSysColor(COLOR_3DFACE);

		CallService(MS_COLOUR_REGISTER, (WPARAM)&colourid, 0);

		fontid.cbSize = sizeof(FontID);
		fontid.flags = FIDF_ALLOWREREGISTER | FIDF_DEFAULTVALID;
		strcpy(fontid.dbSettingsGroup, WEATHERPROTONAME);
		strcpy(fontid.group, WEATHERPROTONAME);
		strcpy(fontid.name, "Frame Font");
		strcpy(fontid.prefix, "fnt0");

		hdc = GetDC(NULL);
		fontid.deffontsettings.size = -MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72);;
		ReleaseDC(0, hdc);

		fontid.deffontsettings.charset = DEFAULT_CHARSET;
		strcpy(fontid.deffontsettings.szFace, "Tahoma");
		strcpy(fontid.backgroundGroup, WEATHERPROTONAME);
		strcpy(fontid.backgroundName, "Frame Background");

		CallService(MS_FONT_REGISTER, (WPARAM)&fontid, 0);

		fontid.deffontsettings.style = DBFONTF_BOLD;
		strcpy(fontid.name, "Frame Title Font");
		strcpy(fontid.prefix, "fnt1");

		CallService(MS_FONT_REGISTER, (WPARAM)&fontid, 0);
	}

	hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while(hContact) 
	{
		// see if the contact is a weather contact
		if(IsMyContact(hContact)) 
		{
			if (DBGetContactSettingDword(hContact, WEATHERPROTONAME, "mwin", 0))
				addWindow(hContact);
		}
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
	hFontHook = HookEvent(ME_FONT_RELOAD, RedrawFrame);
}

void DestroyMwin(void)
{
	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while(hContact) 
	{
		// see if the contact is a weather contact
		if(IsMyContact(hContact)) 
		{
			DWORD frameId = DBGetContactSettingDword(hContact, WEATHERPROTONAME, "mwin", 0);
			if (frameId)
				CallService(MS_CLIST_FRAMES_REMOVEFRAME, frameId, 0);
		}
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
	UnregisterClass("WeatherFrame", hInst);
	UnhookEvent(hFontHook);
}
