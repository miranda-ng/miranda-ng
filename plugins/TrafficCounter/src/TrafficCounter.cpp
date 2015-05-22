/*
Traffic Counter plugin for Miranda IM 
Copyright 2007-2012 Mironych.

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

#include "commonheaders.h"

/*-------------------------------------------------------------------------------------------------------------------*/
//GLOBAL
/*-------------------------------------------------------------------------------------------------------------------*/
uTCFLAGS unOptions;
PROTOLIST *ProtoList; // ������ ��� ���� ���������.
PROTOLIST OverallInfo; // ��������� ������ �� ������� ���������.
int NumberOfAccounts;
HWND TrafficHwnd;

HINSTANCE hInst;

int hLangpack = 0; // ��������� ����������������� ��������.
BOOL bPopupExists = FALSE, bVariablesExists = FALSE, bTooltipExists = FALSE;


TCHAR* TRAFFIC_COUNTER_WINDOW_CLASS = _T("TrafficCounterWnd");

/*-------------------------------------------------------------------------------------------------------------------*/
//TRAFFIC COUNTER
/*-------------------------------------------------------------------------------------------------------------------*/

WORD notify_send_size = 0;
WORD notify_recv_size = 0;
//
// ���� ������ � ����
COLORREF Traffic_BkColor,Traffic_FontColor;

//notify
int  Traffic_PopupBkColor;
int  Traffic_PopupFontColor;
char Traffic_Notify_time_value;
int  Traffic_Notify_size_value;
char Traffic_PopupTimeoutDefault;
char Traffic_PopupTimeoutValue;

unsigned short int Traffic_LineHeight;
//
TCHAR Traffic_CounterFormat[512];
TCHAR Traffic_TooltipFormat[512];
//
HANDLE Traffic_FrameID = NULL;

char Traffic_AdditionSpace;

HFONT Traffic_h_font = NULL;
HMENU TrafficPopupMenu = NULL;
HGENMENU hTrafficMainMenuItem = NULL;

/*-------------------------------------------------------------------------------------------------------------------*/
//TIME COUNTER
/*-------------------------------------------------------------------------------------------------------------------*/
BYTE online_count = 0;

/*-------------------------------------------------------------------------------------------------------------------*/
//font service support
/*-------------------------------------------------------------------------------------------------------------------*/
int TrafficFontHeight = 0;
FontIDT TrafficFontID;
ColourIDT TrafficBackgroundColorID;

//---------------------------------------------------------------------------------------------
// ��� ToolTip
//---------------------------------------------------------------------------------------------
BOOL TooltipShowing;
POINT TooltipPosition;

// ��������������� ���������� ����� ��������� �������� �������� ����
BOOL UseKeyColor;
COLORREF KeyColor;

//---------------------------------------------------------------------------------------------

PLUGININFOEX pluginInfoEx = 
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {82181510-5DFA-49D7-B469-33871E2AE8B5}
	{0x82181510, 0x5dfa, 0x49d7, {0xb4, 0x69, 0x33, 0x87, 0x1e, 0x2a, 0xe8, 0xb5}}
};

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
    return &pluginInfoEx;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	DisableThreadLibraryCalls(hInst);

	return TRUE;
}

extern "C" int __declspec(dllexport) Load(void)
{
	// �������� ���������� ��������� ������.
	mir_getLP(&pluginInfoEx);
	
	HookEvent(ME_OPT_INITIALISE,TrafficCounterOptInitialise);
	HookEvent(ME_SYSTEM_MODULESLOADED,TrafficCounterModulesLoaded);
	HookEvent(ME_PROTO_ACK,ProtocolAckHook);
	HookEvent(ME_PROTO_ACCLISTCHANGED, OnAccountsListChange);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, TrafficCounterShutdown);
	HookEvent(ME_SYSTEM_MODULELOAD, ModuleLoad);
	HookEvent(ME_SYSTEM_MODULEUNLOAD, ModuleLoad);
	
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	// ������� �����.
	if (Traffic_h_font) {
		DeleteObject(Traffic_h_font);
		Traffic_h_font = NULL;
	}

	// ������� ��� ������� ������.
	DestroyProtocolList();
	return 0;
}

int TrafficCounterShutdown(WPARAM wParam, LPARAM lParam)
{
	KillTimer(TrafficHwnd, TIMER_REDRAW);
	KillTimer(TrafficHwnd, TIMER_NOTIFY_TICK);

	SaveSettings(0);

	// ������� ����� �������� ����.
	if (hTrafficMainMenuItem)
	{
		CallService(MO_REMOVEMENUITEM, (WPARAM)hTrafficMainMenuItem, 0);
		hTrafficMainMenuItem = NULL;
	}
	// ������� ����������� ����.
	if (TrafficPopupMenu)
	{
		DestroyMenu(TrafficPopupMenu);
		TrafficPopupMenu = NULL;
	}
	// ��������������� ��������� ��������� ������.
	CallService(MS_SKINENG_REGISTERPAINTSUB, (WPARAM)TrafficHwnd, (LPARAM)NULL);

	// ������� �����.
	if( (ServiceExists(MS_CLIST_FRAMES_REMOVEFRAME)) && Traffic_FrameID )
	{
		CallService(MS_CLIST_FRAMES_REMOVEFRAME, (WPARAM)Traffic_FrameID, 0);
		Traffic_FrameID = NULL;
	}
	return 0;
}

int ModuleLoad(WPARAM wParam, LPARAM lParam)
{
	bPopupExists = ServiceExists(MS_POPUP_ADDPOPUPT);
	bVariablesExists = ServiceExists(MS_VARS_FORMATSTRING) && ServiceExists(MS_VARS_REGISTERTOKEN);
	bTooltipExists = ServiceExists("mToolTip/ShowTipW") || ServiceExists("mToolTip/ShowTip");
	return 0;
}

int TrafficCounterModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;

	CreateProtocolList();
	ModuleLoad(0, 0);

	// ������ �����
	unOptions.Flags = db_get_dw(NULL, TRAFFIC_SETTINGS_GROUP, SETTINGS_WHAT_DRAW, 0x0882);
	Stat_SelAcc = db_get_w(NULL, TRAFFIC_SETTINGS_GROUP, SETTINGS_STAT_ACC_OPT, 0x01);

	//settings for notification
	Traffic_PopupBkColor = db_get_dw(NULL,TRAFFIC_SETTINGS_GROUP,SETTINGS_POPUP_BKCOLOR,RGB(200,255,200));
	Traffic_PopupFontColor = db_get_dw(NULL,TRAFFIC_SETTINGS_GROUP,SETTINGS_POPUP_FONTCOLOR,RGB(0,0,0));
	//
	Traffic_Notify_time_value = db_get_b(NULL,TRAFFIC_SETTINGS_GROUP,SETTINGS_POPUP_NOTIFY_TIME_VALUE,10);
	//
	Traffic_Notify_size_value = db_get_w(NULL,TRAFFIC_SETTINGS_GROUP,SETTINGS_POPUP_NOTIFY_SIZE_VALUE,100);
	//
	//popup timeout
	Traffic_PopupTimeoutDefault = db_get_b(NULL,TRAFFIC_SETTINGS_GROUP,SETTINGS_POPUP_TIMEOUT_DEFAULT,1);
	Traffic_PopupTimeoutValue = db_get_b(NULL,TRAFFIC_SETTINGS_GROUP,SETTINGS_POPUP_TIMEOUT_VALUE,5);
	
	// ������ �������� ��� ������� ��������� ���������
	if (db_get_ts(NULL, TRAFFIC_SETTINGS_GROUP, SETTINGS_COUNTER_FORMAT, &dbv) == 0)
	{
		if(mir_tstrlen(dbv.ptszVal) > 0)
			mir_tstrncpy(Traffic_CounterFormat, dbv.ptszVal, SIZEOF(Traffic_CounterFormat));
		//
		db_free(&dbv);
	}
	else //defaults here
	{
		_tcscpy(Traffic_CounterFormat, _T("{I4}\x0D\x0A\x0A\
{R65}?tc_GetTraffic(%extratext%,now,sent,d)\x0D\x0A\x0A\
{R115}?tc_GetTraffic(%extratext%,now,received,d)\x0D\x0A\x0A\
{R165}?tc_GetTraffic(%extratext%,total,both,d)\x0D\x0A\x0A\
{L180}?if3(?tc_GetTime(%extratext%,now,hh:mm:ss),)\x0D\x0A\x0A\
{L230}?if3(?tc_GetTime(%extratext%,total,d hh:mm),)"));
	}

	// ������ ����������� ���������
	if (db_get_ts(NULL, TRAFFIC_SETTINGS_GROUP, SETTINGS_TOOLTIP_FORMAT, &dbv) == 0)
	{
		if(mir_tstrlen(dbv.ptszVal) > 0)
			mir_tstrncpy(Traffic_TooltipFormat, dbv.ptszVal, SIZEOF(Traffic_TooltipFormat));
		//
		db_free(&dbv);
	}
	else //defaults here
	{
		_tcscpy(Traffic_TooltipFormat, _T("Traffic Counter"));
	}

	Traffic_AdditionSpace = db_get_b(NULL, TRAFFIC_SETTINGS_GROUP, SETTINGS_ADDITION_SPACE, 0);

	// ������� ������� �������
	OverallInfo.Total.Timer = db_get_dw(NULL, TRAFFIC_SETTINGS_GROUP, SETTINGS_TOTAL_ONLINE_TIME, 0);

	//register traffic font
	TrafficFontID.cbSize = sizeof(FontIDT);
	_tcscpy(TrafficFontID.group, LPGENT("Traffic counter"));
	_tcscpy(TrafficFontID.name, LPGENT("Font"));
	strcpy(TrafficFontID.dbSettingsGroup, TRAFFIC_SETTINGS_GROUP);
	strcpy(TrafficFontID.prefix, "Font");
	TrafficFontID.flags = FIDF_DEFAULTVALID | FIDF_SAVEPOINTSIZE;
	TrafficFontID.deffontsettings.charset = DEFAULT_CHARSET;
	TrafficFontID.deffontsettings.colour = GetSysColor(COLOR_BTNTEXT);
	TrafficFontID.deffontsettings.size = 12;
	TrafficFontID.deffontsettings.style = 0;
	_tcscpy(TrafficFontID.deffontsettings.szFace, _T("Arial"));
	TrafficFontID.order = 0;
	FontRegisterT(&TrafficFontID);

	// ������������ ���� ����
	TrafficBackgroundColorID.cbSize = sizeof(ColourIDT);
	_tcscpy(TrafficBackgroundColorID.group, LPGENT("Traffic counter"));
	_tcscpy(TrafficBackgroundColorID.name, LPGENT("Font"));
	strcpy(TrafficBackgroundColorID.dbSettingsGroup, TRAFFIC_SETTINGS_GROUP);
	strcpy(TrafficBackgroundColorID.setting, "FontBkColor");
	TrafficBackgroundColorID.defcolour = GetSysColor(COLOR_BTNFACE);
	ColourRegisterT(&TrafficBackgroundColorID);

	HookEvent(ME_FONT_RELOAD, UpdateFonts);

	// ��������� ��������� ������� Variables
	RegisterVariablesTokens();

	CreateServiceFunction("TrafficCounter/ShowHide", MenuCommand_TrafficShowHide);
	// ������������ ������� ������� ��� ������/������� ������
	{
		HOTKEYDESC hkd = {0};
		hkd.cbSize = sizeof(hkd);
		hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT,'T');
		hkd.pszSection = "Traffic Counter";
		hkd.pszDescription = LPGEN("Show/Hide frame");
		hkd.pszName = "TC_Show_Hide";
		hkd.pszService = "TrafficCounter/ShowHide";
		Hotkey_Register(&hkd);
	}
	
	// ��������� ����� � ������� ����.
	if (unOptions.ShowMainMenuItem)
		Traffic_AddMainMenuItem();

	// ������ ����������� ����.
	if (TrafficPopupMenu = CreatePopupMenu())
	{
		AppendMenu(TrafficPopupMenu,MF_STRING,POPUPMENU_HIDE,TranslateT("Hide traffic window"));
		AppendMenu(TrafficPopupMenu,MF_STRING,POPUPMENU_CLEAR_NOW,TranslateT("Clear the current (Now:) value"));
	}

	// ������������ ����������� ������� Netlib
	HookEvent(ME_NETLIB_FASTRECV, TrafficRecv);
	HookEvent(ME_NETLIB_FASTSEND, TrafficSend);

	CreateTrafficWindow((HWND)CallService(MS_CLUI_GETHWND, 0, 0));
	UpdateFonts(0, 0);	//Load and create fonts here

	return 0;
}

void SaveSettings(BYTE OnlyCnt)
{
	unsigned short int i;

	// ��������� ������� ������� �������
	db_set_dw(NULL, TRAFFIC_SETTINGS_GROUP, SETTINGS_TOTAL_ONLINE_TIME, OverallInfo.Total.Timer);

	if (OnlyCnt) return;

	// ��� ������� ��������� ��������� �����
	for (i = 0; i < NumberOfAccounts; i++)
	{
		if (!ProtoList[i].name) continue;
		db_set_b(NULL, ProtoList[i].name, SETTINGS_PROTO_FLAGS, ProtoList[i].Flags);
	}

	//settings for notification
	db_set_dw(NULL,TRAFFIC_SETTINGS_GROUP,SETTINGS_POPUP_BKCOLOR,Traffic_PopupBkColor);
	db_set_dw(NULL,TRAFFIC_SETTINGS_GROUP,SETTINGS_POPUP_FONTCOLOR,Traffic_PopupFontColor);
	//
	db_set_b(NULL,TRAFFIC_SETTINGS_GROUP,SETTINGS_POPUP_NOTIFY_TIME_VALUE,Traffic_Notify_time_value);
	//
	db_set_w(NULL,TRAFFIC_SETTINGS_GROUP,SETTINGS_POPUP_NOTIFY_SIZE_VALUE,Traffic_Notify_size_value);
	//
	//popup timeout
	db_set_b(NULL,TRAFFIC_SETTINGS_GROUP,SETTINGS_POPUP_TIMEOUT_DEFAULT,Traffic_PopupTimeoutDefault);
	db_set_b(NULL,TRAFFIC_SETTINGS_GROUP,SETTINGS_POPUP_TIMEOUT_VALUE,Traffic_PopupTimeoutValue);
	//
	// ������ ���������
	db_set_ts(NULL, TRAFFIC_SETTINGS_GROUP, SETTINGS_COUNTER_FORMAT, Traffic_CounterFormat);

	db_set_ts(NULL, TRAFFIC_SETTINGS_GROUP, SETTINGS_TOOLTIP_FORMAT, Traffic_TooltipFormat);

	db_set_b(NULL, TRAFFIC_SETTINGS_GROUP, SETTINGS_ADDITION_SPACE, Traffic_AdditionSpace);
	// ��������� �����
	db_set_dw(NULL, TRAFFIC_SETTINGS_GROUP, SETTINGS_WHAT_DRAW, unOptions.Flags);
	db_set_w(NULL, TRAFFIC_SETTINGS_GROUP, SETTINGS_STAT_ACC_OPT, Stat_SelAcc);
}

/*--------------------------------------------------------------------------------------------*/
int TrafficRecv(WPARAM wParam,LPARAM lParam)
{	
	NETLIBNOTIFY *nln = (NETLIBNOTIFY*)wParam;
	NETLIBUSER *nlu = (NETLIBUSER*)lParam;
	int i;
		
	if (nln->result > 0)
		for (i = 0; i < NumberOfAccounts; i++)
			if (!mir_strcmp(ProtoList[i].name, nlu->szSettingsModule)) 
				InterlockedExchangeAdd(&ProtoList[i].AllStatistics[ProtoList[i].NumberOfRecords-1].Incoming, nln->result);
	return 0;
}

int TrafficSend(WPARAM wParam,LPARAM lParam)
{  
	NETLIBNOTIFY *nln = (NETLIBNOTIFY*)wParam;
	NETLIBUSER *nlu = (NETLIBUSER*)lParam;
	int i;
	
	if (nln->result > 0)
		for (i = 0; i < NumberOfAccounts; i++)
			if (!mir_strcmp(ProtoList[i].name, nlu->szSettingsModule))
				InterlockedExchangeAdd(&ProtoList[i].AllStatistics[ProtoList[i].NumberOfRecords-1].Outgoing, nln->result);
	return 0;
}

int TrafficCounter_PaintCallbackProc(HWND hWnd, HDC hDC, RECT * rcPaint, HRGN rgn, DWORD dFlags, void * CallBackData)
{
    return TrafficCounter_Draw(hWnd,hDC);   
}

int TrafficCounter_Draw(HWND hwnd, HDC hDC)
{
  if (hwnd==(HWND)-1) return 0;
  if (GetParent(hwnd) == (HWND)CallService(MS_CLUI_GETHWND, 0, 0))
	  return PaintTrafficCounterWindow(hwnd, hDC);
  else
    InvalidateRect(hwnd,NULL,FALSE);
  return 0;
}

static void TC_AlphaText(HDC hDC, LPCTSTR lpString, RECT* lpRect, UINT format, BYTE ClistModernPresent)
{
	int nCount = mir_tstrlen( lpString );

	if (ClistModernPresent)
		AlphaText(hDC, lpString, nCount, lpRect, format, Traffic_FontColor);
	else 
		DrawText( hDC, lpString, nCount, lpRect, format );
}

static void TC_DrawIconEx( HDC hdc,int xLeft,int yTop,HICON hIcon, HBRUSH hbrFlickerFreeDraw, BYTE ClistModernPresent)
{
	if (ClistModernPresent)
		mod_DrawIconEx_helper( hdc, xLeft, yTop, hIcon, 16, 16, 0, hbrFlickerFreeDraw, DI_NORMAL );
	else 
		DrawIconEx( hdc, xLeft, yTop, hIcon, 16, 16, 0, hbrFlickerFreeDraw, DI_NORMAL );
}

int PaintTrafficCounterWindow(HWND hwnd, HDC hDC)
{
	RECT        rect, rect2;
	HFONT		old_font;
	int			i, dx, height, width;
	HBRUSH		b, t;
	HDC			hdc;
	HBITMAP		hbmp, oldbmp;
	BITMAPINFO  RGB32BitsBITMAPINFO = {0};
	BLENDFUNCTION aga = {AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA};
	DWORD SummarySession, SummaryTotal;

	BYTE ClistModernPresent = (GetModuleHandleA("clist_modern.dll") || GetModuleHandleA("clist_modern_dora.dll"))
			&& !db_get_b(NULL, "ModernData", "DisableEngine", 0)
			&& db_get_b(NULL, "ModernData", "EnableLayering", 1);

	GetClientRect (hwnd, &rect);
	height = rect.bottom - rect.top;
	width = rect.right - rect.left;

	// ���� �������� ����������.
	hdc = CreateCompatibleDC(hDC);
	//
	RGB32BitsBITMAPINFO.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	RGB32BitsBITMAPINFO.bmiHeader.biWidth = width;
	RGB32BitsBITMAPINFO.bmiHeader.biHeight = height;
	RGB32BitsBITMAPINFO.bmiHeader.biPlanes = 1;
	RGB32BitsBITMAPINFO.bmiHeader.biBitCount = 32;
	RGB32BitsBITMAPINFO.bmiHeader.biCompression = BI_RGB;
	hbmp = CreateDIBSection(NULL, 
		&RGB32BitsBITMAPINFO, 
		DIB_RGB_COLORS,
		NULL,
		NULL, 0);
	oldbmp = (HBITMAP)SelectObject(hdc, hbmp);

	b = CreateSolidBrush(Traffic_BkColor);
	t = CreateSolidBrush(KeyColor);
	
	if (   ClistModernPresent
		&& unOptions.DrawFrmAsSkin)
	{
		SKINDRAWREQUEST rq;

		rq.hDC = hdc;
		rq.rcDestRect = rect;
		rq.rcClipRect = rect;  
		strncpy(rq.szObjectID, "Main,ID=EventArea", sizeof(rq.szObjectID));
		if (CallService(MS_SKIN_DRAWGLYPH, (WPARAM)&rq, 0))
		{
			strncpy(rq.szObjectID, "Main,ID=StatusBar", sizeof(rq.szObjectID));
			CallService(MS_SKIN_DRAWGLYPH, (WPARAM)&rq, 0);
		}
	}
	else
	{
		SetTextColor(hdc, Traffic_FontColor);
		SetBkColor(hdc, Traffic_BkColor);
		FillRect(hdc, &rect, b);
		AlphaBlend(hdc, 0, 0, width, height, hdc, 0, 0, width, height, aga);
	}

	old_font = (HFONT)SelectObject(hdc, Traffic_h_font);

	// ������������ ������� ���������
	rect.top += 2;
	rect.left += 3;
	rect.bottom -= 2;
	rect.right -= 3;

//-------------------
// ���� ��� ������� Variables, ������ ���������.
//-------------------
	if (!bVariablesExists)
	{
		SummarySession = SummaryTotal = 0;
		// ��� ������� ��������
		for (i = 0; i < NumberOfAccounts; i++)
		{
			// ������ ���� ��������� ��� �����������.
			if (ProtoList[i].Visible && ProtoList[i].Enabled)
			{
				dx = 0;
				// ���������� ������ ��������.
				if (unOptions.DrawProtoIcon)
				{
					TC_DrawIconEx(hdc, rect.left, rect.top,
						LoadSkinnedProtoIcon(ProtoList[i].name, CallProtoService(ProtoList[i].name,PS_GETSTATUS,0,0)), b, ClistModernPresent);
					dx = 19;
				}
				// ���������� ��� ��������
				if (unOptions.DrawProtoName)
				{
					rect.left += dx;
					TC_AlphaText(hdc, ProtoList[i].tszAccountName, &rect, DT_SINGLELINE | DT_LEFT | DT_TOP, ClistModernPresent);
					rect.left -= dx;
				}
				// ��������� ������� �������� �� ������� ����.
				// ����� �����.
				if (unOptions.DrawTotalTimeCounter)
				{
					TCHAR bu[32];
				
					GetDurationFormatM(ProtoList[i].Total.Timer, _T("h:mm:ss"), bu, 32);
					TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_RIGHT | DT_TOP, ClistModernPresent);
				}
				// ������� �����.
				if (unOptions.DrawCurrentTimeCounter)
				{
					TCHAR bu[32];

					GetDurationFormatM(ProtoList[i].Session.Timer, _T("h:mm:ss"), bu, 32);
					rect.right -= 50;
					TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_RIGHT | DT_TOP, ClistModernPresent);
					rect.right += 50;
				}
				// ���������� ����� ������.
				if (unOptions.DrawTotalTraffic)
				{
					TCHAR bu[32];

					GetFormattedTraffic(ProtoList[i].TotalSentTraffic + ProtoList[i].TotalRecvTraffic, 3, bu, 32);
					rect.right -= 100;
					TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_RIGHT | DT_TOP, ClistModernPresent);
					rect.right += 100;

					if (ProtoList[i].Visible)
						SummaryTotal += ProtoList[i].TotalRecvTraffic + ProtoList[i].TotalSentTraffic;
				}
				// ���������� ������� ������.
				if (unOptions.DrawCurrentTraffic)
				{
					TCHAR bu[32];

					GetFormattedTraffic(ProtoList[i].CurrentRecvTraffic + ProtoList[i].CurrentSentTraffic, 3, bu, 32);
					rect.right -= 150;
					TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_RIGHT | DT_TOP, ClistModernPresent);
					rect.right += 150;

					if (ProtoList[i].Visible)
						SummarySession += ProtoList[i].CurrentRecvTraffic + ProtoList[i].CurrentSentTraffic;
				}

				rect.top += Traffic_LineHeight + Traffic_AdditionSpace;
			}
		}
		// ������ ��������� ������ ��������� ���������.
		if (unOptions.ShowSummary)
		{
			// ���������� ������.
			dx = 0;
			if (unOptions.DrawProtoIcon)
			{
				TC_DrawIconEx(hdc, rect.left, rect.top,
					LoadSkinnedIcon(SKINICON_OTHER_MIRANDA), b, ClistModernPresent);
				dx = 19;
			}
			// ������� �����
			// ���������� ���
			if (unOptions.DrawProtoName)
			{
				TCHAR *bu = mir_a2t("Summary");

				rect.left += dx;
				TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_LEFT | DT_TOP, ClistModernPresent);
				rect.left -= dx;
				mir_free(bu);
			}
			// ����� �����.
			if (unOptions.DrawTotalTimeCounter)
			{
				TCHAR bu[32];
			
				GetDurationFormatM(OverallInfo.Total.Timer, _T("h:mm:ss"), bu, 32);
				TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_RIGHT | DT_TOP, ClistModernPresent);
			}
			// ������� �����.
			if (unOptions.DrawCurrentTimeCounter)
			{
				TCHAR bu[32];

				GetDurationFormatM(OverallInfo.Session.Timer, _T("h:mm:ss"), bu, 32);
				rect.right -= 50;
				TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_RIGHT | DT_TOP, ClistModernPresent);
				rect.right += 50;
			}
			// ���������� ����� ������.
			if (unOptions.DrawTotalTraffic)
			{
				TCHAR bu[32];

				GetFormattedTraffic(SummaryTotal, 3, bu, 32);
				rect.right -= 100;
				TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_RIGHT | DT_TOP, ClistModernPresent);
				rect.right += 100;
			}
			// ���������� ������� ������.
			if (unOptions.DrawCurrentTraffic)
			{
				TCHAR bu[32];

				GetFormattedTraffic(SummarySession, 3, bu, 32);
				rect.right -= 150;
				TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_RIGHT | DT_TOP, ClistModernPresent);
				rect.right += 150;
			}
		}
		rect.top += Traffic_LineHeight + Traffic_AdditionSpace;
		// ������ �������� ������.
		if (unOptions.ShowOverall)
		{
			// ���������� ������.
			dx = 0;
			if (unOptions.DrawProtoIcon)
			{
				TC_DrawIconEx(hdc, rect.left, rect.top,
					LoadSkinnedIcon(SKINICON_OTHER_MIRANDA), b, ClistModernPresent);
				dx = 19;
			}
			// ������� �����
			// ���������� ���
			if (unOptions.DrawProtoName)
			{
				TCHAR *bu = mir_a2t("Overall");

				rect.left += dx;
				TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_LEFT | DT_TOP, ClistModernPresent);
				rect.left -= dx;
				mir_free(bu);
			}
			// ��������� ������� �������� �� ������� ����.
			// ���������� ����� ������.
			if (unOptions.DrawTotalTraffic)
			{
				TCHAR bu[32];

				GetFormattedTraffic(OverallInfo.TotalSentTraffic + OverallInfo.TotalRecvTraffic, 3, bu, 32);
				rect.right -= 100;
				TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_RIGHT | DT_TOP, ClistModernPresent);
				rect.right += 100;
			}
			// ���������� ������� ������.
			if (unOptions.DrawCurrentTraffic)
			{
				TCHAR bu[32];

				GetFormattedTraffic(OverallInfo.CurrentRecvTraffic + OverallInfo.CurrentSentTraffic, 3, bu, 32);
				rect.right -= 150;
				TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_RIGHT | DT_TOP, ClistModernPresent);
				rect.right += 150;
			}
		}
	}
	else
//-------------
// ���� ���� Variables - ������ ��-������
//-------------
	{
		RowItemInfo *ItemsList;
		WORD ItemsNumber, RowsNumber;

		// ������� ������ ����� ��� Variables � ������.
		TCHAR **ExtraText = (TCHAR**)mir_alloc(sizeof(TCHAR*));
		HICON *ahIcon = (HICON*)mir_alloc(sizeof(HICON));
		RowsNumber = 0;
		// ���� �� ���������.
		for (i = 0; i < NumberOfAccounts; i++)
		{
			if (ProtoList[i].Visible && ProtoList[i].Enabled)
			{
				ExtraText = (TCHAR**)mir_realloc(ExtraText, sizeof(TCHAR*) * (RowsNumber + 1));
				ahIcon = (HICON*)mir_realloc(ahIcon, sizeof(HICON) * (RowsNumber + 1));

				ExtraText[RowsNumber] = mir_a2t(ProtoList[i].name);
				ahIcon[RowsNumber++] = LoadSkinnedProtoIcon(ProtoList[i].name, CallProtoService(ProtoList[i].name, PS_GETSTATUS, 0, 0));
			}
		}
		// ��� 2 ������ ��������.
		if (unOptions.ShowSummary)
		{
			ExtraText = (TCHAR**)mir_realloc(ExtraText, sizeof(TCHAR*) * (RowsNumber + 1));
			ahIcon = (HICON*)mir_realloc(ahIcon, sizeof(HICON) * (RowsNumber + 1));

			ExtraText[RowsNumber] = mir_a2t("summary");
			ahIcon[RowsNumber++] = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
		}
		if (unOptions.ShowOverall)
		{
			ExtraText = (TCHAR**)mir_realloc(ExtraText, sizeof(TCHAR*) * (RowsNumber + 1));
			ahIcon = (HICON*)mir_realloc(ahIcon, sizeof(HICON) * (RowsNumber + 1));

			ExtraText[RowsNumber] = mir_a2t("overall");
			ahIcon[RowsNumber++] = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
		}

		// ������ ���� ������� ��� ������� �� ��������� ����������
		for (i = 0; i < RowsNumber; i++)
		{
			TCHAR *buf = variables_parse(Traffic_CounterFormat, ExtraText[i], NULL);
			if (ItemsNumber = GetRowItems(buf, &ItemsList))
			{
				// ������ �����.
				for (dx = 0; dx < ItemsNumber; dx++)
				{
					// ������ ����� �������������� ��� ���������.
					memcpy(&rect2, &rect, sizeof(RECT));
					rect2.bottom = rect2.top + Traffic_LineHeight;
					// ��� �� ���?
					switch (ItemsList[dx].Alignment)
					{
						case 'I':
							TC_DrawIconEx(hdc, ItemsList[dx].Interval, rect2.top, ahIcon[i], b, ClistModernPresent);
							break;
						case 'i':
							TC_DrawIconEx(hdc, width - 16 - ItemsList[dx].Interval, rect2.top, ahIcon[i], b, ClistModernPresent);
							break;
						case 'L':
							rect2.left = ItemsList[dx].Interval;
							TC_AlphaText(hdc, ItemsList[dx].String, &rect2, DT_SINGLELINE | DT_LEFT | DT_VCENTER, ClistModernPresent);
							break;
						case 'R':
							rect2.right = width - ItemsList[dx].Interval;
							TC_AlphaText(hdc, ItemsList[dx].String, &rect2, DT_SINGLELINE | DT_RIGHT | DT_VCENTER, ClistModernPresent);
							break;
						default:
							continue;
					}
					
				}

				// ����� ���������� ������.
				for (; ItemsNumber--; )
				{
					mir_free(ItemsList[ItemsNumber].String);
				}
				mir_free(ItemsList);
			}
			mir_free(buf);
			rect.top += Traffic_LineHeight + Traffic_AdditionSpace;
		}

		// ������� ������ �����.
		for (; RowsNumber--; )
			mir_free(ExtraText[RowsNumber]);
		mir_free(ExtraText);
	}

	DeleteObject(b);
	DeleteObject(t);
	SelectObject(hdc, old_font);

	if (ClistModernPresent)
	{
		AlphaBlend(hDC, 0, 0, width, height, hdc, 0, 0, width, height, aga);
	}
	else
	{
		BitBlt(hDC, 0, 0, width, height, hdc, 0, 0, SRCCOPY);
	}

	SelectObject(hdc, oldbmp);
	DeleteObject(hbmp);
	DeleteDC(hdc);

	return 0;
}

void ProtocolIsOnLine(int num)
{
	DWORD CurrentTimeMs;

	if (ProtoList[num].State) return;

	online_count++;
	CurrentTimeMs = GetTickCount();

	ProtoList[num].State = 1;
	ProtoList[num].Session.TimeAtStart = CurrentTimeMs;
	ProtoList[num].Total.TimeAtStart = CurrentTimeMs
		- 1000 * ProtoList[num].AllStatistics[ProtoList[num].NumberOfRecords - 1].Time;
	ProtoList[num].Session.Timer = 0;

	if (online_count == 1) // ���� �� ���� ������� ������� � ������ - ��������� ������� ��������� �����.
	{
		OverallInfo.Session.TimeAtStart = CurrentTimeMs - 1000 * OverallInfo.Session.Timer;
		OverallInfo.Total.TimeAtStart = CurrentTimeMs - 1000 * OverallInfo.Total.Timer;
	}
}

void ProtocolIsOffLine(int num)
{
	if (!ProtoList[num].State) return;

	online_count--;
	ProtoList[num].State = 0;
}

static POINT ptMouse = {0};

LRESULT CALLBACK TrafficCounterWndProc_MW(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	POINT p;
	int i;
	//
	switch (msg)
    {
		case (WM_USER+697):
			if (lParam == 666)
				DestroyWindow(hwnd);
			break;

		case WM_PAINT:
			{
				if (  !db_get_b(NULL, "ModernData", "DisableEngine", 0)
					&& db_get_b(NULL, "ModernData", "EnableLayering", 1)
					&& ServiceExists(MS_SKINENG_INVALIDATEFRAMEIMAGE))
					CallService(MS_SKINENG_INVALIDATEFRAMEIMAGE, (WPARAM)TrafficHwnd, 0);
				else
				{
					HDC hdc = GetDC(hwnd);
					PaintTrafficCounterWindow(hwnd, hdc);
					ReleaseDC(hwnd, hdc);
				}
			}
			break;

		case WM_ERASEBKGND:
			return 1;

		case WM_LBUTTONDOWN :
			if (db_get_b(NULL,"CLUI","ClientAreaDrag",SETTING_CLIENTDRAG_DEFAULT))
			{
				ClientToScreen(GetParent(hwnd),&p);
				return SendMessage(GetParent(hwnd), WM_SYSCOMMAND, SC_MOVE|HTCAPTION,MAKELPARAM(p.x,p.y));
			}
			break;

		case WM_RBUTTONDOWN :
			p.x = GET_X_LPARAM(lParam);
			p.y = GET_Y_LPARAM(lParam);
			ClientToScreen(hwnd,&p);
			TrackPopupMenu(TrafficPopupMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_VERTICAL,p.x,p.y,0,hwnd,NULL);
			break;

		case WM_COMMAND :
			switch (wParam)
			{
				case POPUPMENU_HIDE:
					MenuCommand_TrafficShowHide(0, 0);
					break;

				case POPUPMENU_CLEAR_NOW:
					for (i = 0; i < NumberOfAccounts; i++)
					{
						ProtoList[i].StartIncoming =
							ProtoList[i].AllStatistics[ProtoList[i].NumberOfRecords - 1].Incoming;
						ProtoList[i].StartOutgoing =
							ProtoList[i].AllStatistics[ProtoList[i].NumberOfRecords - 1].Outgoing;
						ProtoList[i].Session.TimeAtStart = GetTickCount();
					}
					OverallInfo.CurrentRecvTraffic = OverallInfo.CurrentSentTraffic = 0;
			}
			break;

		case WM_SETCURSOR:
			{
				POINT pt;

				GetCursorPos(&pt);
				if ( (abs(pt.x - ptMouse.x) < 20)
			      && (abs(pt.y - ptMouse.y) < 20) )
					return 1;

				if (TooltipShowing)
				{
					KillTimer(TrafficHwnd, TIMER_TOOLTIP);
					CallService("mToolTip/HideTip", 0, 0);
					TooltipShowing = FALSE;
				}
				KillTimer(TrafficHwnd, TIMER_TOOLTIP);
				SetTimer(TrafficHwnd, TIMER_TOOLTIP, CallService(MS_CLC_GETINFOTIPHOVERTIME, 0, 0), 0);
				break;
			}

		case WM_TIMER :
			switch(wParam)
			{
				case TIMER_NOTIFY_TICK:
					NotifyOnRecv();
					NotifyOnSend();
					break;

				case TIMER_REDRAW: // ����������� ��� � ����������.
					{
						DWORD CurrentTimeMs;
						SYSTEMTIME stNow;

						SaveSettings(1);
						// ����� ��������� ���������� ������� �������� ��������.
						OverallInfo.CurrentRecvTraffic =
							OverallInfo.CurrentSentTraffic =
							OverallInfo.TotalRecvTraffic =
							OverallInfo.TotalSentTraffic = 0;

						CurrentTimeMs = GetTickCount();

						for (i = 0; i < NumberOfAccounts; i++)
						{
							if (ProtoList[i].State)
							{
								ProtoList[i].AllStatistics[ProtoList[i].NumberOfRecords-1].Time =
									(CurrentTimeMs - ProtoList[i].Total.TimeAtStart) / 1000;
								ProtoList[i].Session.Timer =
									(CurrentTimeMs - ProtoList[i].Session.TimeAtStart) / 1000;
							}

							Stat_CheckStatistics(i);

							{	// ����� �� ��������� ���������� ����������� �������� ���� �������� � �������.
								DWORD Sum1, Sum2;
								unsigned long int j;

								// �������� ��� ������� ������.
								for (Sum1 = Sum2 = 0, j = ProtoList[i].StartIndex; j < ProtoList[i].NumberOfRecords; j++)
								{
									Sum1 += ProtoList[i].AllStatistics[j].Incoming;
									Sum2 += ProtoList[i].AllStatistics[j].Outgoing;
								}
								ProtoList[i].CurrentRecvTraffic = Sum1 - ProtoList[i].StartIncoming;
								ProtoList[i].CurrentSentTraffic = Sum2 - ProtoList[i].StartOutgoing;
								OverallInfo.CurrentRecvTraffic += ProtoList[i].CurrentRecvTraffic;
								OverallInfo.CurrentSentTraffic += ProtoList[i].CurrentSentTraffic;
								// �������� ��� ���������� �������.
								ProtoList[i].TotalRecvTraffic =
									Stat_GetItemValue(1 << i,
													  unOptions.PeriodForShow + 1,
													  Stat_GetRecordsNumber(i, unOptions.PeriodForShow + 1) - 1,
													  1);
								ProtoList[i].TotalSentTraffic =
									Stat_GetItemValue(1 << i,
													  unOptions.PeriodForShow + 1,
													  Stat_GetRecordsNumber(i, unOptions.PeriodForShow + 1) - 1,
													  2);
								ProtoList[i].Total.Timer =
									Stat_GetItemValue(1 << i,
													  unOptions.PeriodForShow + 1,
													  Stat_GetRecordsNumber(i, unOptions.PeriodForShow + 1) - 1,
													  4);
								OverallInfo.TotalRecvTraffic += ProtoList[i].TotalRecvTraffic;
								OverallInfo.TotalSentTraffic += ProtoList[i].TotalSentTraffic;
							}
						}
						// �� ���� �� ���������?
						if (unOptions.NotifyBySize && Traffic_Notify_size_value)
						{
							if (!((OverallInfo.CurrentRecvTraffic >> 10) % Traffic_Notify_size_value)
								&& notify_recv_size != OverallInfo.CurrentRecvTraffic >> 10 )
									NotifyOnRecv();

							if (!((OverallInfo.CurrentSentTraffic >> 10) % Traffic_Notify_size_value)
								&& notify_send_size != OverallInfo.CurrentSentTraffic >> 10)
									NotifyOnSend();
						}

						// �������� ���������� �������.
						GetLocalTime(&stNow);

						// �� ���� �� �������� ����� �������?
						if (   (unOptions.PeriodForShow == 0
								&& stNow.wHour == 0
								&& stNow.wMinute == 0
								&& stNow.wSecond == 0)
							|| (unOptions.PeriodForShow == 1
								&& DayOfWeek(stNow.wDay, stNow.wMonth,	stNow.wYear) == 1
								&& stNow.wHour == 0
								&& stNow.wMinute == 0
								&& stNow.wSecond == 0)
							|| (unOptions.PeriodForShow == 2
								&& stNow.wDay == 1
								&& stNow.wHour == 0
								&& stNow.wMinute == 0
								&& stNow.wSecond == 0)
							|| (unOptions.PeriodForShow == 3
								&& stNow.wMonth == 1
								&& stNow.wDay == 1
								&& stNow.wHour == 0
								&& stNow.wMinute == 0
								&& stNow.wSecond == 0) )
							OverallInfo.Total.TimeAtStart = CurrentTimeMs;

						if (online_count > 0)
						{
							OverallInfo.Session.Timer =
								(CurrentTimeMs - OverallInfo.Session.TimeAtStart) / 1000;
							OverallInfo.Total.Timer =
								(CurrentTimeMs - OverallInfo.Total.TimeAtStart) / 1000;
						}

						CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)Traffic_FrameID, FU_FMREDRAW);
					}
					break;

				case TIMER_TOOLTIP:
					{
						TCHAR *TooltipText;
						CLCINFOTIP ti = {0};
						RECT rt;

						GetCursorPos(&TooltipPosition);
						if (!TooltipShowing && unOptions.ShowTooltip )
						{
							KillTimer(TrafficHwnd, TIMER_TOOLTIP);
							ScreenToClient(TrafficHwnd, &TooltipPosition);
							GetClientRect(TrafficHwnd, &rt);
							if (PtInRect(&rt, TooltipPosition))
							{
								GetCursorPos(&ptMouse);
								ti.rcItem.left   = TooltipPosition.x - 10;
								ti.rcItem.right  = TooltipPosition.x + 10;
								ti.rcItem.top    = TooltipPosition.y - 10;
								ti.rcItem.bottom = TooltipPosition.y + 10;
								ti.cbSize = sizeof( ti );
								TooltipText = variables_parsedup(Traffic_TooltipFormat, NULL, NULL);

								CallService(MS_TIPPER_SHOWTIPW, (WPARAM)TooltipText, (LPARAM)&ti);

								TooltipShowing = TRUE;
								mir_free(TooltipText);
							}
						}
					}
					break;
			}
			break;

		default:
			return DefWindowProc(hwnd,msg,wParam,lParam); 
	}
	//
	return DefWindowProc(hwnd,msg,wParam,lParam); 
}

void CreateTrafficWindow(HWND hCluiWnd)
{
	WNDCLASSEX wcx = {0};
	CLISTFrame f;

	wcx.cbSize			=	sizeof( WNDCLASSEX );
	wcx.style			=	0;
	wcx.lpfnWndProc		=	TrafficCounterWndProc_MW;
	wcx.cbClsExtra		=	0;
	wcx.cbWndExtra		=	0;
	wcx.hInstance		=	hInst;
	wcx.hIcon			=	NULL;
	wcx.hCursor			=	LoadCursor(hInst,IDC_ARROW);
	wcx.hbrBackground	=	0;
	wcx.lpszMenuName	=	NULL;
	wcx.lpszClassName	=	TRAFFIC_COUNTER_WINDOW_CLASS;
	wcx.hIconSm			=	NULL;
	RegisterClassEx( &wcx );
	TrafficHwnd = CreateWindowEx(WS_EX_TOOLWINDOW, TRAFFIC_COUNTER_WINDOW_CLASS,
								TRAFFIC_COUNTER_WINDOW_CLASS,
								WS_CHILDWINDOW | WS_CLIPCHILDREN,
								0, 0, 0, 0, hCluiWnd, NULL, hInst, NULL);

	if ( ServiceExists(MS_CLIST_FRAMES_ADDFRAME) )
	{
		// ��������� ��������� �����
		memset(&f, 0, sizeof(CLISTFrame));
		f.align = alBottom;
		f.cbSize = sizeof(CLISTFrame);
		f.height = TrafficWindowHeight();
		f.Flags= unOptions.FrameIsVisible | F_LOCKED | F_NOBORDER | F_NO_SUBCONTAINER;
		f.hWnd = TrafficHwnd;
		f.TBname = ("Traffic counter");
		f.name = ("Traffic counter");
		// ������ �����
		Traffic_FrameID = (HANDLE)CallService(MS_CLIST_FRAMES_ADDFRAME,(WPARAM)&f,0);
		CallService(MS_SKINENG_REGISTERPAINTSUB,(WPARAM)f.hWnd,(LPARAM)TrafficCounter_PaintCallbackProc);
	}

	// ������ �������.
	SetTimer(TrafficHwnd, TIMER_REDRAW, 500, NULL);
	UpdateNotifyTimer();
}

INT_PTR MenuCommand_TrafficShowHide(WPARAM wParam,LPARAM lParam)
{
	unOptions.FrameIsVisible = !unOptions.FrameIsVisible;
	if (Traffic_FrameID == NULL)
		ShowWindow(TrafficHwnd, unOptions.FrameIsVisible ? SW_SHOW : SW_HIDE);
	else
		CallService(MS_CLIST_FRAMES_SHFRAME, (WPARAM)Traffic_FrameID, 0);
	db_set_dw(NULL, TRAFFIC_SETTINGS_GROUP, SETTINGS_WHAT_DRAW, unOptions.Flags);
	//
	return 0;
}

void Traffic_AddMainMenuItem(void)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = -0x7FFFFFFF;
	mi.flags = 0;
	mi.hIcon = NULL;
	mi.pszName = LPGEN("Toggle traffic counter");
	mi.pszService="TrafficCounter/ShowHide";

	hTrafficMainMenuItem = Menu_AddMainMenuItem(&mi);
}

/*-------------------------------------------------------------------------------------------------------------------*/
void UpdateNotifyTimer(void)
{
	if (!bPopupExists) return;

	if (Traffic_Notify_time_value && unOptions.NotifyByTime)
		SetTimer(TrafficHwnd, TIMER_NOTIFY_TICK, Traffic_Notify_time_value * 1000 * 60, NULL);
	else
		KillTimer(TrafficHwnd, TIMER_NOTIFY_TICK);
}

void NotifyOnSend(void)
{
	POPUPDATAT ppd;

	memset(&ppd, 0, sizeof(ppd));
	ppd.lchContact = NULL;
	ppd.lchIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
	_tcsncpy(ppd.lptzContactName, TranslateT("Traffic counter notification"), MAX_CONTACTNAME);
	//
	mir_sntprintf(ppd.lptzText, SIZEOF(ppd.lptzText), TranslateT("%d kilobytes sent"),
		notify_send_size = OverallInfo.CurrentSentTraffic >> 10);
	//
	ppd.colorBack = Traffic_PopupBkColor;
	ppd.colorText = Traffic_PopupFontColor;
	ppd.PluginWindowProc = NULL;
	ppd.iSeconds = (Traffic_PopupTimeoutDefault ? 0 : Traffic_PopupTimeoutValue);
	PUAddPopupT(&ppd);
}

void NotifyOnRecv(void)
{
	POPUPDATAT ppd;

	memset(&ppd, 0, sizeof(ppd));
	ppd.lchContact = NULL;
	ppd.lchIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
	_tcsncpy(ppd.lptzContactName, TranslateT("Traffic counter notification"),MAX_CONTACTNAME);
	//
	mir_sntprintf(ppd.lptzText, SIZEOF(ppd.lptzText), TranslateT("%d kilobytes received"),
		notify_recv_size = OverallInfo.CurrentRecvTraffic >> 10);
	//
	ppd.colorBack = Traffic_PopupBkColor;
	ppd.colorText = Traffic_PopupFontColor;
	ppd.PluginWindowProc = NULL;
	ppd.iSeconds = (Traffic_PopupTimeoutDefault ? 0 : Traffic_PopupTimeoutValue);
	PUAddPopupT(&ppd); 
}

void CreateProtocolList(void)
{
	int i;
	PROTOACCOUNT **acc;
	//
	ProtoEnumAccounts(&NumberOfAccounts,&acc);
	//
	ProtoList = (PROTOLIST*)mir_alloc(sizeof(PROTOLIST)*(NumberOfAccounts));
	//
	for (i = 0; i < NumberOfAccounts; i++)
	{
		ProtoList[i].name = (char*)mir_alloc(mir_strlen(acc[i]->szModuleName) + 1);
		strcpy(ProtoList[i].name, acc[i]->szModuleName);
		ProtoList[i].tszAccountName = (TCHAR*)mir_alloc(sizeof(TCHAR) * (1 + mir_tstrlen(acc[i]->tszAccountName)));
		_tcscpy(ProtoList[i].tszAccountName, acc[i]->tszAccountName);
		//
		ProtoList[i].Flags = db_get_b(NULL, ProtoList[i].name, SETTINGS_PROTO_FLAGS, 3);
		ProtoList[i].CurrentRecvTraffic = 
			ProtoList[i].CurrentSentTraffic = 
			ProtoList[i].Session.Timer = 0;
		//
		ProtoList[i].Enabled = acc[i]->bIsEnabled;
		ProtoList[i].State = 0;

		Stat_ReadFile(i);
		ProtoList[i].StartIndex = ProtoList[i].NumberOfRecords - 1;
		ProtoList[i].StartIncoming = ProtoList[i].AllStatistics[ProtoList[i].StartIndex].Incoming;
		ProtoList[i].StartOutgoing = ProtoList[i].AllStatistics[ProtoList[i].StartIndex].Outgoing;
	} // ���� �� ���������
	// ��������� �������� ��� ��������� ����������.
	OverallInfo.Session.Timer =	OverallInfo.Total.Timer = 0;
}

void DestroyProtocolList(void)
{
	for (int i = 0; i < NumberOfAccounts; i++) {
		Stat_CheckStatistics(i);
		CloseHandle(ProtoList[i].hFile);
		mir_free(ProtoList[i].tszAccountName);
		mir_free(ProtoList[i].name);
		mir_free(ProtoList[i].AllStatistics);
	}

	mir_free(ProtoList);
}

int ProtocolAckHook(WPARAM wParam,LPARAM lParam)
{
	ACKDATA* pAck=(ACKDATA*)lParam;
	WORD i;

	if (ACKTYPE_STATUS == pAck->type) 
	{
		if(ID_STATUS_OFFLINE == pAck->lParam) 
		{
			for (i = 0; i < NumberOfAccounts; i++)
			{
				if (!ProtoList[i].name) continue;
				if (!mir_strcmp(ProtoList[i].name, pAck->szModule))
				{
					ProtocolIsOffLine(i);
					break;
				}
			}
		}
		else
		{
			if ((pAck->lParam >= ID_STATUS_ONLINE) && (pAck->lParam <= ID_STATUS_OUTTOLUNCH))
			{
				for (i = 0; i < NumberOfAccounts; i++)
					if (!mir_strcmp(ProtoList[i].name, pAck->szModule))
					{
						ProtocolIsOnLine(i);
						break;
					}
			}
		}
	}
	return 0;
}

int UpdateFonts(WPARAM wParam, LPARAM lParam)
{
	LOGFONT logfont;
	//if no font service
	if (!ServiceExists(MS_FONT_GETT)) return 0;
	//update traffic font
	if (Traffic_h_font) DeleteObject(Traffic_h_font);
	Traffic_FontColor = CallService(MS_FONT_GETT, (WPARAM)&TrafficFontID, (LPARAM)&logfont);
	Traffic_h_font = CreateFontIndirect(&logfont);
	//
	TrafficFontHeight = abs(logfont.lfHeight) + 1;
	Traffic_BkColor = CallService(MS_COLOUR_GETT, (WPARAM)&TrafficBackgroundColorID, 0);

	// �������� ����
	UseKeyColor = db_get_b(NULL, "ModernSettings", "UseKeyColor", 1);
	KeyColor = db_get_dw(NULL, "ModernSettings", "KeyColor", 0);

	UpdateTrafficWindowSize();
	return 0;
}

void UpdateTrafficWindowSize(void)
{
	if (Traffic_FrameID != NULL)
	{
		CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_HEIGHT, Traffic_FrameID), TrafficWindowHeight());
	}
}

unsigned short int TrafficWindowHeight(void)
{
	BYTE HeightLineTime = (unOptions.DrawProtoIcon) ? 16 : TrafficFontHeight,
		 i, ActProto;
	WORD MaxWndHeight;

	for (i = 0, ActProto = unOptions.ShowOverall + unOptions.ShowSummary; i < NumberOfAccounts; i++)
		ActProto += ProtoList[i].Visible && ProtoList[i].Enabled;

	// ������ ������ ������� 16 �������� (��� ������).
	Traffic_LineHeight = TrafficFontHeight > 16 ? TrafficFontHeight: 16;

	// ������ ������ ����� ���������� �����.
	MaxWndHeight = ActProto * Traffic_LineHeight
		+ Traffic_AdditionSpace * (ActProto - 1)
		+ 4;

	return (MaxWndHeight < TrafficFontHeight) ? 0 : MaxWndHeight;
}

// ������� ������ ��������� � ProtoList ��� ���������� ���������
int OnAccountsListChange(WPARAM wParam, LPARAM lParam)
{
	BYTE i;
	PROTOACCOUNT *acc = (PROTOACCOUNT*)lParam;
	
	switch (wParam)
	{
		case PRAC_ADDED:
		case PRAC_REMOVED:
			DestroyProtocolList();
			CreateProtocolList();
			break;
		case PRAC_CHANGED:
		case PRAC_CHECKED:
			for (i = 0; i < NumberOfAccounts; i++)
				if (!mir_strcmp(acc->szModuleName, ProtoList[i].name))
					ProtoList[i].Enabled = acc->bIsEnabled;
			break;
	}
	UpdateTrafficWindowSize();
	return 0;
}
