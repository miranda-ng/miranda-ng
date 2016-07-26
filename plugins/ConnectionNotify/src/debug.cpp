#include "stdafx.h"

void _OutputDebugString(wchar_t* lpOutputString, ...)
{
	CMString format;
	va_list args;
	va_start(args, lpOutputString);
	format.FormatV(lpOutputString, args);
	va_end(args);

	format.AppendChar('\n');
	OutputDebugString(format);
}
