/*
    Variables Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

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
#include "variables.h"
#include "parse_system.h"
#include <tchar.h>
#include <lmcons.h>
#include <sys/stat.h>

#ifdef WINE
  #ifdef _WIN32_WINNT
    #undef _WIN32_WINNT
  #endif
  #define _WIN32_WINNT 0x400
#endif

#ifndef WINE
#include "enumprocs.h"
#endif

static TCHAR *parseComputerName(ARGUMENTSINFO *ai) {

	DWORD len;
	TCHAR *res;

	if (ai->argc != 1)
		return NULL;

	ai->flags |= AIF_DONTPARSE;
	len = MAX_COMPUTERNAME_LENGTH;
	res = ( TCHAR* )calloc((len + 1), sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	if (!GetComputerName(res, &len)) {
		free(res);
		return NULL;
	}
	return res;
}

#if _WIN32_WINNT>=0x0500
#include <pdh.h>
#include <pdhmsg.h>

static TCHAR *parseCpuLoad(ARGUMENTSINFO *ai) {

	HQUERY hQuery;
	PDH_STATUS pdhStatus;
	PDH_FMT_COUNTERVALUE cValue;
	HCOUNTER hCounter;
	TCHAR *szCounter, szVal[32];

	if (ai->argc != 2)
		return NULL;

	if (_tcslen(ai->targv[1]) == 0)
		szCounter = _tcsdup(_T("\\Processor(_Total)\\% Processor Time"));
	else {
		szCounter = ( TCHAR* )malloc((_tcslen(ai->targv[1]) + 32)*sizeof(TCHAR));
		if (szCounter == NULL)
			return NULL;

		wsprintf(szCounter, _T("\\Process(%s)\\%% Processor Time"), ai->targv[1]);
	}
	pdhStatus = PdhValidatePath(szCounter);
	if (pdhStatus != ERROR_SUCCESS) {
		free(szCounter);
		return NULL;
	}
	pdhStatus = PdhOpenQuery(NULL, 0, &hQuery);
	if (pdhStatus != ERROR_SUCCESS) {
		free(szCounter);
		return NULL;
	}
	pdhStatus = PdhAddCounter(hQuery, szCounter, 0, &hCounter);
	if (pdhStatus != ERROR_SUCCESS) {
		free(szCounter);
		pdhStatus = PdhCloseQuery(hQuery);
		return NULL;
	}
	pdhStatus = PdhCollectQueryData(hQuery);
	if (pdhStatus != ERROR_SUCCESS) {
		free(szCounter);
		PdhRemoveCounter(hCounter);
		pdhStatus = PdhCloseQuery(hQuery);
		return NULL;
	}
	Sleep(100);
	pdhStatus = PdhCollectQueryData(hQuery);
	if (pdhStatus != ERROR_SUCCESS) {
		free(szCounter);
		PdhRemoveCounter(hCounter);
		pdhStatus = PdhCloseQuery(hQuery);
		return NULL;
	}
	pdhStatus = PdhGetFormattedCounterValue(hCounter, PDH_FMT_DOUBLE, (LPDWORD)NULL, &cValue);
	if (pdhStatus != ERROR_SUCCESS) {
		free(szCounter);
		PdhRemoveCounter(hCounter);
		pdhStatus = PdhCloseQuery(hQuery);
		return NULL;
	}
	if (cValue.CStatus != ERROR_SUCCESS) {
		free(szCounter);
		PdhRemoveCounter(hCounter);
		pdhStatus = PdhCloseQuery(hQuery);
		return NULL;
	}
	mir_sntprintf(szVal, SIZEOF(szVal), _T("%.0f"), cValue.doubleValue);
	//PdhRemoveCounter(*hCounter);
	PdhCloseQuery(hQuery);
	free(szCounter);
		
	return _tcsdup(szVal);
}
#endif

static TCHAR *parseCurrentDate(ARGUMENTSINFO *ai) {

	int len;
	TCHAR *szFormat, *res;

	if ( ai->argc == 1 || (ai->argc > 1 && _tcslen(ai->targv[1]) == 0 ))
		szFormat = NULL;
	else
		szFormat = ai->targv[1];

	len = GetDateFormat(LOCALE_USER_DEFAULT, 0, NULL, szFormat, NULL, 0);
	res = ( TCHAR* )malloc((len+1)*sizeof(TCHAR));
	if (res == NULL) {
		return NULL;
	}
	if (GetDateFormat(LOCALE_USER_DEFAULT, 0, NULL, szFormat, res, len) == 0) {
		free(res);
		return NULL;
	}

	return res;
}

static TCHAR *parseCurrentTime(ARGUMENTSINFO *ai) {

	int len;
	TCHAR *szFormat, *res;

	if ( ai->argc == 1 || (ai->argc > 1) && (_tcslen(ai->targv[1]) == 0))
		szFormat = NULL;
	else
		szFormat = ai->targv[1];

	len = GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, szFormat, NULL, 0);
	res = ( TCHAR* )malloc((len+1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	if (GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, szFormat, res, len) == 0) {
		free(res);
		return NULL;
	}

	return res;
}

static TCHAR *parseDirectory(ARGUMENTSINFO *ai) {

	int depth;
	TCHAR *res, *ecur, *scur;

	if ( ai->argc < 2 || ai->argc > 3 )
		return NULL;

	depth = 1;
	if (ai->argc == 3)
		depth = ttoi(ai->targv[2]);

	if (depth <= 0)
		return NULL;

	ecur = ai->targv[1]+_tcslen(ai->targv[1]);
	while (depth > 0) {
		while ( (*ecur != _T('\\')) && (ecur > ai->targv[1]) )
			ecur--;

		if (*ecur != _T('\\'))
			return NULL;

		depth -= 1;
		ecur--;
	}
	scur = ecur;
	while ( (*scur != _T('\\')) && (scur > ai->targv[1]) )
		scur--;

	if (*scur == _T('\\'))
		scur++;

	res = ( TCHAR* )calloc((ecur-scur)+2, sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	_tcsncpy(res, scur, (ecur-scur)+1);
	return res;
}

/*
	path, depth
	returns complete path up to "path depth - depth"
*/
static TCHAR *parseDirectory2(ARGUMENTSINFO *ai) {

	int depth;
	TCHAR *res, *ecur;

	if ( ai->argc < 2 || ai->argc > 3 )
		return NULL;

	depth = 1;
	if (ai->argc == 3)
		depth = ttoi(ai->targv[2]);

	if (depth <= 0)
		return NULL;

	ecur = ai->targv[1]+_tcslen(ai->targv[1]);
	while (depth > 0) {
		while ( (*ecur != _T('\\')) && (ecur > ai->targv[1]) )
			ecur--;

		if (*ecur != _T('\\'))
			return NULL;

		depth -= 1;
		ecur--;
	}
	res = ( TCHAR* )calloc((ecur-ai->targv[1])+2, sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	_tcsncpy(res, ai->targv[1], (ecur-ai->targv[1])+1);
	return res;
}

static int getTime(TCHAR *szTime, struct tm *date) {

	TCHAR *cur;
	
	// do some extra checks here
	cur = szTime;
	if (cur >= szTime+_tcslen(szTime))
		return -1;

	date->tm_mon = _tcstoul(cur, &cur, 10)-1;
	cur++;
	if (cur >= szTime+_tcslen(szTime)) {
		return -1;
	}
	date->tm_mday = _tcstoul(cur, &cur, 10);
	cur++;
	if (cur >= szTime+_tcslen(szTime)) {
		return -1;
	}
	date->tm_year = _tcstoul(cur, &cur, 10);
	if (date->tm_year > 2000) {
		date->tm_year -= 2000;
	}
	else if (date->tm_year > 1900) {
		date->tm_year -= 1900;
	}
	date->tm_year = date->tm_year<38?date->tm_year+100:date->tm_year;
	cur++;
	if (cur >= szTime+_tcslen(szTime)) {
		return -1;
	}
	date->tm_hour = _tcstoul(cur, &cur, 10);
	cur++;
	if (cur >= szTime+_tcslen(szTime)) {
		return -1;
	}
	date->tm_min = _tcstoul(cur, &cur, 10);
	cur++;
	if (cur >= szTime+_tcslen(szTime)) {
		return -1;
	}
	date->tm_sec = _tcstoul(cur, &cur, 10);

	return 0;
}
/*
	date,date
	M/d/yy H:m:s
*/
static TCHAR *parseDiffTime(ARGUMENTSINFO *ai) {
	
	struct tm t0, t1;
	TCHAR szTime[32];
	double diff;

	if (ai->argc != 3)
		return NULL;

	ZeroMemory(&t0, sizeof(t0));
	ZeroMemory(&t1, sizeof(t1));
	if (getTime(ai->targv[1], &t0) != 0)
		return NULL;

	if (getTime(ai->targv[2], &t1) != 0)
		return NULL;

	diff = difftime(mktime(&t1), mktime(&t0));
	mir_sntprintf(szTime, SIZEOF(szTime), _T("%.0f"), diff);

	return _tcsdup(szTime);
}

static TCHAR *parseDirExists(ARGUMENTSINFO *ai) {

	HANDLE hFile;

	if (ai->argc != 2) {
		return NULL;
	}
	hFile = CreateFile(ai->targv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		ai->flags |= AIF_FALSE;
	}
	else {
		CloseHandle(hFile);
	}

	return _tcsdup(_T(""));
}

static TCHAR *parseEnvironmentVariable(ARGUMENTSINFO *ai) {

	DWORD len;
	TCHAR *res;

	if (ai->argc != 2) {
		return NULL;
	}
	len = ExpandEnvironmentStrings(ai->targv[1], NULL, 0);
	if (len <= 0) {
		return NULL;
	}
	res = ( TCHAR* )malloc((len+1)*sizeof(TCHAR));
	if (res == NULL) {
		return NULL;
	}
	ZeroMemory(res, (len+1)*sizeof(TCHAR));
	if (ExpandEnvironmentStrings(ai->targv[1], res, len) == 0) {
		free(res);
		return NULL;
	}
	return res;
}

static TCHAR *parseFileExists(ARGUMENTSINFO *ai) {

	HANDLE hFile;

	if (ai->argc != 2)
		return NULL;

	hFile = CreateFile(ai->targv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		ai->flags |= AIF_FALSE;
	else 
		CloseHandle(hFile);

	return _tcsdup(_T(""));
}

static TCHAR *parseFindWindow(ARGUMENTSINFO *ai) {

	HWND hWin;
	TCHAR *res;
	int len;

	if (ai->argc != 2)
		return NULL;

	hWin = FindWindow(ai->targv[1], NULL);
	if (hWin == NULL)
		return NULL;

	len = SendMessage(hWin, WM_GETTEXTLENGTH, 0, 0);
	if (len >= 0) {
		res = ( TCHAR* )malloc((len+1)*sizeof(TCHAR));
		ZeroMemory(res, (len+1)*sizeof(TCHAR));
		GetWindowText(hWin, res, len+1);
		return res;
	}
	return NULL;
}

// 1 = dir
// 2 = filter
// 3 = sperator
// 4 = [f]iles, [d]irs
static TCHAR *parseListDir(ARGUMENTSINFO *ai) {

	HANDLE hFind;
	BOOL bFiles, bDirs;
	WIN32_FIND_DATA ffd;
	TCHAR tszFirst[MAX_PATH], *tszRes, *tszSeperator, *tszFilter;

	if (ai->argc < 2)
		return NULL;

	tszFirst[0] = 0;
	tszSeperator = _T("\r\n");
	tszFilter = _T("*");
	tszRes = NULL;
	bFiles = bDirs = TRUE;
	if (ai->argc > 1) {
		_tcsncpy(tszFirst, ai->targv[1], SIZEOF(tszFirst)-1);
	}
	if (ai->argc > 2) {
		tszFilter = ai->targv[2];
	}
	if (ai->argc > 3) {
		tszSeperator = ai->targv[3];
	}
	if (ai->argc > 4) {
		if (*ai->targv[4] == _T('f')) {
			bDirs = FALSE;
		}
		if (*ai->targv[4] == _T('d')) {
			bFiles = FALSE;
		}
	}
	if (tszFirst[_tcslen(tszFirst)-1] == _T('\\')) {
		_tcsncat(tszFirst, tszFilter, SIZEOF(tszFirst) - _tcslen(tszFirst) - 1);
	}
	else {
		_tcsncat(tszFirst, _T("\\"), SIZEOF(tszFirst) - _tcslen(tszFirst) - 1);
		_tcsncat(tszFirst, tszFilter, SIZEOF(tszFirst) - _tcslen(tszFirst) - 1);
	}
	hFind = FindFirstFile(tszFirst, &ffd);
	if (hFind == INVALID_HANDLE_VALUE) {
		return NULL;
	}
	if ( ((ffd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) && (bDirs)) || ((!(ffd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)) && (bFiles)) ) {
		tszRes = ( TCHAR* )malloc((_tcslen(ffd.cFileName) + _tcslen(tszSeperator) + 1)*sizeof(TCHAR));
		_tcscpy(tszRes, ffd.cFileName);
	}
	while (FindNextFile(hFind, &ffd) != 0) {
		if ( ((ffd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) && (bDirs)) || ((!(ffd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)) && (bFiles)) ) {
			if (tszRes != NULL) {
				_tcscat(tszRes, tszSeperator);
				tszRes = ( TCHAR* )realloc(tszRes, (_tcslen(tszRes) + _tcslen(ffd.cFileName) + _tcslen(tszSeperator) + 1)*sizeof(TCHAR));
			}
			else {
				tszRes = ( TCHAR* )malloc((_tcslen(ffd.cFileName) + _tcslen(tszSeperator) + 1)*sizeof(TCHAR));
				_tcscpy(tszRes, _T(""));
			}
			_tcscat(tszRes, ffd.cFileName);
		}
	}
	FindClose(hFind);

	return tszRes;
}

#ifndef WINE
static BOOL CALLBACK MyProcessEnumerator(DWORD dwPID, WORD wTask, char *szProcess, LPARAM lParam) {

	char **szProc;

	szProc = (char **)lParam;
	if ( (*szProc != NULL) && (!_stricmp(*szProc, szProcess)) ) {
		*szProc = NULL;
	}

	return TRUE;
}

static TCHAR *parseProcessRunning(ARGUMENTSINFO *ai) {

	char *szProc, *ref;

	if (ai->argc != 2) {
		return NULL;
	}
#ifdef UNICODE
	szProc = ref = u2a(ai->targv[1]);
#else
	szProc = ref = _strdup(ai->argv[1]);
#endif
	EnumProcs((PROCENUMPROC) MyProcessEnumerator, (LPARAM)&szProc);
	if (szProc != NULL) {
		ai->flags |= AIF_FALSE;
	}
	free(ref);
	return _tcsdup(_T(""));
}
#endif

static TCHAR *parseRegistryValue(ARGUMENTSINFO *ai) {

	HKEY hKey;
	TCHAR *key, *subKey, *cur, *res;
	DWORD len, type;
	int err;

	if (ai->argc != 3) {
		return NULL;
	}
	key = subKey = _tcsdup(ai->targv[1]);
	if (subKey == NULL) {
		return NULL;
	}
	cur = _tcschr(subKey, _T('\\'));
	if (cur == NULL) {
		return NULL;
	}
	*cur = _T('\0');
	if (!_tcscmp(subKey, _T("HKEY_CLASSES_ROOT"))) {
		hKey = HKEY_CLASSES_ROOT;
	}
	else if (!_tcscmp(subKey, _T("HKEY_CURRENT_USER"))) {
		hKey = HKEY_CURRENT_USER;
	}
	else if (!_tcscmp(subKey, _T("HKEY_LOCAL_MACHINE"))) {
		hKey = HKEY_LOCAL_MACHINE;
	}
	else if (!_tcscmp(subKey, _T("HKEY_USERS"))) {
		hKey = HKEY_USERS;
	}
	else {
		free(key);
		return NULL;
	}
	subKey = cur+1;
	if (RegOpenKeyEx(hKey, subKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
		free(key);
		return NULL;
	}
	free(key);
	len = MAX_REGVALUE_LENGTH+1;
	res = ( TCHAR* )malloc(len*sizeof(TCHAR));
	if (res == NULL) {
		return NULL;
	}
	ZeroMemory(res, len);
	err = RegQueryValueEx(hKey, ai->targv[2], NULL, &type, (BYTE*)res, &len);
	if ( (err != ERROR_SUCCESS) || (type != REG_SZ) ) {
		RegCloseKey(hKey);
		free(res);
		return NULL;
	}
	RegCloseKey(hKey);

	return res;
}

static int TsToSystemTime(SYSTEMTIME *sysTime, time_t timestamp) {

	struct tm *pTime;

	pTime = localtime(&timestamp);
	if (pTime == NULL) {
		return -1;
	}
	ZeroMemory(sysTime, sizeof(SYSTEMTIME));
	sysTime->wDay = pTime->tm_mday;
	sysTime->wDayOfWeek = pTime->tm_wday;
	sysTime->wHour = pTime->tm_hour;
	sysTime->wMinute = pTime->tm_min;
	sysTime->wMonth = pTime->tm_mon + 1;
	sysTime->wSecond = pTime->tm_sec;
	sysTime->wYear = pTime->tm_year + 1900;

	return 0;
}

static TCHAR *parseTimestamp2Date(ARGUMENTSINFO *ai) {

	int len;
	time_t timestamp;
	SYSTEMTIME sysTime;
	TCHAR *szFormat, *res;

	if (ai->argc <= 1) {
		return NULL;
	}
	timestamp = ttoi(ai->targv[1]);
	if (timestamp == 0) {
		return NULL;
	}
	if ( (ai->argc == 2) || ((ai->argc > 2) && (_tcslen(ai->targv[2]) == 0)) ) {
		szFormat = NULL;
	}
	else {
		szFormat = ai->targv[2];
	}
	if (TsToSystemTime(&sysTime, timestamp) != 0) {
		return NULL;
	}	
	len = GetDateFormat(LOCALE_USER_DEFAULT, 0, &sysTime, szFormat, NULL, 0);
	res = ( TCHAR* )calloc((len+1), sizeof(TCHAR));
	if (res == NULL) {
		return NULL;
	}
	if (GetDateFormat(LOCALE_USER_DEFAULT, 0, &sysTime, szFormat, res, len) == 0) {
		free(res);
		return NULL;
	}

	return res;
}

static TCHAR *parseTimestamp2Time(ARGUMENTSINFO *ai) {

	int len;
	time_t timestamp;
	SYSTEMTIME sysTime;
	TCHAR *szFormat, *res;

	if (ai->argc <= 1) {
		return NULL;
	}
	timestamp = ttoi(ai->targv[1]);
	if (timestamp == 0) {
		return NULL;
	}
	if ( (ai->argc == 2) || ((ai->argc > 2) && (_tcslen(ai->targv[2]) == 0)) ) {
		szFormat = NULL;
	}
	else {
		szFormat = ai->targv[2];
	}
	if (TsToSystemTime(&sysTime, timestamp) != 0) {
		return NULL;
	}	
	len = GetTimeFormat(LOCALE_USER_DEFAULT, 0, &sysTime, szFormat, NULL, 0);
	res = ( TCHAR* )malloc((len+1)*sizeof(TCHAR));
	if (res == NULL) {
		return NULL;
	}
	if (GetTimeFormat(LOCALE_USER_DEFAULT, 0, &sysTime, szFormat, res, len) == 0) {
		free(res);
		return NULL;
	}

	return res;
}

static TCHAR *parseTextFile(ARGUMENTSINFO *ai) {

	int lineNo, lineCount, csz;
	unsigned int icur, bufSz;
	HANDLE hFile;
	DWORD fileSz, readSz, totalReadSz;
	unsigned long linePos;
	TCHAR tUC, *res;
	BYTE *pBuf, *pCur;

	if (ai->argc != 3) {
		return NULL;
	}
	lineNo = ttoi(ai->targv[2]);
	hFile = CreateFile(ai->targv[1], GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return NULL;
	}
	fileSz = GetFileSize(hFile, NULL);
	if (fileSz == INVALID_FILE_SIZE) {
		CloseHandle(hFile);
		return NULL;
	}
	ReadFile(hFile, &tUC, sizeof(TCHAR), &readSz, NULL);
	if (tUC != (TCHAR)0xFEFF) {
		tUC = 0;
		csz = sizeof(char);
		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	}
	else {
#ifndef UNICODE
		return NULL;
#else
		csz = sizeof(TCHAR);
#endif
	}
	totalReadSz = 0;
	lineCount = 1;
	if (*ai->targv[2] == _T('0')) {
		// complete file
		bufSz = fileSz + csz;
		pBuf = ( PBYTE )calloc(1, bufSz);
		if (pBuf == NULL)
			return NULL;

		if (ReadFile(hFile, pBuf, bufSz-csz, &readSz, NULL) == 0) {
			CloseHandle(hFile);
			free(pBuf);
			return NULL;
		}
		CloseHandle(hFile);
#ifdef UNICODE
		if (tUC) {
			res = (TCHAR *)pBuf;
		}
		else {
			res = a2u((char *)pBuf);
			free(pBuf);
		}
#else
		res = (char*)pBuf;
#endif

		return res;
	}
	bufSz = TXTFILEBUFSZ*csz;
	pBuf = ( PBYTE )calloc(bufSz, 1);
	if (pBuf == NULL) {
		return NULL;
	}
	// count number of lines
	do {
		ZeroMemory(pBuf, bufSz);
		if (ReadFile(hFile, pBuf, bufSz-csz, &readSz, NULL) == 0) {
			CloseHandle(hFile);
			free(pBuf);
			return NULL;
		}
		totalReadSz += readSz;
		for (pCur = pBuf;*pCur != '\0';pCur += csz) {
			if (tUC) {
				if (!_tcsncmp((TCHAR *)pCur, _T("\r\n"), 2)) {
					lineCount += 1;
					pCur += csz;
				}
				else if (*(TCHAR *)pCur == _T('\n')) {
					lineCount += 1;
				}
			}
			else {
				if (!strncmp((char *)pCur, "\r\n", 2)) {
					lineCount += 1;
					pCur += csz;
				}
				else if (*(char *)pCur == '\n') {
					lineCount += 1;
				}
			}
		}
	} while ( (totalReadSz < fileSz) && (readSz > 0) );
	if (lineNo < 0) {
		lineNo = lineCount + lineNo + 1;
	}
	else if (*ai->targv[2] == _T('r')) {
		lineNo = (rand()%lineCount)+1;
	}
	totalReadSz = 0;
	lineCount = 1;
	linePos = 0xFFFFFFFF;
	SetFilePointer(hFile, tUC?csz:0, NULL, FILE_BEGIN);
	// find the position in the file where the requested line starts
	do {
		if (ReadFile(hFile, pBuf, bufSz-csz, &readSz, NULL) == 0) {
			CloseHandle(hFile);
			return NULL;
		}
		totalReadSz += readSz;
		for (pCur = pBuf;((pCur < pBuf+bufSz) && (linePos == 0xFFFFFFFF));pCur+=csz) {
			if (lineCount == lineNo) {
				linePos = (tUC?csz:0) + totalReadSz - readSz + pCur - pBuf;
			}
			if (tUC) {
				if (!_tcsncmp((TCHAR *)pCur, _T("\r\n"), 2)) {
					lineCount += 1;
					pCur += csz;
				}
				else if (*(TCHAR *)pCur == _T('\n')) {
					lineCount += 1;
				}
			}
			else {
				if (!strncmp((char *)pCur, "\r\n", 2)) {
					lineCount += 1;
					pCur += csz;
				}
				else if (*(char *)pCur == '\n') {
					lineCount += 1;
				}
			}
		}
		if ( ((tUC) && (*(TCHAR *)pCur == _T('\r'))) || ((!tUC) && (*(char *)pCur == '\r')) ) {
			// in case the \r was at the end of the buffer, \n could be next
			SetFilePointer(hFile, -1*csz, NULL, FILE_CURRENT);
			totalReadSz -= csz;
		}
	} while ( (totalReadSz < fileSz) && (readSz > 0) );
	if (linePos < 0) {
		CloseHandle(hFile);
		free(pBuf);
		return NULL;
	}
	if (SetFilePointer(hFile, linePos, NULL, FILE_BEGIN) != linePos) {
		CloseHandle(hFile);
		free(pBuf);
		return NULL;
	}
	ZeroMemory(pBuf, bufSz);
	pCur = pBuf;
	do {
		icur = 0;
		if (ReadFile(hFile, pBuf, bufSz-csz, &readSz, NULL) == 0) {
			free(pBuf);
			CloseHandle(hFile);
			return NULL;
		}
		for (pCur = pBuf;(pCur < pBuf+readSz);pCur+=csz) {
			if ((tUC) && ((!_tcsncmp((TCHAR *)pCur, _T("\r\n"), 2)) || (*(TCHAR *)pCur == _T('\n'))) || 
				((!tUC) && (((!strncmp((char *)pCur, "\r\n", 2)) || (*(char *)pCur == '\n')))) ) {
				CloseHandle(hFile);
				if (tUC) {
					*(TCHAR *)pCur = _T('\0');
				}
				else {
					*(char *)pCur = '\0';
				}
#ifdef UNICODE
				if (tUC) {
					res = (TCHAR *)pBuf;
				}
				else {
					res = a2u((char *)pBuf);
					free(pBuf);
				}
#else
				res = (char *)pBuf;
#endif
				return res;
			}
		}
		if ( ((DWORD)(linePos+(pCur-pBuf)) == fileSz) ) { // eof
			CloseHandle(hFile);
#ifdef UNICODE
			if (tUC) {
				res = (TCHAR *)pBuf;
			}
			else {
				res = a2u((char *)pBuf);
				free(pBuf);
			}
#else
			res = (char *)pBuf;
#endif
			return res;
		}
		if (readSz == bufSz-csz) {
			// buffer must be increased
			bufSz += TXTFILEBUFSZ*csz;
			if ( ((tUC) && (*(TCHAR *)pCur == _T('\r'))) || ((!tUC) && (*(char *)pCur == '\r')) ) {
				pCur -= csz;
			}
			icur = pCur - pBuf;
			pBuf = ( PBYTE )realloc(pBuf, bufSz);
			pCur = pBuf+icur;
			ZeroMemory(pCur+1, TXTFILEBUFSZ*csz);
		}
	} while (readSz > 0);
	CloseHandle(hFile);

	return NULL;
}

#if _WIN32_WINNT>=0x0500
static TCHAR *parseUpTime(ARGUMENTSINFO *ai) {

	HQUERY hQuery;
	PDH_STATUS pdhStatus;
	PDH_FMT_COUNTERVALUE cValue;
	HCOUNTER hCounter;
	TCHAR szVal[32];
	
	if (ai->argc != 1) {
		return NULL;
	}

	pdhStatus = PdhOpenQuery (NULL, 0, &hQuery);
	if (pdhStatus != ERROR_SUCCESS) {
		return NULL;
	}
	pdhStatus = PdhAddCounter(hQuery, _T("\\System\\System Up Time"), 0, &hCounter);
	if (pdhStatus != ERROR_SUCCESS) {
		pdhStatus = PdhCloseQuery(hQuery);
		return NULL;
	}
	pdhStatus = PdhCollectQueryData(hQuery);
	if (pdhStatus != ERROR_SUCCESS) {
		PdhRemoveCounter(hCounter);
		pdhStatus = PdhCloseQuery(hQuery);
		return NULL;
	}
	pdhStatus = PdhGetFormattedCounterValue(hCounter, PDH_FMT_LARGE, (LPDWORD)NULL, &cValue);
	if (pdhStatus != ERROR_SUCCESS) {
		PdhRemoveCounter(hCounter);
		pdhStatus = PdhCloseQuery(hQuery);
		return NULL;
	}
	mir_sntprintf(szVal, SIZEOF(szVal), _T("%u"), cValue.largeValue);
	PdhRemoveCounter(hCounter);
	pdhStatus = PdhCloseQuery(hQuery);
		
	return _tcsdup(szVal);
}
#endif

static TCHAR *parseUserName(ARGUMENTSINFO *ai) {

	DWORD len;
	TCHAR *res;

	if (ai->argc != 1) {
		return NULL;
	}
	ai->flags |= AIF_DONTPARSE;
	len = UNLEN;
	res = ( TCHAR* )malloc(len + 1);
	if (res == NULL) {
		return NULL;
	}
	ZeroMemory(res, len + 1);
	if (!GetUserName(res, &len)) {
		free(res);
		return NULL;
	}
	return res;
}

// clipboard support
static TCHAR *parseClipboard(ARGUMENTSINFO *ai) {

	TCHAR *res;

	if (ai->argc != 1)
		return NULL;

	ai->flags |= AIF_DONTPARSE;

	res = NULL;

	if (IsClipboardFormatAvailable(CF_TEXT)) {
		if (OpenClipboard(NULL)) {
			HANDLE hData = NULL;
			TCHAR* tszText = NULL; 
			int len = 0;
#ifdef _UNICODE
			hData = GetClipboardData(CF_UNICODETEXT);
#else 
			hData = GetClipboardData(CF_TEXT);
#endif
			if (hData != NULL) {
				tszText = (TCHAR*)GlobalLock(hData);
				len = _tcslen(tszText);
				res = (TCHAR*)malloc((len + 1) * sizeof(TCHAR));
				_tcscpy(res, tszText);
				res[len] = _T('\0');
				GlobalUnlock(hData);
			}
			CloseClipboard();
	}	}

	return res;
}

int registerSystemTokens() {

	registerIntToken(_T(COMPUTERNAME), parseComputerName, TRF_FIELD, "System Functions\tcomputer name");
#if _WIN32_WINNT>=0x0500
	registerIntToken(_T(CPULOAD), parseCpuLoad, TRF_FUNCTION, "System Functions\t(x)\tcpu load of process x (without extension) (x is optional)");
#endif
	registerIntToken(_T(CDATE), parseCurrentDate, TRF_FUNCTION, "System Functions\t(x)\tcurrent date in format y (y is optional)");
	registerIntToken(_T(CTIME), parseCurrentTime, TRF_FUNCTION, "System Functions\t(x)\tcurrent time in format y (y is optional)");
	registerIntToken(_T(DIRECTORY), parseDirectory, TRF_FUNCTION, "System Functions\t(x,y)\tthe directory y directories above x");
	registerIntToken(_T(DIRECTORY2), parseDirectory2, TRF_FUNCTION, "System Functions\t(x,y)\tstrips y directories from x");
	registerIntToken(_T(DIFFTIME), parseDiffTime, TRF_FUNCTION, "System Functions\t(x,y)\tnumber of seconds between date x and y (x and y in format: M/d/yy H:m:s)");
	registerIntToken(_T(DIREXISTS), parseDirExists, TRF_FUNCTION, "System Functions\t(x)\tTRUE if directory x exists");
	registerIntToken(_T(FILEEXISTS), parseFileExists, TRF_FUNCTION, "System Functions\t(x)\tTRUE if file x exists");
	registerIntToken(_T(FINDWINDOW), parseFindWindow, TRF_FUNCTION, "System Functions\t(x)\twindow title of first window of class x");
	registerIntToken(_T(LISTDIR), parseListDir, TRF_FUNCTION, "System Functions\t(x,y,z)\tshows files and directories of directory z, with filter y, separated by z (y and z optional)");
#ifndef WINE
	registerIntToken(_T(PROCESSRUNNING), parseProcessRunning, TRF_FUNCTION, "System Functions\t(x)\tTRUE if process x is running");
#endif
	registerIntToken(_T(REGISTRYVALUE), parseRegistryValue, TRF_FUNCTION, "System Functions\t(x,y)\tvalue y from registry key x (REG_SZ (string) values only)");
	registerIntToken(_T(TIMESTAMP2DATE), parseTimestamp2Date, TRF_FUNCTION, "System Functions\t(x,y)\tformats timestamp x (seconds since 1/1/1970) in date format y");
	registerIntToken(_T(TIMESTAMP2TIME), parseTimestamp2Time, TRF_FUNCTION, "System Functions\t(x,y)\tformats timestamp x (seconds since 1/1/1970) in time format y");
	registerIntToken(_T(TXTFILE), parseTextFile, TRF_FUNCTION, "System Functions\t(x,y)\ty > 0: line number y from file x, y = 0: the whole file, y < 0: line y counted from the end, y = r: random line");
#if _WIN32_WINNT>=0x0500
	registerIntToken(_T(UPTIME), parseUpTime, TRF_FIELD, "System Functions\tuptime in seconds");
#endif
	if ( !ServiceExists( MS_UTILS_REPLACEVARS ))
		registerIntToken(_T(ENVIRONMENTVARIABLE), parseEnvironmentVariable, TRF_FUNCTION	, "Miranda Core OS\t(%xxxxxxx%)\tany environment variable defined in current Windows session (like %systemroot%, %allusersprofile%, etc.)");
	else {
		registerIntToken(_T(ENVIRONMENTVARIABLE), parseEnvironmentVariable, TRF_FUNCTION	, "System Functions\t(x)\texpand environment variable x");
		registerIntToken(_T(USERNAME), parseUserName, TRF_FIELD								, "System Functions\tuser name");
	}

	srand((unsigned int)GetTickCount());

	registerIntToken(_T(CLIPBOARD), parseClipboard, TRF_FIELD, "System Functions\ttext from clipboard");

	return 0;
}
