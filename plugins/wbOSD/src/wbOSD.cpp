/*
Wannabe OSD
This plugin tries to become miranda's standard OSD ;-)

(C) 2005 Andrej Krutak

Distributed under GNU's GPL 2 or later
*/

#include "wbOSD.h"

TCHAR szClassName[]	= _T("wbOSD");
const static osdmsg defstr={_T(""), 0, RGB(0, 0, 0), 0, 0};

int DrawMe(HWND hwnd, TCHAR *string, COLORREF color)
{
	PAINTSTRUCT	ps;
	RECT rect, rect2;
	UINT talign=0;
	int sxo, syo;
	plgsettings plgs;

	logmsg("DrawMe");
	if (!string) string=_T("bullshit");
	
	loadDBSettings(&plgs);
	HFONT fh=CreateFontIndirect(&(plgs.lf));

	HDC hdc=BeginPaint(hwnd, &ps);
	SetBkMode(hdc, TRANSPARENT);

	GetClientRect(hwnd, &rect);
	HBRUSH bkb=CreateSolidBrush(plgs.bkclr);
	FillRect(hdc, &rect, bkb);

	DeleteObject(bkb);

	HGDIOBJ oo=SelectObject(hdc, fh);

	rect2 = rect;
	DrawText(hdc, string, -1, &rect2, DT_WORDBREAK|DT_CALCRECT);

	if (plgs.align>=1 && plgs.align<=3)
		rect.top=0;
	if (plgs.align>=4 && plgs.align<=6)
		rect.top=(rect.bottom-rect2.bottom)/2;
	if (plgs.align>=7 && plgs.align<=9)
		rect.top=rect.bottom-rect2.bottom;

	if (((plgs.align-1)%3)==0)
		rect.left=0;
	else if (((plgs.align-2)%3)==0)
		rect.left=(rect.right-rect2.right)/2;
	else if (((plgs.align-3)%3)==0)
		rect.left=rect.right-rect2.right;

	rect.bottom=rect.top+rect2.bottom;
	rect.right=rect.left+rect2.right;

	//draw shadow
	if (plgs.showShadow) {
		logmsg("DrawMe::showShadow");
		if (plgs.salign>=1 && plgs.salign<=3) syo=-plgs.distance;
		if (plgs.salign>=4 && plgs.salign<=6) syo=0;
		if (plgs.salign>=7 && plgs.salign<=9) syo=plgs.distance;

		if (((plgs.salign-1)%3)==0) sxo=-plgs.distance;
		else if (((plgs.salign-2)%3)==0) sxo=0;
		else if (((plgs.salign-3)%3)==0) sxo=plgs.distance;

		SetTextColor(hdc, plgs.clr_shadow);
		if (plgs.altShadow==0) {
			rect2=rect;
			OffsetRect(&rect, sxo, syo);

			DrawText(hdc, string, -1, &rect2, DT_WORDBREAK|talign);
		}
		else {
			rect2 = rect;
			rect2.left += plgs.distance;
			DrawText(hdc, string, -1, &rect2, DT_WORDBREAK|talign);

			rect2 = rect;
			rect2.left -= plgs.distance;
			DrawText(hdc, string, -1, &rect2, DT_WORDBREAK|talign);

			rect2 = rect;
			rect2.top -= plgs.distance;
			DrawText(hdc, string, -1, &rect2, DT_WORDBREAK|talign);

			rect2 = rect;
			rect2.top += plgs.distance;
			DrawText(hdc, string, -1, &rect2, DT_WORDBREAK|talign);

			OffsetRect(&rect, sxo/2, syo/2);
		}
	}

	//draw text
	SetTextColor(hdc, color);
	DrawText(hdc, string, -1, &rect, DT_WORDBREAK);
	
	SelectObject(hdc, oo);
	DeleteObject(fh);
	EndPaint(hwnd, &ps);

	return 0;
}

LRESULT	CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	osdmsg* ms;

	switch (message) {
	case WM_CREATE:
		logmsg("WindowProcedure::CREATE");
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)&defstr);
		return 0;

	case WM_DESTROY:
		logmsg("WindowProcedure::DESTROY");
		return 0;

	case WM_PAINT:
		logmsg("WindowProcedure::PAINT");

		ms=(osdmsg*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (ms)
			return DrawMe(hwnd, ms->text, ms->color);
		
		PAINTSTRUCT ps;
		BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		return 0;

	case WM_NCRBUTTONDOWN:
		logmsg("WindowProcedure::NCRBUTTONDOWN");

		ms = (osdmsg*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if ( ms ) {
			if ( ms->callback) ms->callback(ms->param);
			SendMessage(hwnd, WM_USER+3, 0, 0);
		}
		return 0;

	case WM_TIMER:
		logmsg("WindowProcedure::TIMER");
		SendMessage(hwnd, WM_USER+3, wParam, 0);
		return 0;

	case WM_USER+1: //draw text ((char *)string, (int) timeout
		logmsg("WindowProcedure::USER+1");

		ms=(osdmsg*)mir_alloc(sizeof(osdmsg));
		ms->text = mir_tstrdup((TCHAR *)wParam );
		if ( lParam == 0 )
			lParam = db_get_dw( NULL,THIS_MODULE, "timeout", DEFAULT_TIMEOUT );
		ms->timeout = lParam;
		ms->callback = 0;
		ms->color = db_get_dw(NULL,THIS_MODULE, "clr_msg", DEFAULT_CLRMSG);
		ms->param = 0;
		SendMessage(hwnd, WM_USER+4, (WPARAM)ms, 0);
		mir_free(ms->text);
		mir_free(ms);
		return 0;

	case WM_USER+2: //show
		logmsg("WindowProcedure::USER+2");
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW|SWP_NOACTIVATE);
		return 0;

	case WM_USER+3: //hide
		ms = ( osdmsg* )GetWindowLongPtr(hwnd, GWLP_USERDATA);
		logmsg("WindowProcedure::USER+3");
		if ( !ms )
			return 0;

		logmsg("WindowProcedure::USER+3/om");
		KillTimer(hwnd, (UINT)ms);
		mir_free(ms->text);
		mir_free(ms);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
		ShowWindow(hwnd, SW_HIDE);
		return 0;

	case WM_USER+4:
		logmsg("WindowProcedure::USER+4");

		ms = (osdmsg*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if ( ms != 0 ) {
			logmsg("WindowProcedure::USER+4/old");
			KillTimer(hwnd, (UINT)ms);
			mir_free(ms->text);
			mir_free(ms);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
		}

		ms =(osdmsg*)mir_alloc(sizeof(osdmsg));
		memcpy(ms, (osdmsg*)wParam, sizeof(osdmsg));
		ms->text = mir_tstrdup( ms->text );

		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)ms);
		SetTimer(hwnd, (UINT)ms, (int)ms->timeout, 0);
		InvalidateRect(hwnd, 0, TRUE);
		SendMessage(hwnd, WM_USER+2, 0, 0);
		return 0;

	case WM_NCHITTEST:
		{
			RECT rect;
			GetWindowRect(hwnd, &rect);
			
			logmsg("WindowProcedure::NCHITTEST");

			if (LOWORD(lParam)>=(rect.left+5) && LOWORD(lParam)<=(rect.right-5) && 
				HIWORD(lParam)>=(rect.top+5) && HIWORD(lParam)<=(rect.bottom-5))
				return HTCAPTION;
			return DefWindowProc (hwnd,	message, wParam, lParam);
		}
	//here will be the doubleclick => open-message-window solution ;-)
	//case WM_NCLBUTTONDBLCLK:
	//	CallService(MS_MSG_SENDMESSAGE, wparam,(LPARAM)&odp);
	//	return 0;
	default:
		return DefWindowProc (hwnd,	message, wParam, lParam);
	}

	return 0;
}

INT_PTR OSDAnnounce(WPARAM wParam, LPARAM lParam)
{
	logmsg("OSDAnnounce");
	return SendMessage(hwnd, WM_USER+1, wParam, lParam);
}

int pluginShutDown(WPARAM wparam,LPARAM lparam)
{
	logmsg("pluginShutDown");
	if (hwnd) {
		logmsg("pluginShutDown/hwnd");
		SendMessage(hwnd, WM_USER+3, 0, 0);
		DestroyWindow(hwnd);
		hwnd=0;
	}
	return 0;
}

int MainInit(WPARAM wparam,LPARAM lparam)
{
	logmsg("MainInit");
	HookEvent(ME_OPT_INITIALISE,OptionsInit);
	
	WNDCLASSEX wincl;
	wincl.hInstance	= hI;
	wincl.lpszClassName	= szClassName;
	wincl.lpfnWndProc =	WindowProcedure;
	wincl.style	= CS_DBLCLKS;
	wincl.cbSize = sizeof (WNDCLASSEX);

	wincl.hIcon	= LoadIcon (NULL, IDI_APPLICATION);
	wincl.hIconSm =	LoadIcon (NULL,	IDI_APPLICATION);
	wincl.hCursor =	LoadCursor (NULL, IDC_ARROW);
	wincl.lpszMenuName = NULL;
	wincl.cbClsExtra = 0;
	wincl.cbWndExtra = 0;
	wincl.hbrBackground	= 0;

	if ( !RegisterClassEx( &wincl ))
		return 0;

	hwnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW, szClassName, _T("WannaBeOSD"),
		WS_POPUP,
		db_get_dw(NULL,THIS_MODULE, "winxpos", DEFAULT_WINXPOS),
		db_get_dw(NULL,THIS_MODULE, "winypos", DEFAULT_WINYPOS),
		db_get_dw(NULL,THIS_MODULE, "winx", DEFAULT_WINX),
		db_get_dw(NULL,THIS_MODULE, "winy", DEFAULT_WINY),
		HWND_DESKTOP, NULL, hI, NULL);

	SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);

	SetLayeredWindowAttributes(hwnd, db_get_dw(NULL,THIS_MODULE, "bkclr", DEFAULT_BKCLR), db_get_b(NULL,THIS_MODULE, "alpha", DEFAULT_ALPHA), (db_get_b(NULL,THIS_MODULE, "transparent", DEFAULT_TRANPARENT)?LWA_COLORKEY:0)|LWA_ALPHA);

	hservosda=CreateServiceFunction("OSD/Announce", OSDAnnounce);
	
	hHookedNewEvent = HookEvent(ME_DB_EVENT_ADDED, HookedNewEvent);
	
	// try to create ME_STATUSCHANGE_CONTACTSTATUSCHANGED event... I hope it fails when newstatusnotify or equal creates it before ;-)
	
	hContactStatusChanged = HookEvent(ME_STATUSCHANGE_CONTACTSTATUSCHANGED,ContactStatusChanged);
	if (!hContactStatusChanged) {
		hHookContactStatusChanged = CreateHookableEvent(ME_STATUSCHANGE_CONTACTSTATUSCHANGED);
		hContactSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED,ContactSettingChanged);

		hContactStatusChanged = HookEvent(ME_STATUSCHANGE_CONTACTSTATUSCHANGED,ContactStatusChanged);
	}
	hProtoAck=HookEvent(ME_PROTO_ACK,ProtoAck);

	hpluginShutDown=HookEvent(ME_SYSTEM_SHUTDOWN,pluginShutDown);

	return 0;
}
