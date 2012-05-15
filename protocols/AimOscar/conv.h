/*
Plugin of Miranda IM for communicating with users of the AIM protocol.
Copyright (c) 2008-2009 Boris Krasnovskiy
Copyright (C) 2005-2006 Aaron Myles Landwehr

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef CONV_H
#define CONV_H

char* process_status_msg (const char *src, const char* sn);
void  html_decode(char* str);
char* html_encode(const char *src);
char* html_to_bbcodes(char *src);
char* bbcodes_to_html(const char *src);
void strip_tag(char* begin, char* end);
char* strip_tag_within(char* begin, char* end);
char* rtf_to_html(HWND hwndDlg,int DlgItem);
void wcs_htons(wchar_t * ch);
char* bytes_to_string(char* bytes, int num_bytes);
void string_to_bytes(char* string, char* bytes);
bool is_utf(const char* msg);
char* get_fname(char* path);
TCHAR* get_dir(TCHAR* path);

struct aimString
{
	union
	{
		char* szString;
		wchar_t* wszString;
	};
	size_t size;
	bool unicode;

	aimString(char* str);
	~aimString() { mir_free(szString); }

	bool isUnicode(void) { return unicode; }
	unsigned short getSize(void) { return (unsigned short)size; }
	unsigned short getTermSize(void) { return (unsigned short)(size + (unicode ? sizeof(wchar_t) : sizeof(char))); }
	char* getBuf(void) { return szString; }  
};

#endif
