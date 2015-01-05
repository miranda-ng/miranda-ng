/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

/////////////////////////////////////////////////////////////////////////////////////////

#ifdef DBLOGGING
char* printVariant(DBVARIANT* p)
{
	static char boo[1000];

	switch (p->type) {
	case DBVT_BYTE:	 mir_snprintf(boo, SIZEOF(boo), "byte: %d", p->bVal ); break;
	case DBVT_WORD:	 mir_snprintf(boo, SIZEOF(boo), "word: %d", p->wVal ); break;
	case DBVT_DWORD:	 mir_snprintf(boo, SIZEOF(boo), "dword: %d", p->dVal ); break;
	case DBVT_UTF8:
	case DBVT_ASCIIZ:  mir_snprintf(boo, SIZEOF(boo), "string: '%s'", p->pszVal); break;
	case DBVT_DELETED: strcpy(boo, "deleted"); break;
	default:				 mir_snprintf(boo, SIZEOF(boo), "crap: %d", p->type ); break;
	}
	return boo;
}

void DBLog(const char *file,int line,const char *fmt,...)
{
	FILE *fp;
	va_list vararg;
	char str[1024];

	va_start(vararg,fmt);
	mir_vsnprintf(str,sizeof(str),fmt,vararg);
	va_end(vararg);
	fp=fopen("c:\\mirandadatabase.log.txt","at");
	fprintf(fp,"%u: %s %d: %s\n",GetTickCount(),file,line,str);
	fclose(fp);
}
#endif
