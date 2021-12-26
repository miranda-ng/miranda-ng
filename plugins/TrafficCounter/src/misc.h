#pragma once

typedef struct
{
	wchar_t Alignment;	// Выравнивание. L - к левой границе, R - к правой.
	uint16_t Interval;		// Расстояние, на которое граница строки отстоит от левого края фрейма.
	wchar_t *String;		// Собственно строка.
} RowItemInfo;

/* Функция разбирает строку и возвращает список тегов и соответствующих им строк.
Аргументы:
InputString - строка для разбора;
RowItemsList - список найденных элементов.
Возвращаемое значение - количество элементов в списках. */
uint16_t GetRowItems(wchar_t *InputString, RowItemInfo **RowItemsList);

/* Функция возвращает количество дней в указанном месяце указанного года. */
uint8_t DaysInMonth(uint8_t Month, uint16_t Year);

// Функция определяет день недели по дате
// 7 - ВС, 1 - ПН и т. д.
uint8_t DayOfWeek(uint8_t Day, uint8_t Month, uint16_t Year);

/* Аргументы:
	Value - количество байт;
	Unit - единицы измерения (0 - байты, 1 - килобайты, 2 - мегабайты, 3 - автоматически);
	Buffer - адрес строки для записи результата;
	Size - размер буфера.
Возвращаемое значение: требуемый размер буфера. */
size_t GetFormattedTraffic(uint32_t Value, uint8_t Unit, wchar_t *Buffer, size_t Size);

size_t GetDurationFormatM(uint32_t Duration, wchar_t *Format, wchar_t *Buffer, size_t Size);

signed short int TimeCompare(SYSTEMTIME st1, SYSTEMTIME st2);