#ifndef _statistics_h
#define _statistics_h

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

#define STAT_INTERVAL_HOUR 0
#define STAT_INTERVAL_DAY 1
#define STAT_INTERVAL_WEEK 2
#define STAT_INTERVAL_MONTH 3
#define STAT_INTERVAL_YEAR 4

#define STAT_UNITS_BYTES 0
#define STAT_UNITS_KB 1
#define STAT_UNITS_MB 2
#define STAT_UNITS_ADAPTIVE 3

INT_PTR CALLBACK DlgProcOptStatistics(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void Stat_ReadFile(BYTE);
void Stat_Show(HWND);
void Stat_UpdateTotalTraffic(HWND, DWORD, DWORD);
void Stat_CheckStatistics(BYTE);
DWORD Stat_GetStartIndex(BYTE AccNum, BYTE Interval, DWORD ItemNumber, SYSTEMTIME *st);
void Stat_SetAccShift(BYTE AccNum, BYTE EldestAccount);
DWORD Stat_GetItemValue(WORD SelectedAccs, BYTE Interval, DWORD ItemNum, BYTE SubitemNum);
DWORD Stat_GetRecordsNumber(BYTE AccNum, BYTE Interval);
BYTE Stat_GetEldestAcc(WORD SelectedAccs);

#endif // _statistics_h