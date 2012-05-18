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
#include "parse_external.h"
#include "ac/ac.h"

static int (WINAPI *acEval)(const char *, char *) = NULL;
static int (WINAPI *acFormat)(const char *, char *) = NULL;
static int (WINAPI *acInitClient)(const char *, int, int, int, int) = NULL;
static void (WINAPI *acUninit)() = NULL;

static unsigned int lastAMIPFailure = -1;

static TCHAR *getFullWinampTitleText() {

	TCHAR *szTitle, *szWinText;
	HWND hwndWinamp;

	hwndWinamp = FindWindow(_T("STUDIO"), NULL);
	if (hwndWinamp == NULL)
		hwndWinamp = FindWindow(_T("Winamp v1.x"),NULL);

	if (hwndWinamp == NULL)
		return NULL;

	szWinText = ( TCHAR* )malloc((GetWindowTextLength(hwndWinamp) + 1)*sizeof(TCHAR));
	if (szWinText == NULL)
		return NULL;

	if (GetWindowText(hwndWinamp, szWinText, GetWindowTextLength(hwndWinamp)+1) == 0) {
		free(szWinText);
		return NULL;
	}
	szTitle = ( TCHAR* )malloc((2*_tcslen(szWinText)+1)*sizeof(TCHAR));
	if (szTitle == NULL) {
		free(szWinText);
		return NULL;
	}
	_tcscpy(szTitle, szWinText);
	_tcscpy(szTitle+_tcslen(szTitle), szWinText);
	free(szWinText);

	return szTitle;
}

static TCHAR *parseWinampSong(ARGUMENTSINFO *ai) {

	TCHAR *szTitle, *scur, *cur, *res;

	if (ai->argc != 1) {
		return NULL;
	}
	res = NULL;
	szTitle = getFullWinampTitleText();
	if (szTitle == NULL) {
		return NULL;
	}
	scur = _tcschr(szTitle, _T('.'));
	cur = _tcsstr(scur, _T(" - Winamp"));
	if ( (scur == NULL) || (cur == NULL) || (scur >= cur) || (scur > (szTitle + _tcslen(szTitle) - 2)) || (cur > (szTitle + _tcslen(szTitle))) ) {
		free(szTitle);
		return NULL;
	}
	scur++;
	scur++;
	*cur = '\0';
	res = _tcsdup(scur);
	free(szTitle);
	ai->flags |= AIF_DONTPARSE;
	
	return res;
}

static TCHAR *parseWinampState(ARGUMENTSINFO *ai) {

	TCHAR *szTitle, *scur, *cur, *res;

	if (ai->argc != 1) {
		return NULL;
	}
	res = NULL;
	szTitle = getFullWinampTitleText();
	if (szTitle == NULL) {
		return NULL;
	}
	scur = _tcschr(szTitle, _T('.'));
	cur = _tcsstr(scur, _T(" - Winamp"));
	if ( (scur == NULL) || (cur == NULL) ) {
		free(szTitle);
		return _tcsdup(TranslateT("Stopped"));
	}
	if ( (!_tcsncmp(cur+10, _T("[Stopped]"), 9)) ) {
		free(szTitle);
		return _tcsdup(TranslateT("Stopped"));
	}
	if ( (!_tcsncmp(cur+10, _T("[Paused]"), 8)) ) {
		free(szTitle);
		return _tcsdup(TranslateT("Paused"));
	}
	free(szTitle);
	return _tcsdup(_T("Playing"));
}

static unsigned int checkAMIP() {

	if (lastAMIPFailure == 0) {
		log_debugA("AMIP initialized");
		return 0;
	}
	if (GetTickCount() - lastAMIPFailure < AMIP_TIMEOUT) {
		log_debugA("AMIP not initialized, not attempting");
		return -1;
	}
	if (acInitClient("127.0.0.1", 60333, 1000, 5, 1)) {
		lastAMIPFailure = 0;
		log_debugA("AMIP now initialized");
		
		return 0; // success
	}
	log_debugA("AMIP failed to initialized");
	if (lastAMIPFailure == 0) {
		/* if this is the first failure after a succesful init, call uninit for a cleanup (maybe it'll help for the next try ;) ) */
		acUninit();
	}
	lastAMIPFailure = GetTickCount();
	
	return -1;
}

static TCHAR *parseAMIPEval(ARGUMENTSINFO *ai) {

	TCHAR *tszRes;
	char *cmd;
	char szRes[AC_BUFFER_SIZE];

	tszRes = NULL;
	if (ai->argc != 2) {
		return NULL;
	}
#ifdef UNICODE
	cmd = u2a(ai->targv[1]);
#else
	cmd = _strdup(ai->targv[1]);
#endif
	if (checkAMIP() != 0) {
		log_debugA("checkAMIP failed");
	
		return NULL;
	}
	ZeroMemory(&szRes, sizeof(szRes));
	if (AC_ERR_NOERROR == acEval(cmd, szRes)) {
#ifdef UNICODE
		tszRes = a2u(szRes);
#else
		tszRes = _strdup(szRes);
#endif
	}
	else {
		lastAMIPFailure = GetTickCount();
	}
	free(cmd);

	return tszRes;
}

static TCHAR *parseAMIPFormat(ARGUMENTSINFO *ai) {

	TCHAR *tszRes;
	char *cmd;
	char szRes[AC_BUFFER_SIZE];

	tszRes = NULL;
	if (ai->argc != 2) {
		return NULL;
	}
#ifdef UNICODE
	cmd = u2a(ai->targv[1]);
#else
	cmd = _strdup(ai->targv[1]);
#endif
	if (checkAMIP() != 0) {
	
		return NULL;
	}
	if (AC_ERR_NOERROR == acFormat(cmd, szRes)) {
#ifdef UNICODE
		tszRes = a2u(szRes);
#else
		tszRes = _strdup(szRes);
#endif
	}
	else {
		lastAMIPFailure = GetTickCount();
	}
	free(cmd);

	return tszRes;
}

static int initAMIP() {

	HMODULE hModule;
	
	hModule = LoadLibraryA("ac.dll");
	if (hModule == NULL) {
		char path[MAX_PATH];
		char *cur;

		GetModuleFileNameA(NULL, path, sizeof(path));
		cur = strrchr(path, '\\');
		if (cur != NULL)
			strcpy(cur+1, "ac.dll");
		else
			strcpy(cur, "ac.dll");
		hModule = LoadLibraryA(path);
	}
	if (hModule == NULL) {
		return -1;
	}
	acInitClient = (int (__stdcall *)(const char *,int ,int ,int ,int ))GetProcAddress(hModule, "ac_init_client");
	acEval = (int (__stdcall *)(const char *,char *))GetProcAddress(hModule, "ac_eval");
	acFormat = (int (__stdcall *)(const char *,char *))GetProcAddress(hModule, "ac_format");
	acUninit = (void (__stdcall *)())GetProcAddress(hModule, "ac_uninit");
	
	return 0;
}

int registerExternalTokens() {

	registerIntToken(_T(WINAMPSONG), parseWinampSong, TRF_FIELD, "External Applications\tretrieves song name of the song currently playing in Winamp");
	registerIntToken(_T(WINAMPSTATE), parseWinampState, TRF_FIELD, "External Applications\tretrieves current Winamp state (Playing/Paused/Stopped)");
	if (!initAMIP()) {
		registerIntToken(_T(AMIPEVAL), parseAMIPEval, TRF_FUNCTION, "External Applications\t(x)\tretrieves info from AMIP (x is var_<variable> with any AMIP variable)");
		registerIntToken(_T(AMIPFORMAT), parseAMIPFormat, TRF_FUNCTION|TRF_UNPARSEDARGS, "External Applications\t(x)\tretrieves info from AMIP (x is AMIP format string)");
	}
	else {
		log_infoA("Variables: ac.dll for AMIP not found");
	}

	return 0;
}

int deInitExternal() {

	if (acUninit != NULL) {
		acUninit();
	}
	
	return 0;
}