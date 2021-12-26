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

#include "stdafx.h"

/* Функция разбирает строку и возвращает список тегов и соответствующих им строк.
Аргументы:
InputString - строка для разбора;
RowItemsList - список найденных элементов.
Возвращаемое значение - количество элементов в списках. */

uint16_t GetRowItems(wchar_t *InputString, RowItemInfo **RowItemsList)
{
	wchar_t *begin, *end;
	uint16_t c = 0;

	// Ищем слева открывающую скобку.
	begin = wcschr(InputString, '{');
	// Если скобка найдена...
	if (begin) {
		// Выделяем память под указатели
		*RowItemsList = (RowItemInfo*)mir_alloc(sizeof(RowItemInfo));
	}
	else return 0;

	do {
		// Сразу вслед за ней ищем закрывающую.
		end = wcschr(begin, '}');

		// Выделяем память под указатели
		*RowItemsList = (RowItemInfo*)mir_realloc(*RowItemsList, sizeof(RowItemInfo) * (c + 1));

		// Разбираем тег.
		swscanf(begin + 1, L"%c%hd",
			&((*RowItemsList)[c].Alignment),
			&((*RowItemsList)[c].Interval));

		// Ищем далее открывающую скобку - это конец строки, соответствующей тегу.
		begin = wcschr(end, '{');

		if (begin) {
			// Выделяем память под строку.
			(*RowItemsList)[c].String = (wchar_t*)mir_alloc(sizeof(wchar_t) * (begin - end));
			// Копируем строку.
			wcsncpy((*RowItemsList)[c].String, end + 1, begin - end - 1);
			(*RowItemsList)[c].String[begin - end - 1] = 0;
		}
		else {
			// Выделяем память под строку.
			(*RowItemsList)[c].String = (wchar_t*)mir_alloc(sizeof(wchar_t) * mir_wstrlen(end));
			// Копируем строку.
			wcsncpy((*RowItemsList)[c].String, end + 1, mir_wstrlen(end));
		}

		c++;
	} while (begin);

	return c;
}

/* Функция возвращает количество дней в указанном месяце указанного года. */
uint8_t DaysInMonth(uint8_t Month, uint16_t Year)
{
	switch (Month) {
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
	case 2: return 28 + (uint8_t)!((Year % 4) && ((Year % 100) || !(Year % 400)));
	}
	return 0;
}

// Функция определяет день недели по дате
// 7 - ВС, 1 - ПН и т. д.
uint8_t DayOfWeek(uint8_t Day, uint8_t Month, uint16_t Year)
{
	uint16_t a, y, m;

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
size_t GetFormattedTraffic(uint32_t Value, uint8_t Unit, wchar_t *Buffer, size_t Size)
{
	wchar_t Str1[32], szUnit[4] = { ' ', 0 };
	uint32_t Divider;
	NUMBERFMT nf = { 0, 1, 3, L",", L" ", 0 };
	wchar_t *Res; // Промежуточный результат.

	switch (Unit) {
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
	default:
		return 0;
	}

	mir_snwprintf(Str1, L"%d.%d", Value / Divider, Value % Divider);
	size_t l = GetNumberFormat(LOCALE_USER_DEFAULT, 0, Str1, &nf, nullptr, 0);
	if (!l) return 0;
	l += mir_wstrlen(szUnit) + 1;
	Res = (wchar_t*)malloc(l * sizeof(wchar_t));
	if (!Res) return 0;
	GetNumberFormat(LOCALE_USER_DEFAULT, 0, Str1, &nf, Res, (int)l);
	mir_wstrcat(Res, szUnit);

	if (Size && Buffer) {
		mir_wstrcpy(Buffer, Res);
		l = mir_wstrlen(Buffer);
	}
	else l = mir_wstrlen(Res) + 1;

	free(Res);
	return l;
}

/* Преобразование интервала времени в его строковое представление
Аргументы:
Duration: интервал времени в секундах;
Format: строка формата;
Buffer: адрес буфера, куда функция помещает результат.
Size - размер буфера. */

size_t GetDurationFormatM(uint32_t Duration, wchar_t *Format, wchar_t *Buffer, size_t Size)
{
	size_t Length;
	wchar_t Token[256];  // Аккумулятор.

	wchar_t *Res = (wchar_t*)malloc(sizeof(wchar_t)); // Выделяем чуть-чуть памяти под результат, но это только начало.
	Res[0] = 0;

	for (int FormatIndex = 0; Format[FormatIndex];) {
		// Ищем токены. Считается, что токен - только буквы.
		int TokenIndex = 0;
		int q = iswalpha(Format[FormatIndex]);
		// Копируем символы в аккумулятор до смены флага.
		do {
			Token[TokenIndex++] = Format[FormatIndex++];
		} while (q == iswalpha(Format[FormatIndex]));
		Token[TokenIndex] = 0;

		// Что получили в аккумуляторе?
		if (!mir_wstrcmp(Token, L"d")) {
			q = Duration / (60 * 60 * 24);
			mir_snwprintf(Token, L"%d", q);
			Duration -= q * 60 * 60 * 24;
		}
		else if (!mir_wstrcmp(Token, L"h")) {
			q = Duration / (60 * 60);
			mir_snwprintf(Token, L"%d", q);
			Duration -= q * 60 * 60;
		}
		else if (!mir_wstrcmp(Token, L"hh")) {
			q = Duration / (60 * 60);
			mir_snwprintf(Token, L"%02d", q);
			Duration -= q * 60 * 60;
		}
		else if (!mir_wstrcmp(Token, L"m")) {
			q = Duration / 60;
			mir_snwprintf(Token, L"%d", q);
			Duration -= q * 60;
		}
		else if (!mir_wstrcmp(Token, L"mm")) {
			q = Duration / 60;
			mir_snwprintf(Token, L"%02d", q);
			Duration -= q * 60;
		}
		else if (!mir_wstrcmp(Token, L"s")) {
			q = Duration;
			mir_snwprintf(Token, L"%d", q);
			Duration -= q;
		}
		else if (!mir_wstrcmp(Token, L"ss")) {
			q = Duration;
			mir_snwprintf(Token, L"%02d", q);
			Duration -= q;
		}

		// Добавим памяти, если нужно.
		Length = mir_wstrlen(Res) + mir_wstrlen(Token) + 1;
		Res = (wchar_t*)realloc(Res, Length * sizeof(wchar_t));
		mir_wstrcat(Res, Token);
	}

	if (Size && Buffer) {
		wcsncpy(Buffer, Res, Size);
		Length = mir_wstrlen(Buffer);
	}
	else Length = mir_wstrlen(Res) + 1;

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
