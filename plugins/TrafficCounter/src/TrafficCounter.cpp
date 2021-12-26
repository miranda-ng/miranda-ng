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

#include "stdafx.h"

/*-------------------------------------------------------------------------------------------------------------------*/
//GLOBAL
/*-------------------------------------------------------------------------------------------------------------------*/
uTCFLAGS unOptions;
PROTOLIST *ProtoList; // Данные обо всех аккаунтах.
PROTOLIST OverallInfo; // Суммарные данные по видимым аккаунтам.
int NumberOfAccounts;
HWND TrafficHwnd;

CMPlugin g_plugin;

BOOL bVariablesExists = FALSE, bTooltipExists = FALSE;

static wchar_t wszDefaultFormat[] =
_A2W("{I4}\x0D\x0A\x0A\
{R65}?tc_GetTraffic(%extratext%,now,sent,d)\x0D\x0A\x0A\
{R115}?tc_GetTraffic(%extratext%,now,received,d)\x0D\x0A\x0A\
{R165}?tc_GetTraffic(%extratext%,total,both,d)\x0D\x0A\x0A\
{L180}?if3(?tc_GetTime(%extratext%,now,hh:mm:ss),)\x0D\x0A\x0A\
{L230}?if3(?tc_GetTime(%extratext%,total,d hh:mm),)");

wchar_t *TRAFFIC_COUNTER_WINDOW_CLASS = L"TrafficCounterWnd";

/*-------------------------------------------------------------------------------------------------------------------*/
//TRAFFIC COUNTER
/*-------------------------------------------------------------------------------------------------------------------*/

uint16_t notify_send_size = 0;
uint16_t notify_recv_size = 0;
//
// Цвет шрифта и фона
COLORREF Traffic_BkColor, Traffic_FontColor;

//notify
int  Traffic_PopupBkColor;
int  Traffic_PopupFontColor;
char Traffic_Notify_time_value;
int  Traffic_Notify_size_value;
char Traffic_PopupTimeoutDefault;
char Traffic_PopupTimeoutValue;

unsigned short int Traffic_LineHeight;
//
wchar_t Traffic_CounterFormat[512];
wchar_t Traffic_TooltipFormat[512];
//
int Traffic_FrameID = 0;

char Traffic_AdditionSpace;

HFONT Traffic_h_font = nullptr;
HMENU TrafficPopupMenu = nullptr;
HGENMENU hTrafficMainMenuItem = nullptr;

/*-------------------------------------------------------------------------------------------------------------------*/
//TIME COUNTER
/*-------------------------------------------------------------------------------------------------------------------*/
uint8_t online_count = 0;

/*-------------------------------------------------------------------------------------------------------------------*/
//font service support
/*-------------------------------------------------------------------------------------------------------------------*/
int TrafficFontHeight = 0;
FontIDW TrafficFontID;
ColourIDW TrafficBackgroundColorID;

//---------------------------------------------------------------------------------------------
// Для ToolTip
//---------------------------------------------------------------------------------------------
BOOL TooltipShowing;
POINT TooltipPosition;

// Вспомогательные переменные чтобы заставить работать ключевой цвет
BOOL UseKeyColor;
COLORREF KeyColor;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {82181510-5DFA-49D7-B469-33871E2AE8B5}
	{0x82181510, 0x5dfa, 0x49d7, {0xb4, 0x69, 0x33, 0x87, 0x1e, 0x2a, 0xe8, 0xb5}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

int TrafficCounterShutdown(WPARAM, LPARAM)
{
	KillTimer(TrafficHwnd, TIMER_REDRAW);
	KillTimer(TrafficHwnd, TIMER_NOTIFY_TICK);

	SaveSettings(0);

	// Удаляем пункт главного меню.
	if (hTrafficMainMenuItem) {
		Menu_RemoveItem(hTrafficMainMenuItem);
		hTrafficMainMenuItem = nullptr;
	}
	// Удаляем контекстное меню.
	if (TrafficPopupMenu) {
		DestroyMenu(TrafficPopupMenu);
		TrafficPopupMenu = nullptr;
	}
	// Разрегистрируем процедуру отрисовки фрейма.
	CallService(MS_SKINENG_REGISTERPAINTSUB, (WPARAM)TrafficHwnd, (LPARAM)NULL);

	// Удаляем фрейм.
	if ((ServiceExists(MS_CLIST_FRAMES_REMOVEFRAME)) && Traffic_FrameID) {
		CallService(MS_CLIST_FRAMES_REMOVEFRAME, Traffic_FrameID, 0);
		Traffic_FrameID = 0;
	}
	return 0;
}

int ModuleLoad(WPARAM, LPARAM)
{
	bVariablesExists = ServiceExists(MS_VARS_FORMATSTRING) && ServiceExists(MS_VARS_REGISTERTOKEN);
	bTooltipExists = ServiceExists("mToolTip/ShowTipW") || ServiceExists("mToolTip/ShowTip");
	return 0;
}

void SaveSettings(uint8_t OnlyCnt)
{
	unsigned short int i;

	// Сохраняем счётчик времени онлайна
	g_plugin.setDword(SETTINGS_TOTAL_ONLINE_TIME, OverallInfo.Total.Timer);

	if (OnlyCnt) return;

	// Для каждого протокола сохраняем флаги
	for (i = 0; i < NumberOfAccounts; i++) {
		if (!ProtoList[i].name) continue;
		db_set_b(0, ProtoList[i].name, SETTINGS_PROTO_FLAGS, ProtoList[i].Flags);
	}

	//settings for notification
	g_plugin.setDword(SETTINGS_POPUP_BKCOLOR, Traffic_PopupBkColor);
	g_plugin.setDword(SETTINGS_POPUP_FONTCOLOR, Traffic_PopupFontColor);
	//
	g_plugin.setByte(SETTINGS_POPUP_NOTIFY_TIME_VALUE, Traffic_Notify_time_value);
	//
	g_plugin.setWord(SETTINGS_POPUP_NOTIFY_SIZE_VALUE, Traffic_Notify_size_value);
	//
	//popup timeout
	g_plugin.setByte(SETTINGS_POPUP_TIMEOUT_DEFAULT, Traffic_PopupTimeoutDefault);
	g_plugin.setByte(SETTINGS_POPUP_TIMEOUT_VALUE, Traffic_PopupTimeoutValue);
	//
	// Формат счётчиков
	g_plugin.setWString(SETTINGS_COUNTER_FORMAT, Traffic_CounterFormat);

	g_plugin.setWString(SETTINGS_TOOLTIP_FORMAT, Traffic_TooltipFormat);

	g_plugin.setByte(SETTINGS_ADDITION_SPACE, Traffic_AdditionSpace);
	// Сохраняем флаги
	g_plugin.setDword(SETTINGS_WHAT_DRAW, unOptions.Flags);
	g_plugin.setWord(SETTINGS_STAT_ACC_OPT, Stat_SelAcc);
}

/*--------------------------------------------------------------------------------------------*/
int TrafficRecv(WPARAM wParam, LPARAM lParam)
{
	NETLIBNOTIFY *nln = (NETLIBNOTIFY*)wParam;
	NETLIBUSER *nlu = (NETLIBUSER*)lParam;

	if (nln->result > 0)
		for (int i = 0; i < NumberOfAccounts; i++)
			if (!mir_strcmp(ProtoList[i].name, nlu->szSettingsModule))
				InterlockedExchangeAdd(&ProtoList[i].AllStatistics[ProtoList[i].NumberOfRecords - 1].Incoming, nln->result);
	return 0;
}

int TrafficSend(WPARAM wParam, LPARAM lParam)
{
	NETLIBNOTIFY *nln = (NETLIBNOTIFY*)wParam;
	NETLIBUSER *nlu = (NETLIBUSER*)lParam;

	if (nln->result > 0)
		for (int i = 0; i < NumberOfAccounts; i++)
			if (!mir_strcmp(ProtoList[i].name, nlu->szSettingsModule))
				InterlockedExchangeAdd(&ProtoList[i].AllStatistics[ProtoList[i].NumberOfRecords - 1].Outgoing, nln->result);
	return 0;
}

int TrafficCounter_PaintCallbackProc(HWND hWnd, HDC hDC, RECT*, HRGN, uint32_t, void*)
{
	return TrafficCounter_Draw(hWnd, hDC);
}

int TrafficCounter_Draw(HWND hwnd, HDC hDC)
{
	if (hwnd == (HWND)-1) return 0;
	if (GetParent(hwnd) == g_clistApi.hwndContactList)
		return PaintTrafficCounterWindow(hwnd, hDC);
	else
		InvalidateRect(hwnd, nullptr, FALSE);
	return 0;
}

static void TC_AlphaText(HDC hDC, LPCTSTR lpString, RECT* lpRect, UINT format, uint8_t ClistModernPresent)
{
	int nCount = (int)mir_wstrlen(lpString);

	if (ClistModernPresent)
		AlphaText(hDC, lpString, nCount, lpRect, format, Traffic_FontColor);
	else
		DrawText(hDC, lpString, nCount, lpRect, format);
}

static void TC_DrawIconEx(HDC hdc, int xLeft, int yTop, HICON hIcon, HBRUSH hbrFlickerFreeDraw, uint8_t ClistModernPresent)
{
	if (ClistModernPresent)
		mod_DrawIconEx_helper(hdc, xLeft, yTop, hIcon, 16, 16, 0, hbrFlickerFreeDraw, DI_NORMAL);
	else
		DrawIconEx(hdc, xLeft, yTop, hIcon, 16, 16, 0, hbrFlickerFreeDraw, DI_NORMAL);
}

int PaintTrafficCounterWindow(HWND hwnd, HDC hDC)
{
	RECT        rect, rect2;
	BLENDFUNCTION aga = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA };
	uint32_t SummarySession, SummaryTotal;

	uint8_t ClistModernPresent = (GetModuleHandleA("clist_modern.dll") || GetModuleHandleA("clist_modern_dora.dll"))
		&& !db_get_b(0, "ModernData", "DisableEngine", 0)
		&& db_get_b(0, "ModernData", "EnableLayering", 1);

	GetClientRect(hwnd, &rect);
	int height = rect.bottom - rect.top;
	int width = rect.right - rect.left;

	// Свой контекст устройства.
	HDC hdc = CreateCompatibleDC(hDC);

	BITMAPINFO RGB32BitsBITMAPINFO = { 0 };
	RGB32BitsBITMAPINFO.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	RGB32BitsBITMAPINFO.bmiHeader.biWidth = width;
	RGB32BitsBITMAPINFO.bmiHeader.biHeight = height;
	RGB32BitsBITMAPINFO.bmiHeader.biPlanes = 1;
	RGB32BitsBITMAPINFO.bmiHeader.biBitCount = 32;
	RGB32BitsBITMAPINFO.bmiHeader.biCompression = BI_RGB;

	HBITMAP hbmp = CreateDIBSection(nullptr, &RGB32BitsBITMAPINFO, DIB_RGB_COLORS, nullptr, nullptr, 0);
	HBITMAP oldbmp = (HBITMAP)SelectObject(hdc, hbmp);

	HBRUSH b = CreateSolidBrush(Traffic_BkColor);
	HBRUSH t = CreateSolidBrush(KeyColor);

	if (ClistModernPresent
		&& unOptions.DrawFrmAsSkin) {
		SKINDRAWREQUEST rq;

		rq.hDC = hdc;
		rq.rcDestRect = rect;
		rq.rcClipRect = rect;
		strncpy(rq.szObjectID, "Main,ID=EventArea", sizeof(rq.szObjectID));
		if (CallService(MS_SKIN_DRAWGLYPH, (WPARAM)&rq, 0)) {
			strncpy(rq.szObjectID, "Main,ID=StatusBar", sizeof(rq.szObjectID));
			CallService(MS_SKIN_DRAWGLYPH, (WPARAM)&rq, 0);
		}
	}
	else {
		SetTextColor(hdc, Traffic_FontColor);
		SetBkColor(hdc, Traffic_BkColor);
		FillRect(hdc, &rect, b);
		AlphaBlend(hdc, 0, 0, width, height, hdc, 0, 0, width, height, aga);
	}

	HFONT old_font = (HFONT)SelectObject(hdc, Traffic_h_font);

	// Ограничиваем область рисования
	rect.top += 2;
	rect.left += 3;
	rect.bottom -= 2;
	rect.right -= 3;

	//-------------------
	// Если нет плагина Variables, рисуем упрощённо.
	//-------------------
	if (!bVariablesExists) {
		SummarySession = SummaryTotal = 0;
		// Для каждого аккаунта
		for (int i = 0; i < NumberOfAccounts; i++) {
			// Только если разрешено его отображение.
			if (ProtoList[i].Visible && ProtoList[i].Enabled) {
				int dx = 0;
				// Изображаем иконку аккаунта.
				if (unOptions.DrawProtoIcon) {
					TC_DrawIconEx(hdc, rect.left, rect.top,
						Skin_LoadProtoIcon(ProtoList[i].name, Proto_GetStatus(ProtoList[i].name)), b, ClistModernPresent);
					dx = 19;
				}
				// Изображаем имя аккаунта
				if (unOptions.DrawProtoName) {
					rect.left += dx;
					TC_AlphaText(hdc, ProtoList[i].tszAccountName, &rect, DT_SINGLELINE | DT_LEFT | DT_TOP, ClistModernPresent);
					rect.left -= dx;
				}
				// Следующие позиции строятся от правого края.
				// Общее время.
				if (unOptions.DrawTotalTimeCounter) {
					wchar_t bu[32];

					GetDurationFormatM(ProtoList[i].Total.Timer, L"h:mm:ss", bu, 32);
					TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_RIGHT | DT_TOP, ClistModernPresent);
				}
				// Текущее время.
				if (unOptions.DrawCurrentTimeCounter) {
					wchar_t bu[32];

					GetDurationFormatM(ProtoList[i].Session.Timer, L"h:mm:ss", bu, 32);
					rect.right -= 50;
					TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_RIGHT | DT_TOP, ClistModernPresent);
					rect.right += 50;
				}
				// Изображаем общий трафик.
				if (unOptions.DrawTotalTraffic) {
					wchar_t bu[32];

					GetFormattedTraffic(ProtoList[i].TotalSentTraffic + ProtoList[i].TotalRecvTraffic, 3, bu, 32);
					rect.right -= 100;
					TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_RIGHT | DT_TOP, ClistModernPresent);
					rect.right += 100;

					if (ProtoList[i].Visible)
						SummaryTotal += ProtoList[i].TotalRecvTraffic + ProtoList[i].TotalSentTraffic;
				}
				// Изображаем текущий трафик.
				if (unOptions.DrawCurrentTraffic) {
					wchar_t bu[32];

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
		// Рисуем суммарный трафик выбранных аккаунтов.
		if (unOptions.ShowSummary) {
			// Изображаем иконку.
			int dx = 0;
			if (unOptions.DrawProtoIcon) {
				TC_DrawIconEx(hdc, rect.left, rect.top,
					Skin_LoadIcon(SKINICON_OTHER_MIRANDA), b, ClistModernPresent);
				dx = 19;
			}
			// Выводим текст
			// Изображаем имя
			if (unOptions.DrawProtoName) {
				wchar_t *bu = mir_a2u("Summary");

				rect.left += dx;
				TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_LEFT | DT_TOP, ClistModernPresent);
				rect.left -= dx;
				mir_free(bu);
			}
			// Общее время.
			if (unOptions.DrawTotalTimeCounter) {
				wchar_t bu[32];

				GetDurationFormatM(OverallInfo.Total.Timer, L"h:mm:ss", bu, 32);
				TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_RIGHT | DT_TOP, ClistModernPresent);
			}
			// Текущее время.
			if (unOptions.DrawCurrentTimeCounter) {
				wchar_t bu[32];

				GetDurationFormatM(OverallInfo.Session.Timer, L"h:mm:ss", bu, 32);
				rect.right -= 50;
				TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_RIGHT | DT_TOP, ClistModernPresent);
				rect.right += 50;
			}
			// Изображаем общий трафик.
			if (unOptions.DrawTotalTraffic) {
				wchar_t bu[32];

				GetFormattedTraffic(SummaryTotal, 3, bu, 32);
				rect.right -= 100;
				TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_RIGHT | DT_TOP, ClistModernPresent);
				rect.right += 100;
			}
			// Изображаем текущий трафик.
			if (unOptions.DrawCurrentTraffic) {
				wchar_t bu[32];

				GetFormattedTraffic(SummarySession, 3, bu, 32);
				rect.right -= 150;
				TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_RIGHT | DT_TOP, ClistModernPresent);
				rect.right += 150;
			}
		}
		rect.top += Traffic_LineHeight + Traffic_AdditionSpace;
		// Рисуем всеобщий трафик.
		if (unOptions.ShowOverall) {
			// Изображаем иконку.
			int dx = 0;
			if (unOptions.DrawProtoIcon) {
				TC_DrawIconEx(hdc, rect.left, rect.top,
					Skin_LoadIcon(SKINICON_OTHER_MIRANDA), b, ClistModernPresent);
				dx = 19;
			}
			// Выводим текст
			// Изображаем имя
			if (unOptions.DrawProtoName) {
				wchar_t *bu = mir_a2u("Overall");

				rect.left += dx;
				TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_LEFT | DT_TOP, ClistModernPresent);
				rect.left -= dx;
				mir_free(bu);
			}
			// Следующие позиции строятся от правого края.
			// Изображаем общий трафик.
			if (unOptions.DrawTotalTraffic) {
				wchar_t bu[32];

				GetFormattedTraffic(OverallInfo.TotalSentTraffic + OverallInfo.TotalRecvTraffic, 3, bu, 32);
				rect.right -= 100;
				TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_RIGHT | DT_TOP, ClistModernPresent);
				rect.right += 100;
			}
			// Изображаем текущий трафик.
			if (unOptions.DrawCurrentTraffic) {
				wchar_t bu[32];

				GetFormattedTraffic(OverallInfo.CurrentRecvTraffic + OverallInfo.CurrentSentTraffic, 3, bu, 32);
				rect.right -= 150;
				TC_AlphaText(hdc, bu, &rect, DT_SINGLELINE | DT_RIGHT | DT_TOP, ClistModernPresent);
				rect.right += 150;
			}
		}
	}
	else {
		//-------------
		// Если есть Variables - рисуем по-новому
		//-------------
		RowItemInfo *ItemsList;
		uint16_t ItemsNumber, RowsNumber;

		// Готовим список строк для Variables и иконок.
		wchar_t **ExtraText = (wchar_t**)mir_alloc(sizeof(wchar_t*));
		HICON *ahIcon = (HICON*)mir_alloc(sizeof(HICON));
		RowsNumber = 0;
		// Цикл по аккаунтам.
		for (int i = 0; i < NumberOfAccounts; i++) {
			if (ProtoList[i].Visible && ProtoList[i].Enabled) {
				ExtraText = (wchar_t**)mir_realloc(ExtraText, sizeof(wchar_t*) * (RowsNumber + 1));
				ahIcon = (HICON*)mir_realloc(ahIcon, sizeof(HICON) * (RowsNumber + 1));

				ExtraText[RowsNumber] = mir_a2u(ProtoList[i].name);
				ahIcon[RowsNumber++] = Skin_LoadProtoIcon(ProtoList[i].name, Proto_GetStatus(ProtoList[i].name));
			}
		}
		// Ещё 2 особых элемента.
		if (unOptions.ShowSummary) {
			ExtraText = (wchar_t**)mir_realloc(ExtraText, sizeof(wchar_t*) * (RowsNumber + 1));
			ahIcon = (HICON*)mir_realloc(ahIcon, sizeof(HICON) * (RowsNumber + 1));

			ExtraText[RowsNumber] = mir_a2u("summary");
			ahIcon[RowsNumber++] = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);
		}
		if (unOptions.ShowOverall) {
			ExtraText = (wchar_t**)mir_realloc(ExtraText, sizeof(wchar_t*) * (RowsNumber + 1));
			ahIcon = (HICON*)mir_realloc(ahIcon, sizeof(HICON) * (RowsNumber + 1));

			ExtraText[RowsNumber] = mir_a2u("overall");
			ahIcon[RowsNumber++] = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);
		}

		// Рисуем свой счётчик для каждого из выбранных протоколов
		for (int i = 0; i < RowsNumber; i++) {
			wchar_t *buf = variables_parse(Traffic_CounterFormat, ExtraText[i], NULL);
			if (ItemsNumber = GetRowItems(buf, &ItemsList)) {
				// Рисуем текст.
				for (int dx = 0; dx < ItemsNumber; dx++) {
					// Делаем копию прямоугольника для рисования.
					memcpy(&rect2, &rect, sizeof(RECT));
					rect2.bottom = rect2.top + Traffic_LineHeight;
					// Что за тег?
					switch (ItemsList[dx].Alignment) {
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

				// Нужно освободить память.
				for (; ItemsNumber--;)
					mir_free(ItemsList[ItemsNumber].String);

				mir_free(ItemsList);
			}
			mir_free(buf);
			rect.top += Traffic_LineHeight + Traffic_AdditionSpace;
		}

		// Удаляем список строк.
		for (; RowsNumber--;)
			mir_free(ExtraText[RowsNumber]);
		mir_free(ExtraText);
	}

	DeleteObject(b);
	DeleteObject(t);
	SelectObject(hdc, old_font);

	if (ClistModernPresent)
		AlphaBlend(hDC, 0, 0, width, height, hdc, 0, 0, width, height, aga);
	else
		BitBlt(hDC, 0, 0, width, height, hdc, 0, 0, SRCCOPY);

	SelectObject(hdc, oldbmp);
	DeleteObject(hbmp);
	DeleteDC(hdc);

	return 0;
}

void ProtocolIsOnLine(int num)
{
	uint32_t CurrentTimeMs;

	if (ProtoList[num].State) return;

	online_count++;
	CurrentTimeMs = GetTickCount();

	ProtoList[num].State = 1;
	ProtoList[num].Session.TimeAtStart = CurrentTimeMs;
	ProtoList[num].Total.TimeAtStart = CurrentTimeMs
		- 1000 * ProtoList[num].AllStatistics[ProtoList[num].NumberOfRecords - 1].Time;
	ProtoList[num].Session.Timer = 0;

	if (online_count == 1) // Хотя бы один аккаунт перешёл в онлайн - готовимся считать суммарное время.
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

static POINT ptMouse = { 0 };

LRESULT CALLBACK TrafficCounterWndProc_MW(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case (WM_USER + 697):
		if (lParam == 666)
			DestroyWindow(hwnd);
		break;

	case WM_PAINT:
		{
			if (!db_get_b(0, "ModernData", "DisableEngine", 0)
				&& db_get_b(0, "ModernData", "EnableLayering", 1)
				&& ServiceExists(MS_SKINENG_INVALIDATEFRAMEIMAGE))
				CallService(MS_SKINENG_INVALIDATEFRAMEIMAGE, (WPARAM)TrafficHwnd, 0);
			else {
				HDC hdc = GetDC(hwnd);
				PaintTrafficCounterWindow(hwnd, hdc);
				ReleaseDC(hwnd, hdc);
			}
		}
		break;

	case WM_ERASEBKGND:
		return 1;

	case WM_LBUTTONDOWN:
		if (db_get_b(0, "CLUI", "ClientAreaDrag", SETTING_CLIENTDRAG_DEFAULT)) {
			POINT p;
			ClientToScreen(GetParent(hwnd), &p);
			return SendMessage(GetParent(hwnd), WM_SYSCOMMAND, SC_MOVE | HTCAPTION, MAKELPARAM(p.x, p.y));
		}
		break;

	case WM_RBUTTONDOWN:
		{
			POINT p;
			p.x = GET_X_LPARAM(lParam);
			p.y = GET_Y_LPARAM(lParam);
			ClientToScreen(hwnd, &p);
			TrackPopupMenu(TrafficPopupMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, p.x, p.y, 0, hwnd, nullptr);
		}
		break;

	case WM_COMMAND:
		switch (wParam) {
		case POPUPMENU_HIDE:
			MenuCommand_TrafficShowHide(0, 0);
			break;

		case POPUPMENU_CLEAR_NOW:
			for (int i = 0; i < NumberOfAccounts; i++) {
				auto &p = ProtoList[i];
				p.StartIncoming = p.AllStatistics[p.NumberOfRecords - 1].Incoming;
				p.StartOutgoing = p.AllStatistics[p.NumberOfRecords - 1].Outgoing;
				p.Session.TimeAtStart = GetTickCount();
			}
			OverallInfo.CurrentRecvTraffic = OverallInfo.CurrentSentTraffic = 0;
		}
		break;

	case WM_SETCURSOR:
		{
			POINT pt;

			GetCursorPos(&pt);
			if ((abs(pt.x - ptMouse.x) < 20)
				&& (abs(pt.y - ptMouse.y) < 20))
				return 1;

			if (TooltipShowing) {
				KillTimer(TrafficHwnd, TIMER_TOOLTIP);
				CallService("mToolTip/HideTip", 0, 0);
				TooltipShowing = FALSE;
			}
			KillTimer(TrafficHwnd, TIMER_TOOLTIP);
			SetTimer(TrafficHwnd, TIMER_TOOLTIP, CallService(MS_CLC_GETINFOTIPHOVERTIME, 0, 0), nullptr);
			break;
		}

	case WM_TIMER:
		switch (wParam) {
		case TIMER_NOTIFY_TICK:
			NotifyOnRecv();
			NotifyOnSend();
			break;

		case TIMER_REDRAW: // Перерисовка раз в полсекунды.
			{
				uint32_t CurrentTimeMs;
				SYSTEMTIME stNow;

				SaveSettings(1);
				// Перед подсчётом суммарного трафика обнуляем счётчики.
				OverallInfo.CurrentRecvTraffic =
					OverallInfo.CurrentSentTraffic =
					OverallInfo.TotalRecvTraffic =
					OverallInfo.TotalSentTraffic = 0;

				CurrentTimeMs = GetTickCount();

				for (int i = 0; i < NumberOfAccounts; i++) {
					auto &p = ProtoList[i];
					if (p.State) {
						p.AllStatistics[p.NumberOfRecords - 1].Time = (CurrentTimeMs - p.Total.TimeAtStart) / 1000;
						p.Session.Timer = (CurrentTimeMs - p.Session.TimeAtStart) / 1000;
					}

					Stat_CheckStatistics(p);

					// Здесь на основании статистики вычисляются значения всех трафиков и времени.
					uint32_t Sum1 = 0, Sum2 = 0;
						
					// Значения для текущей сессии.
					for (int j = p.StartIndex; j < p.NumberOfRecords; j++) {
						Sum1 += p.AllStatistics[j].Incoming;
						Sum2 += p.AllStatistics[j].Outgoing;
					}

					p.CurrentRecvTraffic = Sum1 - p.StartIncoming;
					p.CurrentSentTraffic = Sum2 - p.StartOutgoing;
					OverallInfo.CurrentRecvTraffic += p.CurrentRecvTraffic;
					OverallInfo.CurrentSentTraffic += p.CurrentSentTraffic;
					// Значения для выбранного периода.
					p.TotalRecvTraffic =
						Stat_GetItemValue(1 << i,
							unOptions.PeriodForShow + 1,
							Stat_GetRecordsNumber(i, unOptions.PeriodForShow + 1) - 1, 1);
					p.TotalSentTraffic =
						Stat_GetItemValue(1 << i,
							unOptions.PeriodForShow + 1,
							Stat_GetRecordsNumber(i, unOptions.PeriodForShow + 1) - 1, 2);
					p.Total.Timer =
						Stat_GetItemValue(1 << i,
							unOptions.PeriodForShow + 1,
							Stat_GetRecordsNumber(i, unOptions.PeriodForShow + 1) - 1, 4);
					OverallInfo.TotalRecvTraffic += p.TotalRecvTraffic;
					OverallInfo.TotalSentTraffic += p.TotalSentTraffic;
				}

				// Не пора ли уведомить?
				if (unOptions.NotifyBySize && Traffic_Notify_size_value) {
					if (!((OverallInfo.CurrentRecvTraffic >> 10) % Traffic_Notify_size_value)
						&& notify_recv_size != OverallInfo.CurrentRecvTraffic >> 10)
						NotifyOnRecv();

					if (!((OverallInfo.CurrentSentTraffic >> 10) % Traffic_Notify_size_value)
						&& notify_send_size != OverallInfo.CurrentSentTraffic >> 10)
						NotifyOnSend();
				}

				// Счётчики суммарного времени.
				GetLocalTime(&stNow);

				// Не пора ли сбросить общий счётчик?
				if ((unOptions.PeriodForShow == 0 && stNow.wHour == 0 && stNow.wMinute == 0 && stNow.wSecond == 0)
					|| (unOptions.PeriodForShow == 1 && DayOfWeek(stNow.wDay, stNow.wMonth, stNow.wYear) == 1 && stNow.wHour == 0 && stNow.wMinute == 0 && stNow.wSecond == 0)
					|| (unOptions.PeriodForShow == 2 && stNow.wDay == 1 && stNow.wHour == 0 && stNow.wMinute == 0 && stNow.wSecond == 0) 
					|| (unOptions.PeriodForShow == 3 && stNow.wMonth == 1 && stNow.wDay == 1 && stNow.wHour == 0 && stNow.wMinute == 0 && stNow.wSecond == 0))
					OverallInfo.Total.TimeAtStart = CurrentTimeMs;

				if (online_count > 0) {
					OverallInfo.Session.Timer = (CurrentTimeMs - OverallInfo.Session.TimeAtStart) / 1000;
					OverallInfo.Total.Timer = (CurrentTimeMs - OverallInfo.Total.TimeAtStart) / 1000;
				}

				CallService(MS_CLIST_FRAMES_UPDATEFRAME, Traffic_FrameID, FU_FMREDRAW);
			}
			break;

		case TIMER_TOOLTIP:
			GetCursorPos(&TooltipPosition);
			if (!TooltipShowing && unOptions.ShowTooltip) {
				KillTimer(TrafficHwnd, TIMER_TOOLTIP);
				ScreenToClient(TrafficHwnd, &TooltipPosition);

				RECT rt;
				GetClientRect(TrafficHwnd, &rt);
				if (PtInRect(&rt, TooltipPosition)) {
					GetCursorPos(&ptMouse);

					CLCINFOTIP ti = {};
					ti.rcItem.left = TooltipPosition.x - 10;
					ti.rcItem.right = TooltipPosition.x + 10;
					ti.rcItem.top = TooltipPosition.y - 10;
					ti.rcItem.bottom = TooltipPosition.y + 10;
					ti.cbSize = sizeof(ti);
					Tipper_ShowTip(ptrW(variables_parsedup(Traffic_TooltipFormat, nullptr, NULL)), &ti);

					TooltipShowing = TRUE;
				}
			}
			break;
		}
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void CreateTrafficWindow(HWND hCluiWnd)
{
	WNDCLASSEX wcx = { 0 };
	CLISTFrame f;

	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.style = 0;
	wcx.lpfnWndProc = TrafficCounterWndProc_MW;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = g_plugin.getInst();
	wcx.hIcon = nullptr;
	wcx.hCursor = LoadCursor(g_plugin.getInst(), IDC_ARROW);
	wcx.hbrBackground = nullptr;
	wcx.lpszMenuName = nullptr;
	wcx.lpszClassName = TRAFFIC_COUNTER_WINDOW_CLASS;
	wcx.hIconSm = nullptr;
	RegisterClassEx(&wcx);
	TrafficHwnd = CreateWindowEx(WS_EX_TOOLWINDOW, TRAFFIC_COUNTER_WINDOW_CLASS,
		TRAFFIC_COUNTER_WINDOW_CLASS,
		WS_CHILDWINDOW | WS_CLIPCHILDREN,
		0, 0, 0, 0, hCluiWnd, nullptr, g_plugin.getInst(), nullptr);

	if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
		// Готовимся создавать фрейм
		memset(&f, 0, sizeof(CLISTFrame));
		f.align = alBottom;
		f.cbSize = sizeof(CLISTFrame);
		f.height = TrafficWindowHeight();
		f.Flags = unOptions.FrameIsVisible | F_LOCKED | F_NOBORDER | F_NO_SUBCONTAINER;
		f.hWnd = TrafficHwnd;
		f.szName.a = f.szTBname.a = LPGEN("Traffic counter");
		// Создаём фрейм
		Traffic_FrameID = g_plugin.addFrame(&f);
		CallService(MS_SKINENG_REGISTERPAINTSUB, (WPARAM)f.hWnd, (LPARAM)TrafficCounter_PaintCallbackProc);
	}

	// Создаём таймеры.
	SetTimer(TrafficHwnd, TIMER_REDRAW, 500, nullptr);
	UpdateNotifyTimer();
}

INT_PTR MenuCommand_TrafficShowHide(WPARAM, LPARAM)
{
	unOptions.FrameIsVisible = !unOptions.FrameIsVisible;
	if (Traffic_FrameID == 0)
		ShowWindow(TrafficHwnd, unOptions.FrameIsVisible ? SW_SHOW : SW_HIDE);
	else
		CallService(MS_CLIST_FRAMES_SHFRAME, Traffic_FrameID, 0);
	g_plugin.setDword(SETTINGS_WHAT_DRAW, unOptions.Flags);
	//
	return 0;
}

void Traffic_AddMainMenuItem(void)
{
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x7fe75b30, 0x3cf6, 0x4280, 0xb9, 0xd2, 0x88, 0x6b, 0xbb, 0x69, 0xa3, 0x7e);
	mi.position = -0x7FFFFFFF;
	mi.hIcolibItem = nullptr;
	mi.name.a = LPGEN("Toggle traffic counter");
	mi.pszService = "TrafficCounter/ShowHide";

	hTrafficMainMenuItem = Menu_AddMainMenuItem(&mi);
}

/*-------------------------------------------------------------------------------------------------------------------*/
void UpdateNotifyTimer(void)
{
	if (Traffic_Notify_time_value && unOptions.NotifyByTime)
		SetTimer(TrafficHwnd, TIMER_NOTIFY_TICK, Traffic_Notify_time_value * 1000 * 60, nullptr);
	else
		KillTimer(TrafficHwnd, TIMER_NOTIFY_TICK);
}

void NotifyOnSend(void)
{
	POPUPDATAW ppd;
	ppd.lchIcon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
	wcsncpy(ppd.lpwzContactName, TranslateT("Traffic counter notification"), MAX_CONTACTNAME);
	mir_snwprintf(ppd.lpwzText, TranslateT("%d kilobytes sent"), notify_send_size = OverallInfo.CurrentSentTraffic >> 10);
	ppd.colorBack = Traffic_PopupBkColor;
	ppd.colorText = Traffic_PopupFontColor;
	ppd.iSeconds = (Traffic_PopupTimeoutDefault ? 0 : Traffic_PopupTimeoutValue);
	PUAddPopupW(&ppd);
}

void NotifyOnRecv(void)
{
	POPUPDATAW ppd;
	ppd.lchIcon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
	wcsncpy(ppd.lpwzContactName, TranslateT("Traffic counter notification"), MAX_CONTACTNAME);
	mir_snwprintf(ppd.lpwzText, TranslateT("%d kilobytes received"), notify_recv_size = OverallInfo.CurrentRecvTraffic >> 10);
	ppd.colorBack = Traffic_PopupBkColor;
	ppd.colorText = Traffic_PopupFontColor;
	ppd.iSeconds = (Traffic_PopupTimeoutDefault ? 0 : Traffic_PopupTimeoutValue);
	PUAddPopupW(&ppd);
}

void CreateProtocolList(void)
{
	auto &accs = Accounts();

	NumberOfAccounts = accs.getCount();
	ProtoList = (PROTOLIST*)mir_calloc(sizeof(PROTOLIST)*NumberOfAccounts);

	int i = 0;
	for (auto &pa : accs) {
		auto &p = ProtoList[i++];
		p.name = mir_strdup(pa->szModuleName);
		p.tszAccountName = mir_wstrdup(pa->tszAccountName);
		p.Flags = db_get_b(0, p.name, SETTINGS_PROTO_FLAGS, 3);
		p.Enabled = pa->IsEnabled();

		Stat_ReadFile(p);
		p.StartIndex = p.NumberOfRecords - 1;
		p.StartIncoming = p.AllStatistics[p.StartIndex].Incoming;
		p.StartOutgoing = p.AllStatistics[p.StartIndex].Outgoing;
	}

	// Начальные значения для суммарной информации.
	OverallInfo.Session.Timer = OverallInfo.Total.Timer = 0;
}

void DestroyProtocolList(void)
{
	for (int i = 0; i < NumberOfAccounts; i++) {
		auto &p = ProtoList[i];
		Stat_CheckStatistics(p);
		CloseHandle(p.hFile);
		mir_free(p.tszAccountName);
		mir_free(p.name);
		mir_free(p.AllStatistics);
	}

	mir_free(ProtoList);
}

int UpdateFonts(WPARAM, LPARAM)
{
	LOGFONT logfont;
	//update traffic font
	if (Traffic_h_font) DeleteObject(Traffic_h_font);
	Traffic_FontColor = Font_GetW(TrafficFontID, &logfont);
	Traffic_h_font = CreateFontIndirect(&logfont);

	TrafficFontHeight = abs(logfont.lfHeight) + 1;
	Traffic_BkColor = Colour_GetW(TrafficBackgroundColorID);

	// Ключевой цвет
	UseKeyColor = db_get_b(0, "ModernSettings", "UseKeyColor", 1);
	KeyColor = db_get_dw(0, "ModernSettings", "KeyColor", 0);

	UpdateTrafficWindowSize();
	return 0;
}

void UpdateTrafficWindowSize(void)
{
	if (Traffic_FrameID != 0)
		CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_HEIGHT, Traffic_FrameID), TrafficWindowHeight());
}

unsigned short int TrafficWindowHeight(void)
{
	int ActProto = unOptions.ShowOverall + unOptions.ShowSummary;

	for (int i = 0; i < NumberOfAccounts; i++)
		ActProto += ProtoList[i].Visible && ProtoList[i].Enabled;

	// Высота строки минимум 16 пикселей (для иконки).
	Traffic_LineHeight = TrafficFontHeight > 16 ? TrafficFontHeight : 16;

	// Высота фрейма равна количеству строк.
	int MaxWndHeight = ActProto * Traffic_LineHeight + Traffic_AdditionSpace * (ActProto - 1) + 4;
	return (MaxWndHeight < TrafficFontHeight) ? 0 : MaxWndHeight;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int ProtocolAckHook(WPARAM, LPARAM lParam)
{
	ACKDATA *pAck = (ACKDATA*)lParam;

	if (ACKTYPE_STATUS == pAck->type) {
		if (ID_STATUS_OFFLINE == pAck->lParam) {
			for (int i = 0; i < NumberOfAccounts; i++) {
				if (!ProtoList[i].name) continue;
				if (!mir_strcmp(ProtoList[i].name, pAck->szModule)) {
					ProtocolIsOffLine(i);
					break;
				}
			}
		}
		else {
			if ((pAck->lParam >= ID_STATUS_ONLINE) && (pAck->lParam <= ID_STATUS_MAX)) {
				for (int i = 0; i < NumberOfAccounts; i++)
					if (!mir_strcmp(ProtoList[i].name, pAck->szModule)) {
						ProtocolIsOnLine(i);
						break;
					}
			}
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Функция вносит изменения в ProtoList при коммутации аккаунтов

static int OnAccountsListChange(WPARAM wParam, LPARAM lParam)
{
	PROTOACCOUNT *acc = (PROTOACCOUNT*)lParam;

	switch (wParam) {
	case PRAC_ADDED:
	case PRAC_REMOVED:
		DestroyProtocolList();
		CreateProtocolList();
		break;

	case PRAC_CHANGED:
	case PRAC_CHECKED:
		for (int i = 0; i < NumberOfAccounts; i++)
			if (!mir_strcmp(acc->szModuleName, ProtoList[i].name))
				ProtoList[i].Enabled = acc->bIsEnabled;
		break;
	}
	UpdateTrafficWindowSize();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int TrafficCounterModulesLoaded(WPARAM, LPARAM)
{
	CreateProtocolList();
	ModuleLoad(0, 0);

	// Читаем флаги
	unOptions.Flags = g_plugin.getDword(SETTINGS_WHAT_DRAW, 0x0882);
	Stat_SelAcc = g_plugin.getWord(SETTINGS_STAT_ACC_OPT, 0x01);

	// settings for notification
	Traffic_PopupBkColor = g_plugin.getDword(SETTINGS_POPUP_BKCOLOR, RGB(200, 255, 200));
	Traffic_PopupFontColor = g_plugin.getDword(SETTINGS_POPUP_FONTCOLOR, RGB(0, 0, 0));
	Traffic_Notify_time_value = g_plugin.getByte(SETTINGS_POPUP_NOTIFY_TIME_VALUE, 10);
	Traffic_Notify_size_value = g_plugin.getWord(SETTINGS_POPUP_NOTIFY_SIZE_VALUE, 100);

	// popup timeout
	Traffic_PopupTimeoutDefault = g_plugin.getByte(SETTINGS_POPUP_TIMEOUT_DEFAULT, 1);
	Traffic_PopupTimeoutValue = g_plugin.getByte(SETTINGS_POPUP_TIMEOUT_VALUE, 5);

	// Формат счётчика для каждого активного протокола
	ptrW wszFormat(g_plugin.getWStringA(SETTINGS_COUNTER_FORMAT));
	if (mir_wstrlen(wszFormat) > 0)
		mir_wstrncpy(Traffic_CounterFormat, wszFormat, _countof(Traffic_CounterFormat));
	else
		mir_wstrcpy(Traffic_CounterFormat, wszDefaultFormat);

	// Формат всплывающих подсказок
	wszFormat = g_plugin.getWStringA(SETTINGS_TOOLTIP_FORMAT);
	if (mir_wstrlen(wszFormat) > 0)
		mir_wstrncpy(Traffic_TooltipFormat, wszFormat, _countof(Traffic_TooltipFormat));
	else
		mir_wstrcpy(Traffic_TooltipFormat, L"Traffic Counter");

	Traffic_AdditionSpace = g_plugin.getByte(SETTINGS_ADDITION_SPACE, 0);

	// Счётчик времени онлайна
	OverallInfo.Total.Timer = g_plugin.getDword(SETTINGS_TOTAL_ONLINE_TIME, 0);

	//register traffic font
	mir_wstrcpy(TrafficFontID.group, LPGENW("Traffic counter"));
	mir_wstrcpy(TrafficFontID.name, LPGENW("Font"));
	mir_strcpy(TrafficFontID.dbSettingsGroup, MODULENAME);
	mir_strcpy(TrafficFontID.setting, "Font");
	TrafficFontID.flags = FIDF_DEFAULTVALID | FIDF_SAVEPOINTSIZE;
	TrafficFontID.deffontsettings.charset = DEFAULT_CHARSET;
	TrafficFontID.deffontsettings.colour = GetSysColor(COLOR_BTNTEXT);
	TrafficFontID.deffontsettings.size = 12;
	TrafficFontID.deffontsettings.style = 0;
	mir_wstrcpy(TrafficFontID.deffontsettings.szFace, L"Arial");
	TrafficFontID.order = 0;
	g_plugin.addFont(&TrafficFontID);

	// Регистрируем цвет фона
	mir_wstrcpy(TrafficBackgroundColorID.group, LPGENW("Traffic counter"));
	mir_wstrcpy(TrafficBackgroundColorID.name, LPGENW("Font"));
	mir_strcpy(TrafficBackgroundColorID.dbSettingsGroup, MODULENAME);
	mir_strcpy(TrafficBackgroundColorID.setting, "FontBkColor");
	TrafficBackgroundColorID.defcolour = GetSysColor(COLOR_BTNFACE);
	g_plugin.addColor(&TrafficBackgroundColorID);

	HookEvent(ME_FONT_RELOAD, UpdateFonts);

	// Добавляем поддержку плагина Variables
	RegisterVariablesTokens();

	CreateServiceFunction("TrafficCounter/ShowHide", MenuCommand_TrafficShowHide);
	// Регистрируем горячую клавишу для показа/скрытия фрейма
	{
		HOTKEYDESC hkd = {};
		hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, 'T');
		hkd.szSection.a = "Traffic Counter";
		hkd.szDescription.a = LPGEN("Show/Hide frame");
		hkd.pszName = "TC_Show_Hide";
		hkd.pszService = "TrafficCounter/ShowHide";
		g_plugin.addHotkey(&hkd);
	}

	// Добавляем пункт в главное меню.
	if (unOptions.ShowMainMenuItem)
		Traffic_AddMainMenuItem();

	// Создаём контекстное меню.
	if (TrafficPopupMenu = CreatePopupMenu()) {
		AppendMenu(TrafficPopupMenu, MF_STRING, POPUPMENU_HIDE, TranslateT("Hide traffic window"));
		AppendMenu(TrafficPopupMenu, MF_STRING, POPUPMENU_CLEAR_NOW, TranslateT("Clear the current (Now:) value"));
	}

	// Регистрируем обработчики событий Netlib
	HookEvent(ME_NETLIB_FASTRECV, TrafficRecv);
	HookEvent(ME_NETLIB_FASTSEND, TrafficSend);

	CreateTrafficWindow(g_clistApi.hwndContactList);
	UpdateFonts(0, 0);	//Load and create fonts here
	return 0;
}

int CMPlugin::Load()
{
	HookEvent(ME_OPT_INITIALISE, TrafficCounterOptInitialise);
	HookEvent(ME_SYSTEM_MODULESLOADED, TrafficCounterModulesLoaded);
	HookEvent(ME_PROTO_ACK, ProtocolAckHook);
	HookEvent(ME_PROTO_ACCLISTCHANGED, OnAccountsListChange);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, TrafficCounterShutdown);
	HookEvent(ME_SYSTEM_MODULELOAD, ModuleLoad);
	HookEvent(ME_SYSTEM_MODULEUNLOAD, ModuleLoad);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	// Удаляем шрифт.
	if (Traffic_h_font) {
		DeleteObject(Traffic_h_font);
		Traffic_h_font = nullptr;
	}

	// Убиваем все рабочие данные.
	DestroyProtocolList();
	return 0;
}
