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
void Stat_ReadFile(PROTOLIST &p);
void Stat_Show(HWND);
void Stat_UpdateTotalTraffic(HWND, uint32_t, uint32_t);
void Stat_CheckStatistics(PROTOLIST &p);
uint32_t Stat_GetStartIndex(uint8_t AccNum, uint8_t Interval, int ItemNumber, SYSTEMTIME *st);
void Stat_SetAccShift(uint8_t AccNum, uint8_t EldestAccount);
uint32_t Stat_GetItemValue(uint16_t SelectedAccs, uint8_t Interval, uint32_t ItemNum, uint8_t SubitemNum);
uint32_t Stat_GetRecordsNumber(uint8_t AccNum, uint8_t Interval);
uint8_t Stat_GetEldestAcc(uint16_t SelectedAccs);

#endif // _statistics_h