#include "stdafx.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>


void Log(const char *file, int line, const char *fmt, ...)
{


	va_list vararg;
	const char *file_tmp;
	char str[1024];
	char buf[1024];

	file_tmp = strrchr(file, '\\');
	if (file_tmp == nullptr)
		file_tmp = file;
	else
		file_tmp++;

	va_start(vararg, fmt);
	mir_vsnprintf(str, _countof(str), fmt, vararg);
	va_end(vararg);
	{
		char * tmp = str;
		while (*tmp != '\0')
		{
			if (*tmp == '\n') *tmp = ' ';
			tmp++;
		}
	}
	mir_snprintf(buf, "clist_modern:[%u - %u]: %s \t\t(%s Ln %d)\n", GetCurrentThreadId(), GetTickCount(), str, file_tmp, line);
	Netlib_Log(nullptr, buf);
	OutputDebugStringA(buf);
}

