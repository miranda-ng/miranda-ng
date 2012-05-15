/*
    Helper functions for Miranda-IM (www.miranda-im.org)
    Copyright 2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef __DB_HELPERS_H
#define __DB_HELPERS_H

#include "commonheaders.h"

int Hlp_RemoveDatabaseSettings(HANDLE hContact, char *szModule, char *szPrefix);

/* the Hlp_DBGetContactSetting(X)String(Y) functions return a copy in local memory, free it using your own free() */
#define db_getb(c,d)	DBGetContactSettingByte(NULL,MODULENAME,c,d)
#define db_getw(c,d)	DBGetContactSettingWord(NULL,MODULENAME,c,d)
#define db_getd(c,d)	DBGetContactSettingDword(NULL,MODULENAME,c,d)
#define db_getAs(c,d)	Hlp_DBGetContactSettingString(NULL,MODULENAME,c,d)
#define db_gets(c,d)	Hlp_DBGetContactSettingTString(NULL,MODULENAME,c,d)
#define db_getWs(c,d)	Hlp_DBGetContactSettingWString(NULL,MODULENAME,c,d)
#define db_getUs(c,d)	Hlp_DBGetContactSettingStringUtf(NULL,MODULENAME,c,d)

#define db_setb(c,d)	DBWriteContactSettingByte(NULL,MODULENAME,c, (BYTE)d)
#define db_setw(c,d)	DBWriteContactSettingWord(NULL,MODULENAME,c, (WORD)d)
#define db_setd(c,d)	DBWriteContactSettingDword(NULL,MODULENAME,c, (DWORD)d)
#define db_setAs(c,d)	DBWriteContactSettingString(NULL,MODULENAME,c,d)
#define db_sets(c,d)	DBWriteContactSettingTString(NULL,MODULENAME,c,d)
#define db_setWs(c,d)	DBWriteContactSettingWString(NULL,MODULENAME,c,d)
#define db_setUs(c,d)	DBWriteContactSettingStringUtf(NULL,MODULENAME,c,d)

#define db_del(c)		DBDeleteContactSetting(NULL,MODULENAME,c);

#define dbi_getb(a,b,c)		Hlp_DBGetIthSettingByte(a, NULL, MODULENAME, b, c)
#define dbi_getw(a,b,c)		Hlp_DBGetIthSettingWord(a, NULL, MODULENAME, b, c)
#define dbi_getd(a,b,c)		Hlp_DBGetIthSettingDword(a, NULL, MODULENAME, b, c)
#define dbi_getAs(a,b,c)	Hlp_DBGetIthSettingString(a, NULL, MODULENAME, b, c)
#define dbi_gets(a,b,c)		Hlp_DBGetIthSettingTString(a, NULL, MODULENAME, b, c)
#define dbi_getWs(a,b,c)	Hlp_DBGetIthSettingWString(a, NULL, MODULENAME, b, c)
#define dbi_getUs(a,b,c)	Hlp_DBGetIthSettingStringUtf(a, NULL, MODULENAME, b, c)

#define dbi_setb(a,b,c)		Hlp_DBWriteIthSettingByte(a, NULL, MODULENAME, b, (BYTE)c)
#define dbi_setw(a,b,c)		Hlp_DBWriteIthSettingWord(a, NULL, MODULENAME, b, (WORD)c)
#define dbi_setd(a,b,c)		Hlp_DBWriteIthSettingDword(a, NULL, MODULENAME, b, (DWORD)c)
#define dbi_setAs(a,b,c)	Hlp_DBWriteIthSettingString(a, NULL, MODULENAME, b, c)
#define dbi_sets(a,b,c)		Hlp_DBWriteIthSettingTString(a, NULL, MODULENAME, b, c)
#define dbi_setWs(a,b,c)	Hlp_DBWriteIthSettingWString(a, NULL, MODULENAME, b, c)
#define dbi_setUs(a,b,c)	Hlp_DBWriteIthSettingStringUtf(a, NULL, MODULENAME, b, c)

#define dbi_del(a,b)		Hlp_DBDeleteIthSetting(a, NULL, MODULENAME, b)

#define dbs_getb(a,b,c)		Hlp_DBGetXthSettingByte(a, NULL, MODULENAME, b, c)
#define dbs_getw(a,b,c)		Hlp_DBGetXthSettingWord(a, NULL, MODULENAME, b, c)
#define dbs_getd(a,b,c)		Hlp_DBGetXthSettingDword(a, NULL, MODULENAME, b, c)
#define dbs_getAs(a,b,c)	Hlp_DBGetXthSettingString(a, NULL, MODULENAME, b, c)
#define dbs_gets(a,b,c)		Hlp_DBGetXthSettingTString(a, NULL, MODULENAME, b, c)
#define dbs_getWs(a,b,c)	Hlp_DBGetXthSettingWString(a, NULL, MODULENAME, b, c)
#define dbs_getUs(a,b,c)	Hlp_DBGetXthSettingStringUtf(a, NULL, MODULENAME, b, c)

#define dbs_setb(a,b,c)		Hlp_DBWriteXthSettingByte(a, NULL, MODULENAME, b, (BYTE)c)
#define dbs_setw(a,b,c)		Hlp_DBWriteXthSettingWord(a, NULL, MODULENAME, b, (WORD)c)
#define dbs_setd(a,b,c)		Hlp_DBWriteXthSettingDword(a, NULL, MODULENAME, b, (DWORD)c)
#define dbs_setAs(a,b,c)	Hlp_DBWriteXthSettingString(a, NULL, MODULENAME, b, c)
#define dbs_sets(a,b,c)		Hlp_DBWriteXthSettingTString(a, NULL, MODULENAME, b, c)
#define dbs_setWs(a,b,c)	Hlp_DBWriteXthSettingWString(a, NULL, MODULENAME, b, c)
#define dbs_setUs(a,b,c)	Hlp_DBWriteXthSettingStringUtf(a, NULL, MODULENAME, b, c)

#define dbs_del(a,b)		Hlp_DBDeleteXthSetting(a, NULL, MODULENAME, b)

__inline static char *Hlp_DBGetContactSettingString(HANDLE hContact,const char *szModule,
	const char *szSetting,char *errVal)
{
	int rc;
	DBCONTACTGETSETTING cgs;
	DBVARIANT dbv = {0};
	char *res;
	cgs.szModule=szModule;
	cgs.szSetting=szSetting;
	cgs.pValue=&dbv;
	dbv.type = DBVT_ASCIIZ;

	rc=CallService(MS_DB_CONTACT_GETSETTING_STR,(WPARAM)hContact,(LPARAM)&cgs);
#if defined(_DEBUG) && defined(DBCHECKSETTINGS)
	if (rc != 0) {
		char buf[128];
		_snprintf(buf,sizeof(buf),"%s:%d failed to fetch %s/%s",szFile,nLine,szModule,szSetting);
		db_msg_dbg(buf);
	}
#endif
	if (rc == 0) {
		res = _strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
		return res;
	}
	else {
		DBFreeVariant(&dbv);
		return errVal!=NULL?_strdup(errVal):NULL;
	}
}

__inline static TCHAR *Hlp_DBGetContactSettingTString(HANDLE hContact,const char *szModule,
	const char *szSetting,TCHAR *errVal)
{
	int rc;
	DBCONTACTGETSETTING cgs;
	DBVARIANT dbv = {0};
	TCHAR *res;
	cgs.szModule=szModule;
	cgs.szSetting=szSetting;
	cgs.pValue=&dbv;
#if defined(_UNICODE)
	dbv.type = DBVT_WCHAR;
#else
	dbv.type = DBVT_ASCIIZ;
#endif

	rc=CallService(MS_DB_CONTACT_GETSETTING_STR,(WPARAM)hContact,(LPARAM)&cgs);
#if defined(_DEBUG) && defined(DBCHECKSETTINGS)
	if (rc != 0) {
		char buf[128];
		_snprintf(buf,sizeof(buf),"%s:%d failed to fetch %s/%s",szFile,nLine,szModule,szSetting);
		db_msg_dbg(buf);
	}
#endif
	if (rc == 0) {
		res = _tcsdup(dbv.ptszVal);
		DBFreeVariant(&dbv);
		return res;
	}
	else {
		DBFreeVariant(&dbv);
		return errVal!=NULL?_tcsdup(errVal):NULL;
	}
}

__inline static WCHAR *Hlp_DBGetContactSettingWString(HANDLE hContact,const char *szModule,
	const char *szSetting,WCHAR *errVal)
{
	int rc;
	DBCONTACTGETSETTING cgs;
	DBVARIANT dbv = {0};
	WCHAR *res;
	cgs.szModule=szModule;
	cgs.szSetting=szSetting;
	cgs.pValue=&dbv;
	dbv.type = DBVT_WCHAR;

	rc=CallService(MS_DB_CONTACT_GETSETTING_STR,(WPARAM)hContact,(LPARAM)&cgs);
#if defined(_DEBUG) && defined(DBCHECKSETTINGS)
	if (rc != 0) {
		char buf[128];
		_snprintf(buf,sizeof(buf),"%s:%d failed to fetch %s/%s",szFile,nLine,szModule,szSetting);
		db_msg_dbg(buf);
	}
#endif
	if (rc == 0) {
		res = _wcsdup(dbv.pwszVal);
		DBFreeVariant(&dbv);
		return res;
	}
	else {
		DBFreeVariant(&dbv);
		return errVal!=NULL?_wcsdup(errVal):NULL;
	}
}

__inline static char *Hlp_DBGetContactSettingStringUtf(HANDLE hContact,const char *szModule,
	const char *szSetting,char *errVal)
{
	int rc;
	DBCONTACTGETSETTING cgs;
	DBVARIANT dbv = {0};
	char *res;
	cgs.szModule=szModule;
	cgs.szSetting=szSetting;
	cgs.pValue=&dbv;
	dbv.type = DBVT_UTF8;

	rc=CallService(MS_DB_CONTACT_GETSETTING_STR,(WPARAM)hContact,(LPARAM)&cgs);
#if defined(_DEBUG) && defined(DBCHECKSETTINGS)
	if (rc != 0) {
		char buf[128];
		_snprintf(buf,sizeof(buf),"%s:%d failed to fetch %s/%s",szFile,nLine,szModule,szSetting);
		db_msg_dbg(buf);
	}
#endif
	if (rc == 0) {
		res = _strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
		return res;
	}
	else {
		DBFreeVariant(&dbv);
		return errVal!=NULL?_strdup(errVal):NULL;
	}
}

#ifndef PREFIX_ITH
#define PREFIX_ITH		""
#endif

__inline static int Hlp_DBWriteIthSettingByte(unsigned int i, HANDLE hContact,const char *szModule,const char *szSetting,BYTE val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return DBWriteContactSettingByte(hContact, szModule, dbSetting, val);
}

__inline static int Hlp_DBWriteIthSettingWord(unsigned int i, HANDLE hContact,const char *szModule,const char *szSetting,WORD val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return DBWriteContactSettingWord(hContact, szModule, dbSetting, val);
}

__inline static int Hlp_DBWriteIthSettingDword(unsigned int i, HANDLE hContact,const char *szModule,const char *szSetting,DWORD val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return DBWriteContactSettingDword(hContact, szModule, dbSetting, val);
}

__inline static int Hlp_DBWriteIthSettingString(unsigned int i, HANDLE hContact,const char *szModule,const char *szSetting,const char *val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return DBWriteContactSettingString(hContact, szModule, dbSetting, val);
}

__inline static int Hlp_DBWriteIthSettingTString(unsigned int i, HANDLE hContact,const char *szModule,const char *szSetting,const TCHAR *val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return DBWriteContactSettingTString(hContact, szModule, dbSetting, val);
}

__inline static int Hlp_DBWriteIthSettingWString(unsigned int i, HANDLE hContact,const char *szModule,const char *szSetting,const WCHAR *val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return DBWriteContactSettingWString(hContact, szModule, dbSetting, val);
}

__inline static int Hlp_DBWriteIthSettingStringUtf(unsigned int i, HANDLE hContact,const char *szModule,const char *szSetting,const char *val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return DBWriteContactSettingStringUtf(hContact, szModule, dbSetting, val);
}

__inline static int Hlp_DBGetIthSettingByte(unsigned int i, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return DBGetContactSettingByte(hContact, szModule, dbSetting, errorValue);
}

__inline static WORD Hlp_DBGetIthSettingWord(unsigned int i, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return DBGetContactSettingWord(hContact, szModule, dbSetting, errorValue);
}

__inline static DWORD Hlp_DBGetIthSettingDword(unsigned int i, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return DBGetContactSettingDword(hContact, szModule, dbSetting, errorValue);
}

__inline static char *Hlp_DBGetIthSettingString(unsigned int i, HANDLE hContact, const char *szModule, const char *szSetting, char *errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return Hlp_DBGetContactSettingString(hContact, szModule, dbSetting, errorValue);
}

__inline static TCHAR *Hlp_DBGetIthSettingTString(unsigned int i, HANDLE hContact, const char *szModule, const char *szSetting, TCHAR *errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return Hlp_DBGetContactSettingTString(hContact, szModule, dbSetting, errorValue);
}

__inline static WCHAR *Hlp_DBGetIthSettingWString(unsigned int i, HANDLE hContact, const char *szModule, const char *szSetting, WCHAR *errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return Hlp_DBGetContactSettingWString(hContact, szModule, dbSetting, errorValue);
}

__inline static char *Hlp_DBGetIthSettingStringUtf(unsigned int i, HANDLE hContact, const char *szModule, const char *szSetting, char *errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return Hlp_DBGetContactSettingStringUtf(hContact, szModule, dbSetting, errorValue);
}

__inline static int Hlp_DBGetIthSetting(unsigned int i, HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return DBGetContactSetting(hContact, szModule, dbSetting, dbv);
}

__inline static int Hlp_DBDeleteIthSetting(unsigned int i, HANDLE hContact,const char *szModule,const char *szSetting) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return DBDeleteContactSetting(hContact, szModule, dbSetting);
}

#ifndef PREFIX_XTH
#define PREFIX_XTH		""
#endif

__inline static int Hlp_DBWriteXthSettingByte(char *x, HANDLE hContact,const char *szModule,const char *szSetting,BYTE val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%s_%s", PREFIX_XTH, x, szSetting);
	return DBWriteContactSettingByte(hContact, szModule, dbSetting, val);
}

__inline static int Hlp_DBWriteXthSettingWord(char *x, HANDLE hContact,const char *szModule,const char *szSetting,WORD val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%s_%s", PREFIX_XTH, x, szSetting);
	return DBWriteContactSettingWord(hContact, szModule, dbSetting, val);
}

__inline static int Hlp_DBWriteXthSettingDword(char *x, HANDLE hContact,const char *szModule,const char *szSetting,DWORD val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%s_%s", PREFIX_XTH, x, szSetting);
	return DBWriteContactSettingDword(hContact, szModule, dbSetting, val);
}

__inline static int Hlp_DBWriteXthSettingString(char *x, HANDLE hContact,const char *szModule,const char *szSetting,const char *val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%s_%s", PREFIX_XTH, x, szSetting);
	return DBWriteContactSettingString(hContact, szModule, dbSetting, val);
}

__inline static int Hlp_DBWriteXthSettingTString(char *x, HANDLE hContact,const char *szModule,const char *szSetting,const TCHAR *val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%s_%s", PREFIX_XTH, x, szSetting);
	return DBWriteContactSettingTString(hContact, szModule, dbSetting, val);
}

__inline static int Hlp_DBWriteXthSettingWString(char *x, HANDLE hContact,const char *szModule,const char *szSetting,const WCHAR *val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%s_%s", PREFIX_XTH, x, szSetting);
	return DBWriteContactSettingWString(hContact, szModule, dbSetting, val);
}

__inline static int Hlp_DBWriteXthSettingStringUtf(char *x, HANDLE hContact,const char *szModule,const char *szSetting,const char *val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%s_%s", PREFIX_XTH, x, szSetting);
	return DBWriteContactSettingStringUtf(hContact, szModule, dbSetting, val);
}

__inline static int Hlp_DBGetXthSettingByte(char *x, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%s_%s", PREFIX_XTH, x, szSetting);
	return DBGetContactSettingByte(hContact, szModule, dbSetting, errorValue);
}

__inline static WORD Hlp_DBGetXthSettingWord(char *x, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%s_%s", PREFIX_XTH, x, szSetting);
	return DBGetContactSettingWord(hContact, szModule, dbSetting, errorValue);
}

__inline static DWORD Hlp_DBGetXthSettingDword(char *x, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%s_%s", PREFIX_XTH, x, szSetting);
	return DBGetContactSettingDword(hContact, szModule, dbSetting, errorValue);
}

__inline static char *Hlp_DBGetXthSettingString(char *x, HANDLE hContact, const char *szModule, const char *szSetting, char *errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%s_%s", PREFIX_XTH, x, szSetting);
	return Hlp_DBGetContactSettingString(hContact, szModule, dbSetting, errorValue);
}

__inline static TCHAR *Hlp_DBGetXthSettingTString(char *x, HANDLE hContact, const char *szModule, const char *szSetting, TCHAR *errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%s_%s", PREFIX_XTH, x, szSetting);
	return Hlp_DBGetContactSettingTString(hContact, szModule, dbSetting, errorValue);
}

__inline static WCHAR *Hlp_DBGetXthSettingWString(char *x, HANDLE hContact, const char *szModule, const char *szSetting, WCHAR *errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%s_%s", PREFIX_XTH, x, szSetting);
	return Hlp_DBGetContactSettingWString(hContact, szModule, dbSetting, errorValue);
}

__inline static char *Hlp_DBGetXthSettingStringUtf(char *x, HANDLE hContact, const char *szModule, const char *szSetting, char *errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%s_%s", PREFIX_XTH, x, szSetting);
	return Hlp_DBGetContactSettingStringUtf(hContact, szModule, dbSetting, errorValue);
}

__inline static int Hlp_DBGetXthSetting(char *x, HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%s_%s", PREFIX_XTH, x, szSetting);
	return DBGetContactSetting(hContact, szModule, dbSetting, dbv);
}

__inline static int Hlp_DBDeleteXthSetting(char *x, HANDLE hContact,const char *szModule,const char *szSetting) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%s_%s", PREFIX_XTH, x, szSetting);
	return DBDeleteContactSetting(hContact, szModule, dbSetting);
}

#endif
