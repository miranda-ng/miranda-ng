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

#include "commonheaders.h"

/* Функция разбирает строку и возвращает список тегов и соответствующих им строк.
Аргументы:
InputString - строка для разбора;
RowItemsList - список найденных элементов.
Возвращаемое значение - количество элементов в списках. */
WORD GetRowItems(TCHAR *InputString, RowItemInfo **RowItemsList)
{
	TCHAR *begin, *end;
	WORD c = 0;

	// Ищем слева открывающую скобку.
	begin = _tcschr(InputString, '{');
	// Если скобка найдена...
	if (begin)
	{
		// Выделяем память под указатели
		*RowItemsList = (RowItemInfo*)mir_alloc(sizeof(RowItemInfo));
	}
	else return 0;

	do
	{
		// Сразу вслед за ней ищем закрывающую.
		end = _tcschr(begin, '}');

		// Выделяем память под указатели
		*RowItemsList = (RowItemInfo*)mir_realloc(*RowItemsList, sizeof(RowItemInfo) * (c + 1));

		// Разбираем тег.
		_stscanf(begin + 1, _T("%c%d"),
				&((*RowItemsList)[c].Alignment),
				&((*RowItemsList)[c].Interval));

		// Ищем далее открывающую скобку - это конец строки, соответствующей тегу.
		begin = _tcschr(end, '{');

		if (begin)
		{
			// Выделяем память под строку.
			(*RowItemsList)[c].String = (TCHAR*)mir_alloc(sizeof(TCHAR) * (begin - end));
			// Копируем строку.
			_tcsncpy((*RowItemsList)[c].String, end + 1, begin - end - 1);
			(*RowItemsList)[c].String[begin - end - 1] = 0;
		}
		else
		{
			// Выделяем память под строку.
			(*RowItemsList)[c].String = (TCHAR*)mir_alloc(sizeof(TCHAR) * _tcslen(end));
			// Копируем строку.
			_tcsncpy((*RowItemsList)[c].String, end + 1, _tcslen(end));
		}

		c++;
	} while (begin);

	return c;
}

/* Функция возвращает количество дней в указанном месяце указанного года. */
BYTE DaysInMonth(BYTE Month, WORD Year)
{
	switch (Month)
	{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12: return 31;
		case 4:
		case 6:
		case 9:
		case 11: return 30;
		case 2: return 28 + (BYTE)!((Year % 4) && ( (Year % 100) || !(Year % 400) ));
	}
	return 0;
}

// Функция определяет день недели по дате
// 7 - ВС, 1 - ПН и т. д.
BYTE DayOfWeek(BYTE Day, BYTE Month, WORD Year)
{
	WORD a, y, m;

	a = (14 - Month) / 12;
	y = Year - a;
	m = Month + 12 * a - 2;

	a = (7000 + (Day + y + (y >> 2) - y / 100 + y / 400 + (31 * m) / 12)) % 7;
	if (!a) a = 7;

	return a;
}

/*
Аргументы:
	Value - количество байт;
	Unit - единицы измерения (0 - байты, 1 - килобайты, 2 - мегабайты, 3 - автоматически);
	Buffer - адрес строки для записи результата;
	Size - размер буфера.
Возвращаемое значение: требуемый размер буфера.
*/
WORD GetFormattedTraffic(DWORD Value, BYTE Unit, TCHAR *Buffer, WORD Size)
{
	TCHAR Str1[32], szUnit[4] = {' ', 0};
	DWORD Divider;
	NUMBERFMT nf = {0, 1, 3, _T(","), _T(" "), 0};
	TCHAR *Res; // Промежуточный результат.
	WORD l;

	switch (Unit)
	{
		case 0: //bytes
			Divider = 1;
			nf.NumDigits = 0;
			szUnit[0] = 0;
			break;
		case 1: // KB
			Divider = 0x400;
			nf.NumDigits = 2;
			break;
		case 2: // MB
			Divider = 0x100000;
			nf.NumDigits = 2;
			break;
		case 3: // Adaptive
			nf.NumDigits = 2;
			if (Value < 0x100000) { Divider = 0x400; szUnit[1] = 'K'; szUnit[2] = 'B'; }
			else { Divider = 0x100000; szUnit[1] = 'M'; szUnit[2] = 'B'; }
			break;
	}

	mir_sntprintf(Str1, SIZEOF(Str1), _T("%d.%d"), Value / Divider, Value % Divider);
	l = GetNumberFormat(LOCALE_USER_DEFAULT, 0, Str1, &nf, NULL, 0);
	if (!l) return 0;
	l += _tcslen(szUnit) + 1;
	Res = (TCHAR*)malloc(l * sizeof(TCHAR));
	if (!Res) return 0;
	GetNumberFormat(LOCALE_USER_DEFAULT, 0, Str1, &nf, Res, l);
	_tcscat(Res, szUnit);

	if (Size && Buffer)
	{
		_tcscpy(Buffer, Res);
		l = _tcslen(Buffer);
	}
	else
	{
		l = _tcslen(Res) + 1;
	}

	free(Res);
	return l;
}

/* Преобразование интервала времени в его строковое представление
Аргументы:
Duration: интервал времени в секундах;
Format: строка формата;
Buffer: адрес буфера, куда функция помещает результат.
Size - размер буфера. */
WORD GetDurationFormatM(DWORD Duration, TCHAR *Format, TCHAR *Buffer, WORD Size)
{
	DWORD q;
	WORD TokenIndex, FormatIndex, Length;
	TCHAR Token[256],  // Аккумулятор.
		*Res; // Промежуточный результат.

	Res = (TCHAR*)malloc(sizeof(TCHAR)); // Выделяем чуть-чуть памяти под результат, но это только начало.
	//SecureZeroMemory(Res, sizeof(TCHAR));
	Res[0] = 0;

	for (FormatIndex = 0; Format[FormatIndex];)
	{
		// Ищем токены. Считается, что токен - только буквы.
		TokenIndex = 0;
		q = _istalpha(Format[FormatIndex]);
		// Копируем символы в аккумулятор до смены флага.
		do
		{
			Token[TokenIndex++] = Format[FormatIndex++];
		} while (q == _istalpha(Format[FormatIndex]));
		Token[TokenIndex] = 0;

		// Что получили в аккумуляторе?
		if (!_tcscmp(Token, _T("d")))
		{
			q = Duration / (60 * 60 * 24);
			mir_sntprintf(Token, SIZEOF(Token), _T("%d"), q);
			Duration -= q * 60 * 60 * 24;
		}
		else
		if (!_tcscmp(Token, _T("h")))
		{
			q = Duration / (60 * 60);
			mir_sntprintf(Token, SIZEOF(Token), _T("%d"), q);
			Duration -= q * 60 * 60;
		}
		else
		if (!_tcscmp(Token, _T("hh")))
		{
			q = Duration / (60 * 60);
			mir_sntprintf(Token, SIZEOF(Token), _T("%02d"), q);
			Duration -= q * 60 * 60;
		}
		else
		if (!_tcscmp(Token, _T("m")))
		{
			q = Duration / 60;
			mir_sntprintf(Token, SIZEOF(Token), _T("%d"), q);
			Duration -= q * 60;
		}
		else
		if (!_tcscmp(Token, _T("mm")))
		{
			q = Duration / 60;
			mir_sntprintf(Token, SIZEOF(Token), _T("%02d"), q);
			Duration -= q * 60;
		}
		else
		if (!_tcscmp(Token, _T("s")))
		{
			q = Duration;
			mir_sntprintf(Token, SIZEOF(Token), _T("%d"), q);
			Duration -= q;
		}
		else
		if (!_tcscmp(Token, _T("ss")))
		{
			q = Duration;
			mir_sntprintf(Token, SIZEOF(Token), _T("%02d"), q);
			Duration -= q;
		}

		// Добавим памяти, если нужно.
		Length = _tcslen(Res) + _tcslen(Token) + 1;
		Res = (TCHAR*)realloc(Res, Length * sizeof(TCHAR));
		_tcscat(Res, Token);
	}

	if (Size && Buffer)
	{
		_tcsncpy(Buffer, Res, Size);
		Length = _tcslen(Buffer);
	}
	else
	{
		Length = _tcslen(Res) + 1;
	}

	free(Res);
	return Length;
}

/* Результат:
-1 - st1 < st2
 0 - st1 = st2
+1 - st1 > st2
*/
signed short int TimeCompare(SYSTEMTIME st1, SYSTEMTIME st2)
{
	signed short int a, b, c, d;

	a = st1.wYear - st2.wYear;
	b = st1.wMonth - st2.wMonth;
	c = st1.wDay - st2.wDay;
	d = st1.wHour - st2.wHour;

	if (a < 0) return -1;
	if (a > 0) return +1;

	if (b < 0) return -1;
	if (b > 0) return +1;

	if (c < 0) return -1;
	if (c > 0) return +1;

	if (d < 0) return -1;
	if (d > 0) return +1;

	return 0;
}
