/*
 * Miranda-IM Vypress Chat/quickChat plugins
 * Copyright (C) Saulius Menkevicius
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: util.c,v 1.7 2005/04/11 21:44:16 bobas Exp $
 */

#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "util.h"

/* exported routines
 */

/* ip/ipx address funcs
 */
char * util_vqpaddr2str(vqp_addr_t addr)
{
	char * str = malloc(32);
	
	if(addr.conn == VQP_PROTOCOL_CONN_UDP) {
		sprintf(str, "%u.%u.%u.%u",
			(unsigned)(addr.node.ip >> 24) & 0xff,
			(unsigned)(addr.node.ip >> 16) & 0xff,
			(unsigned)(addr.node.ip >> 8) & 0xff, (unsigned)addr.node.ip & 0xff);
	} else {
		sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X",
			addr.node.ipx[5], addr.node.ipx[4], addr.node.ipx[3],
			addr.node.ipx[2], addr.node.ipx[1], addr.node.ipx[0]);
	}
	return str;
}

/* utf8/locale/unicode conversion
 */
char * util_loc2utf(const char * loc_str)
{
	wchar_t * uni_str = util_loc2uni(loc_str);
	char * utf_str = util_uni2utf(uni_str);
	free(uni_str);
	
	return utf_str;
}

char * util_utf2loc(const char * utf_str)
{
	wchar_t * uni_str = util_utf2uni(utf_str);
	char * loc_str = util_uni2loc(uni_str);
	free(uni_str);
	
	return loc_str;
}

wchar_t * util_loc2uni(const char * loc_str)
{
	int loc_len, uni_len;
	wchar_t * uni_str;
	
	loc_len = lstrlenA(loc_str) + 1;
	uni_len = MultiByteToWideChar(CP_ACP, 0, loc_str, loc_len, NULL, 0);
	uni_str = malloc(sizeof(wchar_t) * uni_len);
	MultiByteToWideChar(CP_ACP, 0, loc_str, loc_len, uni_str, uni_len);
	
	return uni_str;
}

char * util_uni2loc(const wchar_t * uni_str)
{
	int uni_len, loc_len;
	char * loc_str;

	uni_len = lstrlenW(uni_str) + 1;
	loc_len = WideCharToMultiByte(CP_ACP, 0, uni_str, uni_len, NULL, 0, NULL, NULL);
	loc_str = malloc(loc_len);
	WideCharToMultiByte(CP_ACP, 0, uni_str, uni_len, loc_str, loc_len, NULL, NULL);
	
	return loc_str;
}

wchar_t * util_utf2uni(const char * utf_str)
{
	int utf_len, uni_len;
	wchar_t * uni_str;
	
	utf_len = lstrlenA(utf_str) + 1;
	uni_len = MultiByteToWideChar(CP_UTF8, 0, utf_str, utf_len, NULL, 0);
	uni_str = malloc(sizeof(wchar_t) * uni_len);
	MultiByteToWideChar(CP_UTF8, 0, utf_str, utf_len, uni_str, uni_len);
	
	return uni_str;
}

char * util_uni2utf(const wchar_t * uni_str)
{
	int uni_len, utf_len;
	char * utf_str;

	uni_len = lstrlenW(uni_str) + 1;
	utf_len = WideCharToMultiByte(CP_UTF8, 0, uni_str, uni_len, NULL, 0, NULL, NULL);
	utf_str = malloc(utf_len);
	WideCharToMultiByte(CP_UTF8, 0, uni_str, uni_len, utf_str, utf_len, NULL, NULL);
	
	return utf_str;
}

/* string functions
 */
char * util_vqp2utf(enum vqp_codepage codepage, const char * vqp_text)
{
	return codepage == VQP_CODEPAGE_UTF8
			? strdup(vqp_text): util_loc2utf(vqp_text);
}

char * util_utf2vqp(enum vqp_codepage codepage, const char * utf_text)
{
	return codepage == VQP_CODEPAGE_UTF8
			? strdup(utf_text): util_utf2loc(utf_text);
}

/* util_split_multiline:
 *	splits a multiline string into it's lines,
 *	optionally emitting the empty ones
 * returns:
 *	an array of (char*), with the last being NULL
 */
char ** util_split_multiline(const char * text, int include_empty)
{
	int n_lines, i;
	const char * line, * next;
	char ** array, * str;
	
	ASSERT_RETURNVALIFFAIL(VALIDPTR(text), NULL);

	/* get the number of lines */
	n_lines = 1;
	line = text;
	while((line = strchr(line, '\n')) != NULL) {
		n_lines ++;
		line ++;	/* skip the '\n' we've found */
	}

	/* alloc the array */
	array = malloc(sizeof(void *) * (n_lines + 1));

	/* split the strings */
	line = text;
	i = 0;
	do {
		int line_len;
		
		/* get the end of this line */
		next = strchr(line, '\n');
		if(next) {
			/* get the length of the line */
			line_len = next - line;
			
			/* skip the '\r' at the end of the line, if exists */
			if(line[line_len - 1] == '\r')
				line_len --;
		} else {
			line_len = strlen(line);
		}

		/* alloc and copy the string */
		if(include_empty || (!include_empty && line_len!=0)) {
			str = malloc(line_len + 1);
			memcpy(str, line, line_len);
			str[line_len] = '\0';

			array[i ++] = str;
		}

		/* advance to the next line */
		if(next) {
			/* skip pas the \n char onto the next line */
			line = next + 1;
		}
	} while(next);

	/* end up the array */
	array[i] = NULL;

	/* return all we got here */
	return array;
}

/* util_free_str_list:
 *	frees a (char*) list, like the one alloced
 *	by util_split_multiline()
 */
void util_free_str_list(char ** str_list)
{
	char ** pstr;
	
	ASSERT_RETURNIFFAIL(VALIDPTR(str_list));

	/* free each of the strings in the list */
	for(pstr = str_list; *pstr; pstr ++)
		free(*pstr);

	free(str_list);
}

/* windows unicode functions
 */
void util_SetWindowTextUtf(HWND hwnd, const char * utf_str)
{
	wchar_t * unistr;

	ASSERT_RETURNIFFAIL(utf_str);
	
	unistr = util_utf2uni(utf_str);
	SetWindowTextW(hwnd, unistr);
	free(unistr);
}

char * util_GetWindowTextUtf(HWND hwnd)
{
	int len;
	wchar_t * uni_str;
	char * utf_str;

	len = GetWindowTextLengthW(hwnd) + 1;
	uni_str = malloc(sizeof(wchar_t) * len);
	GetWindowTextW(hwnd, uni_str, len);

	utf_str = util_uni2utf(uni_str);
	free(uni_str);
	
	return utf_str;
}

void util_SetDlgItemTextUtf(HWND hDialog, int nDlgItemN, const char * utf_str)
{
	util_SetWindowTextUtf(GetDlgItem(hDialog, nDlgItemN), utf_str);
}

char * util_GetDlgItemTextUtf(HWND hDialog, int nDlgItemN)
{
	return util_GetWindowTextUtf(GetDlgItem(hDialog, nDlgItemN));
}

/* db module extension routines
 */

int db_blob_set(
	HANDLE hContact, const char * szModule, const char * szSetting,
	void * pBlob, size_t cpBlob)
{
	DBCONTACTWRITESETTING cws;
	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_BLOB;
	cws.value.cpbVal = cpBlob;
	cws.value.pbVal = pBlob;
	return CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&cws);
}

BOOL db_dword_list_add(
	HANDLE hContact, const char * szModule, const char * szSetting,
	DWORD dw, BOOL unique)
{
	DWORD * list, * new_list;
	size_t list_sz;

	if(unique && db_dword_list_contains(hContact, szModule, szSetting, dw))
		return FALSE;

	/* append dword and write the new list */
	list = db_dword_list(hContact, szModule, szSetting, &list_sz);
	if(list) {
		new_list = malloc(sizeof(DWORD) * (list_sz + 1));
		if(!new_list) {
			free(list);
			return FALSE;
		}
		memcpy(new_list, list, sizeof(DWORD) * list_sz);
		new_list[list_sz] = dw;

		db_blob_set(hContact, szModule, szSetting, new_list, sizeof(DWORD) * (list_sz + 1));

		free(new_list);
		free(list);
	} else {
		db_blob_set(hContact, szModule, szSetting, &dw, sizeof(DWORD));
	}

	return TRUE;
}

void db_dword_list_remove(
	HANDLE hContact, const char * szModule, const char * szSetting, DWORD dw)
{
	DWORD * list;
	size_t list_sz;

	list = db_dword_list(hContact, szModule, szSetting, &list_sz);
	if(list) {
		size_t i;
		for(i = 0; i < list_sz; i++)
			if(list[i] == dw) {
				if(list_sz != 1) {
					char * new_list = malloc(sizeof(DWORD) * (list_sz - 1));
					if(!new_list) break;

					memcpy(new_list, list, sizeof(DWORD) * i);
					memcpy(new_list + i, list + i + 1,
							sizeof(DWORD) * (list_sz - i - 1));

					db_blob_set(hContact, szModule, szSetting,
							new_list, sizeof(DWORD) * (list_sz - 1));
				} else {
					db_unset(hContact, szModule, szSetting);
				}

				break;
			}
	}
	free(list);
}

DWORD * db_dword_list(
	HANDLE hContact, const char * szModule, const char * szSetting, size_t * pCount)
{
	DBVARIANT dbv;
	DWORD * list = NULL;

	ASSERT_RETURNVALIFFAIL(szModule!=NULL && szSetting!=NULL && pCount!=NULL, NULL);

	*pCount = 0;
	
	if(!db_get(hContact, szModule, szSetting, &dbv)) {
		if(dbv.type == DBVT_BLOB && (dbv.cpbVal % sizeof(DWORD))==0 && dbv.cpbVal!=0) {
			list = malloc(dbv.cpbVal);
			if(list) {
				memcpy(list, dbv.pbVal, dbv.cpbVal);
				*pCount = dbv.cpbVal / sizeof(DWORD);
			}
		}
	}
	
	db_free(&dbv);
	return list;
}

BOOL db_dword_list_contains(
	HANDLE hContact, const char * szModule,
	const char * szSetting, DWORD dw)
{
	DWORD * list;
	size_t list_sz, i;

	list = db_dword_list(hContact, szModule, szSetting, &list_sz);
	if(list) {
		for(i = 0; i < list_sz; i++)
			if(list[i]==dw) return TRUE;
	}
	return FALSE;
}

