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
 * $Id: util.h,v 1.8 2005/04/11 21:44:17 bobas Exp $
 */

#ifndef __UTIL_H
#define __UTIL_H

/* ip/ipx address funcs */
char * util_vqpaddr2str(vqp_addr_t addr);

/* utf8/locale/unicode conversion */
char * util_loc2utf(const char * loc_str);
char * util_utf2loc(const char * utf8_str);
wchar_t * util_loc2uni(const char * loc_str);
char * util_uni2loc(const wchar_t * uni_str);
wchar_t * util_utf2uni(const char * utf_str);
char * util_uni2utf(const wchar_t * uni_str);

char * util_vqp2utf(enum vqp_codepage codepage, const char * vqp_text);
char * util_utf2vqp(enum vqp_codepage codepage, const char * utf_text);

/* string functions */
char ** util_split_multiline(const char * text, int include_empty);
void util_free_str_list(char ** str_list);

/* windows unicode functions */
void util_SetWindowTextUtf(HWND hwnd, const char * utf_str);
char * util_GetWindowTextUtf(HWND hwnd);
void util_SetDlgItemTextUtf(HWND hDialog, int nDlgItemN, const char * utf_str);
char * util_GetDlgItemTextUtf(HWND hDialog, int nDlgItemN);

/* additional db helper funcs */
int db_blob_set(
	HANDLE hContact, const char * szModule, const char * szSetting,
	void * pBlob, size_t cpBlob);
BOOL db_dword_list_add(
	HANDLE hContact, const char * szModule, const char * szSetting, DWORD dw, BOOL unique);
void db_dword_list_remove(
	HANDLE hContact, const char * szModule, const char * szSetting, DWORD dw);
DWORD * db_dword_list(
	HANDLE hContact, const char * szModule, const char * szSetting, size_t * pCount);
BOOL db_dword_list_contains(
	HANDLE hContact, const char * szModule, const char * szSetting, DWORD dw);

#endif

