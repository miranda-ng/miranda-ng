/*
Copyright (C) 2006-2007 Scott Ellis
Copyright (C) 2007-2011 Jan Holub

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#ifndef _STR_UTILS_INC
#define _STR_UTILS_INC

void set_codepage();

bool a2w(const char *as, wchar_t *buff, int bufflen);
bool w2a(const wchar_t *ws, char *buff, int bufflen);

bool utf2w(const char *us, wchar_t *buff, int bufflen);
bool w2utf(const wchar_t *ws, char *buff, int bufflen);

bool a2utf(const char *as, char *buff, int bufflen);
bool utf2a(const char *ws, char *buff, int bufflen);

bool t2w(const TCHAR *ts, wchar_t *buff, int bufflen);
bool w2t(const wchar_t *ws, TCHAR *buff, int bufflen);

bool t2a(const TCHAR *ts, char *buff, int bufflen);
bool a2t(const char *as, TCHAR *buff, int bufflen);

bool t2utf(const TCHAR *ts, char *buff, int bufflen);
bool utf2t(const char *us, TCHAR *buff, int bufflen);

// remember to free return value
wchar_t *a2w(const char *as);
char *w2a(const wchar_t *ws);

wchar_t *utf2w(const char *us);
char *w2utf(const wchar_t *ws);

char *utf2a(const char *us);
char *a2utf(const char *as);

wchar_t *t2w(const TCHAR *ts);
TCHAR *w2t(const wchar_t *ws);

TCHAR *utf2t(const char *us);
char *t2utf(const TCHAR *ts);

char *t2a(const TCHAR *ts);
TCHAR *a2t(const char *as);

TCHAR *myfgets(TCHAR *Buf, int MaxCount, FILE *File);

#endif

