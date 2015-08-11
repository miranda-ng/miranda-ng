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
/* ======================================================================================
����� ���������� ���������� � ������� ��� ������ �� �����������
�����: Mironych
=======================================================================================*/

#include "stdafx.h"

WORD Stat_SelAcc; // ��������� �������� � ���� ����������

HWND hListAccs;

INT_PTR CALLBACK DlgProcOptStatistics(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	WORD i;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		// ������ ListBox c �������� ���������.
		hListAccs = CreateWindowEx(WS_EX_CLIENTEDGE,
			_T("ListBox"),
			NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | LBS_NOINTEGRALHEIGHT | LBS_EXTENDEDSEL | LBS_NOTIFY,
			2, 20, 246, 112,
			hwndDlg, NULL, NULL, NULL);
		SendMessage(hListAccs, WM_SETFONT, (WPARAM)(HFONT)GetStockObject(DEFAULT_GUI_FONT), 0);
		for (i = 0; i < NumberOfAccounts; i++) {
			// ������� ������ ���������
			if (ProtoList[i].tszAccountName)
				SendMessage(hListAccs, LB_ADDSTRING, 0, (LPARAM)ProtoList[i].tszAccountName);
		}
		for (i = NumberOfAccounts; i--;)
			SendMessage(hListAccs, LB_SETSEL, (WPARAM)0x01 & (Stat_SelAcc >> i), (LPARAM)i);
		// ������� ������ ������ ���������
		SendDlgItemMessage(hwndDlg, IDC_COMBO_UNITS, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Bytes"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_UNITS, CB_INSERTSTRING, -1, (LPARAM)TranslateT("KB"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_UNITS, CB_INSERTSTRING, -1, (LPARAM)TranslateT("MB"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_UNITS, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Adaptive"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_UNITS, CB_SETCURSEL, unOptions.Stat_Units, 0);
		// ������� ��������
		{
			TCITEM tci;
			tci.mask = TCIF_TEXT;
			tci.pszText = TranslateT("Hourly");
			SendDlgItemMessage(hwndDlg, IDC_TAB_STATS, TCM_INSERTITEM, 0, (LPARAM)&tci);
			tci.pszText = TranslateT("Daily");
			SendDlgItemMessage(hwndDlg, IDC_TAB_STATS, TCM_INSERTITEM, 1, (LPARAM)&tci);
			tci.pszText = TranslateT("Weekly");
			SendDlgItemMessage(hwndDlg, IDC_TAB_STATS, TCM_INSERTITEM, 2, (LPARAM)&tci);
			tci.pszText = TranslateT("Monthly");
			SendDlgItemMessage(hwndDlg, IDC_TAB_STATS, TCM_INSERTITEM, 3, (LPARAM)&tci);
			tci.pszText = TranslateT("Yearly");
			SendDlgItemMessage(hwndDlg, IDC_TAB_STATS, TCM_INSERTITEM, 4, (LPARAM)&tci);
			SendDlgItemMessage(hwndDlg, IDC_TAB_STATS, TCM_SETCURSEL, unOptions.Stat_Tab, 0);
		}
		// ������� ListView - ������� � �����
		{
			SendDlgItemMessage(hwndDlg, IDC_LIST_DATA, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
			SendDlgItemMessage(hwndDlg, IDC_LIST_DATA, LVM_SETUNICODEFORMAT, 1, 0);

			LVCOLUMN lvc = { 0 };
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.fmt = LVCFMT_LEFT;
			lvc.pszText = TranslateT("Period");
			lvc.cx = 135;
			lvc.iSubItem = 0;
			SendDlgItemMessage(hwndDlg, IDC_LIST_DATA, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);
			lvc.fmt = LVCFMT_RIGHT;
			lvc.pszText = TranslateT("Incoming");
			lvc.iSubItem = 1;
			lvc.cx = 72;
			SendDlgItemMessage(hwndDlg, IDC_LIST_DATA, LVM_INSERTCOLUMN, 1, (LPARAM)&lvc);
			lvc.pszText = TranslateT("Outgoing");
			lvc.iSubItem = 2;
			lvc.cx = 72;
			SendDlgItemMessage(hwndDlg, IDC_LIST_DATA, LVM_INSERTCOLUMN, 2, (LPARAM)&lvc);
			lvc.pszText = TranslateT("Sum");
			lvc.iSubItem = 3;
			lvc.cx = 72;
			SendDlgItemMessage(hwndDlg, IDC_LIST_DATA, LVM_INSERTCOLUMN, 3, (LPARAM)&lvc);
			lvc.pszText = TranslateT("Online");
			lvc.iSubItem = 4;
			lvc.cx = 72;
			SendDlgItemMessage(hwndDlg, IDC_LIST_DATA, LVM_INSERTCOLUMN, 4, (LPARAM)&lvc);
		}
		Stat_Show(hwndDlg);
		break; // WM_INITDIALOG
	case WM_COMMAND:
		if ((HWND)lParam == hListAccs) {
			DWORD SelItems[16];
			BYTE SelItemsCount;
			if (HIWORD(wParam) == LBN_SELCHANGE) {
				SelItemsCount = SendMessage(hListAccs, LB_GETSELCOUNT, 0, 0);
				SendMessage(hListAccs,
					LB_GETSELITEMS,
					SelItemsCount,
					(LPARAM)SelItems);
				for (Stat_SelAcc = i = 0; i < SelItemsCount; i++)
					Stat_SelAcc |= 1 << SelItems[i];
				Stat_Show(hwndDlg);
			}
		}
		switch (LOWORD(wParam)) {
		case IDC_COMBO_UNITS:
			if (HIWORD(wParam) != CBN_SELCHANGE) break;
			unOptions.Stat_Units = SendDlgItemMessage(hwndDlg, IDC_COMBO_UNITS, CB_GETCURSEL, 0, 0);
			Stat_Show(hwndDlg);
			break;
		case IDC_BUTTON_CLEAR:
			{
				SYSTEMTIME stNow;

				if (IDOK != MessageBox(hwndDlg,
					TranslateT("Now traffic statistics for selected accounts will be cleared.\nContinue?"),
					TranslateT("Traffic counter"),
					MB_OKCANCEL | MB_ICONWARNING))
					break;
				GetLocalTime(&stNow);
				for (i = NumberOfAccounts; i--;)
					if (0x01 & (Stat_SelAcc >> i)) {
						SetFilePointer(ProtoList[i].hFile, sizeof(HOURLYSTATS), NULL, FILE_BEGIN);
						SetEndOfFile(ProtoList[i].hFile); // ������� ���� �� ����� ������.
						ProtoList[i].NumberOfRecords = 1;
						ProtoList[i].AllStatistics = (HOURLYSTATS*)mir_realloc(ProtoList[i].AllStatistics, sizeof(HOURLYSTATS));
						ProtoList[i].AllStatistics[0].Hour = stNow.wHour;
						ProtoList[i].AllStatistics[0].Day = stNow.wDay;
						ProtoList[i].AllStatistics[0].Month = stNow.wMonth;
						ProtoList[i].AllStatistics[0].Year = stNow.wYear;
						ProtoList[i].AllStatistics[0].Incoming = 0;
						ProtoList[i].AllStatistics[0].Outgoing = 0;
						ProtoList[i].StartIncoming = 0;
						ProtoList[i].StartOutgoing = 0;
						ProtoList[i].AllStatistics[0].Time = 0;
						ProtoList[i].Total.TimeAtStart = GetTickCount();
						Stat_CheckStatistics(i);
					}
				Stat_Show(hwndDlg);
			}
			break;
		}
		break; // WM_COMMAND
	case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;

			switch (lpnmhdr->idFrom) {
			case IDC_TAB_STATS:
				if (lpnmhdr->code != TCN_SELCHANGE) break;
				unOptions.Stat_Tab = SendDlgItemMessage(hwndDlg, IDC_TAB_STATS, TCM_GETCURSEL, 0, 0);
				Stat_Show(hwndDlg);
				break;
			case IDC_LIST_DATA:
				switch (lpnmhdr->code) {
				case LVN_GETDISPINFO:
					{
						NMLVDISPINFO* pdi = (NMLVDISPINFO*)lParam;
						SYSTEMTIME st = { 0 };
						DWORD Index, Value;
						double vartime;
						TCHAR szBufW[64];
						BYTE EldestAcc;

						if (!(pdi->item.mask & LVIF_TEXT)) return 0;

						// ���� ����� �������.
						if (!pdi->item.iSubItem) {
							EldestAcc = Stat_GetEldestAcc(Stat_SelAcc);
							// ������ �������� ������ ��� ������ ������� ��������!
							Index = Stat_GetStartIndex(EldestAcc, unOptions.Stat_Tab, pdi->item.iItem, &st);
							switch (unOptions.Stat_Tab) {
							case 0: // Hourly
								GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, szBufW, 32);
								mir_sntprintf(pdi->item.pszText, 32, _T("%s %02d:00 - %02d:59"),
									szBufW,
									ProtoList[EldestAcc].AllStatistics[Index].Hour,
									ProtoList[EldestAcc].AllStatistics[Index].Hour);
								break;
							case 1: // Dayly
								GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, pdi->item.pszText, 32);
								break;
							case 2: // Weekly
								// ������ � ������� ������������ �����.
								SystemTimeToVariantTime(&st, &vartime);
								vartime -= DayOfWeek(st.wDay, st.wMonth, st.wYear) - 1;
								VariantTimeToSystemTime(vartime, &st);
								GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, pdi->item.pszText, 32);
								// ������ � �����������.
								SystemTimeToVariantTime(&st, &vartime);
								vartime += 6;
								VariantTimeToSystemTime(vartime, &st);
								GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, szBufW, 32);
								mir_sntprintf(pdi->item.pszText, 32, _T("%s - %s"), pdi->item.pszText, szBufW);
								break;
							case 3: // Monthly
								GetDateFormat(LOCALE_USER_DEFAULT, DATE_YEARMONTH, &st, NULL, pdi->item.pszText, 32);
								break;
							case 4:	// Yearly
								mir_sntprintf(pdi->item.pszText, 32, _T("%d"), st.wYear);
								break;
							}
							return 0;
						}

						Value = Stat_GetItemValue(Stat_SelAcc, unOptions.Stat_Tab, pdi->item.iItem, pdi->item.iSubItem);

						// ������ ����� �������� � ListView �������.
						switch (pdi->item.iSubItem) {
						case 1: // ��������
						case 2: // ���������
						case 3: // �����
							GetFormattedTraffic(Value, unOptions.Stat_Units, pdi->item.pszText, 32);
							break;
						case 4: // �����
							{
								TCHAR *Fmt[5] = { _T("m:ss"), _T("h:mm:ss"), _T("h:mm:ss"), _T("d hh:mm:ss"), _T("d hh:mm:ss") };
								GetDurationFormatM(Value, Fmt[unOptions.Stat_Tab], pdi->item.pszText, 32);
							}
							break;
						}
					}
					break;

				case NM_CLICK:
				case LVN_ITEMCHANGED:
					{
						DWORD i, j = -1, dwTotalIncoming = 0, dwTotalOutgoing = 0;

						i = SendDlgItemMessage(hwndDlg, IDC_LIST_DATA, LVM_GETSELECTEDCOUNT, 0, 0);
						for (; i--;) {
							j = SendDlgItemMessage(hwndDlg, IDC_LIST_DATA, LVM_GETNEXTITEM, j, LVNI_SELECTED);
							dwTotalIncoming += Stat_GetItemValue(Stat_SelAcc, unOptions.Stat_Tab, j, 1);
							dwTotalOutgoing += Stat_GetItemValue(Stat_SelAcc, unOptions.Stat_Tab, j, 2);
						}
						Stat_UpdateTotalTraffic(hwndDlg, dwTotalIncoming, dwTotalOutgoing);
					}
					break;

				case NM_CUSTOMDRAW:
					{
						LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;

						switch (lplvcd->nmcd.dwDrawStage) {
						case CDDS_PREPAINT: // ����� ������� ��������� ����� ListView.
							SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW);
							return TRUE;
						case CDDS_ITEMPREPAINT: // ����� ������� ��������� ������.
							{
								COLORREF Color;
								BYTE r, g, b;

								if (lplvcd->nmcd.dwItemSpec & 0x01) {
									Color = SendDlgItemMessage(hwndDlg, IDC_LIST_DATA, LVM_GETBKCOLOR, 0, 0);
									r = GetRValue(Color);
									g = GetGValue(Color);
									b = GetBValue(Color);
									r += r > 0x80 ? -40 : 40;
									g += g > 0x80 ? -40 : 40;
									b += b > 0x80 ? -40 : 40;
									lplvcd->clrTextBk = RGB(r, g, b);
								}
								SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, CDRF_NEWFONT);
								return TRUE;
							}
						}
					}
					break;
				}
				break;
			}
		}
		break; // WM_NOTIFY
	}
	return 0;
}

/*
������� ������ ���������� �� ����� ��� �������� � ������� n.
*/
void Stat_ReadFile(BYTE n)
{
	LARGE_INTEGER Size;
	DWORD BytesRead;
	TCHAR FileName[MAX_PATH], *pszPath;
	SYSTEMTIME stNow;

	pszPath = Utils_ReplaceVarsT(_T("%miranda_userdata%\\statistics"));
	CreateDirectoryTreeT(pszPath);
	mir_sntprintf(FileName, _countof(FileName), _T("%s\\%S.stat"), pszPath, ProtoList[n].name);
	mir_free(pszPath);
	GetLocalTime(&stNow);
	ProtoList[n].hFile = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	GetFileSizeEx(ProtoList[n].hFile, &Size);
	if (Size.QuadPart != 0) // ���� ���� �� ����������� ���������� � ����� ��������� ������...
	{
		// ...�� ������ ���������� �� �����
		ProtoList[n].NumberOfRecords = DWORD(Size.QuadPart / sizeof(HOURLYSTATS));
		ProtoList[n].AllStatistics = (HOURLYSTATS*)mir_alloc(sizeof(HOURLYSTATS)*ProtoList[n].NumberOfRecords);
		ReadFile(ProtoList[n].hFile, &ProtoList[n].AllStatistics[0], sizeof(HOURLYSTATS)*ProtoList[n].NumberOfRecords, &BytesRead, NULL);
		if (!BytesRead) {
			MessageBox(TrafficHwnd, TranslateT("Couldn't read statistics file"), TranslateT("Traffic Counter"), MB_OK);
			return;
		}
	}
	else {
		// ���������� ������� ����� ����.
		ProtoList[n].NumberOfRecords = 1;
		ProtoList[n].AllStatistics = (HOURLYSTATS*)mir_alloc(sizeof(HOURLYSTATS));
		ProtoList[n].AllStatistics[0].Hour = stNow.wHour;
		ProtoList[n].AllStatistics[0].Day = stNow.wDay;
		ProtoList[n].AllStatistics[0].Month = stNow.wMonth;
		ProtoList[n].AllStatistics[0].Year = stNow.wYear;
		ProtoList[n].AllStatistics[0].Incoming =
			ProtoList[n].AllStatistics[0].Outgoing =
			ProtoList[n].AllStatistics[0].Time = 0;
	}
	Stat_CheckStatistics(n);
}

/* ������� ������� ����� � ListView ����������.
���������: hwndDialog - ����� ���� �������. */
void Stat_Show(HWND hwndDialog)
{
	DWORD MaxRecords;

	// ����� ������ ���������� �������.
	MaxRecords = Stat_GetRecordsNumber(Stat_GetEldestAcc(Stat_SelAcc), unOptions.Stat_Tab);
	// ��������� ����� �� ���������� ����� � ListView.
	SendDlgItemMessage(hwndDialog, IDC_LIST_DATA, LVM_SETITEMCOUNT, MaxRecords, 0);
	// ���� �������� ����� ������ ������.
	SendDlgItemMessage(hwndDialog, IDC_LIST_DATA, LVM_ENSUREVISIBLE, (WPARAM)(MaxRecords - 1), 0);
}

void Stat_UpdateTotalTraffic(HWND hwndDialog, DWORD Incoming, DWORD Outgoing)
{
	TCHAR tmp[32];

	GetFormattedTraffic(Incoming, unOptions.Stat_Units, tmp, 32);
	SetDlgItemText(hwndDialog, IDC_STATIC_DNL, tmp);
	GetFormattedTraffic(Outgoing, unOptions.Stat_Units, tmp, 32);
	SetDlgItemText(hwndDialog, IDC_STATIC_UPL, tmp);
	GetFormattedTraffic(Incoming + Outgoing, unOptions.Stat_Units, tmp, 32);
	SetDlgItemText(hwndDialog, IDC_STATIC_SUMM, tmp);
}

/*
������� ���������� � ������� ����� ��������� ������ � ���������� ��� �������� � ������� n.
���� ��� ���������, ������ �� ����������.
���� ������� ����� ������ ������� ��������� ������ (���� �������� �����),
���������� ������� ����������� �� ��������������� ���������� �����.
���� ������� ����� ������, � ���������� ���������� ����������� ���������� ������ �������.
*/
void Stat_CheckStatistics(BYTE n)
{
	SYSTEMTIME stNow, stLast = { 0 };
	HOURLYSTATS htTmp = { 0 };
	signed short int d;//, i;
	DWORD q;

	stLast.wHour = ProtoList[n].AllStatistics[ProtoList[n].NumberOfRecords - 1].Hour;
	stLast.wDay = ProtoList[n].AllStatistics[ProtoList[n].NumberOfRecords - 1].Day;
	stLast.wMonth = ProtoList[n].AllStatistics[ProtoList[n].NumberOfRecords - 1].Month;
	stLast.wYear = ProtoList[n].AllStatistics[ProtoList[n].NumberOfRecords - 1].Year;

	GetLocalTime(&stNow);
	d = TimeCompare(stNow, stLast);
	// ���� ������� ����� ��������� �� �������� ��������� ������...
	if (!d) {
		// ...��������� ������ � ���� � ������.
		SetFilePointer(ProtoList[n].hFile, -LONG(sizeof(HOURLYSTATS)), NULL, FILE_END);
		WriteFile(ProtoList[n].hFile, &ProtoList[n].AllStatistics[ProtoList[n].NumberOfRecords - 1], sizeof(HOURLYSTATS), &q, NULL);
		return;
	}

	// ���� ���� �������� �����.
	if (d < 0) {
		do {
			stLast.wHour--;
			if (stLast.wHour == 0xFFFF) {
				stLast.wDay--;
				if (!stLast.wDay) {
					stLast.wMonth--;
					if (!stLast.wMonth) {
						stLast.wMonth = 12;
						stLast.wYear--;
					}
					stLast.wDay = DaysInMonth(htTmp.Month, htTmp.Year);
				}
			}

			ProtoList[n].NumberOfRecords--;
		} while (TimeCompare(stNow, stLast));
		return;
	}

	if (d > 0) {
		// ���������.
		SetFilePointer(ProtoList[n].hFile, -LONG(sizeof(HOURLYSTATS)), NULL, FILE_END);
		WriteFile(ProtoList[n].hFile, &ProtoList[n].AllStatistics[ProtoList[n].NumberOfRecords - 1], sizeof(HOURLYSTATS), &q, NULL);

		// ��������� ������ �� ���������� ����������� ��� ���������� ����� �������, ������� �������� � (����� ������� � �������).
		memcpy(&htTmp, &ProtoList[n].AllStatistics[ProtoList[n].NumberOfRecords - 1],
			sizeof(HOURLYSTATS) - 2 * sizeof(DWORD) - sizeof(WORD));
		// ������� ������� ������ ��� ������ �������� ������� � ����.
		ProtoList[n].Total.TimeAtStart = GetTickCount() - stNow.wMilliseconds;

		do {
			ProtoList[n].AllStatistics =
				(HOURLYSTATS*)mir_realloc(ProtoList[n].AllStatistics, sizeof(HOURLYSTATS) * ++ProtoList[n].NumberOfRecords);

			htTmp.Hour++;
			if (htTmp.Hour > 23) {
				htTmp.Hour = 0; htTmp.Day++;
				if (htTmp.Day > DaysInMonth(htTmp.Month, htTmp.Year)) {
					htTmp.Day = 1; htTmp.Month++;
					if (htTmp.Month > 12) {
						htTmp.Month = 1; htTmp.Year++;
					}
				}
			}

			stLast.wHour = htTmp.Hour;
			stLast.wDay = htTmp.Day;
			stLast.wMonth = htTmp.Month;
			stLast.wYear = htTmp.Year;

			// ��������� ������ ������������ � ��� � � ����.
			WriteFile(ProtoList[n].hFile, &htTmp, sizeof(HOURLYSTATS), &q, NULL);
			memcpy(&ProtoList[n].AllStatistics[ProtoList[n].NumberOfRecords - 1], &htTmp, sizeof(HOURLYSTATS));

		} while (TimeCompare(stNow, stLast));
	}
}

/* ������� ��������� ������ ������ ������ � ����������, ����������� � ���������� ���������.
��� ���������� ����������� ��������� �������� � ��������.
���������:
ItemNumber - ����� ������ � ListView (����� �������).
stReq - ����, ��������������� ������������ �������.
*/
DWORD Stat_GetStartIndex(BYTE AccNum, BYTE Interval, DWORD ItemNumber, SYSTEMTIME *stReq)
{
	DWORD Left, Right, Probe; // ������� ��������� ��� ������ (������� ����������).
	SYSTEMTIME stProbe = { 0 }; // ����� ����.
	signed short int d = 1;

	if (!ItemNumber) {
		stReq->wHour = ProtoList[AccNum].AllStatistics[0].Hour;
		stReq->wDay = ProtoList[AccNum].AllStatistics[0].Day;
		stReq->wMonth = ProtoList[AccNum].AllStatistics[0].Month;
		stReq->wYear = ProtoList[AccNum].AllStatistics[0].Year;
		return 0;
	}

	// ��������� �����, ��������������� ������ ���������.
	for (Probe = 0, Left = 1; Left < ProtoList[AccNum].NumberOfRecords; Left++) {
		switch (Interval) {
		case STAT_INTERVAL_HOUR:
			stReq->wHour = ProtoList[AccNum].AllStatistics[ItemNumber].Hour;
			stReq->wDay = ProtoList[AccNum].AllStatistics[ItemNumber].Day;
			stReq->wMonth = ProtoList[AccNum].AllStatistics[ItemNumber].Month;
			stReq->wYear = ProtoList[AccNum].AllStatistics[ItemNumber].Year;
			return ItemNumber;
			break;
		case STAT_INTERVAL_DAY:
			if (ProtoList[AccNum].AllStatistics[Left].Day != ProtoList[AccNum].AllStatistics[Left - 1].Day)
				Probe++;
			break;
		case STAT_INTERVAL_WEEK:
			if (!ProtoList[AccNum].AllStatistics[Left].Hour
				&& DayOfWeek(ProtoList[AccNum].AllStatistics[Left].Day,
				ProtoList[AccNum].AllStatistics[Left].Month,
				ProtoList[AccNum].AllStatistics[Left].Year) == 1)
				Probe++;
			break;
		case STAT_INTERVAL_MONTH:
			if (ProtoList[AccNum].AllStatistics[Left].Month != ProtoList[AccNum].AllStatistics[Left - 1].Month)
				Probe++;
			break;
		case STAT_INTERVAL_YEAR:
			if (ProtoList[AccNum].AllStatistics[Left].Year != ProtoList[AccNum].AllStatistics[Left - 1].Year)
				Probe++;
			break;
		}
		if (Probe == ItemNumber) break;
	}

	stReq->wHour = ProtoList[AccNum].AllStatistics[Left].Hour;
	stReq->wDay = ProtoList[AccNum].AllStatistics[Left].Day;
	stReq->wMonth = ProtoList[AccNum].AllStatistics[Left].Month;
	stReq->wYear = ProtoList[AccNum].AllStatistics[Left].Year;

	Left = 0; Right = ProtoList[AccNum].NumberOfRecords - 1;

	// ��������� ������ ������ ���������.
	while (TRUE) {
		if (Right - Left == 1) return Right;
		Probe = (Left + Right) >> 1;
		stProbe.wYear = ProtoList[AccNum].AllStatistics[Probe].Year;
		stProbe.wMonth = ProtoList[AccNum].AllStatistics[Probe].Month;
		stProbe.wDay = ProtoList[AccNum].AllStatistics[Probe].Day;
		stProbe.wHour = ProtoList[AccNum].AllStatistics[Probe].Hour;
		d = TimeCompare(*stReq, stProbe);
		if (!d) break;
		if (d < 0) Right = Probe;
		if (d > 0) Left = Probe;
	}
	return Probe;
}

/* ������� ������������� �������� ������ ��� ��������� ��������,
�� ���� ����� ������ � ���������� ���������� �� ��������� ���������,
���� ������� ������������� ������ ���������� ���������� ��������. */
void Stat_SetAccShift(BYTE AccNum, BYTE EldestAccount)
{
	DWORD Left, Right, Probe = 0; // ������� ��������� ��� ������ (������� ����������).
	SYSTEMTIME stReq = { 0 }, stProbe;
	signed short int d = 1;

	if (AccNum == EldestAccount) {
		ProtoList[AccNum].Shift = 0;
		return;
	}

	stReq.wHour = ProtoList[AccNum].AllStatistics[0].Hour;
	stReq.wDay = ProtoList[AccNum].AllStatistics[0].Day;
	stReq.wMonth = ProtoList[AccNum].AllStatistics[0].Month;
	stReq.wYear = ProtoList[AccNum].AllStatistics[0].Year;

	// ��������� ������ ������ ���������.
	Left = 0; Right = ProtoList[EldestAccount].NumberOfRecords - 1;
	while (TRUE) {
		if (Right - Left == 1) {
			ProtoList[AccNum].Shift = Probe + d;
			return;
		}
		Probe = (Left + Right) >> 1;
		stProbe.wYear = ProtoList[EldestAccount].AllStatistics[Probe].Year;
		stProbe.wMonth = ProtoList[EldestAccount].AllStatistics[Probe].Month;
		stProbe.wDay = ProtoList[EldestAccount].AllStatistics[Probe].Day;
		stProbe.wHour = ProtoList[EldestAccount].AllStatistics[Probe].Hour;
		d = TimeCompare(stReq, stProbe);
		if (!d) break;
		if (d < 0) Right = Probe;
		if (d > 0) Left = Probe;
	}
	ProtoList[AccNum].Shift = Probe;
}

/* ������� ��������� ��������, ��������������� ���������� �����������
��������� ������ ListView.
���������:
SelectedAccs - �����, � ������� ��������� ���� ������������� ��������� ���������;
Interval - ��������� ��������;
ItemNum - ����� ������ � ListVew;
SubitemNum - ����� �������, ���������� ��� ����������. */

DWORD Stat_GetItemValue(WORD SelectedAccs, BYTE Interval, DWORD ItemNum, BYTE SubItemNum)
{
	DWORD Result = 0;
	SYSTEMTIME st = { 0 };
	DWORD Index, IndexP, i;
	signed long int IndexM;
	BYTE a, EldestAcc;

	EldestAcc = Stat_GetEldestAcc(SelectedAccs);
	Index = Stat_GetStartIndex(EldestAcc, Interval, ItemNum, &st);

	for (a = NumberOfAccounts; a--;) {
		if (!((1 << a) & SelectedAccs)) continue;
		Stat_SetAccShift(a, EldestAcc);
		IndexM = Index - ProtoList[a].Shift;
		IndexP = Index;

		for (i = 0;;) {
			if (IndexM >= 0)
				switch (SubItemNum) {
				case 1: // ��������
					Result += ProtoList[a].AllStatistics[IndexM].Incoming;
					break;
				case 2: // ���������
					Result += ProtoList[a].AllStatistics[IndexM].Outgoing;
					break;
				case 3: // �����
					Result += ProtoList[a].AllStatistics[IndexM].Incoming
						+ ProtoList[a].AllStatistics[IndexM].Outgoing;
					break;
				case 4: // �����
					Result += ProtoList[a].AllStatistics[IndexM].Time;
					break;
			}

			IndexM++; IndexP++; // ��������� � ��������� ������.
			if (IndexM == ProtoList[a].NumberOfRecords)
				break;

			// ����� ������������?
			switch (Interval) {
			case STAT_INTERVAL_HOUR:
				i = 1; // ����� ��� ���������� ������ ���.
				break;
			case STAT_INTERVAL_DAY:
				i = (0 == ProtoList[EldestAcc].AllStatistics[IndexP].Hour);
				break;
			case STAT_INTERVAL_WEEK:
				i = (1 == DayOfWeek(ProtoList[EldestAcc].AllStatistics[IndexP].Day,
					ProtoList[EldestAcc].AllStatistics[IndexP].Month,
					ProtoList[EldestAcc].AllStatistics[IndexP].Year))
					&& (0 == ProtoList[EldestAcc].AllStatistics[IndexP].Hour);
				break;
			case STAT_INTERVAL_MONTH:
				i = (1 == ProtoList[EldestAcc].AllStatistics[IndexP].Day)
					&& (0 == ProtoList[EldestAcc].AllStatistics[IndexP].Hour);
				break;
			case STAT_INTERVAL_YEAR:
				i = (1 == ProtoList[EldestAcc].AllStatistics[IndexP].Month)
					&& (1 == ProtoList[EldestAcc].AllStatistics[IndexP].Day)
					&& (0 == ProtoList[EldestAcc].AllStatistics[IndexP].Hour);
				break;
			}

			if (i) break;
		}
	}

	return Result;
}

/* ������� ���������� ���������� ������� � ���������� ���
��������� �������� � ��������� ���������. */
DWORD Stat_GetRecordsNumber(BYTE AccNum, BYTE Interval)
{
	DWORD Result, i;

	// ����� ������ ���������� �������.
	switch (Interval) {
	case STAT_INTERVAL_HOUR:
		Result = ProtoList[AccNum].NumberOfRecords; // ��� ��������� ���������� ���������.
		break;
	case STAT_INTERVAL_DAY:
		for (Result = 1, i = ProtoList[AccNum].NumberOfRecords - 1; i--;)
			if (ProtoList[AccNum].AllStatistics[i].Day != ProtoList[AccNum].AllStatistics[i + 1].Day)
				Result++;
		break;
	case STAT_INTERVAL_WEEK:
		for (Result = 1, i = ProtoList[AccNum].NumberOfRecords; i--;)
			if (ProtoList[AccNum].AllStatistics[i].Hour == 23
				&& DayOfWeek(ProtoList[AccNum].AllStatistics[i].Day,
				ProtoList[AccNum].AllStatistics[i].Month,
				ProtoList[AccNum].AllStatistics[i].Year) == 7)
				Result++;
		break;
	case STAT_INTERVAL_MONTH:
		for (Result = 1, i = ProtoList[AccNum].NumberOfRecords - 1; i--;)
			if (ProtoList[AccNum].AllStatistics[i].Month != ProtoList[AccNum].AllStatistics[i + 1].Month)
				Result++;
		break;
	case STAT_INTERVAL_YEAR:
		for (Result = 1, i = ProtoList[AccNum].NumberOfRecords - 1; i--;)
			if (ProtoList[AccNum].AllStatistics[i].Year != ProtoList[AccNum].AllStatistics[i + 1].Year)
				Result++;
		break;
	default:
		return 0;
	}

	return Result;
}

BYTE Stat_GetEldestAcc(WORD SelectedAccs)
{
	BYTE Result, i;

	// ����� ����� �������� �� ����� ���������, �������� ����� ������ ������ ������.
	// (��� ������� � ������������ ����������� �������.)
	for (Result = i = 0; i < NumberOfAccounts; i++) {
		// ���� � ����-�� ������ �����.
		if (0x01 & (SelectedAccs >> i)) {
			Result = i;
			break;
		}
	}
	// ���������� �����.
	for (; ++i < NumberOfAccounts;) {
		if (0x01 & (SelectedAccs >> i) && (ProtoList[i].NumberOfRecords > ProtoList[Result].NumberOfRecords))
			Result = i;
	}

	return Result;
}
