/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
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

#include "stdafx.h"

uint32_t CDb3Mmap::CreateNewSpace(int bytes)
{
	uint32_t ofsNew = m_dbHeader.ofsFileEnd;
	m_dbHeader.ofsFileEnd += bytes;
	if (m_dbHeader.ofsFileEnd > m_dwFileSize)
		ReMap(m_dbHeader.ofsFileEnd - m_dwFileSize);
	DBWrite(0, &m_dbHeader, sizeof(m_dbHeader));
	log2("newspace %d@%08x", bytes, ofsNew);
	return ofsNew;
}

void CDb3Mmap::DeleteSpace(uint32_t ofs, int bytes)
{
	if (ofs + bytes == m_dbHeader.ofsFileEnd)	{
		log2("freespace %d@%08x", bytes, ofs);
		m_dbHeader.ofsFileEnd = ofs;
	}
	else {
		log2("deletespace %d@%08x", bytes, ofs);
		m_dbHeader.slackSpace += bytes;
	}
	DBWrite(0, &m_dbHeader, sizeof(m_dbHeader));
	DBFill(ofs, bytes);
}

uint32_t CDb3Mmap::ReallocSpace(uint32_t ofs, int oldSize, int newSize)
{
	if (oldSize >= newSize)
		return ofs;

	uint32_t ofsNew;
	if (ofs + oldSize == m_dbHeader.ofsFileEnd) {
		ofsNew = ofs;
		m_dbHeader.ofsFileEnd += newSize - oldSize;
		DBWrite(0, &m_dbHeader, sizeof(m_dbHeader));
		log3("adding newspace %d@%08x+%d", newSize, ofsNew, oldSize);
	}
	else {
		ofsNew = CreateNewSpace(newSize);
		DBMoveChunk(ofsNew, ofs, oldSize);
		DeleteSpace(ofs, oldSize);
	}
	return ofsNew;
}

/////////////////////////////////////////////////////////////////////////////////////////

static uint32_t DatabaseCorrupted = 0;
static wchar_t *msg = nullptr;
static uint32_t dwErr = 0;
static wchar_t tszPanic[] = LPGENW("Miranda has detected corruption in your database. Miranda will now shut down.");

void __cdecl dbpanic(void *)
{
	if (msg) {
		if (dwErr == ERROR_DISK_FULL)
			msg = TranslateT("Disk is full. Miranda will now shut down.");

		wchar_t err[256];
		mir_snwprintf(err, msg, TranslateT("Database failure. Miranda will now shut down."), dwErr);

		MessageBox(nullptr, err, TranslateT("Database Error"), MB_SETFOREGROUND | MB_TOPMOST | MB_APPLMODAL | MB_ICONWARNING | MB_OK);
	}
	else MessageBox(nullptr, TranslateW(tszPanic), TranslateT("Database Panic"), MB_SETFOREGROUND | MB_TOPMOST | MB_APPLMODAL | MB_ICONWARNING | MB_OK);
	TerminateProcess(GetCurrentProcess(), 255);
}

void CDb3Mmap::DatabaseCorruption(wchar_t *text)
{
	int kill = 0;

	mir_cslockfull lck(m_csDbAccess);
	if (DatabaseCorrupted == 0) {
		DatabaseCorrupted++;
		kill++;
		msg = text;
		dwErr = GetLastError();
	}
	else {
		/* db is already corrupted, someone else is dealing with it, wait here
		so that we don't do any more damage */
		Sleep(INFINITE);
		return;
	}
	lck.unlock();

	if (kill) {
		_beginthread(dbpanic, 0, nullptr);
		Sleep(INFINITE);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

#ifdef DBLOGGING
char* printVariant(DBVARIANT* p)
{
	static char boo[1000];

	switch (p->type) {
	case DBVT_BYTE:	 mir_snprintf(boo, "byte: %d", p->bVal ); break;
	case DBVT_WORD:	 mir_snprintf(boo, "word: %d", p->wVal ); break;
	case DBVT_DWORD:	 mir_snprintf(boo, "dword: %d", p->dVal ); break;
	case DBVT_UTF8:
	case DBVT_ASCIIZ:  mir_snprintf(boo, "string: '%s'", p->pszVal); break;
	case DBVT_DELETED: mir_strcpy(boo, "deleted"); break;
	default:				 mir_snprintf(boo, "crap: %d", p->type ); break;
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
