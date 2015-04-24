#ifndef commonheaders_h__
#define commonheaders_h__

/*
Traffic Counter plugin for Miranda IM 
Copyright 2007-2011 Mironych.

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

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include <win2k.h>
#include <m_system.h>

#include <newpluginapi.h>
#include <m_netlib.h>
#include <m_langpack.h>
#include <m_clui.h>
#include <m_clc.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_cluiframes.h>
#include <m_fontservice.h>
#include <m_popup.h>
#include <m_skin.h>
#include <m_hotkeys.h>

#include <m_variables.h>
#include <m_skin_eng.h>
#include <m_tipper.h>

#include "resource.h"
#include "version.h"
#include "misc.h"
#include "opttree.h"
#include "vars.h"
#include "statistics.h"
#include "TrafficCounter.h"

#pragma pack(push)
#pragma pack(1)
typedef	struct
{
	BYTE Hour, Day, Month;
	WORD Year;
	DWORD Incoming, Outgoing;
	WORD Time;
} HOURLYSTATS;
#pragma pack(pop)

typedef struct tagTimer
{
	DWORD TimeAtStart; // Время в момент запуска таймера - в миллисекундах.
	DWORD Timer; // Количество секунд со времени запуска таймера.
} TIMER;

typedef struct
{
	char *name; // Имя аккаунта.

	TIMER Session; // Таймер текущей сессии (протокол в онлайне).
	TIMER Total; // Таймер общий.

	DWORD TotalRecvTraffic, // Общий трафик протокола (за выбранный период)
		  TotalSentTraffic,
		  CurrentRecvTraffic, // Текущий трафик протокола (за сессию)
		  CurrentSentTraffic;
	union
	{
		BYTE Flags;
		struct
		{
			unsigned int Reserv0:1; // Активность потеряла смысл - статистика ведётся по всем аккаунтам.
			unsigned int Visible:1; // = 1 - аккаунт будет показываться во фрейме счётчиков
			unsigned int Enabled:1; // = 1 - аккаунт включен и не прячется
			unsigned int State:1;   // = 1 - аккаунт сейчас онлайн
			unsigned int Reserv1:3;
		};
	};

	// Добавлено в версии 0.1.1.0.
	DWORD NumberOfRecords; // Количество часов в общей статистике.
	HOURLYSTATS *AllStatistics; // Полная статистика вместе со статистикой онлайна.
	HANDLE hFile; // Файл с сохранённой статистикой данного протокола.

	DWORD StartIndex; // Номер записи в статистике, бывший актуальным на момент запуска.
	DWORD StartIncoming; // Значение входящего трафика на момент запуска.
	DWORD StartOutgoing; // Значение исходящего трафика на момент запуска.

	// 0.1.1.5.
	DWORD Shift;	// Номер записи в статистике старейшего выбранного аккаунта,
					// дата которой соответствует началу статистики данного аккаунта.

	// 0.1.1.6
	TCHAR *tszAccountName; // Человеческое имя аккаунта для использования в графическом интерфейсе.
} PROTOLIST;

//---------------------------------------------------------------------------------------------
// Различные флаги
//---------------------------------------------------------------------------------------------
typedef union
{
	DWORD Flags;
	struct
	{
		unsigned int NotifyBySize:1;			//0
		unsigned int DrawCurrentTraffic:1;		//1
		unsigned int DrawTotalTraffic:1;		//2
		unsigned int DrawCurrentTimeCounter:1;	//3
		unsigned int DrawProtoIcon:1;			//4
		unsigned int Reserv0:1;					//5
		unsigned int DrawProtoName:1;			//6
		unsigned int DrawFrmAsSkin:1;			//7
		unsigned int ShowSummary:1;				//8
		unsigned int ShowTooltip:1;				//9
		unsigned int ShowMainMenuItem:1;		//10
		unsigned int ShowOverall:1;				//11
		unsigned int Stat_Units:2;				//12,13
		unsigned int Stat_Tab:3;				//14,15,16
		unsigned int NotifyByTime:1;			//17
		unsigned int Reserv2:1;					//18
		unsigned int PeriodForShow:2;			//19,20
		unsigned int FrameIsVisible:1;			//21
		unsigned int Reserv1:1;					//22
		unsigned int DrawTotalTimeCounter:1;	//23
	};
} uTCFLAGS;

extern HINSTANCE hInst;

extern HWND TrafficHwnd;
extern PROTOLIST *ProtoList;
extern uTCFLAGS unOptions;

extern int  Traffic_PopupBkColor;
extern int  Traffic_PopupFontColor;
extern char Traffic_Notify_time_value;
extern int  Traffic_Notify_size_value;
extern char Traffic_PopupTimeoutDefault;
extern char Traffic_PopupTimeoutValue;
extern char Traffic_AdditionSpace;

extern TCHAR Traffic_CounterFormat[512];
extern TCHAR Traffic_TooltipFormat[512];

extern BOOL bPopupExists, bVariablesExists, bTooltipExists;

extern BOOL UseKeyColor;
extern COLORREF KeyColor;

extern HGENMENU hTrafficMainMenuItem;

extern WORD Stat_SelAcc;
extern OPTTREE_OPTION *pOptions; // Объявлено в модуле TrafficCounter.c.
extern PROTOLIST OverallInfo;

#endif
