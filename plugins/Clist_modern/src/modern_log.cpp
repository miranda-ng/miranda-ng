#include "hdr/modern_commonheaders.h"

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
	if (file_tmp == NULL)
		file_tmp = file;
	else
		file_tmp++;

	va_start(vararg, fmt);
	mir_vsnprintf(str, SIZEOF(str), fmt, vararg);
	va_end(vararg);
	{
		char * tmp = str;
		while (*tmp != '\0')
		{
			if (*tmp == '\n') *tmp = ' ';
			tmp++;
		}
	}
	mir_snprintf(buf, SIZEOF(buf), "clist_modern:[%u - %u]: %s \t\t(%s Ln %d)\n", GetCurrentThreadId(), GetTickCount(), str, file_tmp, line);
#ifdef _FILELOG_
	{
		FILE *fp;
		fp = fopen(_FILELOG_,"at");	
		fprintf(fp,buf);
		fclose(fp);
	}
#else
	OutputDebugStringA(buf);
#endif
}

