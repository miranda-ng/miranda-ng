/*

MUCC Group Chat GUI Plugin for Miranda NG
Copyright (C) 2004  Piotr Piastucki

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
#ifndef UTILS_INCLUDED
#define UTILS_INCLUDED
#include "mucc.h"

class Utils {
public:
	static unsigned long forkThread(void (__cdecl *threadcode)(void*),unsigned long stacksize,void *arg);
	static void			 copyString(char **, const char *b);
	static void			 log(const char *fmt, ...);

//	static MM_INTERFACE  mmi;
	static void			 mucc_mir_free(void*);
	static char*		 mucc_mir_t2a(LPCTSTR);
	static LPTSTR		 mucc_mir_a2t(const char* );

};

#endif
