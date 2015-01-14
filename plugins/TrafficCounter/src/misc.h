typedef struct
{
	TCHAR Alignment;	// ������������. L - � ����� �������, R - � ������.
	WORD Interval;		// ����������, �� ������� ������� ������ ������� �� ������ ���� ������.
	TCHAR *String;		// ���������� ������.
} RowItemInfo;

/* ������� ��������� ������ � ���������� ������ ����� � ��������������� �� �����.
���������:
InputString - ������ ��� �������;
RowItemsList - ������ ��������� ���������.
������������ �������� - ���������� ��������� � �������. */
WORD GetRowItems(TCHAR *InputString, RowItemInfo **RowItemsList);

/* ������� ���������� ���������� ���� � ��������� ������ ���������� ����. */
BYTE DaysInMonth(BYTE Month, WORD Year);

// ������� ���������� ���� ������ �� ����
// 7 - ��, 1 - �� � �. �.
BYTE DayOfWeek(BYTE Day, BYTE Month, WORD Year);

/* ���������:
	Value - ���������� ����;
	Unit - ������� ��������� (0 - �����, 1 - ���������, 2 - ���������, 3 - �������������);
	Buffer - ����� ������ ��� ������ ����������;
	Size - ������ ������.
������������ ��������: ��������� ������ ������. */
size_t GetFormattedTraffic(DWORD Value, BYTE Unit, TCHAR *Buffer, size_t Size);

size_t GetDurationFormatM(DWORD Duration, TCHAR *Format, TCHAR *Buffer, WORD Size);

signed short int TimeCompare(SYSTEMTIME st1, SYSTEMTIME st2);