/*
Copyright (C) 2005 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "commons.h"

// Prototypes /////////////////////////////////////////////////////////////////////////////////////

#define WINDOW_NAME_PREFIX		"mydetails_window"
#define WINDOW_CLASS_NAME		_T("MyDetailsFrame")
#define CONTAINER_CLASS_NAME	_T("MyDetailsFrameContainer")

#define ID_FRAME_TIMER			1011
#define ID_RECALC_TIMER			1012
#define ID_STATUSMESSAGE_TIMER	1013

#define RECALC_TIME				1000

#define IDC_HAND				MAKEINTRESOURCE(32649)

#define DEFAULT_NICKNAME		_T("<no nickname>")
#define DEFAULT_STATUS_MESSAGE	_T("<no status message>")
#define DEFAULT_LISTENING_TO	_T("<nothing playing>")


// Messages
#define MWM_REFRESH				(WM_USER + 10)
#define MWM_NICK_CHANGED		(WM_USER + 11)
#define MWM_STATUS_CHANGED		(WM_USER + 12)
#define MWM_STATUS_MSG_CHANGED	(WM_USER + 13)
#define MWM_AVATAR_CHANGED		(WM_USER + 14)
#define MWM_LISTENINGTO_CHANGED	(WM_USER + 15)


HWND hwnd_frame = NULL;
HWND hwnd_container = NULL;

int frame_id = -1;
bool g_bFramesExist = false;

HGENMENU hMenuShowHideFrame = 0;

#define FONT_NICK 0
#define FONT_PROTO 1
#define FONT_STATUS 2
#define FONT_AWAY_MSG 3
#define FONT_LISTENING_TO 4
#define NUM_FONTS 5

FontIDT font_id[NUM_FONTS];
HFONT hFont[NUM_FONTS];
COLORREF font_colour[NUM_FONTS];

// Defaults
char *font_settings[] = { "NicknameFont", "AccountFont", "StatusFont", "StatusMessageFont", "ListeningToFont" };
TCHAR *font_names[] = { LPGENT("Nickname"), LPGENT("Account"), LPGENT("Status"), LPGENT("Status message"), LPGENT("Listening to") };
char font_sizes[] = { 13, 8, 8, 8, 8 };
BYTE font_styles[] = { DBFONTF_BOLD, 0, 0, DBFONTF_ITALIC, DBFONTF_ITALIC };
COLORREF font_colors[] = { RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(150, 150, 150), RGB(150, 150, 150) };

static ColourID
	bg_colour = { sizeof(bg_colour), LPGEN("My details"), LPGEN("Background"), MODULE_NAME, "BackgroundColor", 0, GetSysColor(COLOR_BTNFACE) },
	av_colour = { sizeof(av_colour), LPGEN("My details"), LPGEN("Avatar border"), MODULE_NAME, "AvatarBorderColor", 0, RGB(0, 0, 0) };

int CreateFrame();
void FixMainMenu();
void RefreshFrame();
void RedrawFrame();


// used when no multiwindow functionality available
bool MyDetailsFrameVisible();
void SetMyDetailsFrameVisible(bool visible);
INT_PTR ShowHideMenuFunc(WPARAM wParam, LPARAM lParam);
INT_PTR ShowFrameFunc(WPARAM wParam, LPARAM lParam);
INT_PTR HideFrameFunc(WPARAM wParam, LPARAM lParam);
INT_PTR ShowHideFrameFunc(WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK FrameContainerWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK FrameWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void SetCycleTime();
void SetCycleTime(HWND hwnd);
void SetStatusMessageRefreshTime();
void SetStatusMessageRefreshTime(HWND hwnd);
int SettingsChangedHook(WPARAM wParam, LPARAM lParam);
int AvatarChangedHook(WPARAM wParam, LPARAM lParam);
int ProtoAckHook(WPARAM wParam, LPARAM lParam);
int SmileyAddOptionsChangedHook(WPARAM wParam, LPARAM lParam);
int ListeningtoEnableStateChangedHook(WPARAM wParam, LPARAM lParam);


#define OUTSIDE_BORDER 6
#define SPACE_IMG_TEXT 6
#define SPACE_TEXT_TEXT 0
#define SPACE_ICON_TEXT 2
#define ICON_SIZE 16

#define BORDER_SPACE 2

struct MyDetailsFrameData
{
	RECT proto_rect;
	bool draw_proto;
	bool mouse_over_proto;

	bool draw_proto_cycle;
	RECT next_proto_rect;
	HWND next_proto_tt_hwnd;
	RECT prev_proto_rect;
	HWND prev_proto_tt_hwnd;

	RECT img_rect;
	bool draw_img;
	bool mouse_over_img;

	RECT nick_rect;
	bool draw_nick;
	bool mouse_over_nick;
	HWND nick_tt_hwnd;

	RECT status_rect;
	RECT status_icon_rect;
	RECT status_text_rect;
	bool draw_status;
	bool mouse_over_status;
	HWND status_tt_hwnd;

	RECT away_msg_rect;
	bool draw_away_msg;
	bool mouse_over_away_msg;
	HWND away_msg_tt_hwnd;

	RECT listening_to_rect;
	RECT listening_to_icon_rect;
	RECT listening_to_text_rect;
	bool draw_listening_to;
	bool mouse_over_listening_to;
	HWND listening_to_tt_hwnd;

	int protocol_number;

	bool showing_menu;
	bool recalc_rectangles;

	bool get_status_messages;
};


// Functions //////////////////////////////////////////////////////////////////////////////////////

void InitFrames()
{
	InitContactListSmileys();

	CreateFrame();

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingsChangedHook);
	HookEvent(ME_AV_MYAVATARCHANGED, AvatarChangedHook);
	HookEvent(ME_PROTO_ACK, ProtoAckHook);
	HookEvent(ME_SMILEYADD_OPTIONSCHANGED, SmileyAddOptionsChangedHook);
	HookEvent(ME_LISTENINGTO_ENABLE_STATE_CHANGED, ListeningtoEnableStateChangedHook);
}

void DeInitFrames()
{
	if (g_bFramesExist && frame_id != -1)
		CallService(MS_CLIST_FRAMES_REMOVEFRAME, (WPARAM)frame_id, 0);

	for (int i = 0; i < NUM_FONTS; i++)
		if (hFont[i] != 0)
			DeleteObject(hFont[i]);

	if (hwnd_frame != NULL)
		DestroyWindow(hwnd_frame);
	if (hwnd_container != NULL)
		DestroyWindow(hwnd_container);
}

int ReloadFont(WPARAM wParam, LPARAM lParam)
{
	for (int i = 0; i < NUM_FONTS; i++) {
		if (hFont[i] != 0)
			DeleteObject(hFont[i]);

		LOGFONT log_font;
		font_colour[i] = CallService(MS_FONT_GETT, (WPARAM)&font_id[i], (LPARAM)&log_font);
		hFont[i] = CreateFontIndirect(&log_font);
	}

	RefreshFrame();
	return 0;
}

int ReloadColour(WPARAM, LPARAM)
{
	opts.bkg_color = (COLORREF)CallService(MS_COLOUR_GET, (WPARAM)&bg_colour, 0);
	opts.draw_avatar_border_color = (COLORREF)CallService(MS_COLOUR_GET, (WPARAM)&av_colour, 0);

	RefreshFrame();
	return 0;
}

int SmileyAddOptionsChangedHook(WPARAM wParam, LPARAM lParam)
{
	RefreshFrame();
	return 0;
}

int CreateFrame()
{
	HDC hdc = GetDC(NULL);

	ColourRegister(&bg_colour);
	ColourRegister(&av_colour);
	ReloadColour(0, 0);
	HookEvent(ME_COLOUR_RELOAD, ReloadColour);

	for (int i = 0; i < NUM_FONTS; i++) {
		ZeroMemory(&font_id[i], sizeof(font_id[i]));

		font_id[i].cbSize = sizeof(FontIDT);
		_tcsncpy(font_id[i].group, LPGENT("My details"), SIZEOF(font_id[i].group));
		_tcsncpy(font_id[i].name, font_names[i], SIZEOF(font_id[i].name));
		strncpy(font_id[i].dbSettingsGroup, MODULE_NAME, SIZEOF(font_id[i].dbSettingsGroup));
		_tcsncpy(font_id[i].backgroundName, LPGENT("Background"), SIZEOF(font_id[i].backgroundName));
		_tcsncpy(font_id[i].backgroundGroup, LPGENT("My details"), SIZEOF(font_id[i].backgroundGroup));

		strncpy(font_id[i].prefix, font_settings[i], SIZEOF(font_id[i].prefix));

		font_id[i].deffontsettings.colour = font_colors[i];
		font_id[i].deffontsettings.size = -MulDiv(font_sizes[i], GetDeviceCaps(hdc, LOGPIXELSY), 72);
		font_id[i].deffontsettings.style = font_styles[i];
		font_id[i].deffontsettings.charset = DEFAULT_CHARSET;
		_tcsncpy(font_id[i].deffontsettings.szFace, _T("Tahoma"), SIZEOF(font_id[i].deffontsettings.szFace));
		font_id[i].order = i;
		font_id[i].flags = FIDF_DEFAULTVALID;
		FontRegisterT(&font_id[i]);
	}

	ReleaseDC(NULL, hdc);

	ReloadFont(0, 0);
	HookEvent(ME_FONT_RELOAD, ReloadFont);

	WNDCLASS wndclass;
	wndclass.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW; //CS_PARENTDC | CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = FrameWindowProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInst;
	wndclass.hIcon = NULL;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = 0; //(HBRUSH)(COLOR_3DFACE+1);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = WINDOW_CLASS_NAME;
	RegisterClass(&wndclass);

	if (g_bFramesExist) {
		hwnd_frame = CreateWindow(WINDOW_CLASS_NAME, TranslateT("My details"), WS_CHILD | WS_VISIBLE,
			0, 0, 10, 10, (HWND)CallService(MS_CLUI_GETHWND, 0, 0), NULL, hInst, NULL);

		CLISTFrame Frame = {0};

		Frame.cbSize = sizeof(Frame);
		Frame.tname = TranslateT("My details");
		Frame.cbSize = sizeof(CLISTFrame);
		Frame.hWnd = hwnd_frame;
		Frame.align = alTop;
		Frame.hIcon = LoadSkinnedIcon(SKINICON_OTHER_FRAME);
		Frame.Flags = F_VISIBLE | F_SHOWTB | F_SHOWTBTIP | F_NOBORDER | F_SKINNED | F_TCHAR;
		Frame.height = 100;
		frame_id = CallService(MS_CLIST_FRAMES_ADDFRAME, (WPARAM)&Frame, 0);

		if (db_get_b(NULL, "MyDetails", "ForceHideFrame", 0)) {
			int flags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frame_id), 0);
			if (flags & F_VISIBLE)
				CallService(MS_CLIST_FRAMES_SHFRAME, frame_id, 0);

			db_unset(NULL, "MyDetails", "ForceHideFrame");
		}

		if (db_get_b(NULL, "MyDetails", "ForceShowFrame", 0)) {
			int flags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frame_id), 0);
			if (!(flags & F_VISIBLE))
				CallService(MS_CLIST_FRAMES_SHFRAME, frame_id, 0);

			db_unset(NULL, "MyDetails", "ForceShowFrame");
		}
	}
	else {
		wndclass.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;//CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = FrameContainerWindowProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = hInst;
		wndclass.hIcon = NULL;
		wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndclass.hbrBackground = 0; //(HBRUSH)(COLOR_3DFACE+1);
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = CONTAINER_CLASS_NAME;
		RegisterClass(&wndclass);

		hwnd_container = CreateWindowEx(WS_EX_TOOLWINDOW, CONTAINER_CLASS_NAME, TranslateT("My details"),
			(WS_THICKFRAME | WS_CAPTION | WS_SYSMENU) & ~WS_VISIBLE,
			0, 0, 200, 130, (HWND)CallService(MS_CLUI_GETHWND, 0, 0), NULL, hInst, NULL);

		hwnd_frame = CreateWindow(WINDOW_CLASS_NAME, TranslateT("My details"), WS_CHILD | WS_VISIBLE,
			0, 0, 10, 10, hwnd_container, NULL, hInst, NULL);

		SetWindowLongPtr(hwnd_container, GWLP_USERDATA, (LONG_PTR)hwnd_frame);
		SendMessage(hwnd_container, WM_SIZE, 0, 0);

		// Create menu item

		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags = CMIF_TCHAR;
		mi.popupPosition = -0x7FFFFFFF;
		mi.ptszPopupName = LPGENT("My details");
		mi.position = 1; // 500010000
		mi.hIcon = LoadSkinnedIcon(SKINICON_OTHER_USERDETAILS);
		mi.ptszName = LPGENT("Show my details");
		mi.pszService = MODULE_NAME"/ShowHideMyDetails";
		hMenuShowHideFrame = Menu_AddMainMenuItem(&mi);

		if (db_get_b(0, MODULE_NAME, SETTING_FRAME_VISIBLE, 1) == 1) {
			ShowWindow(hwnd_container, SW_SHOW);
			FixMainMenu();
		}
	}

	CreateServiceFunction(MS_MYDETAILS_SHOWFRAME, ShowFrameFunc);
	CreateServiceFunction(MS_MYDETAILS_HIDEFRAME, HideFrameFunc);
	CreateServiceFunction(MS_MYDETAILS_SHOWHIDEFRAME, ShowHideFrameFunc);
	return 0;
}

bool FrameIsFloating()
{
	if (frame_id == -1)
		return true; // no frames, always floating

	return (CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLOATING, frame_id), 0) != 0);
}

LRESULT CALLBACK FrameContainerWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_SHOWWINDOW:
		if ((BOOL)wParam)
			Utils_RestoreWindowPosition(hwnd, 0, MODULE_NAME, WINDOW_NAME_PREFIX);
		else
			Utils_SaveWindowPosition(hwnd, 0, MODULE_NAME, WINDOW_NAME_PREFIX);
		break;

	case WM_ERASEBKGND:
		SendMessage((HWND)GetWindowLongPtr(hwnd, GWLP_USERDATA), WM_ERASEBKGND, wParam, lParam);
		break;

	case WM_SIZE:
		{
			HWND child = (HWND)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			RECT r;
			GetClientRect(hwnd, &r);

			SetWindowPos(child, 0, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOACTIVATE);
			InvalidateRect(child, NULL, TRUE);
		}
		return TRUE;

	case WM_CLOSE:
		db_set_b(0, MODULE_NAME, SETTING_FRAME_VISIBLE, 0);
		ShowWindow(hwnd, SW_HIDE);
		FixMainMenu();
		return TRUE;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

BOOL ScreenToClient(HWND hWnd, LPRECT lpRect)
{
	POINT pt;
	pt.x = lpRect->left;
	pt.y = lpRect->top;

	BOOL ret = ScreenToClient(hWnd, &pt);
	if (!ret)
		return ret;

	lpRect->left = pt.x;
	lpRect->top = pt.y;

	pt.x = lpRect->right;
	pt.y = lpRect->bottom;

	ret = ScreenToClient(hWnd, &pt);

	lpRect->right = pt.x;
	lpRect->bottom = pt.y;

	return ret;
}

BOOL MoveWindow(HWND hWnd, const RECT &rect, BOOL bRepaint)
{
	return MoveWindow(hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, bRepaint);
}

RECT GetInnerRect(const RECT &rc, const RECT &clipping)
{
	RECT rc_ret = rc;
	rc_ret.left = max(rc.left, clipping.left);
	rc_ret.top = max(rc.top, clipping.top);
	rc_ret.right = min(rc.right, clipping.right);
	rc_ret.bottom = min(rc.bottom, clipping.bottom);

	return rc_ret;
}

RECT GetRect(HDC hdc, RECT rc, SIZE s, UINT uFormat, int next_top, int text_left, bool frame = true, bool end_elipsis_on_frame = true)
{
	RECT r = rc;

	if (frame && end_elipsis_on_frame) {
		// Add space to ...
		uFormat &= ~DT_END_ELLIPSIS;

		RECT rc_tmp = rc;
		DrawText(hdc, _T(" ..."), 4, &rc_tmp, DT_CALCRECT | uFormat);

		s.cx += rc_tmp.right - rc_tmp.left;
	}

	r.top = next_top;
	r.bottom = r.top + s.cy;

	if (opts.draw_text_align_right)
		r.left = r.right - s.cx;
	else {
		r.left = text_left;
		r.right = r.left + s.cx;
	}

	if (frame) {
		r.bottom += 2 * BORDER_SPACE;

		if (opts.draw_text_align_right)
			r.left -= 2 * BORDER_SPACE;
		else
			r.right += 2 * BORDER_SPACE;
	}

	// Make it fit inside original rc
	r.top = max(next_top, r.top);
 	r.bottom = min(rc.bottom, r.bottom);
	r.left = max(text_left, r.left);
	r.right = min(rc.right, r.right);

	return r;
}

RECT GetRect(HDC hdc, RECT rc, const TCHAR *text, const TCHAR *def_text, Protocol *proto, UINT uFormat,
	int next_top, int text_left, bool smileys = true, bool frame = true, bool end_elipsis_on_frame = true)
{
	const TCHAR *tmp;

	if (text[0] == '\0')
		tmp = TranslateTS(def_text);
	else
		tmp = text;

	uFormat &= ~DT_END_ELLIPSIS;

	SIZE s;
	RECT r_tmp = rc;

	// Only first line
	TCHAR *tmp2 = _tcsdup(tmp);
	TCHAR *pos = _tcschr(tmp2, '\r');
	if (pos != NULL)
		pos[0] = '\0';
	pos = _tcschr(tmp2, '\n');
	if (pos != NULL)
		pos[0] = '\0';

	if (smileys)
		DRAW_TEXT(hdc, tmp2, (int)_tcslen(tmp2), &r_tmp, uFormat | DT_CALCRECT, proto->name);
	else
		DrawText(hdc, tmp2, (int)_tcslen(tmp2), &r_tmp, uFormat | DT_CALCRECT);

	free(tmp2);

	s.cx = r_tmp.right - r_tmp.left;
	s.cy = r_tmp.bottom - r_tmp.top;

	return GetRect(hdc, rc, s, uFormat, next_top, text_left, frame, end_elipsis_on_frame);
}

HWND CreateTooltip(HWND hwnd, RECT &rect)
{
	// struct specifying info about tool in ToolTip control
	unsigned int uid = 0;       // for ti initialization

	// struct specifying control classes to register
	INITCOMMONCONTROLSEX iccex;
	iccex.dwSize = sizeof(iccex);
	iccex.dwICC = ICC_BAR_CLASSES;
	if (!InitCommonControlsEx(&iccex))
		return NULL;

	/* CREATE A TOOLTIP WINDOW */
	HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, NULL, hInst, NULL);                 // handle to the ToolTip control

	/* INITIALIZE MEMBERS OF THE TOOLINFO STRUCTURE */
	TOOLINFO ti;
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = hwnd;
	ti.hinst = hInst;
	ti.uId = uid;
	ti.lpszText = LPSTR_TEXTCALLBACK;
	// ToolTip control will cover the whole window
	ti.rect.left = rect.left;
	ti.rect.top = rect.top;
	ti.rect.right = rect.right;
	ti.rect.bottom = rect.bottom;

	/* SEND AN ADDTOOL MESSAGE TO THE TOOLTIP CONTROL WINDOW */
	SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
	SendMessage(hwndTT, TTM_SETDELAYTIME, (WPARAM)(DWORD)TTDT_AUTOPOP, (LPARAM)MAKELONG(24 * 60 * 60 * 1000, 0));

	return hwndTT;
}

void DeleteTooltipWindows(MyDetailsFrameData *data)
{
	if (data->nick_tt_hwnd != NULL) {
		DestroyWindow(data->nick_tt_hwnd);
		data->nick_tt_hwnd = NULL;
	}

	if (data->status_tt_hwnd != NULL) {
		DestroyWindow(data->status_tt_hwnd);
		data->status_tt_hwnd = NULL;
	}

	if (data->next_proto_tt_hwnd != NULL) {
		DestroyWindow(data->next_proto_tt_hwnd);
		data->next_proto_tt_hwnd = NULL;
	}

	if (data->prev_proto_tt_hwnd != NULL) {
		DestroyWindow(data->prev_proto_tt_hwnd);
		data->prev_proto_tt_hwnd = NULL;
	}

	if (data->away_msg_tt_hwnd != NULL) {
		DestroyWindow(data->away_msg_tt_hwnd);
		data->away_msg_tt_hwnd = NULL;
	}

	if (data->listening_to_tt_hwnd != NULL) {
		DestroyWindow(data->listening_to_tt_hwnd);
		data->listening_to_tt_hwnd = NULL;
	}
}

void CalcRectangles(HWND hwnd)
{
	HDC hdc = GetDC(hwnd);
	HFONT hOldFont = (HFONT)GetCurrentObject(hdc, OBJ_FONT);
	MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (hdc == NULL || data == NULL)
		return;

	Protocol *proto = protocols->Get(data->protocol_number);
	if (proto == NULL)
		return;

	data->recalc_rectangles = false;
	proto->data_changed = false;

	data->draw_proto = false;
	data->draw_proto_cycle = false;
	data->draw_img = false;
	data->draw_nick = false;
	data->draw_status = false;
	data->draw_away_msg = false;
	data->draw_listening_to = false;

	DeleteTooltipWindows(data);

	if (g_bFramesExist && frame_id != -1) {
		int flags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frame_id), 0);
		if (flags & F_UNCOLLAPSED) {
			RECT rf;
			GetClientRect(hwnd, &rf);

			int size = 0;

			if (rf.bottom - rf.top != size) {
				if (FrameIsFloating()) {
					HWND parent = GetParent(hwnd);
					if (parent != NULL) {
						RECT rp_client, rp_window, r_window;
						GetClientRect(parent, &rp_client);
						GetWindowRect(parent, &rp_window);
						GetWindowRect(hwnd, &r_window);

						int diff = (rp_window.bottom - rp_window.top) - (rp_client.bottom - rp_client.top) + r_window.top - rp_window.top;
						SetWindowPos(parent, 0, 0, 0, rp_window.right - rp_window.left, size + diff, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
					}
				}
			}
			SelectObject(hdc, hOldFont);
			ReleaseDC(hwnd, hdc);
			return;
		}
	}

	RECT r;
	GetClientRect(hwnd, &r);

	if (opts.resize_frame)
		r.bottom = 0x7FFFFFFF;

	int next_top;
	int text_left;
	int avatar_bottom = 0;

	UINT uFormat = DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS
		| (opts.draw_text_align_right ? DT_RIGHT : DT_LEFT)
		| (opts.draw_text_rtl ? DT_RTLREADING : 0);

	// make some borders
	r.left += min(opts.borders[LEFT], r.right);
	r.right = max(r.right - opts.borders[RIGHT], r.left);
	r.top += min(opts.borders[TOP], r.bottom);
	r.bottom = max(r.bottom - opts.borders[BOTTOM], r.top);

	next_top = r.top;
	text_left = r.left;

	// Draw image?
	if (proto->CanGetAvatar()) {
		if (proto->avatar_bmp != NULL) {
			data->draw_img = true;

			BITMAP bmp;
			if (GetObject(proto->avatar_bmp, sizeof(bmp), &bmp)) {
				// make bounds
				RECT rc = r;

				LONG width;
				LONG height;

				if (opts.draw_avatar_custom_size) {
					rc.right = opts.draw_avatar_custom_size_pixels;

					width = opts.draw_avatar_custom_size_pixels;
					height = opts.draw_avatar_custom_size_pixels;
				}
				else if (opts.resize_frame) {
					rc.right = rc.left + (rc.right - rc.left) / 3;

					width = rc.right - rc.left;
					height = rc.bottom - rc.top;
				}
				else {
					rc.right = rc.left + min((rc.right - rc.left) / 3, rc.bottom - rc.top);

					width = rc.right - rc.left;
					height = rc.bottom - rc.top;
				}

				// Fit to image proportions
				if (!opts.draw_avatar_allow_to_grow) {
					if (width > bmp.bmWidth)
						width = bmp.bmWidth;

					if (height > bmp.bmHeight)
						height = bmp.bmHeight;
				}

				if (!opts.resize_frame && height * bmp.bmWidth / bmp.bmHeight <= width)
					width = height * bmp.bmWidth / bmp.bmHeight;
				else
					height = width * bmp.bmHeight / bmp.bmWidth;

				rc.right = rc.left + width;
				rc.bottom = rc.top + height;

				data->img_rect = rc;

				avatar_bottom = data->img_rect.bottom + SPACE_TEXT_TEXT;

				// Make space to nick
				text_left = data->img_rect.right + SPACE_IMG_TEXT;
			}
		}
	}

	// Always draw nick
	data->draw_nick = true;
	SelectObject(hdc, hFont[FONT_NICK]);

	data->nick_rect = GetRect(hdc, r, proto->nickname, DEFAULT_NICKNAME, proto, uFormat, next_top, text_left);

	if (proto->nickname[0] != '\0')
		data->nick_tt_hwnd = CreateTooltip(hwnd, data->nick_rect);

	next_top = data->nick_rect.bottom + SPACE_TEXT_TEXT;

	// Fits more?
	if (next_top > r.bottom)
		goto finish;

	if (next_top > avatar_bottom && opts.use_avatar_space_to_draw_text)
		text_left = r.left;

	// Protocol?
	if (opts.draw_show_protocol_name) {
		data->draw_proto = true;

		SelectObject(hdc, hFont[FONT_PROTO]);

		RECT tmp_r = r;
		int tmp_text_left = text_left;
		if (opts.show_protocol_cycle_button)
			tmp_r.right -= 2 * ICON_SIZE;

		data->proto_rect = GetRect(hdc, tmp_r, proto->description, _T(""), proto, uFormat, next_top, tmp_text_left, false, true, false);

		if (opts.show_protocol_cycle_button) {
			data->draw_proto_cycle= true;

			RECT prev = r;
			prev.top = next_top;
			prev.bottom = min(r.bottom, prev.top + ICON_SIZE);

			int diff = (data->proto_rect.bottom - data->proto_rect.top) - (prev.bottom - prev.top);
			if (diff < 0) {
				diff = -diff / 2;
				data->proto_rect.top += diff;
				data->proto_rect.bottom += diff;
			}
			else {
				diff = diff / 2;
				prev.top += diff;
				prev.bottom += diff;
			}

			prev.right -= ICON_SIZE;
			prev.left = prev.right - ICON_SIZE;

			RECT next = prev;
			next.left += ICON_SIZE;
			next.right += ICON_SIZE;

			prev.left = max(text_left, prev.left);
			prev.right = min(r.right, prev.right);
			next.left = max(text_left, next.left);
			next.right = min(r.right, next.right);

			data->prev_proto_rect = prev;
			data->next_proto_rect = next;

			data->next_proto_tt_hwnd = CreateTooltip(hwnd, data->next_proto_rect);
			data->prev_proto_tt_hwnd = CreateTooltip(hwnd, data->prev_proto_rect);


			next_top = max(data->next_proto_rect.bottom, data->proto_rect.bottom) + SPACE_TEXT_TEXT;
		}
		else
			next_top = data->proto_rect.bottom + SPACE_TEXT_TEXT;
	}

	// Fits more?
	if (next_top + 2 * BORDER_SPACE > r.bottom)
		goto finish;

	if (next_top > avatar_bottom && opts.use_avatar_space_to_draw_text)
		text_left = r.left;

	// Status data?
	{
		data->draw_status = true;

		SelectObject(hdc, hFont[FONT_STATUS]);

		// Text size
		RECT r_tmp = r;
		DrawText(hdc, proto->status_name, (int)_tcslen(proto->status_name), &r_tmp, DT_CALCRECT | (uFormat & ~DT_END_ELLIPSIS));

		SIZE s;
		s.cy = max(r_tmp.bottom - r_tmp.top, ICON_SIZE);
		s.cx = ICON_SIZE + SPACE_ICON_TEXT + r_tmp.right - r_tmp.left;

		// Status global rect
		data->status_rect = GetRect(hdc, r, s, uFormat, next_top, text_left, true, false);

		if (proto->status_name[0] != '\0')
			data->status_tt_hwnd = CreateTooltip(hwnd, data->status_rect);

		next_top = data->status_rect.bottom + SPACE_TEXT_TEXT;

		RECT rc_inner = data->status_rect;
		rc_inner.top += BORDER_SPACE;
		rc_inner.bottom -= BORDER_SPACE;
		rc_inner.left += BORDER_SPACE;
		rc_inner.right -= BORDER_SPACE;

		// Icon
		data->status_icon_rect = rc_inner;

		if (opts.draw_text_align_right || opts.draw_text_rtl)
			data->status_icon_rect.left = max(data->status_icon_rect.right - ICON_SIZE, rc_inner.left);
		else
			data->status_icon_rect.right = min(data->status_icon_rect.left + ICON_SIZE, rc_inner.right);

		if (r_tmp.bottom - r_tmp.top > ICON_SIZE) {
			data->status_icon_rect.top += (r_tmp.bottom - r_tmp.top - ICON_SIZE) / 2;
			data->status_icon_rect.bottom = data->status_icon_rect.top + ICON_SIZE;
		}

		// Text
		data->status_text_rect = GetInnerRect(rc_inner, r);

		if (opts.draw_text_align_right || opts.draw_text_rtl)
			data->status_text_rect.right = max(data->status_icon_rect.left - SPACE_ICON_TEXT, rc_inner.left);
		else
			data->status_text_rect.left = min(data->status_icon_rect.right + SPACE_ICON_TEXT, rc_inner.right);

		if (ICON_SIZE > r_tmp.bottom - r_tmp.top) {
			data->status_text_rect.top += (ICON_SIZE - (r_tmp.bottom - r_tmp.top)) / 2;
			data->status_text_rect.bottom = data->status_text_rect.top + r_tmp.bottom - r_tmp.top;
		}
	}

	// Fits more?
	if (next_top + 2 * BORDER_SPACE > r.bottom)
		goto finish;

	if (next_top > avatar_bottom && opts.use_avatar_space_to_draw_text)
		text_left = r.left;

	// Away msg?
	if (proto->CanGetStatusMsg()) {
		data->draw_away_msg = true;

		SelectObject(hdc, hFont[FONT_AWAY_MSG]);

		data->away_msg_rect = GetRect(hdc, r, proto->status_message, DEFAULT_STATUS_MESSAGE, proto, uFormat, next_top, text_left);

		if (proto->status_message[0] != '\0')
			data->away_msg_tt_hwnd = CreateTooltip(hwnd, data->away_msg_rect);

		next_top = data->away_msg_rect.bottom + SPACE_TEXT_TEXT;
	}

	// Fits more?
	if (next_top + 2 * BORDER_SPACE > r.bottom)
		goto finish;

	if (next_top > avatar_bottom && opts.use_avatar_space_to_draw_text)
		text_left = r.left;

	// Listening to
	if (proto->ListeningToEnabled() && proto->GetStatus() > ID_STATUS_OFFLINE) {
		data->draw_listening_to = true;

		if (proto->listening_to[0] == '\0') {
			SelectObject(hdc, hFont[FONT_LISTENING_TO]);

			data->listening_to_rect = GetRect(hdc, r, proto->listening_to, DEFAULT_LISTENING_TO, proto, uFormat, next_top, text_left);

			data->listening_to_text_rect = data->listening_to_rect;
			ZeroMemory(&data->listening_to_icon_rect, sizeof(data->listening_to_icon_rect));

			next_top = data->listening_to_rect.bottom + SPACE_TEXT_TEXT;
		}
		else {
			SelectObject(hdc, hFont[FONT_LISTENING_TO]);

			// Text size
			RECT r_tmp = r;
			DrawText(hdc, proto->listening_to, (int)_tcslen(proto->listening_to), &r_tmp, DT_CALCRECT | (uFormat & ~DT_END_ELLIPSIS));

			SIZE s;
			s.cy = max(r_tmp.bottom - r_tmp.top, ICON_SIZE);
			s.cx = ICON_SIZE + SPACE_ICON_TEXT + r_tmp.right - r_tmp.left;

			// listening to global rect
			data->listening_to_rect = GetRect(hdc, r, s, uFormat, next_top, text_left, true, false);

			data->listening_to_tt_hwnd = CreateTooltip(hwnd, data->listening_to_rect);

			next_top = data->listening_to_rect.bottom + SPACE_TEXT_TEXT;

			RECT rc_inner = data->listening_to_rect;
			rc_inner.top += BORDER_SPACE;
			rc_inner.bottom -= BORDER_SPACE;
			rc_inner.left += BORDER_SPACE;
			rc_inner.right -= BORDER_SPACE;

			// Icon
			data->listening_to_icon_rect = rc_inner;

			if (opts.draw_text_align_right || opts.draw_text_rtl)
				data->listening_to_icon_rect.left = max(data->listening_to_icon_rect.right - ICON_SIZE, rc_inner.left);
			else
				data->listening_to_icon_rect.right = min(data->listening_to_icon_rect.left + ICON_SIZE, rc_inner.right);

			if (r_tmp.bottom - r_tmp.top > ICON_SIZE) {
				data->listening_to_icon_rect.top += (r_tmp.bottom - r_tmp.top - ICON_SIZE) / 2;
				data->listening_to_icon_rect.bottom = data->listening_to_icon_rect.top + ICON_SIZE;
			}

			// Text
			data->listening_to_text_rect = GetInnerRect(rc_inner, r);

			if (opts.draw_text_align_right || opts.draw_text_rtl)
				data->listening_to_text_rect.right = max(data->listening_to_icon_rect.left - SPACE_ICON_TEXT, rc_inner.left);
			else
				data->listening_to_text_rect.left = min(data->listening_to_icon_rect.right + SPACE_ICON_TEXT, rc_inner.right);

			if (ICON_SIZE > r_tmp.bottom - r_tmp.top) {
				data->listening_to_text_rect.top += (ICON_SIZE - (r_tmp.bottom - r_tmp.top)) / 2;
				data->listening_to_text_rect.bottom = data->listening_to_text_rect.top + r_tmp.bottom - r_tmp.top;
			}
		}
	}

	r.bottom = max(next_top - SPACE_TEXT_TEXT, avatar_bottom);

	if (opts.resize_frame && g_bFramesExist && frame_id != -1) {
		RECT rf;
		GetClientRect(hwnd, &rf);

		int size = r.bottom + opts.borders[BOTTOM];

		if (rf.bottom - rf.top != size) {
			if (FrameIsFloating()) {
				HWND parent = GetParent(hwnd);

				if (parent != NULL) {
					RECT rp_client, rp_window, r_window;
					GetClientRect(parent, &rp_client);
					GetWindowRect(parent, &rp_window);
					GetWindowRect(hwnd, &r_window);

					int diff = (rp_window.bottom - rp_window.top) - (rp_client.bottom - rp_client.top) + r_window.top - rp_window.top;
					SetWindowPos(parent, 0, 0, 0, rp_window.right - rp_window.left, size + diff, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
				}
			}
			else if (IsWindowVisible(hwnd)) {
				int flags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frame_id), 0);
				if (flags & F_VISIBLE) {
					CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_HEIGHT, frame_id), (LPARAM)(size));
					CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)frame_id, (LPARAM)(FU_TBREDRAW | FU_FMREDRAW | FU_FMPOS));
				}
			}
		}
	}

finish:
	SelectObject(hdc, hOldFont);
	ReleaseDC(hwnd, hdc);
}

HBITMAP CreateBitmap32(int cx, int cy)
{
	BITMAPINFO RGB32BitsBITMAPINFO;
	UINT *ptPixels;

	ZeroMemory(&RGB32BitsBITMAPINFO, sizeof(BITMAPINFO));
	RGB32BitsBITMAPINFO.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	RGB32BitsBITMAPINFO.bmiHeader.biWidth = cx;//bm.bmWidth;
	RGB32BitsBITMAPINFO.bmiHeader.biHeight = cy;//bm.bmHeight;
	RGB32BitsBITMAPINFO.bmiHeader.biPlanes = 1;
	RGB32BitsBITMAPINFO.bmiHeader.biBitCount = 32;

	HBITMAP DirectBitmap = CreateDIBSection(NULL,
		&RGB32BitsBITMAPINFO,
		DIB_RGB_COLORS,
		(void **)&ptPixels,
		NULL, 0);
	return DirectBitmap;
}

void EraseBackground(HWND hwnd, HDC hdc)
{
	RECT r;
	GetClientRect(hwnd, &r);

	if (opts.use_skinning && isSkinEngineEnabled() && !FrameIsFloating())
		SkinDrawWindowBack(hwnd, hdc, &r, "Main,ID=Background");
	else {
		HBRUSH hB = CreateSolidBrush(opts.bkg_color);
		FillRect(hdc, &r, hB);
		DeleteObject(hB);
	}
}

void DrawTextWithRect(HDC hdc, const TCHAR *text, const TCHAR *def_text, RECT rc, UINT uFormat, bool mouse_over, Protocol *proto)
{
	const TCHAR *tmp;
	if (text[0] == '\0')
		tmp = TranslateTS(def_text);
	else
		tmp = text;

	// Only first line
	TCHAR *tmp2 = _tcsdup(tmp);
	TCHAR *pos = _tcsrchr(tmp2, '\r');
	if (pos != NULL)
		pos[0] = '\0';
	pos = _tcschr(tmp2, '\n');
	if (pos != NULL)
		pos[0] = '\0';


	RECT r = rc;
	r.top += BORDER_SPACE;
	r.bottom -= BORDER_SPACE;
	r.left += BORDER_SPACE;
	r.right -= BORDER_SPACE;

	HRGN rgn = CreateRectRgnIndirect(&r);
	SelectClipRgn(hdc, rgn);

	RECT rc_tmp;
	int text_height;

	if (mouse_over) {
		uFormat &= ~DT_END_ELLIPSIS;

		rc_tmp = r;
		text_height = DrawText(hdc, _T(" ..."), 4, &rc_tmp, DT_CALCRECT | uFormat);
		rc_tmp.top += (r.bottom - r.top - text_height) >> 1;
		rc_tmp.bottom = rc_tmp.top + text_height;

		if (uFormat & DT_RTLREADING) {
			rc_tmp.right = r.left + (rc_tmp.right - rc_tmp.left);
			rc_tmp.left = r.left;

			r.left += rc_tmp.right - rc_tmp.left;
		}
		else {
			rc_tmp.left = r.right - (rc_tmp.right - rc_tmp.left);
			rc_tmp.right = r.right;

			r.right -= rc_tmp.right - rc_tmp.left;
		}
	}

	DRAW_TEXT(hdc, tmp2, (int)_tcslen(tmp2), &r, uFormat, proto->name);

	if (mouse_over)
		DrawText(hdc, _T(" ..."), 4, &rc_tmp, uFormat);

	SelectClipRgn(hdc, NULL);
	DeleteObject(rgn);

	if (mouse_over)
		FrameRect(hdc, &rc, (HBRUSH)GetStockObject(GRAY_BRUSH));

	free(tmp2);
}

void Draw(HWND hwnd, HDC hdc_orig)
{
	MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	Protocol *proto = protocols->Get(data->protocol_number);

	if (proto == NULL) {
		EraseBackground(hwnd, hdc_orig);
		return;
	}

	if (data->recalc_rectangles || proto->data_changed)
		CalcRectangles(hwnd);

	RECT r_full;
	GetClientRect(hwnd, &r_full);
	RECT r = r_full;

	HDC hdc = CreateCompatibleDC(hdc_orig);
	HBITMAP hBmp = CreateBitmap32(r.right, r.bottom);
	HBITMAP old_bitmap = (HBITMAP)SelectObject(hdc, hBmp);

	int old_bk_mode = SetBkMode(hdc, TRANSPARENT);
	HFONT old_font = (HFONT)SelectObject(hdc, hFont[0]);
	COLORREF old_color = GetTextColor(hdc);
	SetStretchBltMode(hdc, HALFTONE);

	// Erase
	EraseBackground(hwnd, hdc);

	r.left += min(opts.borders[LEFT], r.right);
	r.right = max(r.right - opts.borders[RIGHT], r.left);
	r.top += min(opts.borders[TOP], r.bottom);
	r.bottom = max(r.bottom - opts.borders[BOTTOM], r.top);

	// Draw items

	UINT uFormat = DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS
		| (opts.draw_text_align_right ? DT_RIGHT : DT_LEFT)
		| (opts.draw_text_rtl ? DT_RTLREADING : 0);

	// Image
	if (data->draw_img) {
		RECT rc = GetInnerRect(data->img_rect, r);
		HRGN rgn = CreateRectRgnIndirect(&rc);
		SelectClipRgn(hdc, rgn);

		int width = data->img_rect.right - data->img_rect.left;
		int height = data->img_rect.bottom - data->img_rect.top;

		int round_radius;
		if (opts.draw_avatar_round_corner) {
			if (opts.draw_avatar_use_custom_corner_size)
				round_radius = opts.draw_avatar_custom_corner_size;
			else
				round_radius = min(width, height) / 6;
		}
		else
			round_radius = 0;

		AVATARDRAWREQUEST adr = { sizeof(AVATARDRAWREQUEST) };
		adr.hTargetDC = hdc;
		adr.rcDraw = data->img_rect;
		adr.dwFlags = AVDRQ_OWNPIC | AVDRQ_HIDEBORDERONTRANSPARENCY
			| (opts.draw_avatar_border ? AVDRQ_DRAWBORDER : 0 )
			| (opts.draw_avatar_round_corner ? AVDRQ_ROUNDEDCORNER : 0 );
		adr.clrBorder =  opts.draw_avatar_border_color;
		adr.radius = round_radius;
		adr.alpha = 255;
		adr.szProto = proto->name;
		CallService(MS_AV_DRAWAVATAR, 0, (LPARAM)&adr);

		// Clipping rgn
		SelectClipRgn(hdc, NULL);
		DeleteObject(rgn);
	}

	// Nick
	if (data->draw_nick) {
		RECT rc = GetInnerRect(data->nick_rect, r);
		HRGN rgn = CreateRectRgnIndirect(&rc);
		SelectClipRgn(hdc, rgn);

		SelectObject(hdc, hFont[FONT_NICK]);
		SetTextColor(hdc, font_colour[FONT_NICK]);

		DrawTextWithRect(hdc, proto->nickname, DEFAULT_NICKNAME, rc, uFormat, data->mouse_over_nick && proto->CanSetNick(), proto);

		// Clipping rgn
		SelectClipRgn(hdc, NULL);
		DeleteObject(rgn);
	}

	// Protocol cycle icon
	if (data->draw_proto_cycle) {
		RECT rc = GetInnerRect(data->next_proto_rect, r);
		HRGN rgn = CreateRectRgnIndirect(&rc);
		SelectClipRgn(hdc, rgn);

		HICON icon = Skin_GetIcon("MYDETAILS_NEXT_PROTOCOL");
		if (icon == NULL)
			icon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_RIGHT_ARROW));
		DrawIconEx(hdc, data->next_proto_rect.left, data->next_proto_rect.top, icon, ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);
		Skin_ReleaseIcon(icon);

		SelectClipRgn(hdc, NULL);
		DeleteObject(rgn);

		rc = GetInnerRect(data->prev_proto_rect, r);
		rgn = CreateRectRgnIndirect(&rc);
		SelectClipRgn(hdc, rgn);

		icon = Skin_GetIcon("MYDETAILS_PREV_PROTOCOL");
		if (icon == NULL)
			icon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_RIGHT_ARROW));
		DrawIconEx(hdc, data->prev_proto_rect.left, data->prev_proto_rect.top, icon, ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);
		Skin_ReleaseIcon(icon);

		SelectClipRgn(hdc, NULL);
		DeleteObject(rgn);
	}

	// Protocol
	if (data->draw_proto) {
		RECT rc = GetInnerRect(data->proto_rect, r);
		RECT rr = rc;
		rr.top += BORDER_SPACE;
		rr.bottom -= BORDER_SPACE;
		rr.left += BORDER_SPACE;
		rr.right -= BORDER_SPACE;

		HRGN rgn = CreateRectRgnIndirect(&rc);
		SelectClipRgn(hdc, rgn);

		SelectObject(hdc, hFont[FONT_PROTO]);
		SetTextColor(hdc, font_colour[FONT_PROTO]);

		DrawText(hdc, proto->description, (int)_tcslen(proto->description), &rr, uFormat);

		// Clipping rgn
		SelectClipRgn(hdc, NULL);
		DeleteObject(rgn);

		if (data->mouse_over_proto)
			FrameRect(hdc, &rc, (HBRUSH)GetStockObject(GRAY_BRUSH));
	}

	// Status
	if (data->draw_status) {
		RECT rtmp = GetInnerRect(data->status_rect, r);
		RECT rr = rtmp;
		rr.top += BORDER_SPACE;
		rr.bottom -= BORDER_SPACE;
		rr.left += BORDER_SPACE;
		rr.right -= BORDER_SPACE;

		RECT rc = GetInnerRect(data->status_icon_rect, rr);
		HRGN rgn = CreateRectRgnIndirect(&rc);
		SelectClipRgn(hdc, rgn);

		HICON status_icon;
		if (proto->custom_status != 0 && ProtoServiceExists(proto->name, PS_GETCUSTOMSTATUSICON))
			status_icon = (HICON)CallProtoService(proto->name, PS_GETCUSTOMSTATUSICON, proto->custom_status, LR_SHARED);
		else
			status_icon = LoadSkinnedProtoIcon(proto->name, proto->status);

		if (status_icon != NULL) {
			DrawIconEx(hdc, data->status_icon_rect.left, data->status_icon_rect.top, status_icon, ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);
			Skin_ReleaseIcon(status_icon);
		}

		SelectClipRgn(hdc, NULL);
		DeleteObject(rgn);

		rc = GetInnerRect(data->status_text_rect, rr);
		rgn = CreateRectRgnIndirect(&rc);
		SelectClipRgn(hdc, rgn);

		SelectObject(hdc, hFont[FONT_STATUS]);
		SetTextColor(hdc, font_colour[FONT_STATUS]);

		DRAW_TEXT(hdc, proto->status_name, (int)_tcslen(proto->status_name), &rc, uFormat, proto->name);

		SelectClipRgn(hdc, NULL);
		DeleteObject(rgn);

		if (data->mouse_over_status)
			FrameRect(hdc, &rtmp, (HBRUSH)GetStockObject(GRAY_BRUSH));
	}

	// Away message
	if (data->draw_away_msg) {
		RECT rc = GetInnerRect(data->away_msg_rect, r);
		HRGN rgn = CreateRectRgnIndirect(&rc);
		SelectClipRgn(hdc, rgn);

		SelectObject(hdc, hFont[FONT_AWAY_MSG]);
		SetTextColor(hdc, font_colour[FONT_AWAY_MSG]);

		DrawTextWithRect(hdc, proto->status_message, DEFAULT_STATUS_MESSAGE, rc, uFormat,
			data->mouse_over_away_msg && proto->CanSetStatusMsg(), proto);

		// Clipping rgn
		SelectClipRgn(hdc, NULL);
		DeleteObject(rgn);
	}

	// Listening to
	if (data->draw_listening_to) {
		if (data->listening_to_icon_rect.left == 0 && data->listening_to_icon_rect.right == 0) {
			RECT rc = GetInnerRect(data->listening_to_rect, r);
			HRGN rgn = CreateRectRgnIndirect(&rc);
			SelectClipRgn(hdc, rgn);

			SelectObject(hdc, hFont[FONT_LISTENING_TO]);
			SetTextColor(hdc, font_colour[FONT_LISTENING_TO]);

			DrawTextWithRect(hdc, proto->listening_to, DEFAULT_LISTENING_TO, rc, uFormat,
				data->mouse_over_listening_to && protocols->CanSetListeningTo(), proto);

			// Clipping rgn
			SelectClipRgn(hdc, NULL);
			DeleteObject(rgn);
		}
		else {
			RECT rtmp = GetInnerRect(data->listening_to_rect, r);
			RECT rr = rtmp;
			rr.top += BORDER_SPACE;
			rr.bottom -= BORDER_SPACE;
			rr.left += BORDER_SPACE;
			rr.right -= BORDER_SPACE;

			RECT rc = GetInnerRect(data->listening_to_icon_rect, rr);
			HRGN rgn = CreateRectRgnIndirect(&rc);
			SelectClipRgn(hdc, rgn);

			HICON icon = Skin_GetIcon("LISTENING_TO_ICON");
			if (icon == NULL)
				icon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_LISTENINGTO));
			DrawIconEx(hdc, data->listening_to_icon_rect.left, data->listening_to_icon_rect.top, icon, ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);
			Skin_ReleaseIcon(icon);

			SelectClipRgn(hdc, NULL);
			DeleteObject(rgn);

			rc = GetInnerRect(data->listening_to_text_rect, rr);
			rgn = CreateRectRgnIndirect(&rc);
			SelectClipRgn(hdc, rgn);

			SelectObject(hdc, hFont[FONT_LISTENING_TO]);
			SetTextColor(hdc, font_colour[FONT_LISTENING_TO]);

			DrawText(hdc, proto->listening_to, (int)_tcslen(proto->listening_to), &rc, uFormat);

			SelectClipRgn(hdc, NULL);
			DeleteObject(rgn);

			if (data->mouse_over_listening_to && protocols->CanSetListeningTo())
				FrameRect(hdc, &rtmp, (HBRUSH)GetStockObject(GRAY_BRUSH));
		}
	}

	SelectObject(hdc, old_font);
	SetTextColor(hdc, old_color);
	SetBkMode(hdc, old_bk_mode);

	BitBlt(hdc_orig, r_full.left, r_full.top, r_full.right - r_full.left, r_full.bottom - r_full.top, hdc, r_full.left, r_full.top, SRCCOPY);

	SelectObject(hdc, old_bitmap);
	DeleteDC(hdc);
	DeleteObject(hBmp);
}

bool InsideRect(POINT *p, RECT *r)
{
	return p->x >= r->left && p->x < r->right && p->y >= r->top && p->y < r->bottom;
}

void MakeHover(HWND hwnd, bool draw, bool *hover, POINT *p, RECT *r)
{
	if (draw && p != NULL && r != NULL && InsideRect(p, r)) {
		if (!*hover) {
			*hover = true;

			InvalidateRect(hwnd, NULL, FALSE);

			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = hwnd;
			tme.dwHoverTime = HOVER_DEFAULT;
			TrackMouseEvent(&tme);
		}
	}
	else if (*hover) {
		*hover = false;
		InvalidateRect(hwnd, NULL, FALSE);
	}
}

void ShowGlobalStatusMenu(HWND hwnd, MyDetailsFrameData *data, Protocol *proto, POINT &p)
{
	HMENU submenu = (HMENU)CallService(MS_CLIST_MENUGETSTATUS, 0, 0);

	p.x = (opts.draw_text_align_right ? data->status_rect.right : data->status_rect.left);
	p.y = data->status_rect.bottom + 1;
	ClientToScreen(hwnd, &p);

	int ret = TrackPopupMenu(submenu, TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD
		| (opts.draw_text_align_right ? TPM_RIGHTALIGN : TPM_LEFTALIGN), p.x, p.y, 0, hwnd, NULL);

	if (ret)
		CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(ret), MPCF_MAINMENU), 0);
}

void ShowProtocolStatusMenu(HWND hwnd, MyDetailsFrameData *data, Protocol *proto, POINT &p)
{
	HMENU menu = (HMENU)CallService(MS_CLIST_MENUGETSTATUS, 0, 0);
	HMENU submenu = NULL;

	if (menu != NULL) {
		// Find the correct menu item
		int count = GetMenuItemCount(menu);
		for (int i = 0; i < count && submenu == NULL; i++) {
			MENUITEMINFO mii = {0};

			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_STRING;

			GetMenuItemInfo(menu, i, TRUE, &mii);

			if (mii.cch != 0) {
				mii.cch++;
				mii.dwTypeData = (TCHAR *)malloc(sizeof(TCHAR) * mii.cch);
				GetMenuItemInfo(menu, i, TRUE, &mii);

				if (_tcscmp(mii.dwTypeData, proto->description) == 0)
					submenu = GetSubMenu(menu, i);

				free(mii.dwTypeData);
			}
		}

		if (submenu == NULL && protocols->GetSize() == 1)
			submenu = menu;
	}

	if (submenu != NULL) {
		p.x = (opts.draw_text_align_right ? data->status_rect.right : data->status_rect.left);
		p.y =  data->status_rect.bottom + 1;
		ClientToScreen(hwnd, &p);

		int ret = TrackPopupMenu(submenu, TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD
			| (opts.draw_text_align_right ? TPM_RIGHTALIGN : TPM_LEFTALIGN), p.x, p.y, 0, hwnd, NULL);

		if (ret)
			CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(ret), MPCF_MAINMENU), 0);
	}
	else {
		// Well, lets do it by hand
		static int statusModePf2List[] = { 0xFFFFFFFF, PF2_ONLINE, PF2_SHORTAWAY, PF2_LONGAWAY, PF2_LIGHTDND, PF2_HEAVYDND, PF2_FREECHAT,
			PF2_INVISIBLE, PF2_ONTHEPHONE, PF2_OUTTOLUNCH };

		menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
		submenu = GetSubMenu(menu, 0);
		TranslateMenu(submenu);

		// Hide menu
		DWORD flags = CallProtoService(proto->name, PS_GETCAPS, PFLAGNUM_2, 0);
		for (int i = GetMenuItemCount(submenu) - 1; i >= 0; i--)
			if (!(flags & statusModePf2List[i]))
				RemoveMenu(submenu, i, MF_BYPOSITION);

		p.x = (opts.draw_text_align_right ? data->status_rect.right : data->status_rect.left);
		p.y =  data->status_rect.bottom + 1;
		ClientToScreen(hwnd, &p);

		int ret = TrackPopupMenu(submenu, TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD
			| (opts.draw_text_align_right ? TPM_RIGHTALIGN : TPM_LEFTALIGN), p.x, p.y, 0, hwnd, NULL);
		DestroyMenu(menu);
		if (ret)
			proto->SetStatus(ret);
	}
}

void ShowListeningToMenu(HWND hwnd, MyDetailsFrameData *data, Protocol *proto, POINT &p)
{
	HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
	HMENU submenu = GetSubMenu(menu, 5);
	TranslateMenu(submenu);

	// Add this proto to menu
	TCHAR tmp[128];
	mir_sntprintf(tmp, SIZEOF(tmp), TranslateT("Enable listening to for %s"), proto->description);

	MENUITEMINFO mii = {0};
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE;
	mii.fType = MFT_STRING;
	mii.fState = proto->ListeningToEnabled() ? MFS_CHECKED : 0;
	mii.dwTypeData = tmp;
	mii.cch = (int)_tcslen(tmp);
	mii.wID = 1;

	if (!proto->CanSetListeningTo())
		mii.fState |= MFS_DISABLED;

	InsertMenuItem(submenu, 0, TRUE, &mii);

	ZeroMemory(&mii, sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_STATE;
	mii.fState = protocols->ListeningToEnabled() ? MFS_CHECKED : 0;

	if (!protocols->CanSetListeningTo())
		mii.fState |= MFS_DISABLED;

	SetMenuItemInfo(submenu, ID_LISTENINGTOPOPUP_SENDLISTENINGTO, FALSE, &mii);

	p.x = (opts.draw_text_align_right ? data->listening_to_rect.right : data->listening_to_rect.left);
	p.y =  data->listening_to_rect.bottom + 1;
	ClientToScreen(hwnd, &p);

	int ret = TrackPopupMenu(submenu, TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD
		| (opts.draw_text_align_right ? TPM_RIGHTALIGN : TPM_LEFTALIGN), p.x, p.y, 0, hwnd, NULL);
	DestroyMenu(menu);

	switch (ret) {
	case 1:
		CallService(MS_LISTENINGTO_ENABLE, (LPARAM)proto->name, !proto->ListeningToEnabled());
		break;

	case ID_LISTENINGTOPOPUP_SENDLISTENINGTO:
		CallService(MS_LISTENINGTO_ENABLE, 0, !protocols->ListeningToEnabled());
		break;
	}
}

LRESULT CALLBACK FrameWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	Protocol *proto;

	switch (msg) {
	case WM_CREATE:
		{
			data = new MyDetailsFrameData();
			ZeroMemory(data, sizeof(MyDetailsFrameData));
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)data);

			data->recalc_rectangles = true;
			data->get_status_messages = false;
			data->showing_menu = false;

			data->protocol_number = db_get_w(NULL, "MyDetails", "ProtocolNumber", 0);
			if (data->protocol_number >= protocols->GetSize())
				data->protocol_number = 0;

			SetCycleTime(hwnd);
			SetStatusMessageRefreshTime(hwnd);

			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_HOVER | TME_LEAVE;
			tme.hwndTrack = hwnd;
			tme.dwHoverTime = HOVER_DEFAULT;
			TrackMouseEvent(&tme);
		}
		return TRUE;

	case WM_PRINTCLIENT:
		Draw(hwnd, (HDC)wParam);
		return TRUE;

	case WM_PAINT:
		{
			RECT r;
			if (GetUpdateRect(hwnd, &r, FALSE)) {
				PAINTSTRUCT ps;

				HDC hdc = BeginPaint(hwnd, &ps);
				Draw(hwnd, hdc);
				EndPaint(hwnd, &ps);
			}
		}
		return TRUE;

	case WM_SIZE:
		data->recalc_rectangles = true;
		RedrawFrame();
		break;

	case WM_TIMER:
		if (wParam == ID_FRAME_TIMER) {
			if (!data->showing_menu)
				CallService(MS_MYDETAILS_SHOWNEXTPROTOCOL, 0, 0);
		}
		else if (wParam == ID_RECALC_TIMER) {
			KillTimer(hwnd, ID_RECALC_TIMER);

			if (data->get_status_messages) {
				SetStatusMessageRefreshTime(hwnd);
				data->get_status_messages = false;

				protocols->GetStatuses();
				protocols->GetStatusMsgs();

				data->recalc_rectangles = true;
			}

			RedrawFrame();
		}
		else if (wParam == ID_STATUSMESSAGE_TIMER) {
			SetStatusMessageRefreshTime(hwnd);
			PostMessage(hwnd, MWM_STATUS_MSG_CHANGED, 0, 0);
		}

		return TRUE;

	case WM_ERASEBKGND:
		return TRUE;

	case WM_LBUTTONUP:
		proto = protocols->Get(data->protocol_number);
		if (proto != NULL) {
			POINT p;
			p.x = LOWORD(lParam);
			p.y = HIWORD(lParam);

			// In image?
			if (data->draw_img && InsideRect(&p, &data->img_rect) && proto->CanSetAvatar()) {
				if (opts.global_on_avatar)
					CallService(MS_MYDETAILS_SETMYAVATARUI, 0, 0);
				else
					CallService(MS_MYDETAILS_SETMYAVATARUI, 0, (LPARAM)proto->name);
			}
			// In nick?
			else if (data->draw_nick && InsideRect(&p, &data->nick_rect) && proto->CanSetNick()) {
				if (opts.global_on_nickname)
					CallService(MS_MYDETAILS_SETMYNICKNAMEUI, 0, 0);
				else
					CallService(MS_MYDETAILS_SETMYNICKNAMEUI, 0, (LPARAM)proto->name);
			}
			// In proto cycle button?
			else if (data->draw_proto_cycle && InsideRect(&p, &data->next_proto_rect))
				CallService(MS_MYDETAILS_SHOWNEXTPROTOCOL, 0, 0);
			else if (data->draw_proto_cycle && InsideRect(&p, &data->prev_proto_rect))
				CallService(MS_MYDETAILS_SHOWPREVIOUSPROTOCOL, 0, 0);

			// In status message?
			else if (data->draw_away_msg && InsideRect(&p, &data->away_msg_rect) && proto->CanSetStatusMsg()) {
				if (opts.global_on_status_message)
					CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, 0, 0);
				else
					CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, 0, (LPARAM)proto->name);
			}
			// In status?
			else if (data->draw_status && InsideRect(&p, &data->status_rect)) {
				data->showing_menu = true;

				if (opts.global_on_status)
					ShowGlobalStatusMenu(hwnd, data, proto, p);
				else
					ShowProtocolStatusMenu(hwnd, data, proto, p);

				data->showing_menu = false;
			}
			// In listening to?
			else if (data->draw_listening_to && InsideRect(&p, &data->listening_to_rect) && protocols->CanSetListeningTo())
				ShowListeningToMenu(hwnd, data, proto, p);

			// In protocol?
			else if (data->draw_proto && InsideRect(&p, &data->proto_rect)) {
				data->showing_menu = true;

				HMENU menu = CreatePopupMenu();

				for (int i = protocols->GetSize() - 1; i >= 0 ; i--) {
					MENUITEMINFO mii = { sizeof(mii) };
					mii.fMask = MIIM_ID | MIIM_TYPE;
					mii.fType = MFT_STRING;
					mii.dwTypeData = protocols->Get(i)->description;
					mii.cch = (int)_tcslen(protocols->Get(i)->description);
					mii.wID = i + 1;

					if (i == data->protocol_number) {
						mii.fMask |= MIIM_STATE;
						mii.fState = MFS_DISABLED;
					}

					InsertMenuItem(menu, 0, TRUE, &mii);
				}

				if (opts.draw_text_align_right)
					p.x = data->proto_rect.right;
				else
					p.x = data->proto_rect.left;
				p.y =  data->proto_rect.bottom + 1;
				ClientToScreen(hwnd, &p);

				int ret = TrackPopupMenu(menu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, 0, hwnd, NULL);
				DestroyMenu(menu);

				if (ret != 0)
					PluginCommand_ShowProtocol(NULL, (WPARAM)protocols->Get(ret - 1)->name);

				data->showing_menu = false;
			}
		}
		break;

	case WM_MEASUREITEM:
		return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);

	case WM_DRAWITEM:
		return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);

	case WM_CONTEXTMENU:
		proto = protocols->Get(data->protocol_number);
		if (proto != NULL) {
			POINT p;
			p.x = LOWORD(lParam);
			p.y = HIWORD(lParam);

			ScreenToClient(hwnd, &p);

			data->showing_menu = true;

			// In image?
			if (data->draw_img && InsideRect(&p, &data->img_rect)) {
				HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
				HMENU submenu = GetSubMenu(menu, 4);
				TranslateMenu(submenu);

				// Add this proto to menu
				TCHAR tmp[128];
				mir_sntprintf(tmp, SIZEOF(tmp), TranslateT("Set my avatar for %s..."), proto->description);

				MENUITEMINFO mii = {0};
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_TYPE;
				mii.fType = MFT_STRING;
				mii.dwTypeData = tmp;
				mii.cch = (int)_tcslen(tmp);
				mii.wID = 1;

				if (!proto->CanSetAvatar()) {
					mii.fMask |= MIIM_STATE;
					mii.fState = MFS_DISABLED;
				}

				InsertMenuItem(submenu, 0, TRUE, &mii);

				ClientToScreen(hwnd, &p);

				int ret = TrackPopupMenu(submenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, 0, hwnd, NULL);
				DestroyMenu(menu);

				switch (ret) {
				case 1:
					CallService(MS_MYDETAILS_SETMYAVATARUI, 0, (LPARAM)proto->name);
					break;

				case ID_AVATARPOPUP_SETMYAVATAR:
					CallService(MS_MYDETAILS_SETMYAVATARUI, 0, 0);
					break;
				}
			}
			// In nick?
			else if (data->draw_nick && InsideRect(&p, &data->nick_rect)) {
				HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
				HMENU submenu = GetSubMenu(menu, 2);
				TranslateMenu(submenu);

				// Add this proto to menu
				TCHAR tmp[128];
				mir_sntprintf(tmp, SIZEOF(tmp), TranslateT("Set my nickname for %s..."), proto->description);

				MENUITEMINFO mii = {0};
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_TYPE;
				mii.fType = MFT_STRING;
				mii.dwTypeData = tmp;
				mii.cch = (int)_tcslen(tmp);
				mii.wID = 1;

				if (!proto->CanSetNick()) {
					mii.fMask |= MIIM_STATE;
					mii.fState = MFS_DISABLED;
				}

				InsertMenuItem(submenu, 0, TRUE, &mii);

				ClientToScreen(hwnd, &p);

				int ret = TrackPopupMenu(submenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, 0, hwnd, NULL);
				DestroyMenu(menu);

				switch (ret) {
				case 1:
					CallService(MS_MYDETAILS_SETMYNICKNAMEUI, 0, (LPARAM)proto->name);
					break;

				case ID_NICKPOPUP_SETMYNICKNAME:
					CallService(MS_MYDETAILS_SETMYNICKNAMEUI, 0, 0);
					break;
				}
			}
			// In proto cycle button?
			else if (data->draw_proto_cycle && InsideRect(&p, &data->next_proto_rect))
				CallService(MS_MYDETAILS_SHOWPREVIOUSPROTOCOL, 0, 0);
			else if (data->draw_proto_cycle && InsideRect(&p, &data->prev_proto_rect))
				CallService(MS_MYDETAILS_SHOWNEXTPROTOCOL, 0, 0);

			// In status message?
			else if (data->draw_away_msg && InsideRect(&p, &data->away_msg_rect)) {
				TCHAR tmp[128];

				HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
				HMENU submenu = GetSubMenu(menu, 3);
				TranslateMenu(submenu);

				if (protocols->CanSetStatusMsgPerProtocol()) {
					// Add this proto to menu
					mir_sntprintf(tmp, SIZEOF(tmp), TranslateT("Set my status message for %s..."), proto->description);

					MENUITEMINFO mii = {0};
					mii.cbSize = sizeof(mii);
					mii.fMask = MIIM_ID | MIIM_TYPE;
					mii.fType = MFT_STRING;
					mii.dwTypeData = tmp;
					mii.cch = (int)_tcslen(tmp);
					mii.wID = 1;

					if (!proto->CanSetStatusMsg()) {
						mii.fMask |= MIIM_STATE;
						mii.fState = MFS_DISABLED;
					}

					InsertMenuItem(submenu, 0, TRUE, &mii);
				}

				// Add this to menu
				mir_sntprintf(tmp, SIZEOF(tmp), TranslateT("Set my status message for %s..."),
					CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, proto->status, GSMDF_TCHAR));

				MENUITEMINFO mii = {0};
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_TYPE;
				mii.fType = MFT_STRING;
				mii.dwTypeData = tmp;
				mii.cch = (int)_tcslen(tmp);
				mii.wID = 2;

				if (proto->status == ID_STATUS_OFFLINE) {
					mii.fMask |= MIIM_STATE;
					mii.fState = MFS_DISABLED;
				}

				InsertMenuItem(submenu, 0, TRUE, &mii);

				ClientToScreen(hwnd, &p);

				int ret = TrackPopupMenu(submenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, 0, hwnd, NULL);
				DestroyMenu(menu);

				switch (ret) {
				case 1:
					CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, 0, (LPARAM)proto->name);
					break;

				case 2:
					CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, (WPARAM)proto->status, 0);
					break;

				case ID_STATUSMESSAGEPOPUP_SETMYSTATUSMESSAGE:
					CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, 0, 0);
					break;
				}
			}
			// In status?
			else if (data->draw_status && InsideRect(&p, &data->status_rect)) {
				if (opts.global_on_status)
					ShowProtocolStatusMenu(hwnd, data, proto, p);
				else
					ShowGlobalStatusMenu(hwnd, data, proto, p);
			}
			// In listening to?
			else if (data->draw_listening_to && InsideRect(&p, &data->listening_to_rect) && protocols->CanSetListeningTo())
				ShowListeningToMenu(hwnd, data, proto, p);

			// In protocol?
			else if (data->draw_proto && InsideRect(&p, &data->proto_rect)) {
			}
			// Default context menu
			else {
				HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
				HMENU submenu = GetSubMenu(menu, 1);
				TranslateMenu(submenu);

				if (opts.cycle_through_protocols)
					RemoveMenu(submenu, ID_CYCLE_THROUGH_PROTOS, MF_BYCOMMAND);
				else
					RemoveMenu(submenu, ID_DONT_CYCLE_THROUGH_PROTOS, MF_BYCOMMAND);

				// Add this proto to menu
				TCHAR tmp[128];
				MENUITEMINFO mii = {0};

				mir_sntprintf(tmp, SIZEOF(tmp), TranslateT("Enable listening to for %s"), proto->description);

				ZeroMemory(&mii, sizeof(mii));
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE;
				mii.fType = MFT_STRING;
				mii.fState = proto->ListeningToEnabled() ? MFS_CHECKED : 0;
				mii.dwTypeData = tmp;
				mii.cch = (int)_tcslen(tmp);
				mii.wID = 5;
				if (!proto->CanSetListeningTo())
					mii.fState |= MFS_DISABLED;

				InsertMenuItem(submenu, 0, TRUE, &mii);

				// Add this to menu
				mir_sntprintf(tmp, SIZEOF(tmp), TranslateT("Set my status message for %s..."),
					CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, proto->status, GSMDF_TCHAR));

				ZeroMemory(&mii, sizeof(mii));
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_TYPE;
				mii.fType = MFT_STRING;
				mii.dwTypeData = tmp;
				mii.cch = (int)_tcslen(tmp);
				mii.wID = 4;

				if (proto->status == ID_STATUS_OFFLINE) {
					mii.fMask |= MIIM_STATE;
					mii.fState = MFS_DISABLED;
				}

				InsertMenuItem(submenu, 0, TRUE, &mii);

				if (protocols->CanSetStatusMsgPerProtocol()) {
					// Add this proto to menu
					mir_sntprintf(tmp, SIZEOF(tmp), TranslateT("Set my status message for %s..."), proto->description);

					ZeroMemory(&mii, sizeof(mii));
					mii.cbSize = sizeof(mii);
					mii.fMask = MIIM_ID | MIIM_TYPE;
					mii.fType = MFT_STRING;
					mii.dwTypeData = tmp;
					mii.cch = (int)_tcslen(tmp);
					mii.wID = 3;

					if ( !proto->CanSetStatusMsg()) {
						mii.fMask |= MIIM_STATE;
						mii.fState = MFS_DISABLED;
					}

					InsertMenuItem(submenu, 0, TRUE, &mii);
				}

				mir_sntprintf(tmp, SIZEOF(tmp), TranslateT("Set my nickname for %s..."), proto->description);

				ZeroMemory(&mii, sizeof(mii));
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_TYPE;
				mii.fType = MFT_STRING;
				mii.dwTypeData = tmp;
				mii.cch = (int)_tcslen(tmp);
				mii.wID = 2;

				if (!proto->CanSetNick()) {
					mii.fMask |= MIIM_STATE;
					mii.fState = MFS_DISABLED;
				}

				InsertMenuItem(submenu, 0, TRUE, &mii);

				mir_sntprintf(tmp, SIZEOF(tmp), TranslateT("Set my avatar for %s..."), proto->description);

				ZeroMemory(&mii, sizeof(mii));
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_TYPE;
				mii.fType = MFT_STRING;
				mii.dwTypeData = tmp;
				mii.cch = (int)_tcslen(tmp);
				mii.wID = 1;

				if (!proto->CanSetAvatar()) {
					mii.fMask |= MIIM_STATE;
					mii.fState = MFS_DISABLED;
				}

				InsertMenuItem(submenu, 0, TRUE, &mii);

				ZeroMemory(&mii, sizeof(mii));
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_STATE;
				mii.fState = protocols->ListeningToEnabled() ? MFS_CHECKED : 0;

				if (!protocols->CanSetListeningTo())
					mii.fState |= MFS_DISABLED;

				SetMenuItemInfo(submenu, ID_CONTEXTPOPUP_ENABLELISTENINGTO, FALSE, &mii);

				ClientToScreen(hwnd, &p);

				int ret = TrackPopupMenu(submenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, 0, hwnd, NULL);
				DestroyMenu(menu);

				switch (ret) {
				case 1:
					CallService(MS_MYDETAILS_SETMYAVATARUI, 0, (LPARAM)proto->name);
					break;

				case ID_AVATARPOPUP_SETMYAVATAR:
					CallService(MS_MYDETAILS_SETMYAVATARUI, 0, 0);
					break;

				case 2:
					CallService(MS_MYDETAILS_SETMYNICKNAMEUI, 0, (LPARAM)proto->name);
					break;

				case ID_NICKPOPUP_SETMYNICKNAME:
					CallService(MS_MYDETAILS_SETMYNICKNAMEUI, 0, 0);
					break;

				case 3:
					CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, 0, (LPARAM)proto->name);
					break;

				case 4:
					CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, (WPARAM)proto->status, 0);
					break;

				case ID_STATUSMESSAGEPOPUP_SETMYSTATUSMESSAGE:
					CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, 0, 0);
					break;

				case 5:
					CallService(MS_LISTENINGTO_ENABLE, (LPARAM)proto->name, !proto->ListeningToEnabled());
					break;

				case ID_CONTEXTPOPUP_ENABLELISTENINGTO:
					CallService(MS_LISTENINGTO_ENABLE, 0, !protocols->ListeningToEnabled());
					break;

				case ID_SHOW_NEXT_PROTO:
					CallService(MS_MYDETAILS_SHOWNEXTPROTOCOL, 0, 0);
					break;

				case ID_SHOW_PREV_PROTO:
					CallService(MS_MYDETAILS_SHOWPREVIOUSPROTOCOL, 0, 0);
					break;

				case ID_CYCLE_THROUGH_PROTOS:
					CallService(MS_MYDETAILS_CYCLE_THROUGH_PROTOCOLS, TRUE, 0);
					break;

				case ID_DONT_CYCLE_THROUGH_PROTOS:
					CallService(MS_MYDETAILS_CYCLE_THROUGH_PROTOCOLS, FALSE, 0);
					break;
				}
			}

			data->showing_menu = false;
			break;
		}

	case WM_NCMOUSELEAVE:
	case WM_MOUSELEAVE:
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_HOVER;
			tme.hwndTrack = hwnd;
			tme.dwHoverTime = HOVER_DEFAULT;
			TrackMouseEvent(&tme);
		}
	case WM_NCMOUSEMOVE:
		MakeHover(hwnd, data->draw_img, &data->mouse_over_img, NULL, NULL);
		MakeHover(hwnd, data->draw_nick, &data->mouse_over_nick, NULL, NULL);
		MakeHover(hwnd, data->draw_proto, &data->mouse_over_proto, NULL, NULL);
		MakeHover(hwnd, data->draw_status, &data->mouse_over_status, NULL, NULL);
		MakeHover(hwnd, data->draw_away_msg, &data->mouse_over_away_msg, NULL, NULL);
		MakeHover(hwnd, data->draw_listening_to, &data->mouse_over_listening_to, NULL, NULL);
		break;

	case WM_MOUSEHOVER:
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = hwnd;
			tme.dwHoverTime = HOVER_DEFAULT;
			TrackMouseEvent(&tme);
		}
	case WM_MOUSEMOVE:
		proto = protocols->Get(data->protocol_number);
		if (proto != NULL) {
			POINT p = { LOWORD(lParam), HIWORD(lParam) };
			MakeHover(hwnd, data->draw_img, &data->mouse_over_img, &p, &data->img_rect);
			MakeHover(hwnd, data->draw_nick, &data->mouse_over_nick, &p, &data->nick_rect);
			MakeHover(hwnd, data->draw_proto, &data->mouse_over_proto, &p, &data->proto_rect);
			MakeHover(hwnd, data->draw_status, &data->mouse_over_status, &p, &data->status_rect);
			MakeHover(hwnd, data->draw_away_msg, &data->mouse_over_away_msg, &p, &data->away_msg_rect);
			MakeHover(hwnd, data->draw_listening_to, &data->mouse_over_listening_to, &p, &data->listening_to_rect);
		}
		break;

	case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;

			int i = (int)lpnmhdr->code;

			switch (lpnmhdr->code) {
			case TTN_GETDISPINFO:
				{
					MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
					proto = protocols->Get(data->protocol_number);

					LPNMTTDISPINFO lpttd = (LPNMTTDISPINFO)lpnmhdr;
					SendMessage(lpnmhdr->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);

					if (lpnmhdr->hwndFrom == data->nick_tt_hwnd)
						lpttd->lpszText = proto->nickname;
					else if (lpnmhdr->hwndFrom == data->status_tt_hwnd)
						lpttd->lpszText = proto->status_name;
					else if (lpnmhdr->hwndFrom == data->away_msg_tt_hwnd)
						lpttd->lpszText = proto->status_message;
					else if (lpnmhdr->hwndFrom == data->listening_to_tt_hwnd)
						lpttd->lpszText = proto->listening_to;
					else if (lpnmhdr->hwndFrom == data->next_proto_tt_hwnd)
						lpttd->lpszText = TranslateT("Show next account");
					else if (lpnmhdr->hwndFrom == data->prev_proto_tt_hwnd)
						lpttd->lpszText = TranslateT("Show previous account");

					return 0;
				}
			}
		}
		break;

	case WM_DESTROY:
		KillTimer(hwnd, ID_FRAME_TIMER);

		DeleteTooltipWindows(data);
		if (data != NULL)
			delete data;
		break;

	// Custom Messages //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	case MWM_REFRESH:
		KillTimer(hwnd, ID_RECALC_TIMER);
		SetTimer(hwnd, ID_RECALC_TIMER, RECALC_TIME, NULL);
		break;

	case MWM_AVATAR_CHANGED:
		proto = protocols->Get((const char *)wParam);
		if (proto != NULL) {
			proto->GetAvatar();
			RefreshFrame();
		}
		break;

	case MWM_NICK_CHANGED:
		proto = protocols->Get((const char *)wParam);
		if (proto != NULL) {
			proto->GetNick();
			RefreshFrame();
		}

		break;

	case MWM_STATUS_CHANGED:
		proto = protocols->Get((const char *)wParam);
		if (proto != NULL) {
			proto->GetStatus();
			proto->GetStatusMsg();
			proto->GetNick();

			RefreshFrame();
		}

		break;

	case MWM_STATUS_MSG_CHANGED:
		data->get_status_messages = true;
		RefreshFrame();
		break;

	case MWM_LISTENINGTO_CHANGED:
		if (wParam != NULL) {
			proto = protocols->Get((const char *)wParam);
			if (proto != NULL)
				proto->GetListeningTo();
		}

		RefreshFrameAndCalcRects();
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

INT_PTR ShowHideFrameFunc(WPARAM wParam, LPARAM lParam)
{
	if (g_bFramesExist)
		CallService(MS_CLIST_FRAMES_SHFRAME, frame_id, 0);
	else {
		if (MyDetailsFrameVisible())
			SendMessage(hwnd_container, WM_CLOSE, 0, 0);
		else {
			ShowWindow(hwnd_container, SW_SHOW);
			db_set_b(0, MODULE_NAME, SETTING_FRAME_VISIBLE, 1);
		}

		FixMainMenu();
	}
	return 0;
}

INT_PTR ShowFrameFunc(WPARAM wParam, LPARAM lParam)
{
	if (g_bFramesExist) {
		int flags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frame_id), 0);
		if (!(flags & F_VISIBLE))
			CallService(MS_CLIST_FRAMES_SHFRAME, frame_id, 0);
	}
	else {
		if (!MyDetailsFrameVisible()) {
			ShowWindow(hwnd_container, SW_SHOW);
			db_set_b(0, MODULE_NAME, SETTING_FRAME_VISIBLE, 1);

			FixMainMenu();
		}
	}
	return 0;
}


INT_PTR HideFrameFunc(WPARAM wParam, LPARAM lParam)
{
	if (g_bFramesExist) {
		int flags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frame_id), 0);
		if (flags & F_VISIBLE)
			CallService(MS_CLIST_FRAMES_SHFRAME, frame_id, 0);
	}
	else {
		if (MyDetailsFrameVisible()) {
			SendMessage(hwnd_container, WM_CLOSE, 0, 0);
			FixMainMenu();
		}
	}
	return 0;
}

void FixMainMenu()
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_NAME | CMIF_TCHAR;

	if (MyDetailsFrameVisible())
		mi.ptszName = LPGENT("Hide my details");
	else
		mi.ptszName = LPGENT("Show my details");

	Menu_ModifyItem(hMenuShowHideFrame, &mi);
}

#include <math.h>

void RedrawFrame()
{
	if (frame_id == -1)
		InvalidateRect(hwnd_container, NULL, TRUE);
	else
		CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)frame_id, (LPARAM)FU_TBREDRAW | FU_FMREDRAW);
}

void RefreshFrameAndCalcRects()
{
	if (hwnd_frame != NULL) {
		MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLongPtr(hwnd_frame, GWLP_USERDATA);
		data->recalc_rectangles = true;

		PostMessage(hwnd_frame, MWM_REFRESH, 0, 0);
	}
}

void RefreshFrame()
{
	if (hwnd_frame != NULL)
		PostMessage(hwnd_frame, MWM_REFRESH, 0, 0);
}

// only used when no multiwindow functionality is available
bool MyDetailsFrameVisible()
{
	return IsWindowVisible(hwnd_container) ? true : false;
}

void SetMyDetailsFrameVisible(bool visible)
{
	if (frame_id == -1 && hwnd_container != 0)
		ShowWindow(hwnd_container, visible ? SW_SHOW : SW_HIDE);
}

void SetCycleTime()
{
	if (hwnd_frame != NULL)
		SetCycleTime(hwnd_frame);
}

void SetCycleTime(HWND hwnd)
{
	KillTimer(hwnd, ID_FRAME_TIMER);

	if (opts.cycle_through_protocols)
		SetTimer(hwnd, ID_FRAME_TIMER, opts.seconds_to_show_protocol * 1000, 0);
}

void SetStatusMessageRefreshTime()
{
	if (hwnd_frame != NULL)
		SetStatusMessageRefreshTime(hwnd_frame);
}

void SetStatusMessageRefreshTime(HWND hwnd)
{
	KillTimer(hwnd, ID_STATUSMESSAGE_TIMER);

	opts.refresh_status_message_timer = db_get_w(NULL, "MyDetails", "RefreshStatusMessageTimer", 12);
	if (opts.refresh_status_message_timer > 0)
		SetTimer(hwnd, ID_STATUSMESSAGE_TIMER, opts.refresh_status_message_timer * 1000, NULL);
}

INT_PTR PluginCommand_ShowNextProtocol(WPARAM wParam, LPARAM lParam)
{
	if (hwnd_frame == NULL)
		return -1;

	MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLongPtr(hwnd_frame, GWLP_USERDATA);

	data->protocol_number++;
	if (data->protocol_number >= protocols->GetSize())
		data->protocol_number = 0;

	db_set_w(NULL, "MyDetails", "ProtocolNumber", data->protocol_number);

	data->recalc_rectangles = true;

	SetCycleTime();

	RedrawFrame();

	return 0;
}

INT_PTR PluginCommand_ShowPreviousProtocol(WPARAM wParam, LPARAM lParam)
{
	if (hwnd_frame == NULL)
		return -1;

	MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLongPtr(hwnd_frame, GWLP_USERDATA);

	data->protocol_number--;
	if (data->protocol_number < 0)
		data->protocol_number = protocols->GetSize() - 1;

	db_set_w(NULL, "MyDetails", "ProtocolNumber", data->protocol_number);

	data->recalc_rectangles = true;

	SetCycleTime();

	RedrawFrame();

	return 0;
}

INT_PTR PluginCommand_ShowProtocol(WPARAM wParam, LPARAM lParam)
{
	char *proto = (char *)lParam;
	if (proto == NULL)
		return -1;

	int proto_num = -1;
	for (int i = 0 ; i < protocols->GetSize(); i++) {
		if (_stricmp(protocols->Get(i)->name, proto) == 0) {
			proto_num = i;
			break;
		}
	}

	if (proto_num == -1)
		return -2;

	if (hwnd_frame == NULL)
		return -3;

	MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLongPtr(hwnd_frame, GWLP_USERDATA);

	data->protocol_number = proto_num;
	db_set_w(NULL, "MyDetails", "ProtocolNumber", data->protocol_number);

	data->recalc_rectangles = true;

	SetCycleTime();

	RedrawFrame();

	return 0;
}

int SettingsChangedHook(WPARAM wParam, LPARAM lParam)
{
	if (hwnd_frame == NULL)
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;

	if ((HANDLE)wParam == NULL) {
		Protocol *proto = protocols->Get(cws->szModule);

		if (!strcmp(cws->szSetting, "MyHandle")
			|| !strcmp(cws->szSetting, "UIN")
			|| !strcmp(cws->szSetting, "Nick")
			|| !strcmp(cws->szSetting, "FirstName")
			|| !strcmp(cws->szSetting, "e-mail")
			|| !strcmp(cws->szSetting, "LastName")
			|| !strcmp(cws->szSetting, "JID"))
		{
			// Name changed
			if (proto != NULL)
				PostMessage(hwnd_frame, MWM_NICK_CHANGED, (WPARAM)proto->name, 0);
		}
		else if (strstr(cws->szModule, "Away"))
			// Status message changed
			PostMessage(hwnd_frame, MWM_STATUS_MSG_CHANGED, 0, 0);
		else if (proto != NULL && strcmp(cws->szSetting, "ListeningTo") == 0)
			PostMessage(hwnd_frame, MWM_LISTENINGTO_CHANGED, (WPARAM)proto->name, 0);
	}

	return 0;
}

int AvatarChangedHook(WPARAM wParam, LPARAM lParam)
{
	if (hwnd_frame == NULL)
		return 0;

	Protocol *proto = protocols->Get((const char *)wParam);

	if (proto != NULL)
		PostMessage(hwnd_frame, MWM_AVATAR_CHANGED, (WPARAM)proto->name, 0);

	return 0;
}

int ProtoAckHook(WPARAM wParam, LPARAM lParam)
{
	if (hwnd_frame == NULL)
		return 0;

	ACKDATA *ack = (ACKDATA *)lParam;
	if (ack->result != ACKRESULT_SUCCESS)
		return 0;

	Protocol *proto = protocols->Get(ack->szModule);
	switch (ack->type) {
	case ACKTYPE_STATUS:
		if (proto != NULL)
			PostMessage(hwnd_frame, MWM_STATUS_CHANGED, (WPARAM)proto->name, 0);
		break;

	case ACKTYPE_AWAYMSG:
		if (proto != NULL)
			PostMessage(hwnd_frame, MWM_STATUS_MSG_CHANGED, (WPARAM)proto->name, 0);
		break;

	case ACKTYPE_AVATAR:
		if (proto != NULL)
			PostMessage(hwnd_frame, MWM_AVATAR_CHANGED, (WPARAM)proto->name, 0);
		break;
	}

	return 0;
}

int ListeningtoEnableStateChangedHook(WPARAM wParam, LPARAM lParam)
{
	if (hwnd_frame == NULL)
		return 0;

	if (wParam == NULL || protocols->Get((const char *)wParam) != NULL)
		PostMessage(hwnd_frame, MWM_LISTENINGTO_CHANGED, wParam, 0);

	return 0;
}
